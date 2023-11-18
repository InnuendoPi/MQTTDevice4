# MQTTDevice4

MQTTDevice4 enables sensors, actors and an induction hob to be connected via WLAN to [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4).

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. This makes it possible to establish communication between the MQTT broker (eg mosquitto) and an ESP8266 in order to control sensors and actors with CraftBeerPi V4.

![Startseite](docs/img/startseite.jpg)

## Documentation

A detailed documentation is available on github pages: <https://innuendopi.github.io/MQTTDevice4/>
A detailed documentation CraftbeerPi4 is available on github pages:: <https://openbrewing.gitbook.io/craftbeerpi4_support/>

## Sketch Information

Libraries: Version 4.56, 11.2023

- ESP8266 3.1.2
- Arduino IDE 1.8.19
- VSCode 1.84 Arduino-CLI 0.34.2 modified ESP8266FS Plugin (LittleFS)
- OneWire
- DallasTemperature
- PubSubClient
- ArduinoJSON
- WiFiManager
- modified NextionX2
- InnuTicker
- InnuFramework

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)
