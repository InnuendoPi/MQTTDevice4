/* PINMAP       Info          Input           Output

*** GPIO Pins Wemos D1 mini

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
  12  D6        SPI MISO      ok              ok        GGM IDS2 Commandchannel(yellow)     SDL PCF8574
  13  D7        SPI MOSI      ok              ok        GGM IDS2 Relais(white)              digitalPinToInterrupt
  14  D5        SPI SCLK      ok              ok        GGM IDS2 Backchannel(blue)          SDA PCF8574
  15  D8        SPI CS        -               (ok)      Buzzer
  16  D0        HIGH at boot  No interrupt    No PWM    Pump

*** PCF Pins Shield PCF8574 I2C port expander

  17  D9*
  18  D10*
  19  D11*
  20  D12*
  21  D13*
  22  D14*
  23  D15*
  24  D16*
  
*/
