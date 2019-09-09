#include "cc1101.h"

#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "cc1101_rf_settings.h"

void Cc1101::Init() {
  nrf_drv_spi_config_t spi_config = {
      .sck_pin = 6,
      .mosi_pin = 8,
      .miso_pin = 7,
      .ss_pin = 5,
      .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      .orc = 0x00,
      .frequency = NRF_DRV_SPI_FREQ_1M,
      .mode = NRF_DRV_SPI_MODE_0,
      .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
  };
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_, &spi_config, NULL, NULL));

  Reset();
  nrf_delay_ms(20);

  WriteRegister(CC_PKTLEN, 40);
  uint8_t b = ReadRegister(CC_PKTLEN, NULL);
  NRF_LOG_INFO("Read b = %d", b);

  RfConfig();
  FlushRxFIFO();

  SetTxPower(CC_PwrMinus20dBm);
  SetPktSize(sizeof(RadioPacket));
  SetChannel(0);
}

void Cc1101::WriteStrobe(uint8_t v) {
  uint8_t status = 0;
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, &v, 1, &status, 1));
  //NRF_LOG_INFO("WriteStrobe: status = %d", status);
}

uint8_t Cc1101::WriteRegister(uint8_t k, uint8_t v) {
  uint8_t tx[] = {k, v};
  uint8_t status[] = {137, 137};
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, tx, 2, status, 2));
  NRF_LOG_INFO("WriteRegister: status = %d %d", status[0], status[1]);
  return 0;
}

uint8_t Cc1101::ReadRegister(uint8_t ARegAddr, uint8_t* status) {
  ARegAddr = ARegAddr | CC_READ_FLAG;
  uint8_t tx[] = {ARegAddr, 0x00};
  uint8_t rx[] = {137, 137};
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, tx, 2, rx, 2));
  if (status) *status = rx[0];
  //NRF_LOG_INFO("ReadRegister: status = %d", rx[0]);
  return rx[1];
}

uint8_t Cc1101::ReadOneFifo() {
  uint8_t tx[] = {CC_FIFO | CC_READ_FLAG, 0x00};
  uint8_t rx[] = {137, 137};
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, tx, 2, rx, 2));
  //NRF_LOG_INFO("ReadRegister: status = %d", rx[0]);
  return rx[1];
}

bool Cc1101::ReadFifo(RadioPacket* result) {
  for (int i = 0; i < 100; ++i) {
    NRF_LOG_FLUSH();
    uint8_t status;
    uint8_t b = ReadRegister(CC_PKTSTATUS, &status);
    //NRF_LOG_ERROR("PKTSTATUS = %d", b);
    if (b & 0x80) {
      NRF_LOG_ERROR("PKTSTATUS = %u, status byte = %u", b, status);
      break;
    }
    if (i == 99) {
      //NRF_LOG_ERROR("CRC doesn't match!");
      return false;
    }
  }
  nrf_delay_ms(10);
  uint8_t tx_single = CC_FIFO | CC_READ_FLAG | CC_BURST_FLAG;
  uint8_t tx[14];
  for (int i = 0; i < 14; ++i) {
    tx[i] = CC_FIFO | CC_READ_FLAG; // | CC_BURST_FLAG;
  }
  //tx[0] = CC_FIFO | CC_READ_FLAG | CC_BURST_FLAG;

  uint8_t rx[14];
  for (int i = 0; i < 13; ++i) {
    //APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, tx + i, 1, rx + i, 1));
    //tx[i] = 0; //CC_FIFO | CC_READ_FLAG | CC_BURST_FLAG;
    rx[i] = ReadOneFifo();
  }
  //APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &tx_single, 1, rx, 14));
  RadioPacket PktRx;
  memcpy(&PktRx, rx + 1, sizeof(RadioPacket));
  for (int i = 0; i < 14; ++i) {
    //NRF_LOG_INFO("rx[%d] = %u", i, rx[i]);
  }
  NRF_LOG_INFO("--> : From %u; To: %u; TrrID: %u; PktID: %u; Cmd: %u", PktRx.From, PktRx.To, PktRx.TransmitterID, PktRx.PktID, PktRx.Cmd);
  //NRF_LOG_INFO("sizeof = %u", sizeof(struct rPkt_t));

  return true;
}

bool Cc1101::Receive(RadioPacket* result) {
  WriteStrobe(CC_SCAL);
  FlushRxFIFO();
  EnterRX();
  // while (nrf_gpio_pin_read(4)) {}
  return ReadFifo(result);
}

void Cc1101::RfConfig() {
  WriteRegister(CC_FSCTRL1,  CC_FSCTRL1_VALUE);    // Frequency synthesizer control.
  WriteRegister(CC_FSCTRL0,  CC_FSCTRL0_VALUE);    // Frequency synthesizer control.
  WriteRegister(CC_FREQ2,    CC_FREQ2_VALUE);      // Frequency control word, high byte.
  WriteRegister(CC_FREQ1,    CC_FREQ1_VALUE);      // Frequency control word, middle byte.
  WriteRegister(CC_FREQ0,    CC_FREQ0_VALUE);      // Frequency control word, low byte.
  WriteRegister(CC_MDMCFG4,  CC_MDMCFG4_VALUE);    // Modem configuration.
  WriteRegister(CC_MDMCFG3,  CC_MDMCFG3_VALUE);    // Modem configuration.
  WriteRegister(CC_MDMCFG2,  CC_MDMCFG2_VALUE);    // Modem configuration.
  WriteRegister(CC_MDMCFG1,  CC_MDMCFG1_VALUE);    // Modem configuration.
  WriteRegister(CC_MDMCFG0,  CC_MDMCFG0_VALUE);    // Modem configuration.
  WriteRegister(CC_CHANNR,   CC_CHANNR_VALUE);     // Channel number.
  WriteRegister(CC_DEVIATN,  CC_DEVIATN_VALUE);    // Modem deviation setting (when FSK modulation is enabled).
  WriteRegister(CC_FREND1,   CC_FREND1_VALUE);     // Front end RX configuration.
  WriteRegister(CC_FREND0,   CC_FREND0_VALUE);     // Front end RX configuration.
  WriteRegister(CC_MCSM0,    CC_MCSM0_VALUE);      // Main Radio Control State Machine configuration.
  WriteRegister(CC_FOCCFG,   CC_FOCCFG_VALUE);     // Frequency Offset Compensation Configuration.
  WriteRegister(CC_BSCFG,    CC_BSCFG_VALUE);      // Bit synchronization Configuration.
  WriteRegister(CC_AGCCTRL2, CC_AGCCTRL2_VALUE);   // AGC control.
  WriteRegister(CC_AGCCTRL1, CC_AGCCTRL1_VALUE);   // AGC control.
  WriteRegister(CC_AGCCTRL0, CC_AGCCTRL0_VALUE);   // AGC control.
  WriteRegister(CC_FSCAL3,   CC_FSCAL3_VALUE);     // Frequency synthesizer calibration.
  WriteRegister(CC_FSCAL2,   CC_FSCAL2_VALUE);     // Frequency synthesizer calibration.
  WriteRegister(CC_FSCAL1,   CC_FSCAL1_VALUE);     // Frequency synthesizer calibration.
  WriteRegister(CC_FSCAL0,   CC_FSCAL0_VALUE);     // Frequency synthesizer calibration.
  WriteRegister(CC_TEST2,    CC_TEST2_VALUE);      // Various test settings.
  WriteRegister(CC_TEST1,    CC_TEST1_VALUE);      // Various test settings.
  WriteRegister(CC_TEST0,    CC_TEST0_VALUE);      // Various test settings.
  WriteRegister(CC_FIFOTHR,  CC_FIFOTHR_VALUE);    // fifo threshold
  WriteRegister(CC_IOCFG2,   CC_IOCFG2_VALUE);     // GDO2 output pin configuration.
  WriteRegister(CC_IOCFG0,   CC_IOCFG0_VALUE);     // GDO0 output pin configuration.
  WriteRegister(CC_PKTCTRL1, CC_PKTCTRL1_VALUE);   // Packet automation control.
  WriteRegister(CC_PKTCTRL0, CC_PKTCTRL0_VALUE);   // Packet automation control.

  WriteRegister(CC_PATABLE, CC_Pwr0dBm);

  WriteRegister(CC_MCSM2, CC_MCSM2_VALUE);
  WriteRegister(CC_MCSM1, CC_MCSM1_VALUE);
}