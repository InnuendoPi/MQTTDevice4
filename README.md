# MQTTDevice4

MQTTDevice4 enables sensors, actors and an induction hob to be connected via WLAN to [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4).

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. This makes it possible to establish communication between the MQTT broker (eg mosquitto) and an ESP8266 in order to control sensors and actors with CraftBeerPi V4.

![Startseite](docs/img/startseite.jpg)

## Documentation

A detailed documentation is available on github pages: <https://innuendopi.github.io/MQTTDevice4/>
A detailed documentation CraftbeerPi4 is available on github pages:: <https://openbrewing.gitbook.io/craftbeerpi4_support/>

## Sketch Information

Libraries: Version 4.47, 05.2023

- ESP8266 3.1.2 (LittleFS)
- Arduino IDE 1.8.19
- Visual Code + modified ESP8266FS Plugin (VSCode 1.77 + Arduino for Visual Code 0.6)
- PubSubClient 2.8.0 (PubSubClient.h: #define MQTT_MAX_PACKET_SIZE 512)
- ArduinoJSON 6.21
- WiFiManager 2.0
- NextionX2 1.1.2 (NextionX2.h: #define ATTRIBUTE_TEXT_LENGTH 90, added currentPageID, page 0x66)
- PCF8574 0.3.8
- InnuTicker

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)

Debug output:
debug port must be set to serial. Set debug level accordingly to needed debug outputs (default none).
