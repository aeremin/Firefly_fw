#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <cstdint>

#include "ble_advdata.h"

class BluetoothLowEnergy {
public:
  BluetoothLowEnergy();
  typedef void (*BleCallback)(bool);
  void Init(BleCallback callback);

  // Those are only exposed as public, as C-style callbacks need access to them.
  // Don't use them directly.
  void BleEventHandler(const ble_evt_t* event);
  static BleCallback gBleCallback;
private:
  void InitBleStack();
  void InitGapParams();
  void InitGatt();
  void InitServices();
  void InitAdvertising();
  void InitConnectionParams();
  void StartAdvertising();

  // Handle of the current connection (or BLE_CONN_HANDLE_INVALID if disconnected)
  uint16_t m_conn_handle;
  // Advertising handle used to identify an advertising set.
  uint8_t m_adv_handle ;
  // Buffer for storing an encoded advertising set.
  uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
  // Buffer for storing an encoded scan data.
  uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
  // Struct that contains pointers to the encoded advertising data.
  ble_gap_adv_data_t m_adv_data;
};

#endif // BLUETOOTH_H
