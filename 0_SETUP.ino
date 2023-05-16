void setup()
{
  Serial.begin(115200);
  // Serial.begin(9600);
  // Debug Ausgaben prüfen
#ifdef DEBUG_ESP_PORT
  Serial.setDebugOutput(true);
#endif

  snprintf(mqtt_clientid, maxHostSign, "ESP8266-%08X", ESP.getChipId());
  Serial.printf("\n*** SYSINFO: start up MQTTDevice - device ID: %s\n", mqtt_clientid);
  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter p_hint("<small>*Connect your MQTTDevice to WLAN. When connected open http://mqttdevice in your brower</small>");
  wifiManager.addParameter(&p_hint);
  wifiManager.autoConnect(mqtt_clientid);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // Lade Dateisystem
  if (LittleFS.begin())
  {
    Serial.printf("*** SYSINFO: setup LittleFS free heap: %d\n", ESP.getFreeHeap());

    // Prüfe WebUpdate
    updateSys();
    updateTools();

    // Erstelle Ticker Objekte
    setTicker();
    // Starte NTP
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();
    TickerNTP.start();

    if (LittleFS.exists(CONFIG)) // Lade Konfiguration
      loadConfig();
    else
      Serial.println("*** SYSINFO: config file config.txt missing. Load defaults ...");
  }
  else
    Serial.println("*** SYSINFO: error - cannot mount LittleFS!");

  // Starte Webserver
  setupServer();

  if (useDisplay)
  {
    softSerial.begin(9600, SWSERIAL_8N1, D1, D2, false);
    if (softSerial)
    {
      Serial.println("*** SYSINFO: SoftwareSerial init successful");
      pins_used[D1] = true;
      pins_used[D2] = true;
      nextion.begin(softSerial);
      // nextion.debug(Serial);
      initDisplay();
    }
    else
    {
      Serial.println("*** SYSINFO: SoftwareSerial init error");
      if (startBuzzer)
      {
        sendAlarm(ALARM_ERROR);
      }
    }
  }
  if (useI2C)
  {
    if (digitalRead(PIN_SCL) == HIGH && digitalRead(PIN_SDA) == LOW)
      PCF_Reset();
    if (pcf020.begin(D5, D6))
    {
      Serial.printf("*** SYSINFO: PCF8574 init successful lib version %s\n", PCF8574_LIB_VERSION);
      pins_used[D5] = true;
      pins_used[D6] = true;
      // pcf8574.selectNone(); // set all PCF8574 pins to low
      statePCF = true;
    }
    else
    {
      Serial.println("*** SYSINFO: PCF8574 init error");
      statePCF = false;
      if (startBuzzer)
        sendAlarm(ALARM_ERROR);
    }
  }

  // Pinbelegung
  pins_used[ONE_WIRE_BUS] = true;

  // Starte Sensoren
  DS18B20.begin();

  // Starte mDNS
  if (startMDNS)
    EM_MDNSET();
  else
  {
    Serial.printf("*** SYSINFO: ESP8266 IP address: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }

  EM_MQTTCON();
  EM_MQTTSUB();
  TickerPUBSUB.start(); // PubSubClient loop ticker
  EM_LOG();             // webUpdate log
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/index.htm", handleRoot);
  server.on("/index", handleRoot);
  server.on("/index.html", handleRoot);
  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);       // GPIO Pins actors
  server.on("/reqPages", handleRequestPages); // Display page
  server.on("/reqIndu", handleRequestIndu);   // Induction für WebConfig
  server.on("/setSensor", handleSetSensor);   // Sensor ändern
  server.on("/setActor", handleSetActor);     // Aktor ändern
  server.on("/setIndu", handleSetIndu);       // Indu ändern
  server.on("/delSensor", handleDelSensor);   // Sensor löschen
  server.on("/delActor", handleDelActor);     // Aktor löschen
  server.on("/reboot", rebootDevice);         // reboots the whole Device
  server.on("/reqMisc", handleRequestMisc);         // Misc Infos für WebConfig
  server.on("/reqMisc2", handleRequestMisc2); // Misc Infos für WebConfig
  server.on("/reqMisc3", handleRequestMisc3); // Misc Infos für WebConfig
  server.on("/reqFirm", handleRequestFirm);         // Firmware version
  server.on("/setMisc", handleSetMisc);             // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate);   // Firmware WebUpdate
  server.on("/startToolsUpdate", startToolsUpdate); // Firmware WebUpdate
  server.on("/channel", handleChannel);   //Server Sent Events will be handled from this URI
  server.on("/startSSE", startSSE);   //Server Sent Events will be handled from this URI
  // FSBrowser initialisieren
  server.on("/edit", HTTP_GET, handleGetEdit);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on(
      "/edit", HTTP_POST, []()
      { server.send(200, "text/plain", ""); },
      handleFileUpload);
  server.on(
      "/restore", HTTP_POST, []()
      { server.send(200, "text/plain", ""); },
      handleRestore);
  server.onNotFound(handleAll);
  // server.onNotFound(handleWebRequests);
  httpUpdate.setup(&server);
  server.begin();
}
