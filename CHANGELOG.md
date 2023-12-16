# Changelog

Version 4.58b

- Changed:    Default WLAN name in AP Mode changed to MQTTDevice
- Changed:    Reset to factory default (clear config and WLAN settings enabled)
- Fix:        max number of sensors allowed
- Fix:        dyn. increase sensor update intervall fitting number of configured sensors
- Fix:        Reload WebIf after reboot failed
- Fix:        ESP32 mDNS webservice
- Fix:        close modal windows after config restore and reboot
- Changed:    reboot function
- Changed:    Merged source ESP8266 in ESP32 using compiler directives
- Changed:    Rebuild WebIf
- Changed:    Restore config file
- Optimzed:   reduce amount of data transfered to connected clients
- New:        language switch (en, de)
- New:        Mouseover tooltips
- Changed:    select handle sensor type and sensor pins
- Changed:    IDS Pin Interrupt
- Fix:        long sensor/actor names
- Fix:        long mqtt topics
- Fix:        HMI display mDNS name
- Changed:    Added ".local" to mDNS hints, views and docs
- New:        Added Max31865 Amplififer SPI support (PT100/PT1000 support) MOSI: D0 MISO: D1 SCLK: D2 CS: D4/D5/D6
- New:        Added PT100/1000 sensor support
- Changed:    max number of sensors 3
- Fix:        Remove last sensor/actor did not refresh WebIf table

Version 4.56

- Changed:    request sensor intervall (1000ms)
- Fix:        Boot order driver Display
- Fix:        Boot order driver DS18B20
- Fix:        Event on MQTT error
- Fix:        remove debug output
- Fix:        Display flicker BrewPage actual temperature when CBPi4 ID not set
- Fix:        Refactor Web Server parsing-impl.h #9005
- added:      WiFi core events
- Changed:    WebUpdate
- Fix:        File explorer
- Removed:    PCF8574 support (I2C)
- Removed:    TickerNTP (save mem)
- Removed:    TickerWLAN (save mem)
- Changed:    sensor class (getter/setter)
- Changed:    actor class (getter/setter)
- Changed:    induction class (getter/setter)

Version 4.55a

- Changed:    WebUpdate default SSL
- Changed:    WebUpdate Log file (webUpdateLog.txt)
- Changed:    WebUpdate check received file size before overwriting files in flash
- Fix:        github changed MFLN - WebUpdate received 0 byte data
- Fix:        WebUpdate
- renew:      certificates SSL

Version 4.54

- Added:      SSE auto reconnect
- Added:      SSE checkAlive mobiles (tested on iOS only)
- Fix:        Status icon SSE change background color red/green on disconnect/reconnect
- Change:     InnuFramework
- Fix:        Arduino ESP8266 core: No poison after block with current git version (Issue #8952)

Version 4.53 Release

- Changed:    InnuFramework
- Fix:        Reboot reload Webif
- Merge:      dev to main

Version 4.52

- Fixed:      Arduino ESP8266 core: Fix for occasional timeout issues #8944 (net error timeout - WebIf Fehler)
- Fixed:      Arduino ESP8266 core: Fix for dangerous relocation: j: cannot encode #8925
- Changed:    Arduino ESP8266 core: Add support WiFiClientSecure TCP KeepAlive #8940
- Added:      SSE keepalive
- Changed:    Cache control WebIf

Version 4.51

- Fix:        Arduino ESP8266 Fix for occasional timeout issues #8944 (net error timeout - WebIf)
- Changed:    InnuFramework mimeTypes und mimeTables

Version 4.50 SSE

- New:        Server Sent Events SSE
- Added:      SSE sensors, actors, IDS and misc
- Optimize:   JS http polling (SSE)
- Added:      Column SSE badge
- Changed:    PCF display mDNS badges in disabled state
- Update:     JQuery 3.7.0

Version 4.49 - dev & test version\
Version 4.48  - development version

Version 4.47

- Optimize:   CSS/JS load, WebIf performance
- Fixed:      cores ESP8266WebServer
- Fixed:      cores ESPSoftserial
- Changed:    refresh time WebIf

Version 4.46

- Changed:    faster Nextion display

Version 4.45

- Changed:    Sensor handling (isConnected)
- Update:     SoftSerial 8.0.2 (Nextion)

Version 4.44  maintenance update

- Update:     ESP8266 Arduino 3.1.2
- Update:     VSCode 1.76 Arduino 0.6
- Update:     ArduinoJSON 6.21
- Update:     SoftwareSerial 8.0.1
- Optimize:   Framework

Version 4.41

- Update:   Prepare for new release 4.41
- Changed:  File handling (removed filenames)
- Removed:  Debug code
- Update:   Arduino for VSCode 0.5
- Optimize: Bootstrap CSS

Version 4.40e

- Update:   ArduinoJSON lib 6.20.1
- Remove:   debug code
- Fixed:    JSON response misc

Version 4.40d

- Fixed:    temperature digits in CBPi4
- Fixed:    Typo save config IDS2

Version 4.40c

- Update:   ESP8266 cores
- Update:   Arduino for Visual Code 0.4.13
- Fixed:    Ticker lib

- Update:   ESP8266 Arduino 3.1.1
- Changed:  Check value range
- Changed:  Ticker sensors, actors and induction changed from 2500ms to 2000ms

Version 4.40a

- Fixed:    hex sensor address (set resolution)
- Fixed:    MQTT reconnect
- Fixed:    MQTT connection state
- Fixed:    IP address length
- Removed:  mode brew without CBPi (mqttoff)
- Update:   ESP8266 Arduino 3.1.0
- Optimzed: WebIf for Mobiles
- Added:    Toast messages
- Fixed:    AutoTune restart reset values
- Changed:  Auto start WebUpdate tools after WebUpdate firmware
- Added:    Backup and restore configuration
- Changed:  Recipe import css style
- Fixed:    Update buttons
- Resize:   Chart
- Changed:  PCF 0.3.7
- Fixed:    WebUpdate exception OOM
- Added:    Treshold and new power out (IDS2)
- Changed:  Bootstrap 4.6.2
- Changed:  VSCode 1.73.1
- Rebuild:  new lib InnuAPID PID controller
- Rebuild:  new lib InnuAPID AutoTune
- Removed:  QuickPID and sTune
- Changed:  VSCode 1.73
- Optimze:  Performance mash webpage
- Optimze:  Performance index webpage
- Changed:  AutoTune lib calc Ku and Pu only
- Optimze:  Performance PID calcs
- Changed:  Resolution DS18B20 11bit (0.125°C steps)
- Added:    PCF reset at startup
- Fixed:    PCF8574 init
- Fixed:    AutoTune CPU load
- Reworked: PID AutoTune
- Changed:  PID lib to QuickPID
- Reworked: PID calc
- Fixed:    change page event display not working correctly
- Fixed:    Kettlepage no output
- Changed:  max number of sensors 4
- Reworked: Timer mgmt
- Reworked: Ticker mgmt
- Removed:  Eventmanager
- Reworked: WebIf index - Induction - PID
- Reworked: WebIf index - Sparge water - PID
- Reworked: WebIf mash - button size
- Reworked: WebIf async transfer
- Update:   PCF8574 lib
- Changed:  PWN actors
- Changed:  Ticker handling
- Update:   WifiManager lib
- Added:    PWM input in actors table (0..100%)
- Added:    actors PWM
- Fixed:    typo IDS2 setpoint autotune
- Fixed:    free mem below 20k
- Changed:  VSCode 1.72.2
- Fixed:    Order and power button ID actors on mash page
- Reworked: I2C lib - GPIO
- Added:    PCF8574 to index.html
- Reworked: Ticker actors
- Renamed:  PCF pins to P0 - P7 (D9-D16)
- Fixed:    Display startPage
- Fixed:    Bug in NextionX2 lib
- Fixed:    trap causing unexpected restart (exception 28)
- Fixed:    Display switch pages
- Added:    currentPageID to NextionX2 lib
- Added:    lastPageID to NextionX2 lib
- Fixed:    Init progress bar init (Display)
- Reworked: Page up/down (display)
            until currentPageID availible from lib there is no reliable method switching pages
- Added:    HLT sparge object
- Added:    PCF8574 support (I2C)
- Changed:  VSCode 1.71.1
- Fixed:    Bug WebUpdate: unmount FS before restart
- Added:    Import MQTTDevice mashplan format
- Added:    Export mashplan to download file (json)
- Added:    Reload button mashplan table
- Added:    Mashplan table move row up/down
- Fixed:    AutoTune starts only if IDS2 is enabled
- Added:    Display output KettlePage while mqttoff is enabled
- Added:    Display output BrewPage while mqttoff is enabled
- Changed:  play button toggles to red, if waiting for autonext
- changed:  play button start mash step regardless of target temp
- changed:  play button no action if last mash step
- changed:  play button no action if mash step active
- changed:  play button no action if mash step paused
- Changed:  moved manual autonext start from pause to play button
- Optimze:  bootstrap.min.css pured
- Optimze:  awefont pured to only in use glyphs
- Fixed:    MMum import
- Changed:  VSCode 1.71
- Added:    Mash webpage (requires mqttoff)
- Added:    mode mqttoff (mqtt communication disabled)
- Added:    PID Controller GGM IDS2
