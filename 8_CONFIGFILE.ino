bool loadConfig()
{
  DEBUG_MSG("%s\n", "------ loadConfig started ------");
  File configFile = LittleFS.open(CONFIG, "r");
  if (!configFile)
  {
    DEBUG_MSG("%s\n", "Failed to open config file\n");
    DEBUG_MSG("%s\n", "------ loadConfig aborted ------\n");
    return false;
  }

  size_t size = configFile.size();
  if (size > 2048)
  {
    DEBUG_MSG("%s\n", "Config file size is too large");
    DEBUG_MSG("%s\n", "------ loadConfig aborted ------");
    return false;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    DEBUG_MSG("Conf: Error Json %s\n", error.c_str());
    return false;
  }

  // Misc Settings
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  wait_on_Sensor_error_actor = miscObj["del_sen_act"] | 120000;
  wait_on_Sensor_error_induction = miscObj["del_sen_ind"] | 120000;
  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor / 1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction / 1000);

  StopOnMQTTError = miscObj["enable_mqtt"] | 0;
  wait_on_error_mqtt = miscObj["delay_mqtt"] | 120000;

  DEBUG_MSG("Switch off actors on MQTT error: %d after %d sec\n", StopOnMQTTError, (wait_on_error_mqtt / 1000));

  startBuzzer = miscObj["buzzer"] | 0;
  if (startBuzzer)
    mqttBuzzer = miscObj["mqbuz"] | 0;
  else
    mqttBuzzer = false;
  DEBUG_MSG("Buzzer: %d mqttBuzzer: %d\n", startBuzzer, mqttBuzzer);

  senRes = miscObj["res"] | 0;
  useDisplay = miscObj["display"] | 0;
  startPage = miscObj["page"] | 0;
  devBranch = miscObj["devbranch"] | 0;

  DEBUG_MSG("Display: %d startPage: %d\n", useDisplay, startPage);

  strlcpy(nameMDNS, miscObj["mdns_name"] | "", maxHostSign);
  startMDNS = miscObj["mdns"] | 0;
  DEBUG_MSG("mDNS: %d name: %s\n", startMDNS, nameMDNS);
  strlcpy(mqtthost, miscObj["MQTTHOST"] | "", maxHostSign);
  strlcpy(mqttuser, miscObj["MQTTUSER"] | "", maxUserSign);
  strlcpy(mqttpass, miscObj["MQTTPASS"] | "", maxPassSign);
  mqttport = miscObj["MQTTPORT"] | 1883;
  startSPI = miscObj["spi"] | 0;
  DEBUG_MSG("MQTT server IP: %s Port: %d User: %s Pass: %s\n", mqtthost, mqttport, mqttuser, mqttpass);
  DEBUG_MSG("%s\n", "--------------------");

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
      DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d\n", (i + 1), actorObj["NAME"].as<const char *>(), actorObj["SCRIPT"].as<const char *>(), actorObj["PIN"].as<const char *>(), actorObj["INV"].as<int>(), actorObj["SW"].as<int>());
      i++;
    }
  }

  if (numberOfActors == 0)
  {
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  }
  DEBUG_MSG("%s\n", "--------------------");

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
      DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Type: %d Pin: %d\n", (i + 1), sensorsObj["NAME"].as<const char *>(), sensorsObj["ADDRESS"].as<const char *>(), sensorsObj["SCRIPT"].as<const char *>(), sensorsObj["CBPIID"].as<const char *>(), sensorsObj["OFFSET1"].as<float>(), sensorsObj["OFFSET2"].as<float>(), sensorsObj["SW"].as<int>(), sensorsObj["TYPE"].as<int>(), sensorsObj["PIN"].as<int>());
      i++;
    }
    else
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
  }
  // if (startSPI && (sensorsObj["TYPE"] == 1 || sensorsObj["TYPE"] == 2)) // Max31865 aktiviert und Sensortyp PT100x
  if (startSPI)
    setupPT();
  DEBUG_MSG("%s\n", "--------------------");

  // read induction
  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  inductionCooker.setIsEnabled(indObj["ENABLED"] | 0); // 0: Aus 1: IDS1 2: IDS2
  inductionStatus = inductionCooker.getIsEnabled();
  if (inductionStatus)
  {
    inductionCooker.change(StringToPin(indObj["PINWHITE"]), StringToPin(indObj["PINYELLOW"]), StringToPin(indObj["PINBLUE"]), indObj["TOPIC"], true, indObj["PL"]);
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d\n", inductionStatus, indObj["TOPIC"].as<const char *>(), indObj["PINWHITE"].as<const char *>(), indObj["PINYELLOW"].as<const char *>(), indObj["PINBLUE"].as<const char *>(), indObj["PL"].as<int>());
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionStatus);
  }
  DEBUG_MSG("%s\n", "--------------------");

  DEBUG_MSG("%s\n", "------ loadConfig finished ------");

  configFile.close();
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
    DEBUG_MSG("SoftwareSerial init %d\n", int(softSerial));
    pins_used[D1] = true;
    pins_used[D2] = true;
    nextion.begin(softSerial);
    initDisplay();
    TickerDisp.start();
  }

  DEBUG_MSG("Config file size %d\n", size);
  size_t len = measureJson(doc);
  DEBUG_MSG("JSON config length: %d\n", len);
  int memoryUsed = doc.memoryUsage();
  DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);
  DEBUG_MSG("%s\n", "--------------------");

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
  DEBUG_MSG("%s\n", "------ saveConfig started ------");
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
    DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d\n", (i + 1), actors[i].getActorName().c_str(), actors[i].getActorTopic().c_str(), PinToString(actors[i].getPinActor()), actors[i].getInverted(), actors[i].getActorSwitch());
  }
  if (numberOfActors == 0)
  {
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  }
  DEBUG_MSG("%s\n", "--------------------");

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
    DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d Typ: %d Pin: %d\n", (i + 1), sensors[i].getSensorName().c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].getSensorTopic().c_str(), sensors[i].getId().c_str(), sensors[i].getOffset1(), sensors[i].getOffset2(), sensors[i].getSensorSwitch(), sensors[i].getSensType(), , sensors[i].getSensPin());
  }
  if (startSPI) // Max31865 aktiviert und Sensortyp PT100x
      setupPT();

  DEBUG_MSG("%s\n", "--------------------");

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.getIsEnabled())
  {
    inductionStatus = 1;
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.getPinWhite());
    indObj["PINYELLOW"] = PinToString(inductionCooker.getPinYellow());
    indObj["PINBLUE"] = PinToString(inductionCooker.getPinInterrupt());
    indObj["ENABLED"] = inductionCooker.getIsEnabled();
    indObj["TOPIC"] = inductionCooker.getTopic();
    indObj["PL"] = inductionCooker.getPowerLevelOnError();
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s, Command channel (YELLOW): %s, Backchannel (BLUE): %s, PlOnErr: %d\n", inductionCooker.getIsEnabled(), inductionCooker.getTopic().c_str(), PinToString(inductionCooker.getPinWhite()).c_str(), PinToString(inductionCooker.getPinYellow()).c_str(), PinToString(inductionCooker.getPinInterrupt()).c_str(), inductionCooker.getPowerLevelOnError());
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionCooker.getIsEnabled());
  }

  DEBUG_MSG("%s\n", "--------------------");
  // Write Misc Stuff
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor / 1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction / 1000);

  miscObj["delay_mqtt"] = wait_on_error_mqtt;
  miscObj["enable_mqtt"] = (int)StopOnMQTTError;
  DEBUG_MSG("Switch off actors on error enabled after %d sec\n", (wait_on_error_mqtt / 1000));

  miscObj["buzzer"] = (int)startBuzzer;
  if (startBuzzer)
    miscObj["mqbuz"] = (int)mqttBuzzer;
  else
    miscObj["mqbuz"] = 0;

  miscObj["res"] = (int)senRes;
  miscObj["display"] = (int)useDisplay;
  miscObj["page"] = startPage;
  miscObj["devbranch"] = (int)devBranch;
  DEBUG_MSG("Display: %d startPage: %d\n", useDisplay, startPage);
  DEBUG_MSG("DevBranch: %d\n", devBranch);

  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["MQTTPORT"] = mqttport;
  miscObj["MQTTUSER"] = mqttuser;
  miscObj["MQTTPASS"] = mqttpass;
  miscObj["spi"] = (int)startSPI;
  miscObj["VER"] = Version;

  DEBUG_MSG("MQTT broker IP: %s Port: %d User: %s Pass: %s\n", mqtthost, mqttport, mqttuser, mqttpass);
  DEBUG_MSG("%s\n", "--------------------");

  if (measureJson(doc) > 2048 || doc.memoryUsage() > 2500)
  {
    DEBUG_MSG("JSON config length: %d\n", measureJson(doc));
    DEBUG_MSG("JSON memory usage: %d\n", doc.memoryUsage());
    DEBUG_MSG("%s\n", "Failed to write config file - config too large");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  File configFile = LittleFS.open(CONFIG, "w");
  if (!configFile)
  {
    DEBUG_MSG("%s\n", "Failed to open config file for writing");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();

  DEBUG_MSG("Config file size %d\n", configFile.size());
  DEBUG_MSG("JSON config length: %d\n", measureJson(doc));
  DEBUG_MSG("JSON memory usage: %d\n", doc.memoryUsage());
  DEBUG_MSG("%s\n", "------ saveConfig finished ------");
  DEBUG_MSG("Maximum number of pins: %d\n", NUMBEROFPINS);
  DEBUG_MSG("Free heap memory: %d\n", ESP.getFreeHeap());

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

  if (!useDisplay) // Ticker Display
    TickerDisp.stop();
  else
  {
    softSerial.begin(9600, SWSERIAL_8N1, D1, D2, false);
    DEBUG_MSG("SoftwareSerial init %d\n", int(softSerial));
    pins_used[D1] = true;
    pins_used[D2] = true;
    nextion.begin(softSerial);
    initDisplay();
    TickerDisp.start();
  }

  String Network = WiFi.SSID();
  DEBUG_MSG("ESP8266 device IP Address: %s\n", WiFi.localIP().toString().c_str());
  DEBUG_MSG("Configured WLAN SSID: %s\n", Network.c_str());

  if (startBuzzer)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    sendAlarm(ALARM_ON);
  }
  DEBUG_MSG("%s\n", "---------------------------------");
  return true;
}
