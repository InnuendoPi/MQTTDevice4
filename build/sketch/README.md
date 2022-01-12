#line 1 "c:\\Arduino\\git\\MQTTDevice4\\README.md"
# MQTTDevice4

MQTTDevice4 enables sensors, actors and an induction hob to be connected via WLAN to [CraftBeerPi V4](https://github.com/Manuel83/craftbeerpi4).
According to the current development status in Jan 2022, this fork [CraftBeerPi V4](https://github.com/avollkopf/craftbeerpi4) version 4.0.1a10 or above is required!

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. This makes it possible to establish communication between the MQTT broker mosquitto and an ESP8266 in order to control sensors and actors with CraftBeerPi V4. MQTTDevice is optimzed for version 4 of craftbeerpi.

## MQTTDevice2 or MQTTDevice4?

There are only a few differences between MQTTDevice 2 and 4. MQTTDevice2 with firmware version 2.5x and earlier were developed for CBPi3. MQTTDevice4 firmware version 4.x and above is optimzed for CBPi4. There will be no further development in MQTTDevice2 repository. If you are using CBPi3 and Influx-Grafana functions from MQTTDevice you have to stay with MQTTDevice2 firmware version 2.5x. All other users should update to MQTTDevice4 firmware version 4.x.

## Update notice Firmware version 4.x from 2.5x

Firmware update from MQTTDevice2: Update -> file upload -> Firmware -> MQTTDevice4.ino.bin
Firmware update MQTTDevice2 version 2.3 or earlier (SPIFFS) is not possible. Reflash your device.

Please note:
All firmware versions 4.x are dedicated to MQTTDevice4. MQTTDevice4 can ONLY operate with CBPi4! If you are using CBPi3 please use MQTTDevice2 and firmware version 2.x

## Documentation

A detailed documentation is available on github pages: <https://innuendopi.github.io/MQTTDevice4/>

## Support

There is support in the hobby brewer forum <https://hobbybrauer.de/forum/> and in facebook CraftbeerPi User Group

## Sketch Information

Libraries: Version 4.02, 01.2022

- ESP8266 3.0.2 (LittleFS)
- Arduino IDE 1.8.16
- Visual Code + modified ESP8266FS Plugin (VSCode 1.52.1 Aruindo 1.8.16)
- PubSubClient 2.8.0 (PubSubClient.h: #define MQTT_MAX_PACKET_SIZE 512)
- ArduinoJSON 6.18.5
- WiFiManager 2.0.5
- NextionX2 1.1.2 (NextionX2.h: #define ATTRIBUTE_TEXT_LENGTH 90)

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)
SSL support all SSL ciphers (most comp)
Exceptions Disabled
IwIP variant v2 lower mem

Debug output:
For debug output, the debug port must be set to serial. Set the debug level accordingly for special debug outputs (default none).
