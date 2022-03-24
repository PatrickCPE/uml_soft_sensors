# Soft Sensors - UML Nerve Center
### Author: Patrick Hoey

## Overview
The system utilizes capacitive flex sensors to measure how much a person's muscles bend when performing physical exercises.

## uController
### uController Overview
The selected baord was the Arduino Nano 33 BLE. One board is used as the master(PC) and the other is the slave(sensor device). The software was written using the arduino libraries, specifically the ArduinoBLE library. The slave polls the sensors at the desired frequency and advertises if there are any changes. The master writes the current result to the serial monitor at the desired frequency, and will update the result if there is a new value advertised for the sensors.

### uController Setup
1. [Install Arduino IDE: Version >= 2.0](https://www.arduino.cc/en/software)
2. [Install ArduinoBLE Library: Version == 1.2.1](https://www.arduino.cc/en/Reference/ArduinoBLE)
   A. Open Arduino IDE
   B. Select Libraries on the left hand side
   C. Search for ArduinoBLE, select the right version, and click install
3. [Install the Adafruit MPR121 Library: Version == 1.1.1](https://docs.arduino.cc/static/d7a8bf3f62b04aa29ec3036a6a2a4f51/ABX00030-datasheet.pdf) 
   A. Open Arduino IDE
   B. Select Libraries on the left hand side
   C. Search for Adafruit MPR121, select the right version, and click install, install all depenencies as well
4. Install Arduino Mbed OS Nano Board Files: Version == 2.8.0
   A. Open Arduino IDE
   B. Select Boards Manager on the left hand side
   C. Search for Arduino Mbed OS Nano Boards, select the right version, and click install
5. Flash the boards
   A. Open soft_sensor_master.ino and soft_sensor_slave.ino in the Arduino IDE
   B. Select the appropriate board and port from the tools dropdown
   C. Flash one board with the master software, and the other with the slave software

## Host Side Software
### Host Side Overview
The current strategy is to keep it as simple as possible. Download Teraterm, and save the terminal session to a log file.

### Host Side Setup
[Get Teraterm](https://ttssh2.osdn.jp/index.html.en)
[Saving to file with Teraterm](https://ttssh2.osdn.jp/manual/4/en/usage/tips/loging_howto.html)

## Resources
[Arduino BLE](https://www.arduino.cc/en/Reference/ArduinoBLE)
[Arduino Nano 33 BLE Datasheet](https://docs.arduino.cc/static/d7a8bf3f62b04aa29ec3036a6a2a4f51/ABX00030-datasheet.pdf)
[MPR121 Datasheet](https://www.sparkfun.com/datasheets/Components/MPR121.pdf)
Dylan Shah's cap_sensor_driver.ino

