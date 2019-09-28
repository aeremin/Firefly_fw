import { Adapter, AdapterFactory, Characteristic, ConnectionOptions,
  Descriptor, Device, ScanParameters, Service } from "pc-ble-driver-js";
import { BluetoothLowEnergy } from "./ble";

const gAdapter = AdapterFactory.getInstance().createAdapter("v3", "COM18", "");

const LED_BUTTON_SERVICE_UUID = "000015231212efde1523785feabcd123";
const BUTTON_CHARACTERISTIC_UUID = "000015241212efde1523785feabcd123";

const ble = new BluetoothLowEnergy();

async function main(): Promise<void> {
  ble.addCharacteristicListener(LED_BUTTON_SERVICE_UUID, BUTTON_CHARACTERISTIC_UUID, (characteristic) => {
    console.log("AAAAnd value is " + characteristic.value);
  });
  await ble.startScanningAndReporting();
}

main().then().catch((error) => {
  console.error(error);
  process.exit(-1);
});
