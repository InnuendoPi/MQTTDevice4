void setup()
{
  nextion.begin(softSerial);
  // nextion.debug(Serial);
  Serial.begin(115200);

// Debug Ausgaben prüfen
#ifdef DEBUG_ESP_PORT
  Serial.setDebugOutput(true);
#endif

  Serial.println();
  Serial.println();
  // Setze Namen für das MQTTDevice
  snprintf(mqtt_clientid, 16, "ESP8266-%08X", ESP.getChipId());
  Serial.printf("*** SYSINFO: Starte MQTTDevice %s\n", mqtt_clientid);

  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter cstm_mqtthost("host", "MQTT Server IP (CBPi)", mqtthost, 16);
  WiFiManagerParameter p_hint("<small>*Sobald das MQTTDevice mit deinem WLAN verbunden ist, öffne im Browser http://mqttdevice </small>");
  wifiManager.addParameter(&cstm_mqtthost);
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
    Serial.printf("*** SYSINFO Starte Setup LittleFS Free Heap: %d\n", ESP.getFreeHeap());

    // Prüfe WebUpdate
    updateSys();

    // Erstelle Ticker Objekte
    setTicker();

    // Starte NTP
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();
    TickerNTP.start();

    if (shouldSaveConfig) // WiFiManager
    {
      strcpy(mqtthost, cstm_mqtthost.getValue());
      saveConfig();
    }

    if (LittleFS.exists("/config.txt")) // Lade Konfiguration
      loadConfig();
    else
      Serial.println("*** SYSINFO: Konfigurationsdatei config.txt nicht vorhanden. Setze Standardwerte ...");
  }
  else
    Serial.println("*** SYSINFO: Fehler - Dateisystem LittleFS konnte nicht eingebunden werden!");

  // Lege Event Queues an
  gEM.addListener(EventManager::kEventUser0, listenerSystem);
  gEM.addListener(EventManager::kEventUser1, listenerSensors);
  gEM.addListener(EventManager::kEventUser2, listenerActors);
  gEM.addListener(EventManager::kEventUser3, listenerInduction);

  // Starte Webserver
  setupServer();
  // Pinbelegung
  pins_used[ONE_WIRE_BUS] = true;

  // Starte Sensoren
  DS18B20.begin();

  // Starte mDNS
  if (startMDNS)
    cbpiEventSystem(EM_MDNSET);
  else
  {
    Serial.printf("*** SYSINFO: ESP8266 IP Addresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }

  if (useDisplay)
  {
    pins_used[D1] = true;
    pins_used[D2] = true;
    TickerDisp.start();
    initDisplay();
  }

  if (startBuzzer)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
  }

  // Starte MQTT
  cbpiEventSystem(EM_MQTTCON); // MQTT Verbindung
  cbpiEventSystem(EM_MQTTSUB); // MQTT Subscribe

  cbpiEventSystem(EM_LOG); // webUpdate log

  // Verarbeite alle Events Setup
  gEM.processAllEvents();
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);
  server.on("/reqIndu", handleRequestIndu); // Infos der Indu für WebConfig
  server.on("/setSensor", handleSetSensor); // Sensor ändern
  server.on("/setActor", handleSetActor);   // Aktor ändern
  server.on("/setIndu", handleSetIndu);     // Indu ändern
  server.on("/delSensor", handleDelSensor); // Sensor löschen
  server.on("/delActor", handleDelActor);   // Aktor löschen
  server.on("/reboot", rebootDevice);       // reboots the whole Device
  server.on("/reqMisc2", handleRequestMisc2); // Misc Infos für WebConfig
  server.on("/reqMisc", handleRequestMisc); // Misc Infos für WebConfig
  server.on("/reqFirm", handleRequestFirm);
  server.on("/setMisc", handleSetMisc);           // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate); // Firmware WebUpdate

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList); // Verzeichnisinhalt
  server.on("/edit", HTTP_GET, []() {           // Lade Editor
    if (!handleFileRead("/edit.htm"))
    {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate);    // Datei erstellen
  server.on("/edit", HTTP_DELETE, handleFileDelete); // Datei löschen
  server.on(
      "/edit", HTTP_POST, []() { server.send(200, "text/plain", ""); },
      handleFileUpload);

  server.onNotFound(handleWebRequests); // Sonstiges

  httpUpdate.setup(&server);
  server.begin();
}
