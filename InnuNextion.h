#ifndef INNUNEX_H
#define INNUNEX_H

//   Author:	Innuendo
//   Version: 0.0.1
//   More info see base lib https://seithan.com/Easy-Nextion-Library/Custom-Protocol/ and lib documentation


#include "Arduino.h"
#include <SoftwareSerial.h>

#ifdef ESP8266
extern const char *IRAM_ATTR pathToFileName(const char *path);
#endif

// declaration in display.cpp
extern void readCustomCommand();
extern void readCustomCommand() __attribute__((weak));

class InnuNex
{

public:
    InnuNex(SoftwareSerial &serial);
    void begin(unsigned long baud = 9600);
    void writeNum(const char*, uint32_t);
    void writeStr(const char*, String txt = "cmd");
    void checkNex(void);
    uint32_t readNum(const char*);
    String readStr(const char*);
    int readByte();
    void setDebug(bool val);
    bool getDebug();
    
    int currentPageId;
    int lastCurrentPageId;
    byte cmdGroup;
    byte cmdLength;

private:
    SoftwareSerial *_serial;
    void readCommand(void);
    
    // for function readNumber()
    uint8_t _numericBuffer[4];
    uint32_t _numberValue;

    // for General functions
    char _start_char;
    unsigned long _tmr1;
    bool _cmdFound;
    uint8_t _cmd1;
    uint8_t _len;

    // for function readStr()
    String _readString;

    // Serial debug output
    bool debug;
};
#endif
