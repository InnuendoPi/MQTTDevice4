#line 1 "c:\\Arduino\\git\\MQTTDevice4\\README.md"
# MQTTDevice4

MQTTDevice4 enables sensors, actors and an induction hob to be connected via WLAN to [CraftBeerPi V4](https://github.com/Manuel83/craftbeerpi4).

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. This makes it possible to establish communication between the MQTT broker mosquitto and an ESP8266 in order to control sensors and actors with CraftBeerPi V4. MQTTDevice is optimzed for version 4 of craftbeerpi

## Update notice Version 2.6

When updating to MQTTDEVICE4 from MQTTDEVICE2 you must use file upload.

## Documentation

A detailed documentation is available on github pages: <https://innuendopi.github.io/MQTTDevice2/>
(german only)

## Support

There is support in the hobby brewer forum <https://hobbybrauer.de/forum/> and in facebook CraftbeerPi User Group

## Sketch Information

Libraries: (Version 2.60, 12.2021)

- ESP8266 3.0.2 (LittleFS)
- Arduino IDE 1.8.15
- Visual Code 1.52.1 + modified ESP8266FS Plugin (VSCode 1.52.1 Aruindo 1.8.16)
- PubSubClient 2.8.0
- ArduinoJSON 6.18.5
- InfluxDB 3.9.0
- WiFiManager 2.0.5

Board configuration:
Flash size 4MB (FS:2MB OTA:~1019kB)
SSL support all SSL ciphers (most comp)
Exceptions Disabled
IwIP variant v2 lower mem

Debug output:
For debug output, the debug port must be set to serial. Set the debug level accordingly for special debug outputs (default none).
