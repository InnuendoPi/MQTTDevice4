bool loadConfig()
{
  DEBUG_INFO("CFG", "------ loadConfig started ------");
  File configFile = LittleFS.open(CONFIG, "r");
  if (!configFile)
  {
    DEBUG_ERROR("CFG", "error could not open config.txt - permission denied");
    DEBUG_ERROR("CFG", "------ loadConfig aborted ------");
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    DEBUG_ERROR("CFG", "error could not open config.txt - JSON error %s", error.c_str());
    return false;
  }
  // Misc Settings
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  wait_on_Sensor_error_actor = miscObj["del_sen_act"] | 120000;
  wait_on_Sensor_error_induction = miscObj["del_sen_ind"] | 120000;

  StopOnMQTTError = miscObj["enable_mqtt"] | 0;
  wait_on_error_mqtt = miscObj["delay_mqtt"] | 120000;

  PIN_BUZZER = StringToPin(miscObj["buz"]);
  if (PIN_BUZZER != -100)
    mqttBuzzer = miscObj["mqbuz"] | 0;
  else
    mqttBuzzer = false;

  senRes = miscObj["res"] | 0;
  useDisplay = miscObj["display"] | 0;
  useFerm = miscObj["ferm"] | 0;
  startPage = miscObj["page"] | 0;
  devBranch = miscObj["devbranch"] | 0;
  strlcpy(nameMDNS, miscObj["mdns_name"] | "", maxHostSign);
  startMDNS = miscObj["mdns"] | 0;
  strlcpy(mqtthost, miscObj["MQTTHOST"] | "", maxHostSign);
  strlcpy(mqttuser, miscObj["MQTTUSER"] | "", maxUserSign);
  strlcpy(mqttpass, miscObj["MQTTPASS"] | "", maxPassSign);
  mqttport = miscObj["MQTTPORT"] | 1883;
    // strlcpy(ntpServer, miscObj["ntp"] | NTP_ADDRESS, maxHostSign);
  strlcpy(ntpServer, miscObj["ntp"] | NTP_ADDRESS, maxNTPSign);
  strlcpy(ntpZone, miscObj["zone"] | NTP_ZONE, maxNTPSign);
  startSPI = miscObj["spi"] | 0;
  selLang = miscObj["lang"] | 0;
  DUTYCYLCE = miscObj["dutyCycle"] | 5000;
  SENCYLCE = miscObj["senCycle"] | 1;

  DEBUG_INFO("CFG", "Wait on sensor error actors: %d sec", wait_on_Sensor_error_actor / 1000);
  DEBUG_INFO("CFG", "Wait on sensor error induction: %d sec", wait_on_Sensor_error_induction / 1000);
  DEBUG_INFO("CFG", "Switch off actors on MQTT error: %d after %d sec", StopOnMQTTError, (wait_on_error_mqtt / 1000));
  DEBUG_INFO("CFG", "Buzzer: %d mqttBuzzer: %d", PIN_BUZZER, mqttBuzzer);
  DEBUG_INFO("CFG", "Display: %d startPage: %d Fermenter: %d", useDisplay, startPage, useFerm);
  DEBUG_INFO("CFG", "mDNS: %d name: %s", startMDNS, nameMDNS);
  DEBUG_INFO("CFG", "MQTT server IP: %s Port: %d User: %s Pass: %s", mqtthost, mqttport, mqttuser, mqttpass);
  DEBUG_INFO("CFG", TRENNLINIE);
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
      DEBUG_INFO("CFG", "Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d", (i + 1), actorObj["NAME"].as<const char *>(), actorObj["SCRIPT"].as<const char *>(), actorObj["PIN"].as<const char *>(), actorObj["INV"].as<int>(), actorObj["SW"].as<int>());
      i++;
    }
  }
  if (numberOfActors == 0)
  {
    DEBUG_INFO("CFG", "Actors: %d", numberOfActors);
  }
  DEBUG_INFO("CFG", TRENNLINIE);
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
      DEBUG_INFO("CFG", "Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Type: %d Pin: %d", (i + 1), sensorsObj["NAME"].as<const char *>(), sensorsObj["ADDRESS"].as<const char *>(), sensorsObj["SCRIPT"].as<const char *>(), sensorsObj["CBPIID"].as<const char *>(), sensorsObj["OFFSET1"].as<float>(), sensorsObj["OFFSET2"].as<float>(), sensorsObj["SW"].as<int>(), sensorsObj["TYPE"].as<int>(), sensorsObj["PIN"].as<int>());
      i++;
    }
    else
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
  }
  if (startSPI)
    setupPT();
  if (numberOfSensors == 0)
  {
    DEBUG_INFO("CFG", "Sensors: %d", numberOfSensors);
  }
  DEBUG_INFO("CFG", TRENNLINIE);
  // read induction
  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  inductionCooker.change(StringToPin(indObj["PINWHITE"]), StringToPin(indObj["PINYELLOW"]), StringToPin(indObj["PINBLUE"]), indObj["TOPIC"], indObj["ENABLED"], indObj["PL"]);
  DEBUG_INFO("CFG", "Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d", inductionCooker.getIsEnabled(), inductionCooker.getTopic().c_str(), PinToString(inductionCooker.getPinWhite()).c_str(), PinToString(inductionCooker.getPinYellow()).c_str(), PinToString(inductionCooker.getPinInterrupt()).c_str(), inductionCooker.getPowerLevelOnError());
  DEBUG_INFO("CFG", TRENNLINIE);
  
  configFile.close();
  // Setze NTP Server
  setupTime();

  if (numberOfSensors > 0) // Ticker Sensors
  {
    TickerSen.config(tickerSenCallback, (SENCYLCE * SEN_UPDATE), 0);
    TickerSen.start();
    
    // NON-blocking/async requestForConversion
    DS18B20.setWaitForConversion(false); // TRUE : function requestTemperature() etc returns when conversion is ready
    DS18B20.setCheckForConversion(true); // TRUE : function requestTemperature() etc will 'listen' to an IC to determine whether a conversion is complete
    DS18B20.requestTemperatures();       // Alle Sensoren abfragen
    lastRequestSensors = millis();
  }
  if (numberOfActors > 0) // Ticker Actors
    TickerAct.start();

  if (inductionStatus > 0) // Ticker Induction
    TickerInd.start();
  if (!useDisplay) // Ticker Display
    TickerDisp.stop();
  else
  {
    softSerial.begin(DEF_NEXTION, SWSERIAL_8N1, D1, D2, false);
    pins_used[D1] = true;
    pins_used[D2] = true;
    nextion.begin(DEF_NEXTION);
    if (getTagLevel("DIS") > 0)
      nextion.setDebug(true);
    else
      nextion.setDebug(false);
    initDisplay();
    TickerDisp.start();
  }
  DEBUG_INFO("CFG", "JSON config length: %zu", measureJson(doc));
  DEBUG_INFO("CFG", TRENNLINIE);
  if (PIN_BUZZER != -100)
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
  DEBUG_INFO("CFG", "------ saveConfig started ------");
  JsonDocument doc;

  // Write Actors
  JsonArray actorsArray = doc["actors"].to<JsonArray>();
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = actorsArray.add<JsonObject>();
    actorsObj["PIN"] = PinToString(actors[i].getPinActor());
    actorsObj["NAME"] = actors[i].getActorName();
    actorsObj["SCRIPT"] = actors[i].getActorTopic();
    actorsObj["INV"] = (int)actors[i].getInverted();
    actorsObj["SW"] = (int)actors[i].getActorSwitch();
    DEBUG_INFO("CFG", "Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d", (i + 1), actors[i].getActorName().c_str(), actors[i].getActorTopic().c_str(), PinToString(actors[i].getPinActor()), actors[i].getInverted(), actors[i].getActorSwitch());
  }
  DEBUG_INFO("CFG", TRENNLINIE);
  // Write Sensors
  JsonArray sensorsArray = doc["sensors"].to<JsonArray>();
  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = sensorsArray.add<JsonObject>();
    sensorsObj["ADDRESS"] = sensors[i].getSens_adress_string();
    sensorsObj["NAME"] = sensors[i].getSensorName();
    sensorsObj["OFFSET1"] = (int(sensors[i].getOffset1() * 100)) / 100.0;
    sensorsObj["OFFSET2"] = (int(sensors[i].getOffset2() * 100)) / 100.0;
    sensorsObj["SCRIPT"] = sensors[i].getSensorTopic();
    sensorsObj["CBPIID"] = sensors[i].getId();
    sensorsObj["SW"] = (int)sensors[i].getSensorSwitch();
    sensorsObj["TYPE"] = sensors[i].getSensType();
    sensorsObj["PIN"] = sensors[i].getSensPin();
    DEBUG_INFO("CFG", "Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Typ: %d Pin: %d", (i + 1), sensors[i].getSensorName().c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].getSensorTopic().c_str(), sensors[i].getId().c_str(), sensors[i].getOffset1(), sensors[i].getOffset2(), sensors[i].getSensorSwitch(), sensors[i].getSensType(), sensors[i].getSensPin());
  }
  DEBUG_INFO("CFG", TRENNLINIE);

  // Write Induction
  JsonArray indArray = doc["induction"].to<JsonArray>();
  if (inductionCooker.getIsEnabled())
  {
    JsonObject indObj = indArray.add<JsonObject>();
    indObj["PINWHITE"] = PinToString(inductionCooker.getPinWhite());
    indObj["PINYELLOW"] = PinToString(inductionCooker.getPinYellow());
    indObj["PINBLUE"] = PinToString(inductionCooker.getPinInterrupt());
    indObj["ENABLED"] = (int)inductionCooker.getIsEnabled();
    indObj["TOPIC"] = inductionCooker.getTopic();
    indObj["PL"] = inductionCooker.getPowerLevelOnError();
    DEBUG_INFO("CFG", "Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d", inductionCooker.getIsEnabled(), inductionCooker.getTopic().c_str(), PinToString(inductionCooker.getPinWhite()).c_str(), PinToString(inductionCooker.getPinYellow()).c_str(), PinToString(inductionCooker.getPinInterrupt()).c_str(), inductionCooker.getPowerLevelOnError());
  }
  DEBUG_INFO("CFG", TRENNLINIE);
  // Write Misc Stuff
  JsonArray miscArray = doc["misc"].to<JsonArray>();
  JsonObject miscObj = miscArray.add<JsonObject>();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  miscObj["delay_mqtt"] = wait_on_error_mqtt;
  miscObj["enable_mqtt"] = (int)StopOnMQTTError;
  // miscObj["buzzer"] = (int)startBuzzer;
  miscObj["buz"] = PinToString(PIN_BUZZER);
  // if (startBuzzer)
  if (PIN_BUZZER != -100)
    miscObj["mqbuz"] = (int)mqttBuzzer;
  else
    miscObj["mqbuz"] = 0;

  miscObj["res"] = (int)senRes;
  miscObj["display"] = (int)useDisplay;
  miscObj["ferm"] = (int)useFerm;
  miscObj["page"] = startPage;
  miscObj["devbranch"] = (int)devBranch;
  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["MQTTPORT"] = mqttport;
  miscObj["MQTTUSER"] = mqttuser;
  miscObj["MQTTPASS"] = mqttpass;
  miscObj["ntp"] = ntpServer;
  miscObj["zone"] = ntpZone;
  miscObj["spi"] = (int)startSPI;
  miscObj["lang"] = selLang;
  miscObj["dutyCycle"] = DUTYCYLCE;
  miscObj["senCycle"] = SENCYLCE;
  miscObj["VER"] = Version;

  DEBUG_INFO("CFG", "Wait on sensor error actors: %d sec", wait_on_Sensor_error_actor / 1000);
  DEBUG_INFO("CFG", "Wait on sensor error induction: %d sec", wait_on_Sensor_error_induction / 1000);
  DEBUG_INFO("CFG", "Switch off actors on error enabled after %d sec", (wait_on_error_mqtt / 1000));
  DEBUG_INFO("CFG", "Display: %d startPage: %d Fermenter: %d", useDisplay, startPage, useFerm);
  DEBUG_INFO("CFG", "DevBranch: %d", devBranch);
  DEBUG_INFO("CFG", "MQTT broker IP: %s Port: %d User: %s Pass: %s", mqtthost, mqttport, mqttuser, mqttpass);
  DEBUG_INFO("CFG", TRENNLINIE);

  File configFile = LittleFS.open(CONFIG, "w");
  if (!configFile)
  {
    DEBUG_ERROR("CFG", "Failed to open config file for writing");
    DEBUG_ERROR("CFG", "------ saveConfig aborted ------");

    if (PIN_BUZZER != -100)
      sendAlarm(ALARM_ERROR);
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();

  DEBUG_INFO("CFG", "Config file size %d", configFile.size());
  DEBUG_INFO("CFG", "JSON config length: %zu", measureJson(doc));
  DEBUG_INFO("CFG", "------ saveConfig finished ------");
  DEBUG_INFO("CFG", "Maximum number of pins: %d", NUMBEROFPINS);
  DEBUG_INFO("CFG", "Free heap memory: %d", ESP.getFreeHeap());
  if (numberOfSensors > 0) // Ticker Sensors
  {
    if (TickerSen.state() == RUNNING)
      TickerSen.stop();
      
    TickerSen.config(tickerSenCallback, (SENCYLCE * SEN_UPDATE), 0);
    TickerSen.start();
    
    // NON-blocking/async requestForConversion
    DS18B20.setWaitForConversion(false); // TRUE : function requestTemperature() etc returns when conversion is ready
    DS18B20.setCheckForConversion(true); // TRUE : function requestTemperature() etc will 'listen' to an IC to determine whether a conversion is complete
    DS18B20.requestTemperatures();       // Alle Sensoren abfragen
    lastRequestSensors = millis();
  }
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

  String Network = WiFi.SSID();

  if (PIN_BUZZER != -100)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    sendAlarm(ALARM_ON);
  }

  DEBUG_INFO("CFG", "ESP device IP Address: %s", WiFi.localIP().toString().c_str());
  DEBUG_INFO("CFG", "Configured WLAN SSID: %s", Network.c_str());
  DEBUG_INFO("CFG", TRENNLINIE);
  saveLog();
  return true;
}
