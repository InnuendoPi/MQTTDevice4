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
  // WLAN Events
  wifiConnectHandler = WiFi.onStationModeGotIP(EM_WIFICONNECT);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(EM_WIFIDISCONNECT);

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

    // Starte NTP
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();

    // Prüfe WebUpdate
    if (LittleFS.exists(UPDATESYS))
      updateSys();
    if (LittleFS.exists(UPDATETOOLS))
      updateTools();

    // Erstelle Ticker Objekte
    setTicker();

    // Starte Sensoren
    DS18B20.begin();
    pins_used[ONE_WIRE_BUS] = true;

    if (LittleFS.exists(CONFIG)) // Lade Konfiguration
      loadConfig();
    else
      Serial.println("*** SYSINFO: config file config.txt missing. Load defaults ...");
  }
  else
    Serial.println("*** SYSINFO: error - cannot mount LittleFS!");

  // Starte Webserver
  setupServer();

  // Starte mDNS
  if (startMDNS)
    EM_MDNSET();
  else
    Serial.printf("*** SYSINFO: ESP8266 IP address: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());

  // MQTT
  pubsubClient.setBufferSize(512);
  EM_MQTTCONNECT();
  EM_MQTTSUBSCRIBE();
  TickerPUBSUB.start(); // PubSubClient loop ticker
  // webUpdate log
  EM_LOG();
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
  server.on("/reqPages", handleRequestPages);         // Display page
  server.on("/reqIndu", handleRequestIndu);           // Induction für WebConfig
  server.on("/setSensor", handleSetSensor);           // Sensor ändern
  server.on("/setActor", handleSetActor);             // Aktor ändern
  server.on("/setIndu", handleSetIndu);               // Indu ändern
  server.on("/delSensor", handleDelSensor);           // Sensor löschen
  server.on("/delActor", handleDelActor);             // Aktor löschen
  server.on("/reboot", rebootDevice);                 // reboots the whole Device
  server.on("/reqMisc", handleRequestMisc);           // Misc Infos für WebConfig
  server.on("/reqMisc2", handleRequestMisc2);         // Misc Infos für WebConfig
  server.on("/reqMiscAlert", handleRequestMiscAlert); // Misc Alert WebUpdate
  server.on("/reqFirm", handleRequestFirm);           // Firmware version
  server.on("/setMisc", handleSetMisc);               // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate);     // Firmware WebUpdate
  server.on("/channel", handleChannel);               // Server Sent Events will be handled from this URI
  server.on("/startSSE", startSSE);                   // Server Sent Events will be handled from this URI
  server.on("/checkAliveSSE", checkAliveSSE);         // Server Sent Events check IP on channel
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
