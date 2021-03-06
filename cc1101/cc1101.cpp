#include "cc1101.h"

#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "cc1101_rf_settings.h"

namespace {

// See https://www.freertos.org/RTOS_Task_Notification_As_Binary_Semaphore.html
// for an explanation of approach we use here.
TaskHandle_t g_task_to_nofify = 0;

void Gd0Handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  BaseType_t higher_priority_task_woken = pdFALSE;
  if (g_task_to_nofify) {
    vTaskNotifyGiveFromISR(g_task_to_nofify, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
  }
}

}


void Cc1101::Init() {
  nrf_drv_spi_config_t spi_config = {
      .sck_pin = 6,
      .mosi_pin = 8,
      .miso_pin = 7,
      .ss_pin = 5,
      .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      .orc = 0x00,
      .frequency = NRF_DRV_SPI_FREQ_4M,
      .mode = NRF_DRV_SPI_MODE_0,
      .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
  };
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_, &spi_config, NULL, NULL));

  Reset();
  
  // TODO(aeremin) This is a hack. Instead we should wait for 
  // CC1101 to became ready by listening for MISO to become low.
  // This actually should be done every time we initiate a transfer (i.e. pull CS low),
  // but there is no direct support for that in nRF SPI library. From the datasheet it
  // seems that it's actually only required when leaving SLEEP or XOFF states
  // (see p. 29, "4-wire Serial Configuration and Data Interface").
  nrf_delay_ms(20);

  WriteConfigurationRegister(CC_PKTLEN, 40);
  uint8_t b = ReadRegister(CC_PKTLEN, NULL);
  NRF_LOG_INFO("Read b = %d", b);

  RfConfig();
  FlushRxFIFO();

  SetTxPower(CC_PwrMinus30dBm);
  SetChannel(0);

  g_task_to_nofify = xTaskGetCurrentTaskHandle();
  nrf_drv_gpiote_in_config_t in_config;
  in_config.is_watcher = false;
  in_config.hi_accuracy = true;
  in_config.pull = NRF_GPIO_PIN_PULLDOWN;
  in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;
  APP_ERROR_CHECK(nrf_drv_gpiote_in_init(4, &in_config, Gd0Handler));
  nrf_drv_gpiote_in_event_enable(4, true);
}

void Cc1101::WriteStrobe(uint8_t instruction, uint8_t* status) {
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, &instruction, 1, status, (status ? 1 : 0)));
}

void Cc1101::WriteConfigurationRegister(uint8_t reg, uint8_t value, uint8_t* statuses) {
  uint8_t tx[] = {reg, value};
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, tx, 2, statuses, (statuses ? 2 : 0)));
}

uint8_t Cc1101::ReadRegister(uint8_t reg, uint8_t* status) {
  uint8_t tx = reg | CC_READ_FLAG;
  uint8_t rx[] = {0, 0};
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, &tx, 1, rx, 2));
  if (status) *status = rx[0];
  return rx[1];
}

void Cc1101::Recalibrate() {
  EnterIdle();
  WriteStrobe(CC_SCAL);
}

void Cc1101::RfConfig() {
  WriteConfigurationRegister(CC_FSCTRL1,  CC_FSCTRL1_VALUE);    // Frequency synthesizer control.
  WriteConfigurationRegister(CC_FSCTRL0,  CC_FSCTRL0_VALUE);    // Frequency synthesizer control.
  WriteConfigurationRegister(CC_FREQ2,    CC_FREQ2_VALUE);      // Frequency control word, high byte.
  WriteConfigurationRegister(CC_FREQ1,    CC_FREQ1_VALUE);      // Frequency control word, middle byte.
  WriteConfigurationRegister(CC_FREQ0,    CC_FREQ0_VALUE);      // Frequency control word, low byte.
  WriteConfigurationRegister(CC_MDMCFG4,  CC_MDMCFG4_VALUE);    // Modem configuration.
  WriteConfigurationRegister(CC_MDMCFG3,  CC_MDMCFG3_VALUE);    // Modem configuration.
  WriteConfigurationRegister(CC_MDMCFG2,  CC_MDMCFG2_VALUE);    // Modem configuration.
  WriteConfigurationRegister(CC_MDMCFG1,  CC_MDMCFG1_VALUE);    // Modem configuration.
  WriteConfigurationRegister(CC_MDMCFG0,  CC_MDMCFG0_VALUE);    // Modem configuration.
  WriteConfigurationRegister(CC_CHANNR,   CC_CHANNR_VALUE);     // Channel number.
  WriteConfigurationRegister(CC_DEVIATN,  CC_DEVIATN_VALUE);    // Modem deviation setting (when FSK modulation is enabled).
  WriteConfigurationRegister(CC_FREND1,   CC_FREND1_VALUE);     // Front end RX configuration.
  WriteConfigurationRegister(CC_FREND0,   CC_FREND0_VALUE);     // Front end RX configuration.
  WriteConfigurationRegister(CC_MCSM0,    CC_MCSM0_VALUE);      // Main Radio Control State Machine configuration.
  WriteConfigurationRegister(CC_FOCCFG,   CC_FOCCFG_VALUE);     // Frequency Offset Compensation Configuration.
  WriteConfigurationRegister(CC_BSCFG,    CC_BSCFG_VALUE);      // Bit synchronization Configuration.
  WriteConfigurationRegister(CC_AGCCTRL2, CC_AGCCTRL2_VALUE);   // AGC control.
  WriteConfigurationRegister(CC_AGCCTRL1, CC_AGCCTRL1_VALUE);   // AGC control.
  WriteConfigurationRegister(CC_AGCCTRL0, CC_AGCCTRL0_VALUE);   // AGC control.
  WriteConfigurationRegister(CC_FSCAL3,   CC_FSCAL3_VALUE);     // Frequency synthesizer calibration.
  WriteConfigurationRegister(CC_FSCAL2,   CC_FSCAL2_VALUE);     // Frequency synthesizer calibration.
  WriteConfigurationRegister(CC_FSCAL1,   CC_FSCAL1_VALUE);     // Frequency synthesizer calibration.
  WriteConfigurationRegister(CC_FSCAL0,   CC_FSCAL0_VALUE);     // Frequency synthesizer calibration.
  WriteConfigurationRegister(CC_TEST2,    CC_TEST2_VALUE);      // Various test settings.
  WriteConfigurationRegister(CC_TEST1,    CC_TEST1_VALUE);      // Various test settings.
  WriteConfigurationRegister(CC_TEST0,    CC_TEST0_VALUE);      // Various test settings.
  WriteConfigurationRegister(CC_FIFOTHR,  CC_FIFOTHR_VALUE);    // fifo threshold
  WriteConfigurationRegister(CC_IOCFG2,   CC_IOCFG2_VALUE);     // GDO2 output pin configuration.
  WriteConfigurationRegister(CC_IOCFG0,   CC_IOCFG0_VALUE);     // GDO0 output pin configuration.
  WriteConfigurationRegister(CC_PKTCTRL1, CC_PKTCTRL1_VALUE);   // Packet automation control.
  WriteConfigurationRegister(CC_PKTCTRL0, CC_PKTCTRL0_VALUE);   // Packet automation control.

  WriteConfigurationRegister(CC_PATABLE, CC_Pwr0dBm);

  WriteConfigurationRegister(CC_MCSM2, CC_MCSM2_VALUE);
  WriteConfigurationRegister(CC_MCSM1, CC_MCSM1_VALUE);
}