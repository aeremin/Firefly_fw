import { BluetoothLowEnergy } from "./ble";

const LED_BUTTON_SERVICE_UUID = "000015231212efde1523785feabcd123";
const BUTTON_CHARACTERISTIC_UUID = "000015241212efde1523785feabcd123";
const LED_CHARACTERISTIC_UUID = "000015251212efde1523785feabcd123";

const ble = new BluetoothLowEnergy();

import { Firestore } from "@google-cloud/firestore";

process.env.GCLOUD_PROJECT = "larp-hardware";
const db = new Firestore();

async function main(): Promise<void> {
  ble.addCharacteristicListener(LED_BUTTON_SERVICE_UUID, BUTTON_CHARACTERISTIC_UUID,
    async (deviceAddress, characteristic) => {
    console.log(`New characteristic value for ${deviceAddress} is ${characteristic.value}`);

    const docRef = db.collection("firefly").doc(deviceAddress);
    await docRef.set({
        buttonValue: characteristic.value[0],
      }, {merge: true});
  });
  await ble.startScanningAndReporting();
  setInterval(() => {
    v = 1 - v;
    ble.writeCharacteristic("F0:78:E3:7B:D4:FD", LED_BUTTON_SERVICE_UUID, LED_CHARACTERISTIC_UUID, [v])
      .then(() => console.log("Write success!"))
      .catch((err) => console.log("Write fail! " + JSON.stringify(err)));
  }, 3000);
}

let v = 0;

main().then().catch((error) => {
  console.error(error);
  process.exit(-1);
});
