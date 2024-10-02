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

    delay(100); // Wait for the Serial to initialize

    _tmr1 = millis();
    while (_serial->available() > 0)
    { // Read the Serial until it is empty. This is used to clear Serial buffer
        if ((millis() - _tmr1) > 400UL)
        { // Reading... Waiting... But not forever......
            break;
        }
        _serial->read(); // Read and delete bytes
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

/*
 * -- writeStr(String, String): for writing in components' text attributes
 * String No1 = objectname.textAttribute (example: "t0.txt"  or "b0.txt")
 * String No2 = value (example: "Hello World")
 * Syntax: | myObject.writeStr("t0.txt", "Hello World");  |  or  | myObject.writeNum("b0.txt", "Button0"); |
 *         | set the value of textbox t0 to "Hello World" |      | set the text of button b0 to "Button0"  |
 */
void InnuNex::writeStr(const char *command, String _strVal)
{
    if (_strVal == "cmd")
    {
        _serial->print(command);
        _serial->print("\xFF\xFF\xFF");
        // if (debug)
        //     Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component id: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, command, _strVal.c_str());

    }
    else if (_strVal != "cmd")
    {
        _serial->print(command);
        _serial->print("=\"");
        _serial->print(_strVal);
        _serial->print("\"");
        _serial->print("\xFF\xFF\xFF");
    }
    if (debug)
        Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, command, _strVal.c_str());
}

String InnuNex::readStr(const char *_Textcomp)
{
    bool _endOfCommandFound = false;
    char _tempChar;

    _tmr1 = millis();
    while (_serial->available())
    { // Waiting for NO bytes on Serial,
      // as other commands could be sent in that time.
        if ((millis() - _tmr1) > 1000UL)
        { // Waiting... But not forever...after the timeout
            _readString = "ERROR";
            break; // Exit from the loop due to timeout. Return ERROR
        }
        else
        {
            checkNex(); // We run checkNex(), in case that the bytes on Serial are a part of a command
        }
    }

    // As there are NO bytes left in Serial, which means no further commands need to be executed,
    // send a "get" command to Nextion

    _serial->print("get ");
    _serial->print(_Textcomp); // The String of a component you want to read on Nextion
    _serial->print("\xFF\xFF\xFF");

    // And now we are waiting for a reurn data in the following format:
    // 0x70 ... (each character of the String is represented in HEX) ... 0xFF 0xFF 0xFF

    // Example: For the String ab123, we will receive: 0x70 0x61 0x62 0x31 0x32 0x33 0xFF 0xFF 0xFF

    _tmr1 = millis();
    while (_serial->available() < 4)
    { // Waiting for bytes to come to Serial, an empty Textbox will send 4 bytes
      // and this the minimmum number that we are waiting for (70 FF FF FF)
        if ((millis() - _tmr1) > 400UL)
        { // Waiting... But not forever...after the timeout
            _readString = "ERROR";
            break; // Exit the loop due to timeout. Return ERROR
        }
    }

    if (_serial->available() > 3)
    {                                  // Read if more then 3 bytes come (we always wait for more than 3 bytes)
        _start_char = _serial->read(); //  variable (start_char) read and store the first byte on it
        _tmr1 = millis();

        while (_start_char != 0x70)
        { // If the return code 0x70 is not detected,
            if (_serial->available())
            { // read the Serial until you find it
                _start_char = _serial->read();
            }

            if ((millis() - _tmr1) > 100UL)
            {                          // Waiting... But not forever......
                _readString = "ERROR"; // If the 0x70 is not found within the given time, break the while()
                                       // to avoid being stuck inside the while() loop
                break;
            }
        }

        if (_start_char == 0x70)
        {                      // If the return code 0x70 is detected,
            _readString = "";  // We clear the _readString variable, to avoid any accidentally stored text
            int _endBytes = 0; // This variable helps us count the end command bytes of Nextion. The 0xFF 0xFF 0xFF
            _tmr1 = millis();

            while (_endOfCommandFound == false)
            { // As long as the three 0xFF bytes have NOT been found, run the commands inside the loop
                if (_serial->available())
                {
                    _tempChar = _serial->read(); // Read the first byte of the Serial

                    if (_tempChar == 0xFF || _tempChar == 0xFFFFFFFF)
                    {                // If the read byte is the end command byte,
                        _endBytes++; // Add one to the _endBytes counter
                        if (_endBytes == 3)
                        {
                            _endOfCommandFound = true; // If the counter is equal to 3, we have the end command
                        }
                    }
                    else
                    {                             // If the read byte is NOT the end command byte,
                        _readString += _tempChar; // Add the char to the _readString String variable
                    }
                }

                if ((millis() - _tmr1) > 1000UL)
                {                          // Waiting... But not forever......
                    _readString = "ERROR"; // If the end of the command is NOT found in the time given,
                                           // break the while
                    break;
                }
            }
        }
    }
    if (debug)
        Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): component id: %s string: %s\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _Textcomp, _readString.c_str());

    return _readString;
}

/*
 * -- readNumber(String): We use it to read the value of a components' numeric attribute on Nextion
 * In every component's numeric attribute (value, bco color, pco color...etc)
 * String = objectname.numericAttribute (example: "n0.val", "n0.pco", "n0.bco"...etc)
 * Syntax: | myObject.readNumber("n0.val"); |  or  | myObject.readNumber("b0.bco");                       |
 *         | read the value of numeric n0   |      | read the color number of the background of butoon b0 |
 */

uint32_t InnuNex::readNum(const char *_comp)
{
    // _comp = component;
    bool _endOfCommandFound = false;
    char _tempChar;

    _numberValue = 777777; // The function will return this number in case it fails to read the new number

    _tmr1 = millis();
    while (_serial->available())
    { // Waiting for NO bytes on Serial,
      // as other commands could be sent in that time.
        if ((millis() - _tmr1) > 1000UL)
        { // Waiting... But not forever...after the timeout
            _numberValue = 777777;
            break; // Exit from the loop due to timeout. Return 777777
        }
        else
        {
            checkNex(); // We run checkNex(), in case that the bytes on Serial are a part of a command
        }
    }

    // As there are NO bytes left in Serial, which means no further commands need to be executed,
    // send a "get" command to Nextion

    _serial->print("get ");
    _serial->print(_comp); // The String of a component you want to read on Nextion
    _serial->print("\xFF\xFF\xFF");

    // And now we are waiting for a return data in the following format:
    // 0x71 0x01 0x02 0x03 0x04 0xFF 0xFF 0xFF
    // 0x01 0x02 0x03 0x04 is 4 byte 32-bit value in little endian order.

    _tmr1 = millis();
    while (_serial->available() < 8)
    { // Waiting for bytes to come to Serial,
      // we are waiting for 8 bytes
        if ((millis() - _tmr1) > 400UL)
        { // Waiting... But not forever...after the timeout
            _numberValue = 777777;
            break; // Exit the loop due to timeout. Return 777777
        }
    }

    if (_serial->available() > 7)
    {
        _start_char = _serial->read(); //  variable (start_char) read and store the first byte on it
        _tmr1 = millis();

        while (_start_char != 0x71)
        { // If the return code 0x71 is not detected,
            if (_serial->available())
            { // read the Serial until you find it
                _start_char = _serial->read();
            }

            if ((millis() - _tmr1) > 100UL)
            {                          // Waiting... But not forever......
                _numberValue = 777777; // If the 0x71 is not found within the given time, break the while()
                                       // to avoid being stuck inside the while() loop
                break;
            }
        }

        if (_start_char == 0x71)
        { // If the return code 0x71 is detected,

            for (int i = 0; i < 4; i++)
            { // Read the 4 bytes represent the number and store them in the numeric buffer

                _numericBuffer[i] = _serial->read();
            }

            int _endBytes = 0; // This variable helps us count the end command bytes of Nextion. The 0xFF 0xFF 0xFF
            _tmr1 = millis();

            while (_endOfCommandFound == false)
            { // As long as the three 0xFF bytes have NOT been found, run the commands inside the loop

                _tempChar = _serial->read(); // Read the next byte of the Serial

                if (_tempChar == 0xFF || _tempChar == 0xFFFFFFFF)
                {                // If the read byte is the end command byte,
                    _endBytes++; // Add one to the _endBytes counter
                    if (_endBytes == 3)
                    {
                        _endOfCommandFound = true; // If the counter is equal to 3, we have the end command
                    }
                }
                else
                { // If the read byte is NOT the end command byte,
                    _numberValue = 777777;
                    break;
                }

                if ((millis() - _tmr1) > 1000UL)
                {                          // Waiting... But not forever......
                    _numberValue = 777777; // If the end of the command is NOT found in the time given,
                                           // break the while
                    break;
                }
            }
        }
    }

    if (_endOfCommandFound == true)
    {
        // We can continue with the little endian conversion
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

/*
 * -- readByte(): Main purpose and usage is for the custom commands read
 * Where we need to read bytes from Serial inside user code
 */

int InnuNex::readByte()
{

    int _tempInt = _serial->read();
    if (debug)
        Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): byte: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _tempInt);
    return _tempInt;
}

void InnuNex::checkNex()
{
    int temp = 0;
    if (_serial->available() > 2)
    {                                  // Read if more then 2 bytes come (we always send more than 2 <#> <len> <cmd> <id>
        _start_char = _serial->read(); // Create a local variable (start_char) read and store the first byte on it
        // _tmr1 = millis();
        temp = 0;

        // while (_start_char != 0x23 && _start_char != 0x66 && _start_char != 0x65)
        while (_start_char != 0x66 && _start_char != 0x65)
        {
            temp++;

            _start_char = _serial->read(); // whille the start_char is not the start command symbol
                                           //  read the serial (when we read the serial the byte is deleted from the Serial buffer)
            if ((millis() - _tmr1) > 100UL)
            { //   Waiting... But not forever......
                break;
            }
            return;
        }

        if (_start_char == 0x66) // change page
        {
            _len = 2; // change page fix size 2: 0x66 pageID

            // And when we find the character #
            // _len = _serial->read(); //  read and store the value of the second byte
            // <len> is the lenght (number of bytes following)

            _tmr1 = millis();
            // _cmdFound = true;

            // while (_serial->available() < _len)
            // while (_serial->available() <= 2)
            lastCurrentPageId = currentPageId;
            currentPageId = _serial->read(); // lese ID neue page

            while (_serial->available()) // lese bis Ende
            {                            // Waiting for all the bytes that we declare with <len> to arrive

                if ((millis() - _tmr1) > 100UL)
                { // Waiting... But not forever......
                    // _cmdFound = false; // tmr_1 a timer to avoid the stack in the while loop if there is not any bytes on _serial
                    break;
                }
                // int temp = _serial->read();
                temp = _serial->read();
            }
            _cmd1 = 0x66;
            if (debug)
                Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): 0x66 (preinit) newPageId: %d lastPageId: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, currentPageId, lastCurrentPageId);

            readCommand();
            return;
        }

        if (_start_char == 0x65)    // send component id events
        {                           // And when we find the character #
            _len = _serial->read(); //  read and store the value of the second byte
                                    // <len> is the lenght (number of bytes following)

            _tmr1 = millis();
            _cmdFound = true;

            while (_serial->available() < _len)
            { // Waiting for all the bytes that we declare with <len> to arrive
                if ((millis() - _tmr1) > 100UL)
                {                      // Waiting... But not forever......
                    _cmdFound = false; // tmr_1 a timer to avoid the stack in the while loop if there is not any bytes on _serial
                    break;
                }
            }

            if (_cmdFound == true)
            {                            // So..., A command is found (bytes in _serial buffer egual more than len)
                _cmd1 = _serial->read(); // Read and store the next byte. This is the command group
                if (debug)
                    Serial.printf("\033[0;36m[%6lu][V][%s:%d] %s(): 0x65 command: %d length: %d\033[0m\n", millis(), pathToFileName(__FILE__), __LINE__, __FUNCTION__, _cmd1, _len);
                readCommand(); // We call the readCommand(),
                               // in which we read, seperate and execute the commands
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

void InnuNex::readCommand()
{
  // component IDs
  // 65 02 03 00 FF FF FF
  // 65 02 09 00 FF FF FF
  // 65 02 08 00 FF FF FF

  switch (_cmd1)
  {
  case 'P': /*or <case 0x50:>  If 'P' matches, we have the command group "Page".
             *The next byte is the page <Id> according to our protocol.
             *
             * We have write in every page's "preinitialize page" the command
             *  printh 23 02 50 xx (where xx is the page id in HEX, 00 for 0, 01 for 1, etc).
             * <<<<Every event written on Nextion's pages preinitialize page event will run every time the page is Loaded>>>>
             *  it is importand to let the Arduino "Know" when and which Page change.
             */
    lastCurrentPageId = currentPageId;
    currentPageId = _serial->read();
    break;

  default:
    cmdGroup = _cmd1; // stored in the public variable cmdGroup for later use in the main code || Object id
    cmdLength = _len; // stored in the public variable cmdLength for later use in the main code || Page id
    readCustomCommand();  // extern

    break;
  }
}
