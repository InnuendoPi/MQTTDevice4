# Changelog

Version 4.30c

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
