#include <string.h>
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "boards.h"
#include "cc1101/cc1101.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "FreeRTOS.h"
#include "task.h"

#define SPI_INSTANCE 0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

static Cc1101 cc1101(spi);

struct RadiationRadioPacket {
    uint16_t From;  // 2
    uint16_t To;    // 2
    uint16_t TransmitterID; // 2
    uint8_t Cmd; // 1
    uint8_t PktID; // 1
    union {
        struct {
            uint16_t MaxLvlID;
            uint8_t Reply;
        } __attribute__ ((__packed__)) Pong; // 3

        struct {
            int8_t RssiThr;
            uint8_t Damage;
            uint8_t Power;
        } __attribute__ ((__packed__)) Beacon; // 3

        struct {
            uint8_t Power;
            int8_t RssiThr;
            uint8_t Damage;
        } __attribute__ ((__packed__)) LustraParams; // 3

        struct {
            uint8_t ParamID;
            uint16_t Value;
        } __attribute__ ((__packed__)) LocketParam; // 3

        struct {
            int8_t RssiThr;
        } __attribute__ ((__packed__)) Die; // 1
    } __attribute__ ((__packed__)); // union
} __attribute__ ((__packed__));

struct MagicPathRadioPacket {
    uint32_t ID;
} __attribute__ ((__packed__));

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

void SetupTimer() {
  APP_ERROR_CHECK(nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request(NULL);
}

static TaskHandle_t g_ouch_task_handle = 0;
static void OuchTask(void*) {
  while (true) {
    if (accumulator.ProcessAndCheck()) {
      NRF_LOG_INFO("OUCH!!!");
      bsp_board_led_invert(0);
    } else {
      NRF_LOG_INFO("You are fine!");
    }
    NRF_LOG_FLUSH();
    nrf_delay_ms(1000);
  }  
}

const bool g_transmit = true;
const bool g_receive = true;

static TaskHandle_t g_radio_task_handle = 0;
static void RadioTask(void*) {
  cc1101.Init();
  cc1101.SetChannel(1);
  MagicPathRadioPacket r;
  r.ID = 1;
  while (true) {
    if (g_transmit) {
      cc1101.Transmit(r);
      nrf_delay_ms(100);
    }
    if (g_receive) {
      RadiationRadioPacket r;
      if (cc1101.Receive(360, &r)) {
        accumulator.Cnt++;
        accumulator.Summ += 80 + 132;
        accumulator.RssiThr = r.Beacon.RssiThr;
        accumulator.Damage = r.Beacon.Damage + 1;
      }
    }
    NRF_LOG_FLUSH();
  } 
}

int main(void) {
  bsp_board_init(BSP_INIT_LEDS);
  APP_ERROR_CHECK(nrf_drv_gpiote_init());

  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Firefly started!");

  SetupTimer();

  xTaskCreate(RadioTask, "Radio", /*stack depth = */configMINIMAL_STACK_SIZE + 100,
              /*pvParameters=*/nullptr, /*priority = */3, &g_radio_task_handle);

  xTaskCreate(OuchTask, "Ouch", /*stack depth = */configMINIMAL_STACK_SIZE + 100,
              /*pvParameters=*/nullptr, /*priority = */1, &g_ouch_task_handle);

  vTaskStartScheduler();

  ASSERT(false);
}
