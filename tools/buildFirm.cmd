copy build\MQTTDevice4.ino.bin tools\* /Y
copy build\MQTTDevice4.mklittlefs.bin tools\* /Y
cd tools
del firmware.zip
"C:\Program Files\7-Zip\7z.exe" a Firmware.zip MQTTDevice4.ino.bin MQTTDevice4.mklittlefs.bin Flashen.cmd esptool.exe
