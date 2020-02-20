#ifndef CC1101_H
#define CC1101_H

#include "nrf_drv_spi.h"
#include "FreeRTOS.h"
#include "task.h"

#include "cc1101defins.h"

// Representation of CC1101 radio tranceiver chip.
// See datasheet here: http://www.ti.com/lit/ds/symlink/cc1101.pdf
class Cc1101 {
 public:
  // TODO(aeremin) Also pass pin numbers (sck, mosi, miso, ss and potentially gd0)
  Cc1101(nrf_drv_spi_t spi) : spi_(spi) {}

  // TODO(aeremin) Return bool (false if failed to init)
  void Init();

  void SetChannel(uint8_t AChannel) { WriteConfigurationRegister(CC_CHANNR, AChannel); }

  template<typename RadioPacketT>
  bool Receive(uint32_t timeout_ms, RadioPacketT* result) {
    SetPktSize(sizeof(RadioPacketT));
    Recalibrate();
    FlushRxFIFO();
    EnterRX();

    if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms))) {
      return ReadFifo(result);
    } else {
      EnterIdle();
      return false;
    }
  }

  template<typename RadioPacketT>
  void Transmit(const RadioPacketT& packet) {
    SetPktSize(sizeof(RadioPacketT));
    Recalibrate();
    EnterTX();
    WriteTX(packet);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }

 private:
  // Sends a single-byte instruction to the CC1101.
  // See documentation of instructions in datasheet, p.32,
  // 10.4 Command Strobes
  // If status is provided, status byte will be written into it.
  void WriteStrobe(uint8_t instruction, uint8_t* status = nullptr);

  // Sets a configuration register to a provided value.
  // See detailed description of available registers in datasheet, p.66
  // 29 Configuration Registers and Table 45: SPI Address Space.
  // If statuses is provided (must be a 2-byte array), status bytes will be written into it.
  void WriteConfigurationRegister(uint8_t reg, uint8_t value, uint8_t* statuses = nullptr);

  // Reads a configuration or status register.
  // If status is provided, status byte will be written into it.
  uint8_t ReadRegister(uint8_t reg, uint8_t* status = nullptr);

  template<typename RadioPacketT>
  bool ReadFifo(RadioPacketT* result) {
    uint8_t status = 0;
    uint8_t b = ReadRegister(CC_PKTSTATUS, &status);
    if (!(b & 0x80)) {
      return false;
    }

    uint8_t tx = CC_FIFO | CC_READ_FLAG | CC_BURST_FLAG;
    uint8_t rx[sizeof(RadioPacketT) + 3];
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, &tx, 1, rx, sizeof(RadioPacketT) + 3));

    memcpy(result, rx + 1, sizeof(RadioPacketT));
    return true;
  }

  template<typename RadioPacketT>
  void WriteTX(const RadioPacketT& packet) {
    uint8_t tx[sizeof(RadioPacketT) + 1] = { CC_FIFO | CC_WRITE_FLAG | CC_BURST_FLAG };
    memcpy(tx + 1, &packet, sizeof(RadioPacketT));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_, tx, sizeof(RadioPacketT) + 1, nullptr, 0));
  }

  void RfConfig();

  void Reset()       { WriteStrobe(CC_SRES); }
  void EnterTX()     { WriteStrobe(CC_STX);  }
  void EnterRX()     { WriteStrobe(CC_SRX);  }
  void EnterIdle()   { WriteStrobe(CC_SIDLE); }
  void FlushRxFIFO() { WriteStrobe(CC_SFRX); }
  void SetTxPower(uint8_t APwr)  { WriteConfigurationRegister(CC_PATABLE, APwr); }
  void SetPktSize(uint8_t ASize) { WriteConfigurationRegister(CC_PKTLEN, ASize); }

  void Recalibrate();

  const nrf_drv_spi_t spi_;
};

#endif // CC1101_H