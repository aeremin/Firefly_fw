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
  static BleCallback ble_callback;
private:
  void InitBleStack();
  void InitGapParams();
  void InitGatt();
  void InitServices();
  void InitAdvertising();
  void InitConnectionParams();
  void StartAdvertising();

  // Handle of the current connection (or BLE_CONN_HANDLE_INVALID if disconnected)
  uint16_t conn_handle_;
  // Advertising handle used to identify an advertising set.
  uint8_t adv_handle_;
  // Buffer for storing an encoded advertising set.
  uint8_t enc_advdata_[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
  // Buffer for storing an encoded scan data.
  uint8_t enc_scan_response_data_[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
  // Struct that contains pointers to the encoded advertising data.
  ble_gap_adv_data_t adv_data_;

  // A tag identifying the SoftDevice BLE configuration.
  // We aren't going to change configurations after first initialization.
  constexpr static uint8_t connection_configuraion_tag_ = 1;
};

#endif // BLUETOOTH_H
