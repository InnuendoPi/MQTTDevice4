#ifndef INNUNEX_H
#include "InnuNextion.h"
#endif

InnuNex::InnuNex(SoftwareSerial &serial)
{
    _serial = &serial;
}

void InnuNex::begin(unsigned long baud)
{
    _serial->begin(baud);

    delay(100);

    _tmr1 = millis();
    while (_serial->available() > 0)
    {
        if ((millis() - _tmr1) > 400UL)
        {
            break;
        }
        _serial->read();
    }
}

void InnuNex::writeNum(const char *_component, uint32_t _numVal)
{
    _serial->print(_component);
    _serial->print("=");
    _serial->print(_numVal);
    _serial->print("\xFF\xFF\xFF");
    // if (debug)
    //     Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component: %s num: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _component, _numVal);
}

void InnuNex::writeStr(const char *command, String _strVal)
{
    if (_strVal == "cmd")
    {
        _serial->print(command);
        _serial->print("\xFF\xFF\xFF");
#ifdef ESP32
        if (debug)
            Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component id: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, command, _strVal.c_str());
#endif
    }
    else if (_strVal != "cmd")
    {
        _serial->print(command);
        _serial->print("=\"");
        _serial->print(_strVal);
        _serial->print("\"");
        _serial->print("\xFF\xFF\xFF");
    }
    // if (debug)
    //     Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, command, _strVal.c_str());
}

String InnuNex::readStr(const char *_Textcomp)
{
    bool _endOfCommandFound = false;
    char _tempChar;

    _tmr1 = millis();
    while (_serial->available())
    {
        if ((millis() - _tmr1) > 1000UL)
        {
            _readString = "ERROR";
            break;
        }
        else
            checkNex();
    }

    _serial->print("get ");
    _serial->print(_Textcomp);
    _serial->print("\xFF\xFF\xFF");

    _tmr1 = millis();
    while (_serial->available() < 4)
    {
        if ((millis() - _tmr1) > 400UL)
        {
            _readString = "ERROR";
            break;
        }
    }

    if (_serial->available() > 3)
    {
        _start_char = _serial->read();
        _tmr1 = millis();

        while (_start_char != 0x70)
        {
            if (_serial->available())
                _start_char = _serial->read();

            if ((millis() - _tmr1) > 100UL)
            {
                _readString = "ERROR";
                break;
            }
        }

        if (_start_char == 0x70)
        {
            _readString = "";
            int _endBytes = 0;
            _tmr1 = millis();

            while (_endOfCommandFound == false)
            {
                if (_serial->available())
                {
                    _tempChar = _serial->read();
                    if (_tempChar == 0xFF || _tempChar == 0xFFFFFFFF)
                    {
                        _endBytes++;
                        if (_endBytes == 3)
                        {
                            _endOfCommandFound = true;
                        }
                    }
                    else
                        _readString += _tempChar;
                }

                if ((millis() - _tmr1) > 1000UL)
                {
                    _readString = "ERROR";
                    break;
                }
            }
        }
    }
#ifdef ESP32
    if (debug)
        Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component id: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _Textcomp, _readString.c_str());
#endif

    return _readString;
}
uint32_t InnuNex::readNum(const char *_comp)
{
    bool _endOfCommandFound = false;
    char _tempChar;

    _numberValue = 777777;

    _tmr1 = millis();
    while (_serial->available())
    {
        if ((millis() - _tmr1) > 1000UL)
        {
            _numberValue = 777777;
            break;
        }
        else
            checkNex();
    }
    _serial->print("get ");
    _serial->print(_comp);
    _serial->print("\xFF\xFF\xFF");

    _tmr1 = millis();
    while (_serial->available() < 8)
    {
        if ((millis() - _tmr1) > 400UL)
        {
            _numberValue = 777777;
            break;
        }
    }

    if (_serial->available() > 7)
    {
        _start_char = _serial->read();
        _tmr1 = millis();

        while (_start_char != 0x71)
        {
            if (_serial->available())
                _start_char = _serial->read();

            if ((millis() - _tmr1) > 100UL)
            {
                _numberValue = 777777;
                break;
            }
        }

        if (_start_char == 0x71)
        {
            for (int i = 0; i < 4; i++)
            {
                _numericBuffer[i] = _serial->read();
            }

            int _endBytes = 0;
            _tmr1 = millis();

            while (_endOfCommandFound == false)
            {
                _tempChar = _serial->read();
                if (_tempChar == 0xFF || _tempChar == 0xFFFFFFFF)
                {
                    _endBytes++;
                    if (_endBytes == 3)
                        _endOfCommandFound = true;
                }
                else
                {
                    _numberValue = 777777;
                    break;
                }

                if ((millis() - _tmr1) > 1000UL)
                {
                    _numberValue = 777777;
                    break;
                }
            }
        }
    }

    if (_endOfCommandFound == true)
    {
        _numberValue = _numericBuffer[3];
        _numberValue <<= 8;
        _numberValue |= _numericBuffer[2];
        _numberValue <<= 8;
        _numberValue |= _numericBuffer[1];
        _numberValue <<= 8;
        _numberValue |= _numericBuffer[0];
    }
    else
    {
        _numberValue = 777777;
    }
    // if (debug)
    //     Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component: %s num: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _comp, _numberValue);

    return _numberValue;
}

int InnuNex::readByte()
{

    int _tempInt = _serial->read();
#ifdef ESP32
    if (debug)
        Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): byte: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _tempInt);
#endif
    return _tempInt;
}

void InnuNex::checkNex()
{
    int temp = 0;
    if (_serial->available() > 2)
    {
        _start_char = _serial->read();
        _tmr1 = millis();
        temp = 0;

        while (_start_char != 0x66 && _start_char != 0x65)
        {
            temp++;

            _start_char = _serial->read();
            if ((millis() - _tmr1) > 100UL)
                break;
            return;
        }

        if (_start_char == 0x66) // change page
        {
            _len = _serial->read();
            _tmr1 = millis();
            _cmdFound = true;
            while (_serial->available() < _len) 
            {
                if ((millis() - _tmr1) > 100UL)
                {
                    _cmdFound = false;
                    break;
                }
            }
            _cmd1 = 0x66;

#ifdef ESP32
            if (debug)
                Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): 0x66 (preinit) command: %x length: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _cmd1, _len);
#endif
            if (_cmdFound)
            {
                lastCurrentPageId = currentPageId;
                currentPageId = _len;
                cmdGroup = _cmd1;
                cmdLength = _len;
                readCustomCommand(); // extern
            }
            return;
        }

        if (_start_char == 0x65)
        {
            _len = _serial->read();
            _tmr1 = millis();
            _cmdFound = true;

            while (_serial->available() < _len)
            {
                if ((millis() - _tmr1) > 100UL)
                {
                    _cmdFound = false;
                    break;
                }
            }

            if (_cmdFound == true)
            {
                _cmd1 = _serial->read();
#ifdef ESP32
                if (debug)
                    Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): 0x65 command: %d length: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _cmd1, _len);
#endif
                cmdGroup = _cmd1;
                cmdLength = _len;
                readCustomCommand(); // extern
            }
            return;
        }
    }
}

void InnuNex::setDebug(bool val)
{
    debug = val;
}

bool InnuNex::getDebug()
{
    return debug;
}
