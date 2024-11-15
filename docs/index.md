# MQTTDevice Version 4

*What is MQTTDevice?**

MQTTDevice4 is an Arduino sketch for the ESP8266 Wemos D1 mini modules. MQTTDevice connects to a MQTT Broker in order to control sensors and actors with CraftBeerPi V4. Multiple MQTTDevices can be connected to a MQTT broker.  

![Startseite](img/startseite.jpg)

**What does this firmware offer?**

* A configuration web interface (WebIf)
* Server Sent Events (SSE) for WebClients refresh
* Temperature sensors DS18B20 (max 6)
  * Search for connected sensors based on OneWire addresses
* Actors (max 10)
  * PIN selection (GPIO)
  * PINs in use are hidden
  * Inverted GPIO
  * Power Percentage: Values ​​between 0 and 100% are sent. The ESP8266 "pulses" with a cycle of 1000ms
* Induction hob
  * the induction hob GGM IDS2 can be controlled directly
* Nextion HMI Touchdisplay support (optional)
* WebUpdate firmware
* mDNS support
* Event handling
* File explorer

This project was started in the hobbybrauer forum and serves the exchange of information.
Forum: <https://hobbybrauer.de/forum/viewtopic.php?f=58&t=23509>

---

## Installation

The installation is divided into three steps:

1. Installation of RaspberryPi and CraftbeerPi4
2. Installation MQTT Broker
3. Installation MQTTDevice

The installation and configuration of CraftbeerPi4 is described here: <https://openbrewing.gitbook.io/craftbeerpi4_support/master/server-installation>
You will need version 4.0.1.11 or above. Earlier versions do not support MQTT actors.

The installation and configuration of RaspberryPi is available in many good instructions on the internet.

The communication between CraftbeerPi and MQTTDevice takes place via WLAN. Sensors send temperature values ​​to CraftbeerPi and CraftbeerPi sends commands to actors (e.g. switch agitator on / off). The MQTT protocol is used for this communication. The MQTT protocol requires a MQTT broker.

**MQTT in short:**

MQTT is a publish-and-subscribe protocol. Clients, devices and applications publish and subscribe into topics handled by a broker. In a CraftbeerPi environment a sensor is a publishing and an actor is a subscribing device. Devices or applications never communicate directly, instead they send and receive messages managed in topics by the MQTT broker. A topic looks like a named channel or folder, eg induction/temp or upstairs/bathroom/light. Hundreds of different topics are possible. Each topic behave like a message in- and outbox. Messages are send in a compact JSON format, also known as payloads. A simple payload from a sensor device can look like this: { "value": 21.2 }. The sensor publishes this message into a topic. All subscribed clients, devices or application to this topic will receive the sensor message "value 21.2". A simple payload from CraftbeerPi to an actor can be { "state": "on"}. CBPi4 publishes this message into a topic to switch on an actor. An actor must subcribe to this topics to receive this message.

MQTT summary:

* sensors are publishing payloads (sensor values) into topics
* actors subscribe to topics to receive payloads (commands)
* every published payload will always be sent to a MQTT broker
* the MQTT broker sends every received payload to all subscribing devices and applications
* craftbeerpi subscribes to all sensor topics and publishes actor commands in actor topics
* all sensor and actor topics are unique

**MQTT CraftbeerPi4:**

MQTT installation and configuration is described here in detail: <https://openbrewing.gitbook.io/craftbeerpi4_support/readme/craftbeerpi-4-server/mqtt-connectivity>

After CBPi4 restart MQTT is available for sensors and actors

![mqttSensor](img/mqttSensor.jpg)

![mqttAktor](img/mqttAktor.jpg)

Please note the dot in PayloadDictionary: Sensor.Value (Sensor dot Value)

MQTT mosquitto instalaltion and configuration for windows is nearly the same. If MQTT actors and sensors are used instead of GPIOs hardware, craftbeerpi4 can be installed on ms windows systems.

**MQTTDevice flash firmware:**

With the help of esptool.exe (<https://github.com/igrr/esptool-ck/releases>) in the github tools subfolder, the firmware can be flashed onto the ESP module. The ESPTool is available for different operating systems.
ESPtool-ck Copyright (C) 2014 Christian Klippel <ck@atelier-klippel.de>. This code is licensed under GPL v2.

The USB driver CH341SER is required under Win10: <http://www.wch.cn/download/CH341SER_ZIP.html>

Example for an ESP8266 module of the type Wemos D1 mini with 4MB Flash connected via USB as COM3

* Download the Firmware.zip archive from the tools folder on github and extract it to any folder

  * The archive contains the esptool for flashing, the Flashen.cmd script and the two firmware files

  * Double click on the file Flashen.cmd.

  The firmware is now installed on the MQTT device.

  *The Flash script uses COM3 as the connection for the MQTTDevice. If COM3 is not the correct port for the Wemos D1 mini, COM3 must be replaced by the correct port in two places in the Flashen.cmd script.*

  After flashing, the MQTT device starts in access point mode with the name "ESP8266-xxxx" und address <http://192.168.4.1>

![wlan](img/wlan-ap.jpg)

The MQTT device must now be connected to the WLAN.

The MQTT sensor "Induction temperature" and the MQTT actor "Agitator" are now created on the MQTTdevice with the identical CBPi4 topics:

![mqttSensor2](img/mqttSensor2.jpg)

![mqttAktor2](img/mqttAktor2.jpg)

The sensor or actor name can be different. These steps complete the exemplary installation and configuration of MQTT sensors and actors. Up to 6 sensors and 8 actors can be set up per MQTT device. (Almost) any number of MQTT devices can be connected to CraftbeerPi via MQTT. Three MQTT devices are used very often:

MQTTDevice 1: Mash tun with temperature sensors and agitator.

MQTTDevice 2: HLT with temperature sensors, pumps and valves etc.

MQTTDevice 3: Fermenter with temperature sensors, an actor for heating and an actor for cooling

Any combination is possible. Because the MQTT communication is implemented via topics, a temperature sensor and an induction hob for a CraftbeerPi Kettle do not have to be configured on the same MQTTDeivce.

![induction](img/induction.jpg)

The picture above is an example on how to configure an induction hob GGM IDS2. Do not reduce fan run on after power off below 120sec: savely cool down induction hob after mash or boil. GPIOs D5, D6 and D7 are highly recommended. Check ESP8266 manual for further information about GPIO states (High/Low) on startup.

**Updates:**

The firmware offers two options for installing updates very easily.

1. Firmware Update file upload

    Call up the URL <http://mqttdevice/update> in the web browser or use the "Update" -> "Upload" button in the WebIf.
    Firmware and the LittleFS file system can be updated here. If the file system LittleFS is updated with file upload, the configuration file is overwritten. See backup and restore.

2. WebUpdate

    Open the webfront of your MQTTDevice in your web browser and call the "Update" -> "WebUpdate" function.
    WebUpdate updates the firmware, the index file and certificates. The configuration file is not overwritten by WebUpdate.

The WebUpdate can take a few minutes, depending on your internet connection. The web interface is not available during the web update. If only a very slow internet connection is available, the message "Browser not responding" is displayed after approx. 60 seconds. Please wait and let the WebUpdate run through.

**Backup and Restore:**

The file explorer can be reached via the web browser <http://mqttdevice/edit>

1. Backup

   Click on the file config.txt (left mouse button) and select download from the popup.

2. Restore

    Click on Select file, select the config.txt from the backup and click on upload

**Supported and tested hardware:**

Hardware (02.2022)

| Anzahl | Link |
| ------------ | ---- |
| ESP8266 D1 Mini | <https://www.amazon.de/dp/B01N9RXGHY/ref=cm_sw_em_r_mt_dp_0KzyFbK6YG2BE> |
| Relais Board 4 Kanal | <https://www.amazon.de/dp/B078Q8S9S9/ref=cm_sw_em_r_mt_dp_PHzyFbSR1PKCH> |
| Relais Board 1 Kanal | <https://www.amazon.de/dp/B07CNR7K9B/ref=cm_sw_em_r_mt_dp_FIzyFbKXXYE0H> |
| Nextion Display 3.5" | <https://www.amazon.de/dp/B07SSG86VC/ref=cm_sw_em_r_mt_dp_5Q2FNPMRRV25G4TPW68A> |
| Nextion Display 3.5" | <https://www.amazon.de/dp/B091YL88ZL/ref=cm_sw_em_r_mt_dp_R158FR0XZSWVWAKMZD62> |
| Piezo Buzzer | <https://www.amazon.de/dp/B07DPR4BTN/ref=cm_sw_em_r_mt_dp_aKzyFbJ0ZVK67> |

*The links to amazon are purely informative as a search aid*
*to be understood by well-known providers in germany*

---

## Using the firmware

Most of the functions of the firmware are self-explanatory. The addition or deletion of sensors and actors is therefore not described here.

**Main functions:**

    * Adding, editing and deleting sensors
    * Auto reconnect MQTT
    * Auto reconnect WiFi
    * Optional configure HMI touchdisplay
    * Inverted GPIO
    * Firmware and LittleFS updates via file upload
    * Firmware WebUpdate
    * Filebrowser for simple file management (e.g. backup and restore config.json)
    * DS18B20 temperature offset: easy calibration of the sensors

**Sensor calibration:**

Note: sensor calibration is an optional task. A two-point calibrtation is recommended espacially for mash tune sensors.

The firmware provides an easy option for sensor calibration. Three options availble:

    * no calibration
    
    leave deflaut value 0.0 (zero) for offset 1 and offset 2 (eg do nothing)
    
    * single point calibration or constant offset: enter a value for constant offset to offset 1 and set offset 2 to 0.0 (zero)

    The difference in a single temperture measure is your constant offset 1. There is no predeefined temperature for a single point measure.

    * two-point calibration: you need to measure at two predefined temperatures two offsets

A two point calibration provides a more accurate correction of each sensor by re-scaling it at two poinst instad of just one (constant offset). All you need is your mash tune, a calibrated temperature sensor and a cbpi4 mash profil, which includes two mash steps: at 40°C (low mashin temperature) and at 78°C (mashout temperature). Set both mash step timers to something lik 3 to 5 minutes. Fill your mash tune with water. If possible turn on agitator. Now start the two step mash profile in CraftbeerPi4. When temperatur 40°C is reached measure the temperature with a calibrated sensor. The difference between sensor value and calibrated temperatur sensor is offset 1. Repeat measurement when 78°C is reached. The difference is offset 2. In a two-point calibration it is very important to do the first measure at exactly 40°C and the second measure at exactly 78°C!

Calibration example:
Measurement
![sencal1](img/sensor_calibration2.jpg)

Sensor setting
![sencal2](img/sensor_calibration.jpg)

**Misc settings:**

1. System

    ![misc](img/misc.jpg)

    **Piezo Buzzer:**

    A piezo buzzer can only be connected to PIN D8. A piezo buzzer is optional. The firmware supports 4 different signals: ON, OFF, OK and ERROR

    **HMI display:**

    If you want to use a display check jumper settings J1 and J2 first. GPIOs D1 and D2 are used for TX and RX.

    **mDNS:**

    Multicast DNS is used to resolve hostnames to IP addresses in small networks. A mDNS name can be used instead of the IP address to open the mqttdevice web interface in a browser. Default mDNS name is mqttdevice. Open the configuration panel <http://mqttdevice.local> in your web browser. The name is freely selectable. The mDNS name must be unique in local network and must not contain any spaces or special characters. Please note: if you use two or more MQTTDevices you must change default mDNS "mqttdevice" into an unique identifier! Reboot your mqttdevice after changing mDNS.

    **NTP Zeitserver:**

    The Network Time Protocol (NTP) regularly synchronizes the time with a time server. The default time server is europe.pool.ntp.org (Berlin, Germany) and is a timer on the Internet. If there is a time server on the local network, this time server can be used. For example, if a Fritz.box (c) is in use, fritz.box can be entered as the time server. If a local NTP server is configured, the Brautomat does not need access to the Internet.

    **NTP Zeitzone:**

    The time zone is determined from a table. The time zone Europe Berlin is preset and is <CET-1CEST,M3.5.0,M10.5.0/3>.
    The time zone is used to determine the correct time, including daylightsavings. All time zones are listed in the zones.csv file.


2. MQTT Settings

    ![mqtt](img/mqtt.jpg)

    **IP address MQTT Server (CBPi):**

    On this page you have to enter IP address, Port and credentials of your MQTT broker. In most cases, this is likely to be mosquitto on the CBPi. The default port is 1883.
    Important: the firmware MQTTDevice tries constantly to establish a connection with the MQTT broker. If the MQTT broker is not available, this will severely affect the speed of the MQTT device (web interface).

3. Event manager

    The event manager handles events and misconduct. Handling of malfunctions (event handling) is deactivated in the standard setting!

    Events are
    * communication with the MQTT server is interrupted
    * Suddenly no temperature data is supplied from sensor

    Without event handling, the MQTTDevice doesn't do anything automatically. All actor states remain unchanged.

    The first thing that will takes place in a misconduct is a delay. Default delay is 120 seconds. Delays are configured for MQTT errors, sensor errors and actors. All actor states remain unchanged during the delay. While WLAN or MQTT communication is interrupted the MQTTDevice tries every 20 seconds to reconnect, regardless of any event handling setting. A successful reconnect will stop a started event delay.
    The delays are configured in misc settings in tab EventManager:

    1. Delay in MQTT errors
    2. Delay for the induction hob before a failed sensor triggers an event
    3. Delay for actors before a failed sensor triggers an event

    Standard delay is 120 seconds. After the delay, the MQTTdevice can change the status of the actors and induction hob. While actors like heating or cooling element, pumps or agitator can be switched off the induction hob can be set to a (lower) powerlevel, e.g. to 25% power to hold mash temperture while temperature sensor is in error state.
    MQTT event handling can generally be activated or deactivated. If MQTT event handling is activated, event handling must also be activated in each sensor and actor settings and for the induction hob. Each device can be configured individually.

    Every sensor also has an event handling property. If event handling is activated for a sensor, this sensor can start event handling when sensor connection is lost or sensor state changes into error. A sensor that is deactivated for event handling can not start event handling.

    An event handling example:

    * Misc settings in tab EventManager: Event handling enabled with default delay 120sec

    * Event handling temperature sensor in mash tune is enabled

    * Induction hob event handling is enabled and event powerlevel is set to 25%

    * Agitator actor event handling is disabled

    This easy example prevents the mash kettle from uncontrolled heating, when MQTT connection drops or DS18B20 sensor suddenly reports device unplugged.

---

## Touchdisplay

This firmware supports Nextion Touchdisplay HMI TFT 3.5" NX4832T035 (basic series), NX4832K035 (enhanced series) an NX4832F035 (discovery series). Three pages are availible:

Mode BrewPage: max 4 CBPi kettles overview

![BrewPage](img/Nextion1.jpg)

Mode KettlePage: current and target temperature

![KettlePage](img/Nextion2.jpg)

Attention: you must enter sensors IDs from CraftbeerPi4 in the sensor configuration page. Otherwise displayed tempertures may be wrong or mixed up between different sensors.

InductionPage: manual induction hob controller

![InductionPage](img/Nextion3.jpg)

BrewPage is usefull while brewing. When your mash process completed the use of BrewPage (overview) ends. The Kettlepage can be used any time as a kettle temperature information pannel. The induction mode can be usefull beside automated brew. Instead the InductionPage offers manual control of your induction cooker. The display can be configured via the WebIf. While display is activated, GPIO D1 (SDL) and D2 (SDA) are in use (software serial tx/rx).

Files naming:
MQTTDevice4-Display_\<displaysize\>

Please note: github repository also provide 2,8" Display files. These files are shared by MQTTDevice users. These files are untested - i do not have other displays than 3.5".

**Instructions to flash NextionsX2 touchdisplay:**

Copy the file info/mqttdevice4-\<displaytype>.tft in the root directory of your SD card. Put your SD card into the cardreader of your Nextion display and power on. The MQTTDevice display template will be flashed. When finished power off and remove SD card.

**Connect NextionsX2 touchdisplay:**

Please check the manual of your display first! Use this information on your own risk!

![disp1](img/disp1.jpg)

Nextion red cable power+ plugged into screw terminal display port Vcc

Nextion black cable power- plugged into screw terminal display port GND

Nextion blue cable TX plugged into screw terminal display port D1 (SDL)

Nextion yellow cable RX plugged into screw terminal display port D2 (SDA)

---

## MQTTDevice circuit board

**Important note:**

The circuit board was created from a hobby project. A fully assembled board is not offered. The project has no commercial intent. The information shared here represents a state of development and is used for further development as well as for checking, correcting and improving. Content from external links (e.g. hobby brewer forum) and information on external content (e.g. articles from well-known providers) are subject to the respective rights of the owner. External content is to be viewed solely as an informative start-up aid.

*All information about the board is purely informative and may be incorrect.*
*Use this information at your own risk. Any liability is excluded.*

![Platine-bestückt1](img/platine-best1.jpg) ![Platine-bestückt2](img/platine-best2.jpg)

In this project, a circuit board for the MQTT device was developed in order to offer a simple connection to sensors, actors and the induction hob GGM IDS2 with clamping screw blocks. The board is equipped with only a few components. The board offers the following advantages:

* the Wemos D1 mini is on a base and can be removed at any time.
* all GPIOs are led to screw terminals.
* A LevelShifter provides 5V control voltage to the screw terminals GPIOs (Logic Level Converter).
* The power supply from the Wemos can be used directly from the induction hob when using a GGM IDS2.
* Temperature sensors DS18B20 fixed to D3 can be connected directly to the screw terminals.
* An optional Touchdisplay HMI TFT can be connected using jumpers J1 and J2 via D1 (SDL) and D2 (SDA J2).
* PIN D4 can either be routed to the display port via jumper J3 or to D4 via the LevelShifter.
* PIN D8 is routed to D8 (3V3) without LevelShifter.
* Power supply 5V via screw terminal

**Jumper settings:**

![Jumper](img/platine_jumper.jpg)

There are 4 jumpers on the board:

1. Jumper J1: PIN D1
    1. In position 1-2, D1 is led to the display connection (SDL)
    2. In position 2-3, D1 is led to connection D1 via the LevelShifter

2. Jumper J2: PIN D2
    1. In position 1-2, D2 is led to the display connection (SDA)
    2. In position 2-3, D2 is led to connection D2 via the LevelShifter

3. Jumper J3: PIN D4
    1. In position 1-2, D4 is led to the display connection as D4, if necessary for a TFT
    2. In position 2-3, D4 is led to connection D4 via the LevelShifter

4. Jumper J4: 5V power connection GGM IDS2
    1. If the jumper is bridged, the 5V power supply from the induction hob (JST-HX socket) is used
    2. If the jumper is not set, the Wemos needs a power supply via the 5V connection
    Jumper J4 is optional. If the GGM IDS2 is not used, the jumper and connection socket can be omitted.

    *If the power supply is drawn from the induction hob (jumper J4 set), no additional voltage supply may be connected via the 5V input.*
    *GPIO0, GPIO2 and GPIO15 map the boot mode for the Wemos D1 Mini. GPIO15 is not connected via the LevelShifter and must be set to low for Flash Boot Mode. GPIO0 and GPIO2 are on high during flash boot*

* Layout circuit board:**

![Platine](img/platine.jpg)

In the Info folder there is an EasyEDA file that can be used to create the circuit board. STL files for a 3D print MQTTDevice housing are also located in the Info folder.
Please share corrections, improvements and further developments.

**Circuit board parts list:**

The following components are required:

| Anzahl | Artikel | ArtikelNr |
| ------ | ------- | --------- |
| 1 | screw terminal block 2pol 2,54 | (eg voelkner S84366) |
| 3 | screw terminal block 3pol 2,54 | (eg voelkner S84893) |
| 2 | screw terminal block 5pol 2,54 | (eg voelkner S84806) |
| 2 | screw terminal block 8pol 2,54 | (eg voelkner S84611) |
| 1 | JST-HX connector 90° 2,54 | (eg voelkner D17526) |
| 1 | Header single row 2,54 | (eg voelkner D19990) |
| 4 | Jumper 2,54 | (eg voelkner S655251) |
| 1 | Resistor 4,7kOhm | (eg voelkner S620751) |
| 1 | D1 mini NodeMcu ESP8266 | [amazon](<https://amzn.to/3RWwyL5>) |
| 1 | LevelShifter 8 Channel 5V 3.3V | [amazon](<https://amzn.to/3xjkN7S>) |

*amazon and voelkner are purely informative as a search aid*
*links are amazon partner and voelkner tradetracker*

![LevelShifter](img/platine_levelshifter.jpg)

When selecting LevelShifter (Logic Level Converter), the assignment must be observed! The LevelShifter must have this order in the Low Voltage (LV) input:

`LV1 - LV2 - LV3 - LV4 - LV (3V3) - Ground - LV5 - LV6 - LV7 - LV8`

**Notes on construction:**

The resistor R 4.7kOhm for the temperature sensors DS18B20 is placed under the Wemos D1 mini. Therefore the Wemos has to be socketed. The sockets also offer the advantage that the Wemos can be removed from the circuit board at any time, e.g. for flashing or testing. The DS18B20 are supplied with 5V at VCC. This ensures a stable supply even with longer supply lines. The resistance is from Data (PIN D3) to 3V3. The JST-HX socket and the J4 jumper for the induction hob are optional.

## Connecting MQTTDevice to induction GGM IDS2

*The following description deletes the guarantee claims for the induction hob*
*Use this manual at your own risk*

The GGM IDS2 induction hob can be connected to the circuit board. The GGM IDS2 is supplied with an external control unit. When the control panel is opened, the cable connection from the control panel to the induction hob can be removed. All you have to do is pull the cable out of the socket in the control panel.
The same socket (JST-HX) is located on the MQTTDevice board. This makes the electrical connection very easy.

The connections must be configured via the web interface:

* PIN White (relay) connected to PIN D7 (Interrupt!)
* PIN Yellow (Command Channel) connected to pin D6
* PIN Blue (back channel) connected to pin D5

A separate power supply is not required for the MQTT device when using the GGM IDS2. GPIOs D5, D6 and D7 are strongly recommended!

## Connecting sensors DS18B20

Temperature sensors of type DS18B20 with 3 connection cables (data, VCC and GND) are supported. Temperature sensors are tied to D3. The necessary resistance 4k7 to 3V3 is provided on the board. The voltage supply for the temperature sensors is connected to 5V.

## Connecting relais boards

In addition to a GPIO, relay boards require a 5V power supply. 5V can be tapped at one of the three connections for the temperature sensors DS18B20 at VCC and GND.

## Technical information

Wemos D1 min GPIOs

  0   D3                                      OK        OnewWire Dallas temp

  1   TX        HIGH at boot  TX pin          (ok)

  2   D4        HIGH at boot                  OK        Agitator, onboard LED

  3   RX        HIGH at boot  (ok)            RX pin

  4   D2        SCA           OK              OK        Display, the most safe GPIO to operate with relays

  5   D1        SCL           OK              OK        Display, the most safe GPIO to operate with relays

  6   -                                                 GPIO6 to GPIO11 connected to flash (not useable)

  7   -

  8   -

  9   -

  10  -

  11  -

  12  D6        SPI MISO      ok              ok        GGM IDS2 Commandchannel(yellow)

  13  D7        SPI MOSI      ok              ok        GGM IDS2 Relais(white)
  
  14  D5        SPI SCLK      ok              ok        GGM IDS2 Backchannel(blue)

  15  D8        SPI CS        -               (ok)      Buzzer

  16  D0        HIGH at boot  No interrupt    No PWM    Pump
  
---

## Case 3D print

![case2](img/case2.png)
![base](img/base.png)

3D print files are located in the info folder. With the current housing design, the circuit board and a 3.5" Nextion touchdisplay are glued into the housing. Holes for housing, base and circuit board are made with a diameter for M3 screws. M3x10mm screws fit for display, circuit board and XLR connectors. M3x16 screws match for groundplate to case holes and pads. You need to cut M3 threads into the six pads. Both objects are build parameterized with FreeCAD. Improvments are welcome!

A friendly user from german hobbybrauerforum send me STL and HMI files for 2,8" displays. thx!
Files nameing:

MQTTDevice4-Base_\<displaysize\>
MQTTDevice4-Case_\<displaysize\>

A mash kettle example:

![case3](img/case3.jpg)

GPIOs D5, D6 and D7 are used by induction hob GGM IDS2. A buzzer is connected to GPIO D8. GPIO D1 and D2 are bound to display. GPIO D4 is used to handle an agitator. The yellow knob is a PWM modul. The connector below shows the cable to the agitator. This MQTTDevice is connected with a 6 cores cable: two lines for 5V power MQTTDevice. Another two lines are used for 24V agitator power, conected to the PWM modul IN. And the last two lines ares used by GPIO D4 + GND and connected to a SSR to switch on/off the agitator power adapter (24V). The PWM modul OUT is connected to the agitator. Both power adapters (MQTTDevice and agitator) are installed on a DIN rail in a central swtich cabinet.

---

## NTP time zones table

|Africa/Abidjan                |GMT0                                        |
|------------------------------|--------------------------------------------|
|Africa/Accra                  |GMT0                                        |
|Africa/Addis_Ababa            |EAT-3                                       |
|Africa/Algiers                |CET-1                                       |
|Africa/Asmara                 |EAT-3                                       |
|Africa/Bamako                 |GMT0                                        |
|Africa/Bangui                 |WAT-1                                       |
|Africa/Banjul                 |GMT0                                        |
|Africa/Bissau                 |GMT0                                        |
|Africa/Blantyre               |CAT-2                                       |
|Africa/Brazzaville            |WAT-1                                       |
|Africa/Bujumbura              |CAT-2                                       |
|Africa/Cairo                  |EET-2EEST,M4.5.5/0,M10.5.4/24               |
|Africa/Casablanca             |<+01>-1                                     |
|Africa/Ceuta                  |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Africa/Conakry                |GMT0                                        |
|Africa/Dakar                  |GMT0                                        |
|Africa/Dar_es_Salaam          |EAT-3                                       |
|Africa/Djibouti               |EAT-3                                       |
|Africa/Douala                 |WAT-1                                       |
|Africa/El_Aaiun               |<+01>-1                                     |
|Africa/Freetown               |GMT0                                        |
|Africa/Gaborone               |CAT-2                                       |
|Africa/Harare                 |CAT-2                                       |
|Africa/Johannesburg           |SAST-2                                      |
|Africa/Juba                   |CAT-2                                       |
|Africa/Kampala                |EAT-3                                       |
|Africa/Khartoum               |CAT-2                                       |
|Africa/Kigali                 |CAT-2                                       |
|Africa/Kinshasa               |WAT-1                                       |
|Africa/Lagos                  |WAT-1                                       |
|Africa/Libreville             |WAT-1                                       |
|Africa/Lome                   |GMT0                                        |
|Africa/Luanda                 |WAT-1                                       |
|Africa/Lubumbashi             |CAT-2                                       |
|Africa/Lusaka                 |CAT-2                                       |
|Africa/Malabo                 |WAT-1                                       |
|Africa/Maputo                 |CAT-2                                       |
|Africa/Maseru                 |SAST-2                                      |
|Africa/Mbabane                |SAST-2                                      |
|Africa/Mogadishu              |EAT-3                                       |
|Africa/Monrovia               |GMT0                                        |
|Africa/Nairobi                |EAT-3                                       |
|Africa/Ndjamena               |WAT-1                                       |
|Africa/Niamey                 |WAT-1                                       |
|Africa/Nouakchott             |GMT0                                        |
|Africa/Ouagadougou            |GMT0                                        |
|Africa/Porto-Novo             |WAT-1                                       |
|Africa/Sao_Tome               |GMT0                                        |
|Africa/Tripoli                |EET-2                                       |
|Africa/Tunis                  |CET-1                                       |
|Africa/Windhoek               |CAT-2                                       |
|America/Adak                  |HST10HDT,M3.2.0,M11.1.0                     |
|America/Anchorage             |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/Anguilla              |AST4                                        |
|America/Antigua               |AST4                                        |
|America/Araguaina             |<-03>3                                      |
|America/Argentina/Buenos_Aires|<-03>3                                      |
|America/Argentina/Catamarca   |<-03>3                                      |
|America/Argentina/Cordoba     |<-03>3                                      |
|America/Argentina/Jujuy       |<-03>3                                      |
|America/Argentina/La_Rioja    |<-03>3                                      |
|America/Argentina/Mendoza     |<-03>3                                      |
|America/Argentina/Rio_Gallegos|<-03>3                                      |
|America/Argentina/Salta       |<-03>3                                      |
|America/Argentina/San_Juan    |<-03>3                                      |
|America/Argentina/San_Luis    |<-03>3                                      |
|America/Argentina/Tucuman     |<-03>3                                      |
|America/Argentina/Ushuaia     |<-03>3                                      |
|America/Aruba                 |AST4                                        |
|America/Asuncion              |<-04>4<-03>,M10.1.0/0,M3.4.0/0              |
|America/Atikokan              |EST5                                        |
|America/Bahia                 |<-03>3                                      |
|America/Bahia_Banderas        |CST6                                        |
|America/Barbados              |AST4                                        |
|America/Belem                 |<-03>3                                      |
|America/Belize                |CST6                                        |
|America/Blanc-Sablon          |AST4                                        |
|America/Boa_Vista             |<-04>4                                      |
|America/Bogota                |<-05>5                                      |
|America/Boise                 |MST7MDT,M3.2.0,M11.1.0                      |
|America/Cambridge_Bay         |MST7MDT,M3.2.0,M11.1.0                      |
|America/Campo_Grande          |<-04>4                                      |
|America/Cancun                |EST5                                        |
|America/Caracas               |<-04>4                                      |
|America/Cayenne               |<-03>3                                      |
|America/Cayman                |EST5                                        |
|America/Chicago               |CST6CDT,M3.2.0,M11.1.0                      |
|America/Chihuahua             |CST6                                        |
|America/Costa_Rica            |CST6                                        |
|America/Creston               |MST7                                        |
|America/Cuiaba                |<-04>4                                      |
|America/Curacao               |AST4                                        |
|America/Danmarkshavn          |GMT0                                        |
|America/Dawson                |MST7                                        |
|America/Dawson_Creek          |MST7                                        |
|America/Denver                |MST7MDT,M3.2.0,M11.1.0                      |
|America/Detroit               |EST5EDT,M3.2.0,M11.1.0                      |
|America/Dominica              |AST4                                        |
|America/Edmonton              |MST7MDT,M3.2.0,M11.1.0                      |
|America/Eirunepe              |<-05>5                                      |
|America/El_Salvador           |CST6                                        |
|America/Fortaleza             |<-03>3                                      |
|America/Fort_Nelson           |MST7                                        |
|America/Glace_Bay             |AST4ADT,M3.2.0,M11.1.0                      |
|America/Godthab               |<-02>2<-01>,M3.5.0/-1,M10.5.0/0             |
|America/Goose_Bay             |AST4ADT,M3.2.0,M11.1.0                      |
|America/Grand_Turk            |EST5EDT,M3.2.0,M11.1.0                      |
|America/Grenada               |AST4                                        |
|America/Guadeloupe            |AST4                                        |
|America/Guatemala             |CST6                                        |
|America/Guayaquil             |<-05>5                                      |
|America/Guyana                |<-04>4                                      |
|America/Halifax               |AST4ADT,M3.2.0,M11.1.0                      |
|America/Havana                |CST5CDT,M3.2.0/0,M11.1.0/1                  |
|America/Hermosillo            |MST7                                        |
|America/Indiana/Indianapolis  |EST5EDT,M3.2.0,M11.1.0                      |
|America/Indiana/Knox          |CST6CDT,M3.2.0,M11.1.0                      |
|America/Indiana/Marengo       |EST5EDT,M3.2.0,M11.1.0                      |
|America/Indiana/Petersburg    |EST5EDT,M3.2.0,M11.1.0                      |
|America/Indiana/Tell_City     |CST6CDT,M3.2.0,M11.1.0                      |
|America/Indiana/Vevay         |EST5EDT,M3.2.0,M11.1.0                      |
|America/Indiana/Vincennes     |EST5EDT,M3.2.0,M11.1.0                      |
|America/Indiana/Winamac       |EST5EDT,M3.2.0,M11.1.0                      |
|America/Inuvik                |MST7MDT,M3.2.0,M11.1.0                      |
|America/Iqaluit               |EST5EDT,M3.2.0,M11.1.0                      |
|America/Jamaica               |EST5                                        |
|America/Juneau                |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/Kentucky/Louisville   |EST5EDT,M3.2.0,M11.1.0                      |
|America/Kentucky/Monticello   |EST5EDT,M3.2.0,M11.1.0                      |
|America/Kralendijk            |AST4                                        |
|America/La_Paz                |<-04>4                                      |
|America/Lima                  |<-05>5                                      |
|America/Los_Angeles           |PST8PDT,M3.2.0,M11.1.0                      |
|America/Lower_Princes         |AST4                                        |
|America/Maceio                |<-03>3                                      |
|America/Managua               |CST6                                        |
|America/Manaus                |<-04>4                                      |
|America/Marigot               |AST4                                        |
|America/Martinique            |AST4                                        |
|America/Matamoros             |CST6CDT,M3.2.0,M11.1.0                      |
|America/Mazatlan              |MST7                                        |
|America/Menominee             |CST6CDT,M3.2.0,M11.1.0                      |
|America/Merida                |CST6                                        |
|America/Metlakatla            |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/Mexico_City           |CST6                                        |
|America/Miquelon              |<-03>3<-02>,M3.2.0,M11.1.0                  |
|America/Moncton               |AST4ADT,M3.2.0,M11.1.0                      |
|America/Monterrey             |CST6                                        |
|America/Montevideo            |<-03>3                                      |
|America/Montreal              |EST5EDT,M3.2.0,M11.1.0                      |
|America/Montserrat            |AST4                                        |
|America/Nassau                |EST5EDT,M3.2.0,M11.1.0                      |
|America/New_York              |EST5EDT,M3.2.0,M11.1.0                      |
|America/Nipigon               |EST5EDT,M3.2.0,M11.1.0                      |
|America/Nome                  |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/Noronha               |<-02>2                                      |
|America/North_Dakota/Beulah   |CST6CDT,M3.2.0,M11.1.0                      |
|America/North_Dakota/Center   |CST6CDT,M3.2.0,M11.1.0                      |
|America/North_Dakota/New_Salem|CST6CDT,M3.2.0,M11.1.0                      |
|America/Nuuk                  |<-02>2<-01>,M3.5.0/-1,M10.5.0/0             |
|America/Ojinaga               |CST6CDT,M3.2.0,M11.1.0                      |
|America/Panama                |EST5                                        |
|America/Pangnirtung           |EST5EDT,M3.2.0,M11.1.0                      |
|America/Paramaribo            |<-03>3                                      |
|America/Phoenix               |MST7                                        |
|America/Port-au-Prince        |EST5EDT,M3.2.0,M11.1.0                      |
|America/Port_of_Spain         |AST4                                        |
|America/Porto_Velho           |<-04>4                                      |
|America/Puerto_Rico           |AST4                                        |
|America/Punta_Arenas          |<-03>3                                      |
|America/Rainy_River           |CST6CDT,M3.2.0,M11.1.0                      |
|America/Rankin_Inlet          |CST6CDT,M3.2.0,M11.1.0                      |
|America/Recife                |<-03>3                                      |
|America/Regina                |CST6                                        |
|America/Resolute              |CST6CDT,M3.2.0,M11.1.0                      |
|America/Rio_Branco            |<-05>5                                      |
|America/Santarem              |<-03>3                                      |
|America/Santiago              |<-04>4<-03>,M9.1.6/24,M4.1.6/24             |
|America/Santo_Domingo         |AST4                                        |
|America/Sao_Paulo             |<-03>3                                      |
|America/Scoresbysund          |<-02>2<-01>,M3.5.0/-1,M10.5.0/0             |
|America/Sitka                 |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/St_Barthelemy         |AST4                                        |
|America/St_Johns              |NST3:30NDT,M3.2.0,M11.1.0                   |
|America/St_Kitts              |AST4                                        |
|America/St_Lucia              |AST4                                        |
|America/St_Thomas             |AST4                                        |
|America/St_Vincent            |AST4                                        |
|America/Swift_Current         |CST6                                        |
|America/Tegucigalpa           |CST6                                        |
|America/Thule                 |AST4ADT,M3.2.0,M11.1.0                      |
|America/Thunder_Bay           |EST5EDT,M3.2.0,M11.1.0                      |
|America/Tijuana               |PST8PDT,M3.2.0,M11.1.0                      |
|America/Toronto               |EST5EDT,M3.2.0,M11.1.0                      |
|America/Tortola               |AST4                                        |
|America/Vancouver             |PST8PDT,M3.2.0,M11.1.0                      |
|America/Whitehorse            |MST7                                        |
|America/Winnipeg              |CST6CDT,M3.2.0,M11.1.0                      |
|America/Yakutat               |AKST9AKDT,M3.2.0,M11.1.0                    |
|America/Yellowknife           |MST7MDT,M3.2.0,M11.1.0                      |
|Antarctica/Casey              |<+08>-8                                     |
|Antarctica/Davis              |<+07>-7                                     |
|Antarctica/DumontDUrville     |<+10>-10                                    |
|Antarctica/Macquarie          |AEST-10AEDT,M10.1.0,M4.1.0/3                |
|Antarctica/Mawson             |<+05>-5                                     |
|Antarctica/McMurdo            |NZST-12NZDT,M9.5.0,M4.1.0/3                 |
|Antarctica/Palmer             |<-03>3                                      |
|Antarctica/Rothera            |<-03>3                                      |
|Antarctica/Syowa              |<+03>-3                                     |
|Antarctica/Troll              |<+00>0<+02>-2,M3.5.0/1,M10.5.0/3            |
|Antarctica/Vostok             |<+05>-5                                     |
|Arctic/Longyearbyen           |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Asia/Aden                     |<+03>-3                                     |
|Asia/Almaty                   |<+05>-5                                     |
|Asia/Amman                    |<+03>-3                                     |
|Asia/Anadyr                   |<+12>-12                                    |
|Asia/Aqtau                    |<+05>-5                                     |
|Asia/Aqtobe                   |<+05>-5                                     |
|Asia/Ashgabat                 |<+05>-5                                     |
|Asia/Atyrau                   |<+05>-5                                     |
|Asia/Baghdad                  |<+03>-3                                     |
|Asia/Bahrain                  |<+03>-3                                     |
|Asia/Baku                     |<+04>-4                                     |
|Asia/Bangkok                  |<+07>-7                                     |
|Asia/Barnaul                  |<+07>-7                                     |
|Asia/Beirut                   |EET-2EEST,M3.5.0/0,M10.5.0/0                |
|Asia/Bishkek                  |<+06>-6                                     |
|Asia/Brunei                   |<+08>-8                                     |
|Asia/Chita                    |<+09>-9                                     |
|Asia/Choibalsan               |<+08>-8                                     |
|Asia/Colombo                  |<+0530>-5:30                                |
|Asia/Damascus                 |<+03>-3                                     |
|Asia/Dhaka                    |<+06>-6                                     |
|Asia/Dili                     |<+09>-9                                     |
|Asia/Dubai                    |<+04>-4                                     |
|Asia/Dushanbe                 |<+05>-5                                     |
|Asia/Famagusta                |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Asia/Gaza                     |EET-2EEST,M3.4.4/50,M10.4.4/50              |
|Asia/Hebron                   |EET-2EEST,M3.4.4/50,M10.4.4/50              |
|Asia/Ho_Chi_Minh              |<+07>-7                                     |
|Asia/Hong_Kong                |HKT-8                                       |
|Asia/Hovd                     |<+07>-7                                     |
|Asia/Irkutsk                  |<+08>-8                                     |
|Asia/Jakarta                  |WIB-7                                       |
|Asia/Jayapura                 |WIT-9                                       |
|Asia/Jerusalem                |IST-2IDT,M3.4.4/26,M10.5.0                  |
|Asia/Kabul                    |<+0430>-4:30                                |
|Asia/Kamchatka                |<+12>-12                                    |
|Asia/Karachi                  |PKT-5                                       |
|Asia/Kathmandu                |<+0545>-5:45                                |
|Asia/Khandyga                 |<+09>-9                                     |
|Asia/Kolkata                  |IST-5:30                                    |
|Asia/Krasnoyarsk              |<+07>-7                                     |
|Asia/Kuala_Lumpur             |<+08>-8                                     |
|Asia/Kuching                  |<+08>-8                                     |
|Asia/Kuwait                   |<+03>-3                                     |
|Asia/Macau                    |CST-8                                       |
|Asia/Magadan                  |<+11>-11                                    |
|Asia/Makassar                 |WITA-8                                      |
|Asia/Manila                   |PST-8                                       |
|Asia/Muscat                   |<+04>-4                                     |
|Asia/Nicosia                  |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Asia/Novokuznetsk             |<+07>-7                                     |
|Asia/Novosibirsk              |<+07>-7                                     |
|Asia/Omsk                     |<+06>-6                                     |
|Asia/Oral                     |<+05>-5                                     |
|Asia/Phnom_Penh               |<+07>-7                                     |
|Asia/Pontianak                |WIB-7                                       |
|Asia/Pyongyang                |KST-9                                       |
|Asia/Qatar                    |<+03>-3                                     |
|Asia/Qyzylorda                |<+05>-5                                     |
|Asia/Riyadh                   |<+03>-3                                     |
|Asia/Sakhalin                 |<+11>-11                                    |
|Asia/Samarkand                |<+05>-5                                     |
|Asia/Seoul                    |KST-9                                       |
|Asia/Shanghai                 |CST-8                                       |
|Asia/Singapore                |<+08>-8                                     |
|Asia/Srednekolymsk            |<+11>-11                                    |
|Asia/Taipei                   |CST-8                                       |
|Asia/Tashkent                 |<+05>-5                                     |
|Asia/Tbilisi                  |<+04>-4                                     |
|Asia/Tehran                   |<+0330>-3:30                                |
|Asia/Thimphu                  |<+06>-6                                     |
|Asia/Tokyo                    |JST-9                                       |
|Asia/Tomsk                    |<+07>-7                                     |
|Asia/Ulaanbaatar              |<+08>-8                                     |
|Asia/Urumqi                   |<+06>-6                                     |
|Asia/Ust-Nera                 |<+10>-10                                    |
|Asia/Vientiane                |<+07>-7                                     |
|Asia/Vladivostok              |<+10>-10                                    |
|Asia/Yakutsk                  |<+09>-9                                     |
|Asia/Yangon                   |<+0630>-6:30                                |
|Asia/Yekaterinburg            |<+05>-5                                     |
|Asia/Yerevan                  |<+04>-4                                     |
|Atlantic/Azores               |<-01>1<+00>,M3.5.0/0,M10.5.0/1              |
|Atlantic/Bermuda              |AST4ADT,M3.2.0,M11.1.0                      |
|Atlantic/Canary               |WET0WEST,M3.5.0/1,M10.5.0                   |
|Atlantic/Cape_Verde           |<-01>1                                      |
|Atlantic/Faroe                |WET0WEST,M3.5.0/1,M10.5.0                   |
|Atlantic/Madeira              |WET0WEST,M3.5.0/1,M10.5.0                   |
|Atlantic/Reykjavik            |GMT0                                        |
|Atlantic/South_Georgia        |<-02>2                                      |
|Atlantic/Stanley              |<-03>3                                      |
|Atlantic/St_Helena            |GMT0                                        |
|Australia/Adelaide            |ACST-9:30ACDT,M10.1.0,M4.1.0/3              |
|Australia/Brisbane            |AEST-10                                     |
|Australia/Broken_Hill         |ACST-9:30ACDT,M10.1.0,M4.1.0/3              |
|Australia/Currie              |AEST-10AEDT,M10.1.0,M4.1.0/3                |
|Australia/Darwin              |ACST-9:30                                   |
|Australia/Eucla               |<+0845>-8:45                                |
|Australia/Hobart              |AEST-10AEDT,M10.1.0,M4.1.0/3                |
|Australia/Lindeman            |AEST-10                                     |
|Australia/Lord_Howe           |<+1030>-10:30<+11>-11,M10.1.0,M4.1.0        |
|Australia/Melbourne           |AEST-10AEDT,M10.1.0,M4.1.0/3                |
|Australia/Perth               |AWST-8                                      |
|Australia/Sydney              |AEST-10AEDT,M10.1.0,M4.1.0/3                |
|Europe/Amsterdam              |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Andorra                |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Astrakhan              |<+04>-4                                     |
|Europe/Athens                 |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Belgrade               |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Berlin                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Bratislava             |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Brussels               |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Bucharest              |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Budapest               |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Busingen               |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Chisinau               |EET-2EEST,M3.5.0,M10.5.0/3                  |
|Europe/Copenhagen             |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Dublin                 |IST-1GMT0,M10.5.0,M3.5.0/1                  |
|Europe/Gibraltar              |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Guernsey               |GMT0BST,M3.5.0/1,M10.5.0                    |
|Europe/Helsinki               |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Isle_of_Man            |GMT0BST,M3.5.0/1,M10.5.0                    |
|Europe/Istanbul               |<+03>-3                                     |
|Europe/Jersey                 |GMT0BST,M3.5.0/1,M10.5.0                    |
|Europe/Kaliningrad            |EET-2                                       |
|Europe/Kiev                   |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Kirov                  |MSK-3                                       |
|Europe/Lisbon                 |WET0WEST,M3.5.0/1,M10.5.0                   |
|Europe/Ljubljana              |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/London                 |GMT0BST,M3.5.0/1,M10.5.0                    |
|Europe/Luxembourg             |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Madrid                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Malta                  |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Mariehamn              |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Minsk                  |<+03>-3                                     |
|Europe/Monaco                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Moscow                 |MSK-3                                       |
|Europe/Oslo                   |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Paris                  |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Podgorica              |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Prague                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Riga                   |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Rome                   |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Samara                 |<+04>-4                                     |
|Europe/San_Marino             |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Sarajevo               |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Saratov                |<+04>-4                                     |
|Europe/Simferopol             |MSK-3                                       |
|Europe/Skopje                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Sofia                  |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Stockholm              |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Tallinn                |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Tirane                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Ulyanovsk              |<+04>-4                                     |
|Europe/Uzhgorod               |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Vaduz                  |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Vatican                |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Vienna                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Vilnius                |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Volgograd              |MSK-3                                       |
|Europe/Warsaw                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Zagreb                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Europe/Zaporozhye             |EET-2EEST,M3.5.0/3,M10.5.0/4                |
|Europe/Zurich                 |CET-1CEST,M3.5.0,M10.5.0/3                  |
|Indian/Antananarivo           |EAT-3                                       |
|Indian/Chagos                 |<+06>-6                                     |
|Indian/Christmas              |<+07>-7                                     |
|Indian/Cocos                  |<+0630>-6:30                                |
|Indian/Comoro                 |EAT-3                                       |
|Indian/Kerguelen              |<+05>-5                                     |
|Indian/Mahe                   |<+04>-4                                     |
|Indian/Maldives               |<+05>-5                                     |
|Indian/Mauritius              |<+04>-4                                     |
|Indian/Mayotte                |EAT-3                                       |
|Indian/Reunion                |<+04>-4                                     |
|Pacific/Apia                  |<+13>-13                                    |
|Pacific/Auckland              |NZST-12NZDT,M9.5.0,M4.1.0/3                 |
|Pacific/Bougainville          |<+11>-11                                    |
|Pacific/Chatham               |<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45|
|Pacific/Chuuk                 |<+10>-10                                    |
|Pacific/Easter                |<-06>6<-05>,M9.1.6/22,M4.1.6/22             |
|Pacific/Efate                 |<+11>-11                                    |
|Pacific/Enderbury             |<+13>-13                                    |
|Pacific/Fakaofo               |<+13>-13                                    |
|Pacific/Fiji                  |<+12>-12                                    |
|Pacific/Funafuti              |<+12>-12                                    |
|Pacific/Galapagos             |<-06>6                                      |
|Pacific/Gambier               |<-09>9                                      |
|Pacific/Guadalcanal           |<+11>-11                                    |
|Pacific/Guam                  |ChST-10                                     |
|Pacific/Honolulu              |HST10                                       |
|Pacific/Kiritimati            |<+14>-14                                    |
|Pacific/Kosrae                |<+11>-11                                    |
|Pacific/Kwajalein             |<+12>-12                                    |
|Pacific/Majuro                |<+12>-12                                    |
|Pacific/Marquesas             |<-0930>9:30                                 |
|Pacific/Midway                |SST11                                       |
|Pacific/Nauru                 |<+12>-12                                    |
|Pacific/Niue                  |<-11>11                                     |
|Pacific/Norfolk               |<+11>-11<+12>,M10.1.0,M4.1.0/3              |
|Pacific/Noumea                |<+11>-11                                    |
|Pacific/Pago_Pago             |SST11                                       |
|Pacific/Palau                 |<+09>-9                                     |
|Pacific/Pitcairn              |<-08>8                                      |
|Pacific/Pohnpei               |<+11>-11                                    |
|Pacific/Port_Moresby          |<+10>-10                                    |
|Pacific/Rarotonga             |<-10>10                                     |
|Pacific/Saipan                |ChST-10                                     |
|Pacific/Tahiti                |<-10>10                                     |
|Pacific/Tarawa                |<+12>-12                                    |
|Pacific/Tongatapu             |<+13>-13                                    |
|Pacific/Wake                  |<+12>-12                                    |
|Pacific/Wallis                |<+12>-12                                    |
|Etc/GMT                       |GMT0                                        |
|Etc/GMT-0                     |GMT0                                        |
|Etc/GMT-1                     |<+01>-1                                     |
|Etc/GMT-2                     |<+02>-2                                     |
|Etc/GMT-3                     |<+03>-3                                     |
|Etc/GMT-4                     |<+04>-4                                     |
|Etc/GMT-5                     |<+05>-5                                     |
|Etc/GMT-6                     |<+06>-6                                     |
|Etc/GMT-7                     |<+07>-7                                     |
|Etc/GMT-8                     |<+08>-8                                     |
|Etc/GMT-9                     |<+09>-9                                     |
|Etc/GMT-10                    |<+10>-10                                    |
|Etc/GMT-11                    |<+11>-11                                    |
|Etc/GMT-12                    |<+12>-12                                    |
|Etc/GMT-13                    |<+13>-13                                    |
|Etc/GMT-14                    |<+14>-14                                    |
|Etc/GMT0                      |GMT0                                        |
|Etc/GMT+0                     |GMT0                                        |
|Etc/GMT+1                     |<-01>1                                      |
|Etc/GMT+2                     |<-02>2                                      |
|Etc/GMT+3                     |<-03>3                                      |
|Etc/GMT+4                     |<-04>4                                      |
|Etc/GMT+5                     |<-05>5                                      |
|Etc/GMT+6                     |<-06>6                                      |
|Etc/GMT+7                     |<-07>7                                      |
|Etc/GMT+8                     |<-08>8                                      |
|Etc/GMT+9                     |<-09>9                                      |
|Etc/GMT+10                    |<-10>10                                     |
|Etc/GMT+11                    |<-11>11                                     |
|Etc/GMT+12                    |<-12>12                                     |
|Etc/UCT                       |UTC0                                        |
|Etc/UTC                       |UTC0                                        |
|Etc/Greenwich                 |GMT0                                        |
|Etc/Universal                 |UTC0                                        |
|Etc/Zulu                      |UTC0                                        |
