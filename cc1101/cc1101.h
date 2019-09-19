#ifndef CC1101_H
#define CC1101_H

#include "nrf_drv_spi.h"
#include "FreeRTOS.h"

#include "cc1101defins.h"

struct RadioPacket {
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

// Representation of CC1101 radio tranceiver chip.
// See datasheet here: http://www.ti.com/lit/ds/symlink/cc1101.pdf
class Cc1101 {
 public:
  // TODO(aeremin) Also pass pin numbers (sck, mosi, miso, ss and potentially gd0)
  Cc1101(nrf_drv_spi_t spi) : spi_(spi) {}

  // TODO(aeremin) Return bool (false if failed to init)
  void Init();

  bool Receive(uint32_t timeout_ms, RadioPacket* result);

  void Transmit(const RadioPacket& packet);

  void SetChannel(uint8_t AChannel) { WriteConfigurationRegister(CC_CHANNR, AChannel); }

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
  
  bool ReadFifo(RadioPacket* result);
  void WriteTX(const RadioPacket& packet);

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