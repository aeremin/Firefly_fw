#ifndef BLE_H
#define BLE_H

typedef void (*BleCallback)(bool);

void InitBle(BleCallback callback);

#endif // BLE_H
