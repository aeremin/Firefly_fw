import { Adapter, AdapterFactory, Characteristic, ConnectionOptions,
  Descriptor, Device, ScanParameters, Service } from "pc-ble-driver-js";

const BLE_UUID_CCCD = "2902";

export class BluetoothLowEnergy {
  private listeners: Array<{
    serviceUuid: string, characteristicUuid: string,
    listener: (deviceAddress: string, characteristic: Characteristic) => void}> = [];
  private adapter: Adapter = AdapterFactory.getInstance().createAdapter("v3", "COM18", "");
  private knownCharacteristics: { [deviceAddress: string]: Characteristic[] } = {};
  private connectListener: (deviceAddress: string) => void = undefined;
  private disconnectListener: (deviceAddress: string) => void = undefined;

  public addCharacteristicListener(
    serviceUuid: string,
    characteristicUuid: string,
    listener: (deviceAddress: string, characteristic: Characteristic) => void,
  ) {
    this.listeners.push({serviceUuid: serviceUuid.toLowerCase(),
      characteristicUuid: characteristicUuid.toLowerCase(), listener});
  }

  public setConnectListener(connectListener: (deviceAddress: string) => void) {
    this.connectListener = connectListener;
  }

  public setDisconnectListener(disconnectListener: (deviceAddress: string) => void) {
    this.disconnectListener = disconnectListener;
  }

  public async startScanningAndReporting() {
    this.addAdapterListeners();
    await this.openAdapter();
    console.log("Opened adapter.");
    await this.startScan();
    console.log("Scanning.");
  }

  public async writeCharacteristic(deviceAddress: string, serviceUuid: string,
                                   characteristicUuid: string, value: number[]): Promise<void> {
    const device = this.knownCharacteristics[deviceAddress];
    if (device == undefined) {
      throw new Error("No such device connected");
    }
    const characteristic = device.find(
      (ch) => ch.uuid.toLowerCase() == characteristicUuid.toLowerCase());
    if (characteristic == undefined) {
      throw new Error("No such characteristic found");
    }

    return new Promise((resolve, reject) => {
      this.adapter.writeCharacteristicValue(characteristic.instanceId, value, true, (err) => {
        if (err) {
          reject(err);
        } else {
          resolve();
        }
      });
    });
  }

  private addAdapterListeners(): void {
    this.adapter.on("logMessage", (severity, message) => {
      if (Number(severity) > 3) {
        console.log(`${message}.`);
      }
    });
    this.adapter.on("error", (error) => { console.error(`error: ${JSON.stringify(error, null, 1)}.`); });
    this.adapter.on("deviceConnected", async (device) => {
      try {
        console.log(`Device ${device.address}/${device.addressType} connected.`);
        this.knownCharacteristics[device.address] = [];
        const services = await this.discoverServices(device);
        console.log("Discovered services.");
        const relevantServices = services.filter((s) =>
          this.listeners.find((l) => l.serviceUuid == s.uuid.toLowerCase()) != undefined);
        for (const service of relevantServices) {
          const characteristics = await this.discoverharacteristics(service);
          this.knownCharacteristics[device.address].push(...characteristics);
          const relevantCharacteristics = characteristics.filter((ch) =>
            this.listeners.find((l) => l.serviceUuid == service.uuid.toLowerCase() &&
                                       l.characteristicUuid == ch.uuid.toLowerCase()) != undefined);
          for (const characteristic of relevantCharacteristics) {
            console.log("Discovered characteristic.");
            const descriptor = await this.discoverCharacteristicDescriptor(characteristic);
            console.log("Discovered the descriptor");
            await this.subscribeToCharacteristicNotifications(descriptor);
          }
        }
        if (this.connectListener) {
          this.connectListener(device.address);
        }
      } catch (error) {
        console.error(error);
      }
    });

    this.adapter.on("deviceDisconnected", async (device) => {
      console.log(`Device ${device.address} disconnected.`);
      delete this.knownCharacteristics[device.address];
      if (this.disconnectListener) {
        this.disconnectListener(device.address);
      }
      try {
        await this.startScan();
        console.log("Successfully initiated the scanning procedure.");
      } catch (error) {
        console.error(error);
      }
    });

    this.adapter.on("deviceDiscovered", async (device) => {
      if (device.name == "Nordic_Blinky") {
        try {
          await this.connectToPeripheral(device.address);
        } catch (error) {
          console.error(error);
          process.exit(1);
        }
      }
    });

    this.adapter.on("scanTimedOut", () => {
      console.error("scanTimedOut: Scanning timed-out. Exiting.");
      process.exit(1);
    });

    this.adapter.on("characteristicValueChanged", (characteristic) => {
      // TODO: Also confirm that this is relevant service.
      const listener = this.listeners.find((l) => l.characteristicUuid == characteristic.uuid.toLowerCase());
      if (listener == undefined) {
        return;
      }
      for (const deviceAddress in this.knownCharacteristics) {
        if (this.knownCharacteristics[deviceAddress].find((ch) => ch.instanceId == characteristic.instanceId)) {
          listener.listener(deviceAddress, characteristic);
          return;
        }
      }
      console.error("Corresponding device not found");
    });
  }

  private openAdapter(): Promise<void> {
    return new Promise((resolve, reject) => {
      const baudRate = 1000000;
      console.log(`Opening adapter with ID: ${this.adapter.instanceId} and baud rate: ${baudRate}...`);
      this.adapter.open({ baudRate, logLevel: "error" }, (err) => {
        if (err) {
          reject(Error(`Error opening adapter: ${JSON.stringify(err)}.`));
        } else {
          resolve();
        }
      });
    });
  }

  private startScan(): Promise<void> {
    return new Promise((resolve, reject) => {
      console.log("Starting scanning...");

      const scanParameters: ScanParameters = {
        active: true,
        interval: 100,
        window: 50,
        timeout: 0,
      };

      this.adapter.startScan(scanParameters, (err) => {
        if (err) {
          reject(new Error(`Error starting scanning: ${JSON.stringify(err)}.`));
        } else {
          resolve();
        }
      });
    });
  }

  private connectToPeripheral(connectToAddress: string): Promise<void> {
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
          min_conn_interval: 100,
          max_conn_interval: 200,
          slave_latency: 0,
          conn_sup_timeout: 4000,
        },
      };

      this.adapter.connect(connectToAddress, options, (err) => {
        if (err) {
          reject(Error(`Error connecting to target device: ${JSON.stringify(err)}.`));
        } else {
          resolve();
        }
      });
    });
  }

  private discoverServices(device: Device): Promise<Service[]> {
    return new Promise((resolve, reject) => {
      this.adapter.getServices(device.instanceId, (err, services) => {
        if (err) {
          reject(Error(`Error discovering the service: ${JSON.stringify(err)}.`));
          return;
        }
        resolve(services);
      });
    });
  }

  private discoverharacteristics(service: Service): Promise<Characteristic[]> {
    return new Promise((resolve, reject) => {
      this.adapter.getCharacteristics(service.instanceId, (err, characteristics) => {
        if (err) {
          reject(Error(`Error discovering characteristics: ${JSON.stringify(err)}.`));
        } else {
          resolve(characteristics);
        }
      });
    });
  }

  private discoverCharacteristicDescriptor(characteristic: Characteristic): Promise<Descriptor> {
    return new Promise((resolve, reject) => {
      this.adapter.getDescriptors(characteristic.instanceId, (err, descriptors) => {
        if (err) {
          reject(Error(`Error discovering the characteristic's descriptor: ${JSON.stringify(err)}.`));
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

  private subscribeToCharacteristicNotifications(descriptor: Descriptor): Promise<void> {
    return new Promise((resolve, reject) => {
      this.adapter.writeDescriptorValue(descriptor.instanceId, [1, 0], false, (err) => {
        if (err) {
          reject(`Error enabling notifications on the characteristic: ${JSON.stringify(err)}.`);
        } else {
          console.log("Notifications turned on for characteristic.");
          resolve();
        }
      });
    });
  }
}
