#ifndef CC1101_H
#define CC1101_H

#include "nrf_drv_spi.h"

#include "cc1101defins.h"

struct RadioPacket {
  uint16_t From;
  uint16_t To;
  uint16_t TransmitterID;
  uint8_t Cmd;
  uint8_t PktID;
  int8_t RssiThr;
  uint8_t Damage;
  uint8_t Power;
} __attribute__((__packed__));

// Representation of CC1101 radio tranceiver chip.
// See datasheet here: http://www.ti.com/lit/ds/symlink/cc1101.pdf
class Cc1101 {
 public:
  // TODO(aeremin) Also pass pin numbers (sck, mosi, miso, ss and potentially gd0)
  Cc1101(nrf_drv_spi_t spi) : spi_(spi) {}

  // TODO(aeremin) Return bool (false if failed to init)
  void Init();

  // TODO(aeremin) Support setting an explicit timeout
  bool Receive(RadioPacket* result);

 private:
  // Sends a single-byte instruction to the CC1101.
  // See documentation of instructions in datasheet, p.32, 
  // 10.4 Command Strobes
  // If status is provided, status byte will be written into it.
  void WriteStrobe(uint8_t instruction, uint8_t* status = nullptr);

  uint8_t WriteRegister(uint8_t k, uint8_t v);
  uint8_t ReadRegister(uint8_t ARegAddr, uint8_t* status);
  uint8_t ReadOneFifo();
  bool ReadFifo(RadioPacket* result);
  void RfConfig();

  void Reset()       { WriteStrobe(CC_SRES); }
  void EnterTX()     { WriteStrobe(CC_STX);  }
  void EnterRX()     { WriteStrobe(CC_SRX);  }
  void FlushRxFIFO() { WriteStrobe(CC_SFRX); }
  void SetTxPower(uint8_t APwr)  { WriteRegister(CC_PATABLE, APwr); }
  void SetPktSize(uint8_t ASize) { WriteRegister(CC_PKTLEN, ASize); }
  void SetChannel(uint8_t AChannel) { WriteRegister(CC_CHANNR, AChannel); }

  const nrf_drv_spi_t spi_;
};

#endif // CC1101_H