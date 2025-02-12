void setup()
{
  Serial.begin(DEF_SERIAL);

#ifdef ESP32
  snprintf(mqtt_clientid, maxHostSign, "ESP32-%llX", ESP.getEfuseMac());
  DEBUG_INFO("SYS", "\n*** SYSINFO: MQTTDevice32 ID: %X", mqtt_clientid);
  // WLAN Events
  WiFi.onEvent(WiFiEvent);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                                       { DEBUG_INFO("SYS", "WiFiStationDisconnected reason: %d", info.wifi_sta_disconnected.reason); },
                                       WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#elif ESP8266
  snprintf(mqtt_clientid, maxHostSign, "ESP8266-%08X", ESP.getChipId());
  DEBUG_INFO("SYS", "\n*** SYSINFO: start up MQTTDevice - device ID: %s", mqtt_clientid);
  // WLAN Events
  wifiConnectHandler = WiFi.onStationModeGotIP(EM_WIFICONNECT);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(EM_WIFIDISCONNECT);
#endif

  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter p_hint("<small>*Connect your MQTTDevice to WLAN. When connected open http://mqttdevice.local in your brower</small>");
  wifiManager.addParameter(&p_hint);
  wifiManager.autoConnect(nameMDNS);
  wifiManager.setWiFiAutoReconnect(true);
  WiFi.mode(WIFI_STA);
#ifdef ESP32
  WiFi.setSleep(false);
#elif ESP8266
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // Lade Dateisystem
  if (LittleFS.begin())
  {
    // set Logging
    readLog();
    DEBUG_INFO("SYS", "*** SYSINFO: setup LittleFS free heap: %d", ESP.getFreeHeap());

    // Prüfe WebUpdate
    if (LittleFS.exists(UPDATESYS))
      updateSys();
    if (LittleFS.exists(UPDATETOOLS))
      updateTools();
    // webUpdate log
    if (LittleFS.exists(LOGUPDATESYS) || LittleFS.exists(LOGUPDATETOOLS))
      EM_LOG();

    // Erstelle Ticker Objekte
    setTicker();

// ISR
#ifdef ESP32
    gpio_install_isr_service(0);
#endif

    // Starte Sensoren
    DS18B20.begin();
    DS18B20.setWaitForConversion(false); // async mode
    DS18B20.setCheckForConversion(false);
    pins_used[ONE_WIRE_BUS] = true;
    // Lade Konfiguration
    if (LittleFS.exists(CONFIG))
      loadConfig();
    else
    {
      DEBUG_INFO("SYS", "*** SYSINFO: config file config.txt missing. Load defaults ...");
      // NTP
      setupTime();
    }
    // Starte MQTT
    EM_MQTTCONNECT();
    EM_MQTTSUBSCRIBE();
    TickerPUBSUB.start(); // Ticker PubSubClient
  }
  else
    DEBUG_ERROR("SYS", "*** SYSINFO: error - cannot mount LittleFS!");

  // Starte Webserver
  setupServer();

  // Starte mDNS
  if (startMDNS)
    EM_MDNSET();
  else
    DEBUG_INFO("SYS", "*** SYSINFO: ESP8266 IP address: %s Time: %s RSSI: %d", WiFi.localIP().toString().c_str(), zeit, WiFi.RSSI());
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
  server.on("/reqPins", handlereqPins);               // GPIO Pins actors
  server.on("/reqIndu", handleRequestIndu);           // Induction für WebConfig
  server.on("/setSensor", handleSetSensor);           // Sensor ändern
  server.on("/setSenErr", handleSetSenErr);           // Sensor ändern
  server.on("/setActor", handleSetActor);             // Aktor ändern
  server.on("/setIndu", handleSetIndu);               // Indu ändern
  server.on("/delSensor", handleDelSensor);           // Sensor löschen
  server.on("/delActor", handleDelActor);             // Aktor löschen
  server.on("/reboot", EM_REBOOT);                    // reboots the whole Device
  server.on("/reqMisc", handleRequestMisc);           // Misc Infos für WebConfig
  server.on("/reqMisc2", handleRequestMisc2);         // Misc Infos für WebConfig
  server.on("/reqMiscAlert", handleRequestMiscAlert); // Misc Infos für WebConfig
  server.on("/reqFirm", handleRequestFirm);           // Firmware version
  server.on("/setMisc", handleSetMisc);               // Misc ändern
  server.on("/setMiscLang", handleSetMiscLang);       // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate);     // Firmware WebUpdate
  server.on("/channel", handleChannel);               // Server Sent Events will be handled from this URI
  server.on("/startSSE", startSSE);                   // Server Sent Events will be handled from this URI
  server.on("/checkAliveSSE", checkAliveSSE);         // Server Sent Events check IP on channel
  server.on("/language", handleGetLanguage);
  server.on("/title", handleGetTitle);
  server.on("/reqSys", handleReqSys);
  // FSBrowser initialisieren
  server.on("/edit", HTTP_GET, handleGetEdit);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on(
      "/edit", HTTP_POST, []()
      { replyOK(); },
      handleFileUpload);
  server.on(
      "/restore", HTTP_POST, []()
      { replyOK(); },
      handleRestore);

  server.on("/rest/events/0", handleAll);
  server.on("/rest/events/1", handleAll);
  server.on("/rest/events/2", handleAll);
  server.on("/rest/events/3", handleAll);
  server.on("/rest/events/4", handleAll);
  server.on("/rest/events/5", handleAll);

  server.on("/mqttdevice.min.css", handleAll);
  server.on("/mqttdevice.min.js", handleAll);
  server.on("/mqttdevice.ttf", handleAll);
  server.on("/favicon.ico", handleAll);
  server.on("/de.json", handleAll);
  server.on("/en.json", handleAll);
  server.on("/config.txt", handleAll);
  server.on("/log_cfg.json", handleAll);

  server.serveStatic("/mqttdevice.min.css", LittleFS, "/mqttdevice.min.css", "public, max-age=86400");
  server.serveStatic("/mqttdevice.min.js", LittleFS, "/mqttdevice.min.js", "public, max-age=86400");
  server.serveStatic("/mqttfont.ttf", LittleFS, "/mqttfont.ttf", "public, max-age=86400");
  server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico", "public, max-age=86400");
  server.serveStatic("/de.json", LittleFS, "/de.json", "no-store, must-revalidate");
  server.serveStatic("/en.json", LittleFS, "/en.json", "no-store, must-revalidate");
  server.serveStatic("/config.txt", LittleFS, "/config.txt", "no-cache, no-store, must-revalidate");
  server.serveStatic("/log_cfg.json", LittleFS, "/log_cfg.json", "no-cache, no-store, must-revalidate");

  server.onNotFound(handleAll);
#ifdef ESP32
  httpUpdateServer.setup(&server); // DateiUpdate
#elif ESP8266
  httpUpdate.setup(&server);
#endif
  server.begin();
}
