#include <string.h>

#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "boards.h"
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
#include "cc1101/cc1101.h"
#include "rgb_led.h"
#include "ble/bluetooth.h"

const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE_0;
Cc1101 cc1101(spi);
RgbLed led;

struct MagicPathRadioPacket {
    uint8_t r, g, b;
} __attribute__ ((__packed__));

void SetupTimer() {
  APP_ERROR_CHECK(nrf_drv_clock_init());
  nrf_drv_clock_lfclk_request(NULL);
}

MagicPathRadioPacket gColor = {/*r = */30, /*g = */0, /*b = */30};

static TaskHandle_t g_radio_task_handle = 0;
static void RadioTask(void*) {
  cc1101.Init();
  cc1101.SetChannel(1);
  
  while (true) {
    cc1101.Transmit(gColor);
    nrf_delay_ms(100);
    NRF_LOG_FLUSH();
  } 
}

void ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
  // We are only interested in button presses
  if (!button_action) return;

  // TODO: For whatever reason, adding logging here leads to hard fault. So no logging :(
  switch (pin_no)
  {
    case BUTTON_1:
      BluetoothLowEnergy::Instance().SetButtonState(1);
      break;

    case BUTTON_2:
      BluetoothLowEnergy::Instance().SetButtonState(0);
      break;

    default:
      APP_ERROR_HANDLER(pin_no);
      break;
  }
}


void ButtonsInit() {
  static app_button_cfg_t buttons[] =
  {
      {BUTTON_1, false, BUTTON_PULL, ButtonEventHandler},
      {BUTTON_2, false, BUTTON_PULL, ButtonEventHandler}
  };
  APP_ERROR_CHECK(app_button_init(buttons, ARRAY_SIZE(buttons), APP_TIMER_TICKS(50)));
  APP_ERROR_CHECK(app_button_enable());
}


void OnBle(bool on) {
  // FIXME: gColor should probably be mutex-protected.
  gColor = on ? MagicPathRadioPacket{/*r = */30, /*g = */0, /*b = */30} : MagicPathRadioPacket{/*r = */0, /*g = */30, /*b = */0};
  led.SetColor(2 * gColor.r, 2 * gColor.g, 2 * gColor.b);
}

int main(void) {
  bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
  APP_ERROR_CHECK(nrf_drv_gpiote_init());

  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Firefly started!");

  SetupTimer();
  led.Init();
  BluetoothLowEnergy::Instance().Init(OnBle);
  OnBle(false);
  ButtonsInit();

  xTaskCreate(RadioTask, "Radio", /*stack depth = */configMINIMAL_STACK_SIZE + 100,
              /*pvParameters=*/nullptr, /*priority = */3, &g_radio_task_handle);

  vTaskStartScheduler();

  ASSERT(false);
}
