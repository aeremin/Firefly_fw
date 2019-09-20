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

struct MagicPathRadioPacket {
    uint8_t r, g, b;
} __attribute__ ((__packed__));

void SetupTimer() {
  APP_ERROR_CHECK(nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request(NULL);
}

MagicPathRadioPacket color = {/*r = */30, /*g = */0, /*b = */10};

static TaskHandle_t g_radio_task_handle = 0;
static void RadioTask(void*) {
  cc1101.Init();
  cc1101.SetChannel(1);
  
  while (true) {
    cc1101.Transmit(color);
    nrf_delay_ms(100);
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

  vTaskStartScheduler();

  ASSERT(false);
}
