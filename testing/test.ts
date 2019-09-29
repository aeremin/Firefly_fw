import { BluetoothLowEnergy } from "./ble";

const LED_BUTTON_SERVICE_UUID = "000015231212efde1523785feabcd123";
const BUTTON_CHARACTERISTIC_UUID = "000015241212efde1523785feabcd123";
const LED_CHARACTERISTIC_UUID = "000015251212efde1523785feabcd123";

const ble = new BluetoothLowEnergy();

import { Firestore } from "@google-cloud/firestore";

process.env.GCLOUD_PROJECT = "larp-hardware";
const db = new Firestore();
const fireflyCollection = db.collection("firefly");
const unsubs: {[deviceAddress: string]: () => void} = {};

async function main(): Promise<void> {
  ble.addCharacteristicListener(LED_BUTTON_SERVICE_UUID, BUTTON_CHARACTERISTIC_UUID,
    async (deviceAddress, characteristic) => {
    console.log(`New characteristic value for ${deviceAddress} is ${characteristic.value}`);

    await fireflyCollection.doc(deviceAddress).set({
        buttonValue: characteristic.value[0],
      }, {merge: true});
  });

  ble.setConnectListener(async (deviceAddress) => {
    unsubs[deviceAddress] = fireflyCollection.doc(deviceAddress).onSnapshot((snapshot) => {
      ble.writeCharacteristic(deviceAddress, LED_BUTTON_SERVICE_UUID, LED_CHARACTERISTIC_UUID, [snapshot.data().color])
        .then(() => console.log("Write success!"))
        .catch((err) => console.log("Write fail! " + JSON.stringify(err)));
    });
    await fireflyCollection.doc(deviceAddress).set({connected: true}, {merge: true});
  });

  ble.setDisconnectListener(async (deviceAddress) => {
    unsubs[deviceAddress]();
    await fireflyCollection.doc(deviceAddress).set({connected: false}, {merge: true});
  });

  await ble.startScanningAndReporting();
}

main().then().catch((error) => {
  console.error(error);
  process.exit(-1);
});
