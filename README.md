# MQTTDevice4

[![en](https://img.shields.io/badge/lang-en-yellow.svg)](https://github.com/InnuendoPi/MQTTDevice4/blob/main/README.en.md)
[![ESP32](https://img.shields.io/static/v1?label=Arduino&message=ESP32%20&#8594;&logo=arduino&logoColor=white&color=blue)](https://github.com/InnuendoPi/MQTTDevice32)

MQTTDevice4 ist ein Arduino Sketch für Wemos ESP8266 D1 mini Module. Mit dem MQTTDevice4 können Sensoren, Aktoren und das Induktionskochfeld GGM IDS2 per WLAN über einen MQTTBroker (bspw. mosquitto) mit [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4) verbunden werden.

![Web Interface](docs/img/startseite.jpg)

## ✅ Funktionen

* Web Interface (WebIf)
* Backup und Restore der Konfiguration
* Server Sent Events (SSE) für WebClients
* Temperatursensoren (max 3)
  * Dallas DS18B20 Sensoren
    * Suchfunktion für Dallas DS18B20 OneWire Sensoren
  * PT100 und PT1000 Sensoren
    * MAX31865 Amplifyer
* Aktoren (max 10)
  * GPIO Auswahl
  * belegte GPIOs werden in der Auswahl ausgeblendet
  * GPIO invertieren
  * Einfaches PWM: Aktoren können auf 0 bis 100% Leistung eingestellt werden. Das MQTTDevice takten im Zyklus von 1000ms
* Induktionskochfeld
  * Induktionskochfeld GGM IDS2 wird direkt gesteuert
* Nextion HMI Touchdisplay Unterstützunh (optional)
* WebUpdate Firmware
* DateiUpdate Firmware
* mDNS Support
* Event handling
* Dateiexplorer
* Unterstützung für versch. Sprachen

## 💻 Installation

* Download [Firmeware.zip](https://github.com/InnuendoPi/MQTTDevice4/blob/main/tools/Firmware.zip)
* Firmware.zip entpacken
* Flashen.cmd editieren:
* "COM3" in Zeile 6  und Zeile 8"esptool.exe -p COM3" anpassen
* Eingabeaufforderung (cmd.exe) öffnen und in das Verzeichnis von firmware.zip wechseln
* Firmware auf ESP8266 laden mit "flashen.cmd"

Das Script flashen.cmd verwendet [esptool](https://github.com/espressif/esptool) (im ZIP Archiv enthalten).

## 📚 Anleitung und Dokumentation

Beschreibung & Anleitung: [Anleitung](https://innuendopi.gitbook.io/mqttdevice32/)\
CraftBeerPi V4 Dokumentation und Support: [CraftBeerPi](https://openbrewing.gitbook.io/craftbeerpi4_support/)

## 💠 Pin-Belegung

| Bezeichner |   GPIO   |  Input  |  Output  | Beschreibung |
| ---------- | -------- | ------- | -------- | ------------ |
|     D0     |  GPIO026 |   ok    |   ok     | MAX31865 MOSI (High at boot, no int, no PWM) |
|     D1     |  GPIO022 |   ok    |   ok     | SCL Display, MAX31865 MISO  |
|     D2     |  GPIO021 |   ok    |   ok     | SDA Display, MAX31865 CLK  |
|     D3     |  GPIO017 |   ok    |   ok     | DS18B20 sensors |
|     D4     |  GPIO016 |         |   ok     | MAX31865 CS0 (High at boot, onboard LED)|
|     D5     |  GPIO018 |   ok    |   ok     | IDS2 blau SCLK, MAX31865 CS1   |
|     D6     |  GPIO019 |   ok    |   ok     | IDS2 gelb, MAX31865 CS2 |
|     D7     |  GPIO023 |   ok    |   ok     | IDS2 weiß |
|     D8     |  GPIO005 |         |   ok     | Buzzer       |

## 📰 Sketch Information

Libraries: Version 4.58, 12.2023

* ESP8266 3.1.2
* Arduino IDE 1.8.19
* VSCode 1.85 Arduino-CLI ESP8266FS Plugin (modifiziert für LittleFS)
* OneWire
* DallasTemperature
* PubSubClient
* ArduinoJSON
* WiFiManager
* NextionX2 modifiziert
* InnuTicker
* InnuFramework

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)
