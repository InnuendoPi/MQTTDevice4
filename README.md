# MQTTDevice4

[![en](https://img.shields.io/badge/lang-en-yellow.svg)](https://github.com/InnuendoPi/MQTTDevice4/blob/main/README.en.md)
[![ESP32](https://img.shields.io/static/v1?label=Arduino&message=ESP32%20&#8594;&logo=arduino&logoColor=white&color=blue)](https://github.com/InnuendoPi/MQTTDevice32)

MQTTDevice32 ist ein Arduino Sketch für Wemos ESP32 D1 mini Module. Mit dem MQTTDevice32 können Sensoren, Aktoren und das Induktionskochfeld GGM IDS2 über WLAN mit [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4) verbunden werden.

![Startseite](docs/img/startseite.jpg)

## 💻 Installation

* Download [Firmeware.zip](https://github.com/InnuendoPi/MQTTDevice4/blob/main/tools/Firmware.zip)
* Firmware.zip entpacken
* Flashen.cmd editieren:
* "COM3" in Zeile 6  und Zeile 8"esptool.exe -p COM3" anpassen
* Eingabeaufforderung (cmd.exe) öffnen und in das Verzeichnis von firmware.zip wechseln
* Firmware auf ESP32 ladeen mit "flashen.cmd"

Das Script flashen.cmd verwendet [esptool](https://github.com/espressif/esptool) (im ZIP Archiv enthalten).

## 📚 Documentation

Beschreibung & Anleitung: [Anleitung](https://innuendopi.gitbook.io/mqttdevice32/)

## 📰 Sketch Information

Libraries: Version 4.58, 12.2023

* ESP8266 3.1.2
* Arduino IDE 1.8.19
* VSCode 1.85 Arduino-CLI ESP8266FS Plugin (LittleFS)
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
