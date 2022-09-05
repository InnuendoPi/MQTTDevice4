bool upTools(String url, String fname)
{
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    clientup->setInsecure();
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
                fsUploadFile = LittleFS.open("/" + fname, "w");
                if (!fsUploadFile)
                {
                    Serial.println("Cancel update!");
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
                    }
                    fsUploadFile.write(buff, c);
                    if (len > 0)
                    {
                        len -= c;
                    }
                }

                fsUploadFile.close();
                Serial.printf("*** SYSINFO: %s update finished.\n", fname.c_str());
                // return true;
            }
            else
                return false;
        }
        else
        {
            Serial.println("Cancel Update!");
            Serial.printf("*** SYSINFO: error update %s: %s\n", fname, https.errorToString(httpCode).c_str());
            https.end();
            return false;
        }
        https.end();
        return true;
    }
    return true;
}

void upCerts()
{
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
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
                uint8_t buff[128] = {0};
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
                    // read up to 128 byte
                    int c = clientup->readBytes(buff, std::min((size_t)len, sizeof(buff)));
                    // Serial.printf("readBytes: %d\n", c);
                    if (!c)
                    {
                        Serial.println("read timeout");
                    }
                    fsUploadFile.write(buff, c);
                    if (len > 0)
                    {
                        len -= c;
                    }
                }

                fsUploadFile.close();
                Serial.println("*** SYSINFO: Certs update finished.");
                bool check = LittleFS.remove("/update.txt");
            }
            else
                return;
        }
        else
        {
            Serial.println("Cancel Update!");
            Serial.printf("*** SYSINFO: error update certs: %s\n", https.errorToString(httpCode).c_str());
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
    // ESPhttpUpdate.onProgress(update_progress);
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

void updateTools()
{
    if (LittleFS.exists("/updateTools.txt"))
    {
        fsUploadFile = LittleFS.open("/updateTools.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/updateTools.txt");
            Serial.println("*** SYSINFO: ERROR update firmware");
            return;
        }
        fsUploadFile = LittleFS.open("/updateTools.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/logtools.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update tools started - free heap: ");
        Serial.println(ESP.getFreeHeap());
        String url;
        if (devBranch)
            url = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/";
        else
            url = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/";

        bool test = upTools(url, "bootstrap.min.css");
        if (test)
            test = upTools(url, "fontawesome-webfont.woff");
        if (test)
            test = upTools(url, "jquery.min.js");
        if (test)
            test = upTools(url, "fontawesome-webfont.woff2");
        if (test)
            test = upTools(url, "bootstrap.min.js");
        if (test)
            test = upTools(url, "font-awesome.min.css");
        if (test)
            test = upTools(url, "jquery.tabletojson.min.js");

        test = LittleFS.remove("/updateTools.txt");
        cbpiEventSystem(EM_REBOOT);
    }
}
void updateSys()
{
    if (LittleFS.exists("/updateSys.txt"))
    {
        fsUploadFile = LittleFS.open("/updateSys.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/updateSys.txt");
            Serial.println("*** SYSINFO: ERROR update firmware");
            return;
        }
        fsUploadFile = LittleFS.open("/updateSys.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/logsys.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO: Update firmware started - free heap: ");
        Serial.println(ESP.getFreeHeap());
        upCerts();
        upFirm();
    }
}

void startToolsUpdate()
{
    fsUploadFile = LittleFS.open("/updateTools.txt", "w");
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
        // url = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/development/data/";

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
    else
    {
        bool check = LittleFS.remove("/dev.txt");
        // url = "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/";
    }
    cbpiEventSystem(EM_REBOOT);
}

void startHTTPUpdate()
{
    // Starte Updates
    fsUploadFile = LittleFS.open("/updateSys.txt", "w");
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
    else
    {
        if (LittleFS.exists("/dev.txt"))
            bool check = LittleFS.remove("/dev.txt");
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
