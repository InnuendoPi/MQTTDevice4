bool upTools(String url, String fname, BearSSL::WiFiClientSecure &clientup)
{
    HTTPClient https;
    char line[120];
    millis2wait(100);
    if (https.begin(clientup, url + fname))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            if (httpCode == HTTP_CODE_OK)
            {
                uint8_t buff[128] = {0};
                // get tcp stream
                WiFiClient *stream = https.getStreamPtr();
                fsUploadFile = LittleFS.open("/" + fname + ".new", "w");
                int len = https.getSize();
                int startlen = len;
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = stream->available();
                    if (size > 0)
                    {
                        // read up to 128 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        // write to file
                        fsUploadFile.write(buff, c);
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
                    sprintf(line, "Framwork/Tools update Fehler %s getSize: %d fileSize: %d", fname.c_str(), startlen, fsUploadFile.size());
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
            sprintf(line, "Framwork/Tools update Fehler %s %d", fname.c_str(), https.errorToString(httpCode).c_str());
            debugLog(UPDATELOG, line);
            Serial.printf("*** SYSINFO: error update %s\n", fname.c_str());
            https.end();
            return false;
        }
    }
    else
    {
        sprintf(line, "Framwork/Tools update Fehler https start %s", fname.c_str());
        debugLog(UPDATELOG, line);
        Serial.printf("*** SYSINFO: error https.begin %s\n", fname.c_str());
        return false;
    }
}

void upFirm()
{
    BearSSL::WiFiClientSecure clientup;
    clientup.setInsecure();
    // BearSSL::CertStore certStore;
    // int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR(CERT));
    // if (numCerts == 0)
    // {
    //     sprintf(line, "CA certificates not found: %d - insecure SSL connection!", numCerts);
    //     debugLog(UPDATELOG, line);
    //     clientup.setInsecure();
    // }
    // else
    // {
    //     sprintf(line, "Number of CA certificates: %d - secure SSL connection", numCerts);
    //     debugLog(UPDATELOG, line);
    //     clientup.setCertStore(&certStore);
    // }
    char line[120];
    bool mfln = clientup.probeMaxFragmentLength("raw.githubusercontent.com", 443, MAXFRAGLEN);
    if (mfln)
    {
        clientup.setBufferSizes(MAXFRAGLEN, MAXFRAGLEN);
    }
    if (clientup.connect("raw.githubusercontent.com", 443))
    {
        sprintf(line, "MFLN Status: %s - connected", clientup.getMFLNStatus() ? "true" : "false");
        debugLog(UPDATELOG, line);
    }
    else
    {
        sprintf(line, "MFLN Status: %s - unable to connect", clientup.getMFLNStatus() ? "true" : "false");
        debugLog(UPDATELOG, line);
        LittleFS.end(); // unmount LittleFS
        millis2wait(100);
        ESP.restart();
        return;
    }

    ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onError(update_error);
    t_httpUpdate_return ret;

    if (LittleFS.exists(DEVBRANCH))
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/build/MQTTDevice4.ino.bin");
    else
        ret = ESPhttpUpdate.update(clientup, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/build/MQTTDevice4.ino.bin");

    return;
}

//   // Stack dump
//   // https://github.com/esp8266/Arduino/blob/master/doc/Troubleshooting/stack_dump.md

void updateTools()
{
    BearSSL::WiFiClientSecure clientup;
    clientup.setInsecure();
    // BearSSL::CertStore certStore;
    // int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR(CERT));
    // if (numCerts == 0)
    // {
    //     sprintf(line, "CA certificates not found: %d - insecure SSL connection!", numCerts);
    //     debugLog(UPDATELOG, line);
    //     clientup.setInsecure();
    // }
    // else
    // {
    //     sprintf(line, "Number of CA certificates: %d - secure SSL connection", numCerts);
    //     debugLog(UPDATELOG, line);
    //     clientup.setCertStore(&certStore);
    // }
    char line[120];
    if (LittleFS.exists(UPDATETOOLS))
    {
        fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "r");
        int anzahlVersuche = 0;
        if (fsUploadFile)
        {
            char anzahlV = char(fsUploadFile.read()) - '0';
            anzahlVersuche = (int)anzahlV;
        }
        fsUploadFile.close();
        if (anzahlVersuche > 3)
        {
            LittleFS.remove(UPDATETOOLS);
            Serial.printf("*** SYSINFO: ERROR update tools - %d\n", anzahlVersuche);
            return;
        }
        anzahlVersuche++;
        fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "w");
        int bytesWritten = fsUploadFile.print((anzahlVersuche));
        fsUploadFile.close();
        Serial.printf("*** SYSINFO: ESP8266 IP Adresse: %s Zeit: %s WLAN Signal: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
        Serial.printf("*** SYSINFO: Update tools #%d started - free heap: %d\n", anzahlVersuche, ESP.getFreeHeap());
        sprintf(line, "Update Framework/Tools #%d started - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
        debugLog(UPDATELOG, line);
        if (anzahlVersuche == 1)
        {
            sprintf(line, "Firmware Version: %s", Version);
            debugLog(UPDATELOG, line);
        }
        bool mfln = clientup.probeMaxFragmentLength("raw.githubusercontent.com", 443, MAXFRAGLEN);
        if (mfln)
        {
            clientup.setBufferSizes(MAXFRAGLEN, MAXFRAGLEN);
        }
        if (clientup.connect("raw.githubusercontent.com", 443))
        {
            sprintf(line, "MFLN Status: %s - connected", clientup.getMFLNStatus() ? "true" : "false");
            debugLog(UPDATELOG, line);
        }
        else
        {
            sprintf(line, "MFLN Status: %s - unable to connect", clientup.getMFLNStatus() ? "true" : "false");
            debugLog(UPDATELOG, line);
            return;
        }
        bool test;
        if (LittleFS.exists(DEVBRANCH))
        {
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttfont.ttf", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.css", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.js", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "favicon.ico", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "ce.rts", clientup);
        }
        else
        {
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttfont.ttf", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.css", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.js", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "favicon.ico", clientup);
            test = upTools("https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "ce.rts", clientup);
        }

        LittleFS.remove(UPDATETOOLS);
        LittleFS.end();
        Serial.print("*** SYSINFO: Update tools finished\n");
        ESP.restart();
    }
}
void updateSys()
{
    timeClient.update();
    char line[120];
    if (LittleFS.exists(UPDATESYS))
    {
        fsUploadFile = LittleFS.open(LOGUPDATESYS, "r");

        int anzahlVersuche = 0;
        if (fsUploadFile)
        {
            char anzahlV = char(fsUploadFile.read()) - '0';
            anzahlVersuche = (int)anzahlV;
        }
        fsUploadFile.close();
        if (anzahlVersuche > 3)
        {
            LittleFS.remove(UPDATESYS);
            Serial.println("*** SYSINFO: ERROR update firmware");
            return;
        }
        fsUploadFile = LittleFS.open(LOGUPDATESYS, "w");
        anzahlVersuche++;
        int bytesWritten = fsUploadFile.print(anzahlVersuche);
        fsUploadFile.close();
        Serial.printf("*** SYSINFO: WebUpdate firmware #%d started - free heap: %d\n", anzahlVersuche, ESP.getFreeHeap());
        if (anzahlVersuche == 1)
            bool check = LittleFS.remove(UPDATELOG);
        sprintf(line, "WebUpdate firmware #%d started - free heap: %d", anzahlVersuche, ESP.getFreeHeap());
        debugLog(UPDATELOG, line);
        sprintf(line, "Firmware Version: %s", Version);
        debugLog(UPDATELOG, line);
        upFirm();
    }
}

void startHTTPUpdate()
{
    char line[120];
    server.send(200, FPSTR("text/plain"), "ok");
    fsUploadFile = LittleFS.open(UPDATESYS, "w");
    if (!fsUploadFile)
    {
        Serial.println("*** Error WebUpdate firmware create file (LittleFS)");
        return;
    }
    else
    {
        int bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
    bool check = LittleFS.remove(UPDATELOG);
    debugLog(UPDATELOG, "WebUpdate started");
    if (devBranch)
    {
        fsUploadFile = LittleFS.open(DEVBRANCH, "w");
        if (!fsUploadFile)
        {
            Serial.println("*** Error WebUpdate firmware dev create file (LittleFS)");
            return;
        }
        else
        {
            debugLog(UPDATELOG, "WebUpdate development branch");
            int bytesWritten = fsUploadFile.print("0");
            fsUploadFile.close();
        }
    }
    else
    {
        if (LittleFS.exists(DEVBRANCH))
            bool check = LittleFS.remove(DEVBRANCH);
    }
    debugLog(UPDATELOG, "*** WebUpdate firmware reboot");
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_finished()
{
    debugLog(UPDATELOG, "Firmware Update successful finished");
    Serial.println("*** SYSINFO:  Firmware update finished");
    fsUploadFile = LittleFS.open(UPDATETOOLS, "w");
    if (fsUploadFile)
    {
        uint8_t bytesWritten = fsUploadFile.print(0);
        fsUploadFile.close();
    }
    LittleFS.remove(UPDATESYS);
    LittleFS.end();
}

void update_error(int err)
{
    char line[120];
    sprintf(line, "Firmware Update error code %d: %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    debugLog(UPDATELOG, line);
    Serial.printf("*** SYSINFO:  Firmware update error code %d\n", err);
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
}
