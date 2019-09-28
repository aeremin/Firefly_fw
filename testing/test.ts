import { AdapterFactory, Adapter, Device, Service, Characteristic, Descriptor, ConnectionOptions, ScanParameters } from "pc-ble-driver-js";

const gAdapter = AdapterFactory.getInstance().createAdapter("v3", "COM18", "");

const LED_BUTTON_SERVICE_UUID = "000015231212efde1523785feabcd123";
const BUTTON_CHARACTERISTIC_UUID = "000015241212efde1523785feabcd123";
const BLE_UUID_CCCD = "2902";

function discoverService(adapter: Adapter, device: Device, serviceUuid: string): Promise<Service> {
  return new Promise((resolve, reject) => {
    adapter.getServices(device.instanceId, (err, services) => {
      if (err) {
        reject(Error(`Error discovering the service: ${err}.`));
        return;
      }

      for (const service of services) {
        if (service.uuid.toLowerCase() == serviceUuid.toLowerCase()) {
          resolve(service);
          return;
        }
      }

      reject(Error("Did not discover the service in peripheral\'s GATT attribute table."));
    });
  });
}

function discoverharacteristic(adapter: Adapter, service: Service, characteristicUuid: string): Promise<Characteristic> {
  return new Promise((resolve, reject) => {
    adapter.getCharacteristics(service.instanceId, (err, characteristics) => {
      if (err) {
        reject(Error(`Error discovering characteristics: ${err}.`));
        return;
      }

      for (const characteristic of characteristics) {
        if (characteristic.uuid.toLowerCase() == characteristicUuid) {
          resolve(characteristic);
          return;
        }
      }

      reject(Error("Did not discover required characteristic in peripheral\'s GATT attribute table."));
    });
  });
}

function discoverCharacteristicDescriptor(adapter: Adapter, characteristic: Characteristic): Promise<Descriptor> {
  return new Promise((resolve, reject) => {
    adapter.getDescriptors(characteristic.instanceId, (err, descriptors) => {
      if (err) {
        reject(Error(`Error discovering the characteristic's descriptor: ${err}.`));
        return;
      }

      for (const descriptor of descriptors) {
        if (descriptor.uuid == BLE_UUID_CCCD) {
          resolve(descriptor);
          return;
        }
      }

      reject(Error("Did not discover the hrm chars CCCD in peripheral\'s GATT attribute table."));
    });
  });
}

function subscribeToCharacteristicNotifications(adapter: Adapter, descriptor: Descriptor): Promise<void> {
  return new Promise((resolve, reject) => {
    adapter.writeDescriptorValue(descriptor.instanceId, [1, 0], false, err => {
      if (err) {
        reject(`Error enabling notifications on the characteristic: ${err}.`);
      } else {
        console.log("Notifications turned on for characteristic.");
        resolve();
      }
    });
  });
}

function connectToPeripheral(adapter: Adapter, connectToAddress: string): Promise<void> {
  return new Promise((resolve, reject) => {
    console.log(`Connecting to device ${connectToAddress}...`);

    const options: ConnectionOptions = {
      scanParams: {
        active: false,
        interval: 100,
        window: 50,
        timeout: 0,
      },
      connParams: {
        min_conn_interval: 7.5,
        max_conn_interval: 7.5,
        slave_latency: 0,
        conn_sup_timeout: 4000,
      },
    };

    adapter.connect(connectToAddress, options, err => {
      if (err) {
        reject(Error(`Error connecting to target device: ${err}.`));
      } else {
        resolve();
      }
    });
  });
}

function startScan(adapter: Adapter): Promise<void> {
  return new Promise((resolve, reject) => {
    console.log("Starting scanning...");

    const scanParameters: ScanParameters = {
      active: true,
      interval: 100,
      window: 50,
      timeout: 0,
    };

    adapter.startScan(scanParameters, err => {
      if (err) {
        reject(new Error(`Error starting scanning: ${err}.`));
      } else {
        resolve();
      }
    });
  });
}

function addAdapterListeners(adapter: Adapter): void {
  adapter.on("logMessage", (severity, message) => { if (Number(severity) > 3) console.log(`${message}.`); });
  adapter.on("error", error => { console.error(`error: ${JSON.stringify(error, null, 1)}.`); });
  adapter.on("deviceConnected", async device => {
    try {console.log(`Device ${device.address}/${device.addressType} connected.`);
      const service = await discoverService(adapter, device, LED_BUTTON_SERVICE_UUID);
      console.log("Discovered service.");
      const characteristic = await discoverharacteristic(adapter, service, BUTTON_CHARACTERISTIC_UUID);
      console.log("Discovered characteristic.");
      const descriptor = await discoverCharacteristicDescriptor(adapter, characteristic);
      console.log("Discovered the descriptor");
      await subscribeToCharacteristicNotifications(adapter, descriptor);
    } catch(error) {
      console.error(error);
      process.exit(1);
    }
  });

  adapter.on("deviceDisconnected", async device => {
    console.log(`Device ${device.address} disconnected.`);
    try {
      await startScan(adapter);
      console.log("Successfully initiated the scanning procedure.");
    } catch (error) {
      console.error(error);
    }
  });

  adapter.on("deviceDiscovered", async device => {
    if (device.name == "Nordic_Blinky") {
      try {
        await connectToPeripheral(adapter, device.address);
      } catch (error) {
        console.error(error);
        process.exit(1);
      }
    }
  });

  adapter.on("scanTimedOut", () => {
    console.error("scanTimedOut: Scanning timed-out. Exiting.");
    process.exit(1);
  });

  adapter.on("characteristicValueChanged", characteristic => {
    console.log(`Value of characteristic with UUID ${characteristic.uuid} is ${characteristic.value}.`);
  });
}

function openAdapter(adapter: Adapter): Promise<void> {
  return new Promise((resolve, reject) => {
    const baudRate = 1000000;
    console.log(`Opening adapter with ID: ${adapter.instanceId} and baud rate: ${baudRate}...`);

    adapter.open({ baudRate, logLevel: "error" }, err => {
      if (err) {
        reject(Error(`Error opening adapter: ${err}.`));
      } else {
        resolve();
      }
    });
  });
}

async function main(): Promise<void> {
  addAdapterListeners(gAdapter);
  await openAdapter(gAdapter);
  console.log("Opened adapter.");
  await startScan(gAdapter);
  console.log("Scanning.");
}

main().then().catch((error) => {
  console.error(error);
  process.exit(-1);
});
