bool loadConfig()
{
#ifdef ESP32
  log_e("%s", "------ loadConfig started ------");
#endif
  File configFile = LittleFS.open(CONFIG, "r");
  if (!configFile)
  {
#ifdef ESP32
    log_e("%s", "Failed to open config file");
    log_e("%s", "------ loadConfig aborted ------");
#endif
    return false;
  }

  size_t size = configFile.size();
  if (size > 2048)
  {
#ifdef ESP32
    log_e("%s", "Config file size is too large");
    log_e("%s", "------ loadConfig aborted ------");
#endif
    return false;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
#ifdef ESP32
    log_e("Conf: Error Json %s", error.c_str());
#endif
    return false;
  }

  // Misc Settings
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  wait_on_Sensor_error_actor = miscObj["del_sen_act"] | 120000;
  wait_on_Sensor_error_induction = miscObj["del_sen_ind"] | 120000;

  StopOnMQTTError = miscObj["enable_mqtt"] | 0;
  wait_on_error_mqtt = miscObj["delay_mqtt"] | 120000;

  startBuzzer = miscObj["buzzer"] | 0;
  if (startBuzzer)
    mqttBuzzer = miscObj["mqbuz"] | 0;
  else
    mqttBuzzer = false;

  senRes = miscObj["res"] | 0;
  useDisplay = miscObj["display"] | 0;
  startPage = miscObj["page"] | 0;
  devBranch = miscObj["devbranch"] | 0;
  strlcpy(nameMDNS, miscObj["mdns_name"] | "", maxHostSign);
  startMDNS = miscObj["mdns"] | 0;
  strlcpy(mqtthost, miscObj["MQTTHOST"] | "", maxHostSign);
  strlcpy(mqttuser, miscObj["MQTTUSER"] | "", maxUserSign);
  strlcpy(mqttpass, miscObj["MQTTPASS"] | "", maxPassSign);
  mqttport = miscObj["MQTTPORT"] | 1883;
  strlcpy(ntpServer, miscObj["ntp"] | NTP_ADDRESS, maxHostSign);
  startSPI = miscObj["spi"] | 0;
  selLang = miscObj["lang"] | 0;
#ifdef ESP32
  log_e("Wait on sensor error actors: %d sec", wait_on_Sensor_error_actor / 1000);
  log_e("Wait on sensor error induction: %d sec", wait_on_Sensor_error_induction / 1000);
  log_e("Switch off actors on MQTT error: %d after %d sec", StopOnMQTTError, (wait_on_error_mqtt / 1000));
  log_e("Buzzer: %d mqttBuzzer: %d", startBuzzer, mqttBuzzer);
  log_e("Display: %d startPage: %d", useDisplay, startPage);
  log_e("mDNS: %d name: %s", startMDNS, nameMDNS);
  log_e("MQTT server IP: %s Port: %d User: %s Pass: %s", mqtthost, mqttport, mqttuser, mqttpass);
  log_e("%s", "--------------------");
#endif
  // read actors
  JsonArray actorsArray = doc["actors"];
  numberOfActors = actorsArray.size();
  if (numberOfActors > NUMBEROFACTORSMAX)
    numberOfActors = NUMBEROFACTORSMAX;
  int i = 0;
  for (JsonObject actorObj : actorsArray)
  {
    if (i < numberOfActors)
    {
      actors[i].change(StringToPin(actorObj["PIN"]), actorObj["SCRIPT"], actorObj["NAME"], actorObj["INV"], actorObj["SW"]);
#ifdef ESP32
      log_e("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d", (i + 1), actorObj["NAME"].as<const char *>(), actorObj["SCRIPT"].as<const char *>(), actorObj["PIN"].as<const char *>(), actorObj["INV"].as<int>(), actorObj["SW"].as<int>());
#endif
      i++;
    }
  }

  if (numberOfActors == 0)
  {
#ifdef ESP32
    log_e("Actors: %d", numberOfActors);
#endif
  }
#ifdef ESP32
  log_e("%s", "--------------------");
#endif
  // read sensors
  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > NUMBEROFSENSORSMAX)
    numberOfSensors = NUMBEROFSENSORSMAX;
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      sensors[i].change(sensorsObj["ADDRESS"], sensorsObj["SCRIPT"], sensorsObj["NAME"], sensorsObj["CBPIID"], sensorsObj["OFFSET1"], sensorsObj["OFFSET2"], sensorsObj["SW"], sensorsObj["TYPE"], sensorsObj["PIN"]);
#ifdef ESP32
      log_e("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Type: %d Pin: %d", (i + 1), sensorsObj["NAME"].as<const char *>(), sensorsObj["ADDRESS"].as<const char *>(), sensorsObj["SCRIPT"].as<const char *>(), sensorsObj["CBPIID"].as<const char *>(), sensorsObj["OFFSET1"].as<float>(), sensorsObj["OFFSET2"].as<float>(), sensorsObj["SW"].as<int>(), sensorsObj["TYPE"].as<int>(), sensorsObj["PIN"].as<int>());
#endif
      i++;
    }
    else
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
  }
  if (startSPI)
    setupPT();
#ifdef ESP32
  log_e("%s", "--------------------");
#endif
  // read induction
  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  inductionCooker.change(StringToPin(indObj["PINWHITE"]), StringToPin(indObj["PINYELLOW"]), StringToPin(indObj["PINBLUE"]), indObj["TOPIC"], indObj["ENABLED"], indObj["PL"]);
  /*
  inductionStatus = indObj["ENABLED"] | 0;
  // inductionCooker.setIsEnabled(inductionStatus);
  
  if (inductionStatus)
  {
    inductionCooker.change(StringToPin(indObj["PINWHITE"]), StringToPin(indObj["PINYELLOW"]), StringToPin(indObj["PINBLUE"]), indObj["TOPIC"], true, indObj["PL"]);
#ifdef ESP32
    log_e("Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d", inductionStatus, indObj["TOPIC"].as<const char *>(), indObj["PINWHITE"].as<const char *>(), indObj["PINYELLOW"].as<const char *>(), indObj["PINBLUE"].as<const char *>(), indObj["PL"].as<int>());
#endif
  }
  else
  {
    inductionStatus = 0;
  }
  */
#ifdef ESP32
  log_e("%s", "--------------------");
  log_e("%s", "------ loadConfig finished ------");
#endif
  configFile.close();

  // Setze NTP Server
  if (ntpServer[0] != '\0') // leeres char array
    timeClient.setPoolServerName(ntpServer);

  if (numberOfSensors > 0) // Ticker Sensors
    TickerSen.start();

  if (numberOfActors > 0) // Ticker Actors
    TickerAct.start();

  if (inductionStatus > 0) // Ticker Induction
    TickerInd.start();

  if (!useDisplay) // Ticker Display
    TickerDisp.stop();
  else
  {
    softSerial.begin(9600, SWSERIAL_8N1, D1, D2, false);
#ifdef ESP32
    log_e("SoftwareSerial init %d", int(softSerial));
#endif
    pins_used[D1] = true;
    pins_used[D2] = true;
    nextion.begin(softSerial);
    initDisplay();
    TickerDisp.start();
  }
  size_t len = measureJson(doc);
  int memoryUsed = doc.memoryUsage();
#ifdef ESP32
  log_e("Config file size %d", size);
  log_e("JSON config length: %d", len);
  log_e("JSON memory usage: %d", memoryUsed);
  log_e("%s", "--------------------");
#endif
  if (startBuzzer)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    sendAlarm(ALARM_ON);
  }

  return true;
}

void saveConfigCallback()
{
  shouldSaveConfig = true;
  if (LittleFS.begin())
  {
    // saveConfig();
    shouldSaveConfig = true;
  }
}

bool saveConfig()
{
#ifdef ESP32
  log_e("%s", "------ saveConfig started ------");
#endif
  DynamicJsonDocument doc(2500);

  // Write Actors
  JsonArray actorsArray = doc.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = actorsArray.createNestedObject();
    actorsObj["PIN"] = PinToString(actors[i].getPinActor());
    actorsObj["NAME"] = actors[i].getActorName();
    actorsObj["SCRIPT"] = actors[i].getActorTopic();
    actorsObj["INV"] = (int)actors[i].getInverted();
    actorsObj["SW"] = (int)actors[i].getActorSwitch();
#ifdef ESP32
    log_e("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d", (i + 1), actors[i].getActorName().c_str(), actors[i].getActorTopic().c_str(), PinToString(actors[i].getPinActor()), actors[i].getInverted(), actors[i].getActorSwitch());
#endif
  }
#ifdef ESP32
  log_e("%s", "--------------------");
#endif
  // Write Sensors
  JsonArray sensorsArray = doc.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = sensorsArray.createNestedObject();
    sensorsObj["ADDRESS"] = sensors[i].getSens_adress_string();
    sensorsObj["NAME"] = sensors[i].getSensorName();
    sensorsObj["OFFSET1"] = (int(sensors[i].getOffset1() * 100)) / 100.0;
    sensorsObj["OFFSET2"] = (int(sensors[i].getOffset2() * 100)) / 100.0;
    sensorsObj["SCRIPT"] = sensors[i].getSensorTopic();
    sensorsObj["CBPIID"] = sensors[i].getId();
    sensorsObj["SW"] = (int)sensors[i].getSensorSwitch();
    sensorsObj["TYPE"] = sensors[i].getSensType();
    sensorsObj["PIN"] = sensors[i].getSensPin();
#ifdef ESP32
    log_e("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Typ: %d Pin: %d", (i + 1), sensors[i].getSensorName().c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].getSensorTopic().c_str(), sensors[i].getId().c_str(), sensors[i].getOffset1(), sensors[i].getOffset2(), sensors[i].getSensorSwitch(), sensors[i].getSensType(), , sensors[i].getSensPin());
#endif
  }
#ifdef ESP32
  log_e("%s", "--------------------");
#endif

  // Write Induction
  // inductionStatus = 0;
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.getIsEnabled())
  {
    // inductionStatus = 1;
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.getPinWhite());
    indObj["PINYELLOW"] = PinToString(inductionCooker.getPinYellow());
    indObj["PINBLUE"] = PinToString(inductionCooker.getPinInterrupt());
    indObj["ENABLED"] = (int)inductionCooker.getIsEnabled();
    indObj["TOPIC"] = inductionCooker.getTopic();
    indObj["PL"] = inductionCooker.getPowerLevelOnError();
#ifdef ESP32
    log_e("Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d", inductionCooker.getIsEnabled(), inductionCooker.getTopic().c_str(), PinToString(inductionCooker.getPinWhite()).c_str(), PinToString(inductionCooker.getPinYellow()).c_str(), PinToString(inductionCooker.getPinInterrupt()).c_str(), inductionCooker.getPowerLevelOnError());
#endif
  }
#ifdef ESP32
  log_e("%s", "--------------------");
#endif
  // Write Misc Stuff
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  miscObj["delay_mqtt"] = wait_on_error_mqtt;
  miscObj["enable_mqtt"] = (int)StopOnMQTTError;
  miscObj["buzzer"] = (int)startBuzzer;
  if (startBuzzer)
    miscObj["mqbuz"] = (int)mqttBuzzer;
  else
    miscObj["mqbuz"] = 0;

  miscObj["res"] = (int)senRes;
  miscObj["display"] = (int)useDisplay;
  miscObj["page"] = startPage;
  miscObj["devbranch"] = (int)devBranch;
  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["MQTTPORT"] = mqttport;
  miscObj["MQTTUSER"] = mqttuser;
  miscObj["MQTTPASS"] = mqttpass;
  miscObj["ntp"] = ntpServer;
  miscObj["spi"] = (int)startSPI;
  miscObj["lang"] = selLang;
  miscObj["VER"] = Version;

#ifdef ESP32
  log_e("Wait on sensor error actors: %d sec", wait_on_Sensor_error_actor / 1000);
  log_e("Wait on sensor error induction: %d sec", wait_on_Sensor_error_induction / 1000);
  log_e("Switch off actors on error enabled after %d sec", (wait_on_error_mqtt / 1000));
  log_e("Display: %d startPage: %d", useDisplay, startPage);
  log_e("DevBranch: %d", devBranch);
  log_e("MQTT broker IP: %s Port: %d User: %s Pass: %s", mqtthost, mqttport, mqttuser, mqttpass);
  log_e("%s", "--------------------");
#endif
  if (measureJson(doc) > 2048 || doc.memoryUsage() > 2500)
  {
#ifdef ESP32
    log_e("JSON config length: %d", measureJson(doc));
    log_e("JSON memory usage: %d", doc.memoryUsage());
    log_e("%s", "Failed to write config file - config too large");
    log_e("%s", "------ saveConfig aborted ------");
#endif
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  File configFile = LittleFS.open(CONFIG, "w");
  if (!configFile)
  {
#ifdef ESP32
    log_e("%s", "Failed to open config file for writing");
    log_e("%s", "------ saveConfig aborted ------");
#endif
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();
#ifdef ESP32
  log_e("Config file size %d", configFile.size());
  log_e("JSON config length: %d", measureJson(doc));
  log_e("JSON memory usage: %d", doc.memoryUsage());
  log_e("%s", "------ saveConfig finished ------");
  log_e("Maximum number of pins: %d", NUMBEROFPINS);
  log_e("Free heap memory: %d", ESP.getFreeHeap());
#endif
  if (numberOfSensors > 0) // Ticker Sensors
    TickerSen.start();
  else
    TickerSen.stop();

  if (numberOfActors > 0) // Ticker Sensors
    TickerAct.start();
  else
    TickerAct.stop();

  if (inductionStatus > 0) // Ticker Induktion
    TickerInd.start();
  else
    TickerInd.stop();

  if (useDisplay) // Ticker Display
  {
    softSerial.begin(9600, SWSERIAL_8N1, D1, D2, false);
    pins_used[D1] = true;
    pins_used[D2] = true;
    nextion.begin(softSerial);
    initDisplay();
    TickerDisp.start();
  }
  else
    TickerDisp.stop();

  String Network = WiFi.SSID();

  if (startBuzzer)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    sendAlarm(ALARM_ON);
  }
#ifdef ESP32
  log_e("ESP8266 device IP Address: %s", WiFi.localIP().toString().c_str());
  log_e("Configured WLAN SSID: %s", Network.c_str());
  log_e("%s", "---------------------------------");
#endif
  return true;
}
