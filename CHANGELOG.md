# Changelog

Version 4.31b

- Fixed:    Display startPage
- Fixed:    Bug in NextionX2 lib (temp removed lastPageID)
- Fixed:    trap causing unexpected restart (exception 28)
- Fixed:    Display switch pages
- Added:    currentPageID to NextionX2 lib
- Added:    lastPageID to NextionX2 lib
- Fixed:    Init progress bar init (Display)
- Reworked: Page up/down (display)
            until currentPageID availible from lib there is no reliable method switching pages
- Added:    HLT sparge object
- Added:    PCF8574 support (I2C)
- Changed:  VSCode 1.72
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
- Optimze:  bootstrap.min.css pured (from 160kb to 26kb)
- Optimze:  awefont pured to only in use glyphs
- Fixed:    MMum import
- Changed:  VSCode 1.71
- Added:    Mash webpage (requires mqttoff)
- Added:    mode mqttoff (mqtt communication disabled)
- Added:    PID Controller GGM IDS2
