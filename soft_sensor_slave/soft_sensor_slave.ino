/*
  soft_sensor_slave.ino

  This program scans for local BLE peripherals and will connect to the Soft
  Sensor Slave once it detects it. Upon a succesful connection, the characteristic
  containing the sensor readings is updated, and printed via serial. The uController
  should be plugged into the host PC, and will either show up on /dev/ACM* on linux
  or as a standard windows COM port.

  This program publishes to local BLE peripherals. It's assumes the soft_sensor_master.ino
  program is running on a PC connected board nearby. This program reads from the capacitive
  sensor and sends the data out via bluetooth.

  Expected BLE Attributes for Slave:
  localName == "SoftSensorSlave"
  service == SoftSensorService: uuid ("19B10000-E8F2-537E-4F6C-D104768A1214")
  characteristic == BLECharacteristic SoftSensorChar: uuid ("19B10001-E8F2-537E-4F6C-D104768A1214")

  characteristic data is stored in the form:
  uint8_t data[12];

  The circuit:
  Arduino Nano 33 BLE
  LM117-3.3V Regulator
  MPR121 Capacitive Touch Sensor
*/

#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>

// Max # of Sensors is 11 due to BLE transmision limitations
#define NUM_SENSORS 11
// Update Period in miliseconds (ex: 200 = 5hz) Lowest tested is 10hz (100)
#define PERIOD 100

// Only enable when printing to terminal from slave, will break if battery powered
#define DEBUG 0
// Only enable if you want random test values rather than real data
#define MPR_EMULATED 1

BLEService SoftSensorSlaveService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// Soft Sensor Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic SoftSensorChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, NUM_SENSORS * 2, sizeof(byte));

const int ledPin = LED_BUILTIN; // pin to use for the LED

Adafruit_MPR121 cap_sensor = Adafruit_MPR121();

unsigned long start_time;


void setup() {
  Wire.begin(); //Startup I2C for cap sensor
  if(DEBUG){
    Serial.begin(9600);
    while (!Serial);
  }
  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // MPR121 Setup

  // Advanced users: Default MPRS121 address is 0x5A. If the ADDR pin on the MPR121 is connected to 3.3V, the I2C address is 0x5B. If connected to SDA it's 0x5C, and if SCL then 0x5D.
  if(!MPR_EMULATED){
    if (!cap_sensor.begin(0x5A)) {
      if(DEBUG){
        Serial.println("MPR121 not found, check wiring?"); // Something went wrong when the Arduino tried to intialize the MPR121 that has the specified address
        while(true);
      }
    }

    // Auto-configure charge time and charge current for MPR121.
    // Advanced users: See section 12 (p 16) of 2010 datasheet, or Page 17 of 2013 datasheet. Note that AFES was renamed as FFI, but the register is the same 0x7B (which is what really matters)
    // Only keep 8 least-significant bits, convert to binary. FFI --6 samples (matches the Adafruit_MPR121.cpp MPR121_CONFIG1, as required). RETRY --> Retry 2 times. BVA --> No change. Auto-reconfig disabled, enable auto-config
    cap_sensor.writeRegister(MPR121_AUTOCONFIG0, 0b00010001); 
    // Specify the search boundaries and target for the auto-configuration. Values for Vdd = 3.3V are 200, 180, 130.
    cap_sensor.writeRegister(MPR121_UPLIMIT, 200);     // ((Vdd - 0.7)/Vdd) * 256
    cap_sensor.writeRegister(MPR121_TARGETLIMIT, 180); // UPLIMIT * 0.9
    cap_sensor.writeRegister(MPR121_LOWLIMIT, 130);    // UPLIMIT * 0.65

  }
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("SoftSensorSlave");
  BLE.setAdvertisedService(SoftSensorSlaveService);

  // add the characteristic to the service
  SoftSensorSlaveService.addCharacteristic(SoftSensorChar);

  // add service
  BLE.addService(SoftSensorSlaveService);

  // set the initial value for the characeristic:
  uint8_t values[NUM_SENSORS * 2] = {0x00}; // 16 bit analog reads
  SoftSensorChar.writeValue(values, sizeof(values));

  // start advertising
  BLE.advertise();
  if(DEBUG){
    Serial.println("Soft Sensor Slave Online");
  }
}

void loop() {
  // Used for delay
  start_time = millis();

  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    if(DEBUG){
      Serial.print("Connected to central: ");
      // print the central's MAC address:
      Serial.println(central.address());
    }

    // while the central is still connected to peripheral:
    uint8_t values[NUM_SENSORS * 2] = {0};
    while (central.connected()) {
      // While we are connected

      //Update Sensor Reads
      // Only Leave One of the following blocks uncommented
      if(DEBUG){
        Serial.print("\nTest_Vals: ");
      }
      for(int i = 0; i < NUM_SENSORS; i++){
        if(MPR_EMULATED){
          uint16_t test_val = (uint16_t)random(1 << 15);
          if(DEBUG){
            Serial.print(test_val);
            Serial.print(' ');
          }

          values[i * 2] = test_val & 0xFF; // Low Order Byte
          values[(i * 2) + 1] = test_val >> 8; // High Order Byte

        } else {
          // For Real Cap Sensor
          uint16_t curr_val = cap_sensor.filteredData(i);
          values[i * 2] = curr_val & 0xFF; // Low Order Byte
          values[(i * 2) + 1] = curr_val >> 8; // High Order Byte
        }
      }

      // Publish Data via BLE
      if(DEBUG){
        Serial.print("\nCasted Vals:");

        for (int i = 0; i < NUM_SENSORS * 2; i++){
          Serial.print(values[i]);
          Serial.print(' '); // Replace this and the println below with ',' for csv format
        }
        Serial.println();

        Serial.print("Start Time: ");
        Serial.print(start_time);
        unsigned long curr_time = millis();
        Serial.print(" curr_time: ");
        Serial.print(curr_time);
      }

      SoftSensorChar.writeValue(values, sizeof(values));
      unsigned long delay_time = PERIOD - (millis() - start_time);
      if(delay_time <= 0){
        Serial.println("System can't process that fast. Lower PERIOD");
        while(true);
      }
      if (DEBUG){
        Serial.print(" Delay: ");
        Serial.print(delay_time);
      }

      delay(delay_time);
      start_time = millis();
    }

    // when the central disconnects, print it out:
    if(DEBUG){
      Serial.print("Disconnected from central: ");
      Serial.println(central.address());
    }
  }
}
