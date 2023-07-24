# GPS Tracker with SIM800L and NEO-6M

This Arduino sketch allows you to track GPS coordinates using an ESP32 board, SIM800L GSM module, and NEO-6M GPS module. The GPS coordinates are sent to an MQTT broker for further processing or visualization.

## Prerequisites

To use this sketch, you will need the following hardware components:

- ESP32 board
- SIM800L GSM module
- NEO-6M GPS module

Make sure you have the necessary libraries installed:

- [TinyGsm](https://github.com/vshymanskyy/TinyGSM)
- [TinyGPSPlus](https://github.com/mikalhart/TinyGPSPlus)
- [ArduinoJson](https://arduinojson.org/)
- [PubSubClient](https://pubsubclient.knolleary.net/)

## Configuration

Before uploading the sketch, you may need to modify the following settings:

- **GPRS credentials**: Set the correct Access Point Name (APN), GPRS username, and password for your GSM network provider.

- **MQTT broker details**: Set the server address (mqtt_server) and port (mqtt_port) for your MQTT broker. Also, update the `sensor1_topic` to your desired topic name.

- **Serial port settings**: If your ESP32 is connected to a different serial port than the default pins (16 and 17), update the `SIM800_RX` and `SIM800_TX` definitions accordingly.