/*
  soft_sensor_master.ino

  This program scans for local BLE peripherals and will connect to the Soft
  Sensor Slave once it detects it. Upon a succesful connection, the characteristic
  containing the sensor readings is updated, and printed via serial. The uController
  should be plugged into the host PC, and will either show up on /dev/ACM* on linux
  or as a standard windows COM port.

  9600 Baud - 1 Stop - No Parity

  Expected BLE Attributes for Slave:
  localName == "SoftSensorSlave"
  service == SoftSensorService: uuid ("19B10000-E8F2-537E-4F6C-D104768A1214")
  characteristic == BLECharacteristic SoftSensorChar: uuid ("19B10001-E8F2-537E-4F6C-D104768A1214")

  characteristic data is stored in the form:
  uint8_t data[NUM_SENSORS * 2];  
    
  characteristic data is printed in the form:
  %d %d %d %d %d %d %d %d %d %d\n  

  Ensure NUM_SENSORS is defined theh same in both files

  The circuit:
    Arduino Nano 33 BLE
*/

#include <ArduinoBLE.h>

//Max # of Sensors is 11 due to BLE transmision limitations
#define NUM_SENSORS 11

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the BLE hardware
  BLE.begin();

  Serial.println("Soft Sensor Master Online");

  // start scanning for Soft Sensor Slave
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service


    if (peripheral.localName() != "SoftSensorSlave") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();    

    controlSlave(peripheral);

    // peripheral disconnected, start scanning again
    Serial.println("Slave disconnected");
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
  }
}

void controlSlave(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the Soft Sensor characteristic
  BLECharacteristic SoftSensorChar = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!SoftSensorChar) {
    Serial.println("Peripheral does not Soft Sensor Characteristic!");
    peripheral.disconnect();
    return;
  } else if (!SoftSensorChar.canWrite()) {
    Serial.println("Peripheral does not have a writable SoftSensorChar characteristic!");
    peripheral.disconnect();
    return;
  }

  uint8_t prev_values[NUM_SENSORS * 2] = {0x00};
  while (peripheral.connected()) {  
    uint8_t curr_vals[NUM_SENSORS * 2] = {0x00};
    SoftSensorChar.readValue(curr_vals, sizeof(curr_vals));
    bool changed = false;
    for (int i = 0; i < NUM_SENSORS * 2; i++){
      if(prev_values[i] != curr_vals[i]){
        changed = true;
      }
    }
    if (changed & (peripheral.connected())){
      Serial.print("ms:");
      Serial.print(millis());
      Serial.print(" Sensor Values:");
      // Reconstruct the uint16_t
      for (int i = 0; i < NUM_SENSORS; i++){
        uint16_t curr_val = (curr_vals[(i * 2) + 1] << 8) | curr_vals[i * 2];
        Serial.print(curr_val);
        Serial.print(' '); // Replace this and the println below with ',' for csv format
      }
      Serial.println(); 
    }

   
    for (int i = 0; i < NUM_SENSORS * 2; i++){
      prev_values[i] = curr_vals[i];
    }
  }
}
