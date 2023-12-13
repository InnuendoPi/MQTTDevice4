bool upTools(String url, String fname, WiFiClientSecure &clientup)
{
    HTTPClient https;
    char line[120];
    millis2wait(100);
    if (https.begin(clientup, url + fname))
    {
        int16_t httpCode = https.GET();
        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                uint8_t buff[128] = {0};
                WiFiClient *stream = https.getStreamPtr(); // get tcp stream
                bool check = LittleFS.remove("/" + fname);
                fsUploadFile = LittleFS.open("/" + fname + ".new", "w");
                int32_t len = https.getSize();
                int32_t startlen = len;
                while (https.connected() && (len > 0 || len == -1))
                {
                    size_t size = stream->available(); // get available data size
                    if (size > 0)
                    {
                        int32_t c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // lese maximal 128 byte in buff
                        fsUploadFile.write(buff, c);                                                        // schreibe buff in Datei
                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                if (fsUploadFile.size() == startlen)
                {
                    sprintf(line, "Framwork/Tools update %s getSize: %d fileSize: %d", fname.c_str(), startlen, fsUploadFile.size());
                    debugLog(UPDATELOG, line);
                    fsUploadFile.close();
                    bool check = LittleFS.remove("/" + fname);
                    check = LittleFS.rename("/" + fname + ".new", "/" + fname);
                }
                else
                {
                    sprintf(line, "Framwork/Tools update error %s getSize: %d fileSize: %d", fname.c_str(), startlen, fsUploadFile.size());
                    debugLog(UPDATELOG, line);
                    Serial.printf("*** SYSINFO: %s update finished with error startLen: %d file size: %d\n", fname.c_str(), startlen, fsUploadFile.size());
                    fsUploadFile.close();
                }

                https.end();
                return true;
            }
            else
                return false;
        }
        else
        {
            sprintf(line, "Framwork/Tools update error %s %s", fname.c_str(), https.errorToString(httpCode).c_str());
            debugLog(UPDATELOG, line);
#ifdef ESP32
            log_e("%s", line);
#endif
            https.end();
            return false;
        }
    }
    else
    {
        sprintf(line, "Framwork/Tools update error https start %s", fname.c_str());
        debugLog(UPDATELOG, line);
#ifdef ESP32
        log_e("%s", line);
#endif
        return false;
    }
}

void upFirm()
{

    WiFiClientSecure clientup;
    clientup.setInsecure();
    char line[120];

#ifdef ESP32
    httpUpdate.onEnd(update_finished);
    httpUpdate.onError(update_error);
    t_httpUpdate_return ret;
    if (LittleFS.exists(DEVBRANCH))
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/build/MQTTDevice32.ino.bin");
    else
        ret = httpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/build/MQTTDevice32.ino.bin");
#elif ESP8266
    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onError(update_error);
    t_httpUpdate_return ret;
    if (LittleFS.exists(DEVBRANCH))
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/build/MQTTDevice4.ino.bin");
    else
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/build/MQTTDevice4.ino.bin");
#endif

    return;
}

//   // Stack dump
//   // https://github.com/esp8266/Arduino/blob/master/doc/Troubleshooting/stack_dump.md

void updateTools()
{
    WiFiClientSecure clientup;
    clientup.setInsecure();
    fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "r");
    int8_t anzahlVersuche = 0;
    if (fsUploadFile)
    {
        char anzahlV = char(fsUploadFile.read()) - '0';
        anzahlVersuche = (int8_t)anzahlV;
    }
    fsUploadFile.close();
    if (anzahlVersuche > 3)
    {
        LittleFS.remove(UPDATETOOLS);
#ifdef ESP32
        log_e("ERROR update tools - %d", anzahlVersuche);
#endif
        return;
    }
    anzahlVersuche++;
    fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "w");
    int32_t bytesWritten = fsUploadFile.print((anzahlVersuche));
    fsUploadFile.close();
#ifdef ESP32
    log_e("ESP8266 IP address: %s time: %s WLAN Signal: %d", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
#endif
    char line[120];
    sprintf(line, "Update Framework/Tools #%d started - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
    debugLog(UPDATELOG, line);
#ifdef ESP32
    log_e("%s", line);
#endif
    if (anzahlVersuche == 1)
    {
        sprintf(line, "Firmware Version: %s", Version);
        debugLog(UPDATELOG, line);
#ifdef ESP32
        log_e("%s", line);
#endif
    }
    bool test;
    if (LittleFS.exists(DEVBRANCH))
    {
#ifdef ESP32
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "mqttfont.ttf", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "mqttdevice.min.css", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "mqttdevice.min.js", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "en.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "de.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/development/data/", "favicon.ico", clientup);
#elif ESP8266
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttfont.ttf", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.css", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.js", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "favicon.ico", clientup);
#endif
    }
    else
    {
#ifdef ESP32
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "mqttfont.ttf", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "mqttdevice.min.css", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "mqttdevice.min.js", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "en.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "de.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice32/master/data/", "favicon.ico", clientup);
#elif ESP8266
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttfont.ttf", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.css", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.js", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "en.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "de.json", clientup);
        test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "favicon.ico", clientup);
#endif
    }

    LittleFS.remove(UPDATETOOLS);
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void updateSys()
{
    char line[120];
    fsUploadFile = LittleFS.open(LOGUPDATESYS, "r");
    int8_t anzahlVersuche = 0;
    if (fsUploadFile)
    {
        char anzahlV = char(fsUploadFile.read()) - '0';
        anzahlVersuche = (int8_t)anzahlV;
    }
    fsUploadFile.close();
    if (anzahlVersuche > 3)
    {
        LittleFS.remove(UPDATESYS);
        return;
    }
    fsUploadFile = LittleFS.open(LOGUPDATESYS, "w");
    anzahlVersuche++;
    int32_t bytesWritten = fsUploadFile.print(anzahlVersuche);
    fsUploadFile.close();
    if (anzahlVersuche == 1)
        bool check = LittleFS.remove(UPDATELOG);

    sprintf(line, "WebUpdate firmware #%d started - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
    debugLog(UPDATELOG, line);
#ifdef ESP32
    log_e("%s", line);
#endif
    sprintf(line, "Firmware Version: %s", Version);
    debugLog(UPDATELOG, line);
#ifdef ESP32
    log_e("%s", line);
#endif
    upFirm();
}

void startHTTPUpdate()
{
    server.send(200, FPSTR("text/plain"), "ok");
    fsUploadFile = LittleFS.open(UPDATESYS, "w");
    if (!fsUploadFile)
    {
#ifdef ESP32
        log_e("%s", "Error WebUpdate firmware create file (LittleFS)");
#endif
        return;
    }
    else
    {
#ifdef ESP32
        log_e("%s", "WebUpdate firmware create file (LittleFS)");
#endif
        int32_t bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
    char line[120];
    bool check = LittleFS.remove(UPDATELOG);
    debugLog(UPDATELOG, "WebUpdate started");
    sprintf(line, "Firmware version: %s", Version);
    debugLog(UPDATELOG, line);
#ifdef ESP32
    log_e("%s", line);
#endif
    if (devBranch)
    {
        fsUploadFile = LittleFS.open(DEVBRANCH, "w");
        if (!fsUploadFile)
        {
#ifdef ESP32
            log_e("%s", "Error WebUpdate firmware dev create file (LittleFS)");
#endif
            return;
        }
        else
        {
#ifdef ESP32
            log_e("%s", "WebUpdate firmware dev create file (LittleFS)");
#endif
            int32_t bytesWritten = fsUploadFile.print("0");
            debugLog(UPDATELOG, "WebUpdate development branch ausgewählt");
            fsUploadFile.close();
        }
    }
    else
    {
        if (LittleFS.exists(DEVBRANCH))
            bool check = LittleFS.remove(DEVBRANCH);
    }
// debugLog(UPDATELOG, "*** WebUpdate Firmware reboot");
#ifdef ESP32
    log_e("%s", "WebUpdate Firmware reboot");
#endif
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_finished()
{
    debugLog(UPDATELOG, "Firmware update successful");
#ifdef ESP32
    log_e("%s", "Firmware update finished");
#endif
    fsUploadFile = LittleFS.open(UPDATETOOLS, "w");
    if (fsUploadFile)
    {
        int32_t bytesWritten = fsUploadFile.print(0);
        fsUploadFile.close();
    }
    LittleFS.remove(UPDATESYS);
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_error(int16_t err)
{
    char line[120];
#ifdef ESP32
    sprintf(line, "Firmware update error code %d: %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#elif ESP8266
    sprintf(line, "Firmware update error code %d: %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif
    debugLog(UPDATELOG, line);
#ifdef ESP32
    log_e("%s", line);
#endif
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}
