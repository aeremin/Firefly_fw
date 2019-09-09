#include <string.h>
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "boards.h"
#include "cc1101/cc1101.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define SPI_INSTANCE 0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

static Cc1101 cc1101(spi);

class RxData_t {
 public:
  int32_t Cnt;
  int32_t Summ;
  int8_t RssiThr;
  uint8_t Damage;
  bool ProcessAndCheck() {
    bool Rslt = false;
    if (Cnt >= 3L) {
      Summ /= Cnt;
      if (Summ >= RssiThr) Rslt = true;
    }
    Cnt = 0;
    Summ = 0;
    return Rslt;
  }
};

RxData_t accumulator;

APP_TIMER_DEF(m_ouch_timer_id);

static void timer_callback(void* p_context) {
  UNUSED_PARAMETER(p_context);
  if (accumulator.ProcessAndCheck()) {
    NRF_LOG_INFO("OUCH!!!");
    bsp_board_led_invert(0);
  } else {
    NRF_LOG_INFO("You are fine!");
  }
  NRF_LOG_FLUSH();
}

void SetupTimer() {
  APP_ERROR_CHECK(nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request(NULL);

  APP_ERROR_CHECK(app_timer_init());
  APP_ERROR_CHECK(app_timer_create(&m_ouch_timer_id,
                                   APP_TIMER_MODE_REPEATED,
                                   timer_callback));
  APP_ERROR_CHECK(app_timer_start(m_ouch_timer_id, APP_TIMER_TICKS(1000), NULL));
}

int main(void) {
  bsp_board_init(BSP_INIT_LEDS);

  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Firefly started!");

  cc1101.Init();

  SetupTimer();

  // nrf_gpio_cfg_input(4, NRF_GPIO_PIN_NOPULL);

  while (true) {
    RadioPacket r;
    if (cc1101.Receive(&r)) {
      accumulator.Cnt++;
      accumulator.Summ += 80 + 132;
      accumulator.RssiThr = r.RssiThr;
      accumulator.Damage = r.Damage + 1;
    }
    NRF_LOG_FLUSH();
    nrf_delay_ms(100);
  }
}
