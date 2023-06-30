bool upTools(String url, String fname)
{
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    clientup->setInsecure();
    clientup->setBufferSizes(1024, 1024); // ohne setBufferSize exc OOM!
    HTTPClient https;
    if (https.begin(*clientup, url + fname))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // Serial.printf("*** SYSINFO: [HTTPS] GET certs.ar Antwort: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK)
            {
                int len = https.getSize();
                uint8_t buff[128] = {0};
                bool check = LittleFS.remove("/" + fname);
                if (!check)
                    Serial.printf("*** SYSINFO: error remove %s\n", fname.c_str());

                fsUploadFile = LittleFS.open("/" + fname, "w");
                if (!fsUploadFile)
                {
                    Serial.printf("*** SYSINFO: error write %s\n", fname.c_str());
                    https.end();
                    return false;
                }
                while (https.connected() && (len > 0 || len == -1))
                {
                    // read up to 128 byte
                    int c = clientup->readBytes(buff, std::min((size_t)len, sizeof(buff)));
                    // Serial.printf("readBytes: %d\n", c);
                    if (!c)
                    {
                        Serial.println("read timeout");
                        https.end();
                        return false;
                    }
                    fsUploadFile.write(buff, c);
                    if (len > 0)
                    {
                        len -= c;
                    }
                }

                fsUploadFile.close();
                // bool check = LittleFS.remove("/" + fname);
                // check = LittleFS.rename("/" + fname + ".new", "/" + fname);

                Serial.printf("*** SYSINFO: %s update finished.\n", fname.c_str());
                https.end();
                return true;
            }
            else
                return false;
        }
        else
        {
            // Serial.printf("*** SYSINFO: error update %s: %s\n", fname, https.errorToString(httpCode).c_str());
            Serial.printf("*** SYSINFO: error update %s\n", fname);
            https.end();
            return false;
        }
    }
    else
        return false;
}
void upFirm()
{
    BearSSL::CertStore certStore;
    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR(CERT));
    // Serial.print(F("Number of CA certs read: "));
    // Serial.println(numCerts);
    if (numCerts == 0)
    {
        // Serial.println(F("*** SYSINFO: No certs found. Did you run certs-from-mozill.py and upload the LittleFS directory before running?"));
        return; // Can't connect to anything w/o certs!
    }

    BearSSL::WiFiClientSecure clientFirm;
    clientFirm.setBufferSizes(1024, 1024); // ohne setBufferSize exc OOM!
    clientFirm.setCertStore(&certStore);
    clientFirm.setInsecure();

    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret;
    if (LittleFS.exists(DEVBRANCH))
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

void updateTools()
{
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
        Serial.printf("*** SYSINFO: Update tools #%d started - free heap: ", anzahlVersuche);
        Serial.println(ESP.getFreeHeap());
        bool test;
        if (LittleFS.exists(DEVBRANCH))
        {
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttfont.ttf");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.css");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "mqttdevice.min.js");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/", "favicon.ico");
        }
        else
        {
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttfont.ttf");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.css");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "mqttdevice.min.js");
            test = upTools("https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/", "favicon.ico");
        }

        LittleFS.remove(UPDATETOOLS);
        LittleFS.end();
        Serial.print("*** SYSINFO: Update tools finished\n");
        ESP.restart();
    }
}
void updateSys()
{
    if (LittleFS.exists(UPDATESYS))
    {
        fsUploadFile = LittleFS.open(LOGUPDATESYS , "r");

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
        fsUploadFile = LittleFS.open(LOGUPDATESYS , "w");
        anzahlVersuche++;
        int bytesWritten = fsUploadFile.print(anzahlVersuche);
        fsUploadFile.close();
        Serial.printf("*** SYSINFO: Update firmware #%d started - free heap: ", anzahlVersuche);
        Serial.println(ESP.getFreeHeap());
        if (LittleFS.exists(CERT))
            upFirm();
    }
}

void startToolsUpdate()
{
    server.send_P(200, "text/plain", "ok");
    millis2wait(1000);
    fsUploadFile = LittleFS.open(UPDATETOOLS, "w");
    if (!fsUploadFile)
    {
        Serial.println("*** Error WebUpdate tools create file (LittleFS)");
        return;
    }
    else
    {
        Serial.println("*** WebUpdate tools create file (LittleFS)");
        uint8_t bytesWritten = fsUploadFile.print(0);
        fsUploadFile.close();
    }

    if (devBranch)
    {
        fsUploadFile = LittleFS.open(DEVBRANCH, "w");
        if (!fsUploadFile)
        {
            Serial.println("*** Error WebUpdate tools dev create file (LittleFS)");
            return;
        }
        else
        {
            Serial.println("*** WebUpdate tools dev create file (LittleFS)");
            uint8_t bytesWritten = fsUploadFile.print("0");
            fsUploadFile.close();
        }
    }
    else
    {
        bool check = LittleFS.remove(DEVBRANCH);
        // url = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/";
    }
    Serial.println("*** WebUpdate tools reboot");
    LittleFS.end(); // unmount LittleFS
    millis2wait(1000);
    ESP.restart();
}

void startHTTPUpdate()
{
    server.send_P(200, "text/plain", "ok");
    millis2wait(1000);
    fsUploadFile = LittleFS.open(UPDATESYS, "w");
    if (!fsUploadFile)
    {
        Serial.println("*** Error WebUpdate firmware create file (LittleFS)");
        return;
    }
    else
    {
        Serial.println("*** WebUpdate firmware create file (LittleFS)");
        int bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
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
            Serial.println("*** WebUpdate firmware dev create file (LittleFS)");
            int bytesWritten = fsUploadFile.print("0");
            fsUploadFile.close();
        }
    }
    else
    {
        if (LittleFS.exists(DEVBRANCH))
            bool check = LittleFS.remove(DEVBRANCH);
    }
    Serial.println("*** WebUpdate firmware reboot");
    LittleFS.end();
    millis2wait(1000);
    ESP.restart();
}

void update_finished()
{
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
    Serial.printf("*** SYSINFO:  Firmware update error code %d\n", err);
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
}
