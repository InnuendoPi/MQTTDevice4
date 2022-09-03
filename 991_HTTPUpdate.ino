// void upIn()
// {
//     //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
//     std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
//     //clientup->setFingerprint(fingerprint);
//     clientup->setInsecure();

//     HTTPClient https;
//     String indexURL;
//     if (LittleFS.exists("/dev.txt"))
//         indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/index.html";
//     else
//         indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html";

//     // if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
//     if (https.begin(*clientup, indexURL))
//     {
//         int httpCode = https.GET();
//         if (httpCode > 0)
//         {
//             // HTTP header has been send and Server response header has been handled
//             // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

//             // file found at server
//             if (httpCode == HTTP_CODE_OK)
//             {

//                 // get lenght of document (is -1 when Server sends no Content-Length header)
//                 int len = https.getSize();

//                 // create buffer for read
//                 static uint8_t buff[128] = {0};

//                 // Open file for write
//                 fsUploadFile = LittleFS.open("/index.html", "w");
//                 if (!fsUploadFile)
//                 {
//                     //Serial.printf( F("file open failed"));
//                     Serial.println("Abbruch!");
//                     Serial.println("*** SYSINFO: error save index.html");
//                     https.end();
//                     return;
//                 }

//                 // read all data from server
//                 while (https.connected() && (len > 0 || len == -1))
//                 {
//                     // get available data size
//                     size_t size = clientup->available();
//                     //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

//                     if (size)
//                     {
//                         //read up to 128 byte
//                         int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

//                         // write it to file
//                         fsUploadFile.write(buff, c);

//                         if (len > 0)
//                         {
//                             len -= c;
//                         }
//                     }
//                     delay(1);
//                 }

//                 Serial.println("*** SYSINFO: Update index finished");
//                 fsUploadFile.close();
//                 LittleFS.remove("/update.txt");
//                 fsUploadFile = LittleFS.open("/update1.txt", "w");
//                 int bytesWritten = fsUploadFile.print("0");
//                 fsUploadFile.close();
//             }
//             else
//                 return;
//         }
//         else
//         {
//             Serial.println("Cancel update!");
//             Serial.printf("*** SYSINFO: error update index: %s\n", https.errorToString(httpCode).c_str());
//             https.end();
//             LittleFS.end(); // unmount LittleFS
//             ESP.restart();
//             return;
//         }
//         https.end();
//         return;
//     }
//     return;
// }

void upFontTTF()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/fontawesome-webfont.ttf";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/fontawesome-webfont.ttf";

    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/fontawesome-webfont.ttf", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save font ttf");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update font ttf finished");
                fsUploadFile.close();
                // LittleFS.remove("/update.txt");
                // fsUploadFile = LittleFS.open("/update1.txt", "w");
                // int bytesWritten = fsUploadFile.print("0");
                // fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update font ttf: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}
void upFontWOFF()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/fontawesome-webfont.woff";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/fontawesome-webfont.woff";

    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/fontawesome-webfont.woff", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save font woff");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update font woff finished");
                fsUploadFile.close();
                // LittleFS.remove("/update.txt");
                // fsUploadFile = LittleFS.open("/update1.txt", "w");
                // int bytesWritten = fsUploadFile.print("0");
                // fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update font woff: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upFontWOFF2()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/fontawesome-webfont.woff2";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/fontawesome-webfont.woff2";

    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/fontawesome-webfont.woff2", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save font woff2");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update font woff2 finished");
                fsUploadFile.close();
                LittleFS.remove("/update.txt");
                fsUploadFile = LittleFS.open("/update1.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update font woff2: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}



void upCSS()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/bootstrap.min.css";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/bootstrap.min.css";

    // if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/bootstrap.min.css", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save css");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update css finished");
                fsUploadFile.close();
                LittleFS.remove("/update1.txt");
                fsUploadFile = LittleFS.open("/update11.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update css: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upJS()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/bootstrap.min.js";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/bootstrap.min.js";

    // if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/bootstrap.min.js", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save js");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update js finished");
                fsUploadFile.close();
                LittleFS.remove("/update11.txt");
                fsUploadFile = LittleFS.open("/update111.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update js: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upJQ()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/jquery.min.js";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/jquery.min.js";

    // if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/jquery.min.js", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save JQuery");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update JQuery finished");
                fsUploadFile.close();
                // LittleFS.remove("/update111.txt");
                // fsUploadFile = LittleFS.open("/update2.txt", "w");
                // int bytesWritten = fsUploadFile.print("0");
                // fsUploadFile.close();
                bool check = LittleFS.remove("/bootstrap.bundle.min.js");
                check = LittleFS.remove("/bootstrap.bundle.min.js.map");
                check = LittleFS.remove("/bootstrap.min.css.map");
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update JQuery: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upTableJQ()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;
    String indexURL;
    if (LittleFS.exists("/dev.txt"))
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/jquery.tabletojson.min.js";
    else
        indexURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/jquery.tabletojson.min.js";

    // if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
    if (https.begin(*clientup, indexURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/jquery.tabletojson.min.js", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: error save TableJSON JQuery");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Update TableJSON JQuery finished");
                fsUploadFile.close();
                LittleFS.remove("/update111.txt");
                fsUploadFile = LittleFS.open("/update2.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();

                // bool check = LittleFS.remove("/bootstrap.bundle.min.js");
                // check = LittleFS.remove("/bootstrap.bundle.min.js.map");
                // check = LittleFS.remove("/bootstrap.min.css.map");
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel update!");
            Serial.printf("*** SYSINFO: error update JQuery: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}



void upCerts()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();
    HTTPClient https;
    String certURL;
    if (LittleFS.exists("/dev.txt"))
        certURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/Info/ce.rts";
    else
        certURL = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/Info/ce.rts";

    if (https.begin(*clientup, certURL))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // Serial.printf("*** SYSINFO: [HTTPS] GET certs.ar Antwort: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK)
            {
                int len = https.getSize();
                static uint8_t buff[128] = {0};
                fsUploadFile = LittleFS.open("/certs.ar", "w");
                if (!fsUploadFile)
                {
                    Serial.println("Cancel update!");
                    Serial.println("*** SYSINFO: error write certs.ar");
                    https.end();
                    return;
                }

                while (https.connected() && (len > 0 || len == -1))
                {
                    size_t size = clientup->available();
                    if (size)
                    {
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        fsUploadFile.write(buff, c);
                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }
                fsUploadFile.close();
                Serial.println("*** SYSINFO: Certs update finished.");
                LittleFS.remove("/update2.txt");
                fsUploadFile = LittleFS.open("/update3.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Abbruch!");
            Serial.printf("*** SYSINFO: error udate certs: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upFirm()
{
    BearSSL::CertStore certStore;
    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    Serial.print(F("Number of CA certs read: "));
    Serial.println(numCerts);
    if (numCerts == 0)
    {
        Serial.println(F("*** SYSINFO: No certs found. Did you run certs-from-mozill.py and upload the LittleFS directory before running?"));
        return; // Can't connect to anything w/o certs!
    }

    BearSSL::WiFiClientSecure clientFirm;
    clientFirm.setCertStore(&certStore);
    clientFirm.setInsecure();

    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    //ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret;
    if (LittleFS.exists("/dev.txt"))
        ret = ESPhttpUpdate.update(clientFirm, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/build/MQTTDevice4.ino.bin");
    else
        ret = ESPhttpUpdate.update(clientFirm, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/build/MQTTDevice4.ino.bin");

    // t_httpUpdate_return ret = ESPhttpUpdate.update(clientFirm, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/build/MQTTDevice4.ino.bin");

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("*** SYSINFO: HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("*** SYSINFO: HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("*** SYSINFO: HTTP_UPDATE_OK");
        break;
    }
    return;
}

//   // Stack dump
//   // https://github.com/esp8266/Arduino/blob/master/doc/Troubleshooting/stack_dump.md

void updateSys()
{
    if (LittleFS.exists("/update.txt"))
    {
        fsUploadFile = LittleFS.open("/update.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update.txt");
            Serial.println("*** SYSINFO: ERROR update index");
            return;
        }
        fsUploadFile = LittleFS.open("/update.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update 1. step fonts started - free heap: ");
        Serial.println(ESP.getFreeHeap());
        // upIn();
        upFontTTF();
        upFontWOFF();
        upFontWOFF2();
    }
    if (LittleFS.exists("/update1.txt"))
    {
        fsUploadFile = LittleFS.open("/update1.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update1.txt");
            Serial.println("*** SYSINFO: ERROR update css");
            return;
        }
        fsUploadFile = LittleFS.open("/update1.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log1.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update css started- free heap: ");
        Serial.println(ESP.getFreeHeap());
        upCSS();
    }
    if (LittleFS.exists("/update11.txt"))
    {
        fsUploadFile = LittleFS.open("/update11.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update11.txt");
            Serial.println("*** SYSINFO: ERROR update js");
            return;
        }
        fsUploadFile = LittleFS.open("/update11.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log11.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update js started - free heap: ");
        Serial.println(ESP.getFreeHeap());
        upJS();
    }
    if (LittleFS.exists("/update111.txt"))
    {
        fsUploadFile = LittleFS.open("/update111.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update111.txt");
            Serial.println("*** SYSINFO: ERROR update JQuery");
            return;
        }
        fsUploadFile = LittleFS.open("/update111.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log111.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update JQuery started - free heap: ");
        Serial.println(ESP.getFreeHeap());
        upJQ();
        upTableJQ();
    }
    if (LittleFS.exists("/update2.txt"))
    {
        fsUploadFile = LittleFS.open("/update2.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update2.txt");
            Serial.println("*** SYSINFO: ERROR Cert Update");
            return;
        }
        fsUploadFile = LittleFS.open("/update2.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log2.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Start cert update - free heap: ");
        Serial.println(ESP.getFreeHeap());
        upCerts();
    }
    if (LittleFS.exists("/update3.txt"))
    {
        fsUploadFile = LittleFS.open("/update3.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update3.txt");
            Serial.println("*** SYSINFO: ERROR Firmware Update");
            return;
        }
        fsUploadFile = LittleFS.open("/update3.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log3.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();

        Serial.print("*** SYSINFO: Start firmware update - free heap: ");
        Serial.println(ESP.getFreeHeap());
        upFirm();
    }
}

void startHTTPUpdate()
{
    // Starte Updates
    fsUploadFile = LittleFS.open("/update.txt", "w");
    if (!fsUploadFile)
    {
        DEBUG_MSG("%s\n", "*** Error WebUpdate create file (LittleFS)");
        return;
    }
    else
    {
        int bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
    if (devBranch)
    {
        fsUploadFile = LittleFS.open("/dev.txt", "w");
        if (!fsUploadFile)
        {
            DEBUG_MSG("%s\n", "*** Error WebUpdate create file (LittleFS)");
            return;
        }
        else
        {
            int bytesWritten = fsUploadFile.print("0");
            fsUploadFile.close();
        }
    }
    cbpiEventSystem(EM_REBOOT);
}

void update_progress(int cur, int total)
{
    Serial.printf("*** SYSINFO:  Firmware Update %d - %d Bytes\n", cur, total);
}

void update_started()
{
    Serial.println("*** SYSINFO:  Firmware Update started");
}

void update_finished()
{
    Serial.println("*** SYSINFO:  Firmware Update beendet");
    LittleFS.remove("/update3.txt");
}

void update_error(int err)
{
    Serial.printf("*** SYSINFO:  Firmware update error code %d\n", err);
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
}
