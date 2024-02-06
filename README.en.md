# MQTTDevice4

[![de](https://img.shields.io/badge/lang-de-yellow.svg)](https://github.com/InnuendoPi/MQTTDevice4/blob/main/README.md)
[![ESP32](https://img.shields.io/static/v1?label=Arduino&message=ESP32%20&#8594;&logo=arduino&logoColor=white&color=blue)](https://github.com/InnuendoPi/MQTTDevice32)

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. MQTTDevice4 enables sensors, actors and an induction hob GGM IDS2 to be communicate via WLAN and MQTT with [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4).

![Web Interface](docs/img/startseite.jpg)

## ✅ Features

* Web Interface (WebIf) for easy configuration, backup and restore
* Server Sent Events (SSE) for WebClients
* Temperature sensors
  * Dallas DS18B20 Sensors (digital)
    * Search for connected sensors based on OneWire addresses
  * PT100 and PT1000 sensors (analog)
    * MAX31865 Amplifyer
  * MQTTDevice32 supports max 6 sensors
  * MQTTDevice4 supports max 3 sensors
* Actors (max 15)
  * GPIO selection
  * used GPIOs are hidden
  * inverted GPIO
  * PWM values ​​between 0 and 100% are sent. MQTTDevice "pulses" with a cycle of 1000ms
  * MQTTDevice32 supports up to 15 actors
  * MQTTDevice4 supports 10 actors
* Induction hob
  * induction hob GGM IDS2 can be controlled directly
* Nextion HMI Touchdisplay support
* Audio signals Piezo Buzzer
* WebUpdate firmware
* mDNS support
* Event handling
* File explorer
* support for different languages

## 💻 Installation

* Download [Firmeware.zip](https://github.com/InnuendoPi/MQTTDevice4/blob/main/tools/Firmware.zip)
* unzip Firmware.zip
* edit Flashen.cmd:
* change "COM3" in line 6 und line 8 "esptool.exe -p COM3" as you need
* open command line (cmd.exe) and change into firmware.zip directory
* start script "flashen.cmd"

Script flashen.cmd use [esptool](https://github.com/espressif/esptool).

## 📚 Documentation

A detailed MQTTDevice documentation is available on gitbook: [documentation](https://innuendopi.gitbook.io/mqttdevice32/)

## 🗺️ Multilingual

MQTTDevice supports different languages. Each language has its own language file. The language files in JSON format are stored in the data folder.

_Supported the project and translate MQTTDevice into a new language or extend existing language files!_

## 💠 Pin-Belegung

| Name |   GPIO   |  Input  |  Output  | Description |
| ---------- | -------- | ------- | -------- | ------------ |
|     D0     |  GPIO026 |   ok    |   ok     | MAX31865 MOSI (High at boot, no int, no PWM) |
|     D1     |  GPIO022 |   ok    |   ok     | SCL Display, MAX31865 MISO  |
|     D2     |  GPIO021 |   ok    |   ok     | SDA Display, MAX31865 CLK  |
|     D3     |  GPIO017 |   ok    |   ok     | DS18B20 sensors |
|     D4     |  GPIO016 |         |   ok     | MAX31865 CS0 (High at boot, onboard LED)|
|     D5     |  GPIO018 |   ok    |   ok     | IDS2 blue, MAX31865 CS1   |
|     D6     |  GPIO019 |   ok    |   ok     | IDS2 yellow, MAX31865 CS2 |
|     D7     |  GPIO023 |   ok    |   ok     | IDS2 white |
|     D8     |  GPIO005 |         |   ok     | Buzzer       |

## 📰 Sketch Information

Libraries: Version 4.58, 12.2023

* ESP8266 3.1.2
* Arduino IDE 1.8.19
* VSCode 1.85 Arduino-CLI ESP8266FS Plugin (modified LittleFS)
* OneWire
* DallasTemperature
* PubSubClient
* ArduinoJSON
* WiFiManager
* modified NextionX2
* InnuTicker
* InnuFramework

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)
