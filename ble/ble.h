#ifndef BLE_H
#define BLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*BleCallback)(bool);

void InitBle(BleCallback callback);

#ifdef __cplusplus
}
#endif

#endif // BLE_H
