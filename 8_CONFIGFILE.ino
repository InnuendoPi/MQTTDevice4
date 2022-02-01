bool loadConfig()
{
  DEBUG_MSG("%s\n", "------ loadConfig started ------");
  File configFile = LittleFS.open("/config.txt", "r");
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
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    DEBUG_MSG("Conf: Error Json %s\n", error.c_str());
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  JsonArray actorsArray = doc["actors"];
  numberOfActors = actorsArray.size();
  if (numberOfActors > numberOfActorsMax)
    numberOfActors = numberOfActorsMax;
  int i = 0;
  for (JsonObject actorObj : actorsArray)
  {
    if (i < numberOfActors)
    {
      actors[i].change(actorObj["PIN"] | "", actorObj["SCRIPT"] | "", actorObj["NAME"] | "", actorObj["INV"] | 0, actorObj["SW"] | 0);
      DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d\n", (i + 1), actorObj["NAME"].as<const char *>(), actorObj["SCRIPT"].as<const char *>(), actorObj["PIN"].as<const char *>(), actorObj["INV"].as<int>(), actorObj["SW"].as<int>());
      i++;
    }
  }

  if (numberOfActors == 0)
  {
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  }
  DEBUG_MSG("%s\n", "--------------------");

  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > numberOfSensorsMax)
    numberOfSensors = numberOfSensorsMax;
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      sensors[i].change(sensorsObj["ADDRESS"] | "", sensorsObj["SCRIPT"] | "", sensorsObj["NAME"] | "", sensorsObj["CBPIID"] | "", sensorsObj["OFFSET"] | 0, sensorsObj["SW"] | 0);
      DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset: %f SW: %d\n", (i + 1), sensorsObj["NAME"].as<const char *>(), sensorsObj["ADDRESS"].as<const char *>(), sensorsObj["SCRIPT"].as<const char *>(), sensorsObj["CBPIID"].as<const char *>(), sensorsObj["OFFSET"].as<float>(), sensorsObj["SW"].as<int>());
      i++;
    }
    else
      sensors[i].change("", "", "", "", 0.0, false);
  }
  DEBUG_MSG("%s\n", "--------------------");

  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  if (indObj.containsKey("ENABLED"))
  {
    inductionStatus = 1;
    inductionCooker.change(StringToPin(indObj["PINWHITE"] | "D5"), StringToPin(indObj["PINYELLOW"] | "D6"), StringToPin(indObj["PINBLUE"] | "D7"), indObj["TOPIC"] | "", indObj["DELAY"] | DEF_DELAY_IND, true, indObj["PL"] | 100);
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d\n", inductionStatus, indObj["TOPIC"].as<const char *>(), indObj["PINWHITE"].as<const char *>(), indObj["PINYELLOW"].as<const char *>(), indObj["PINBLUE"].as<const char *>(), indObj["DELAY"].as<int>(), indObj["PL"].as<int>());
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionStatus);
  }
  DEBUG_MSG("%s\n", "--------------------");

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
  DEBUG_MSG("Buzzer: %d\n", startBuzzer);
  useDisplay = miscObj["display"] | 0;
  DEBUG_MSG("Display: %d\n", useDisplay);
  devBranch = miscObj["devbranch"] | 0;
  DEBUG_MSG("devBranch: %d\n", devBranch);

  strlcpy(nameMDNS, miscObj["mdns_name"] | "", maxHostSign);
  startMDNS = miscObj["mdns"] | 0;
  DEBUG_MSG("mDNS: %d name: %s\n", startMDNS, nameMDNS);
  strlcpy(mqtthost, miscObj["MQTTHOST"] | "", maxHostSign);
  strlcpy(mqttuser, miscObj["MQTTUSER"] | "", maxUserSign);
  strlcpy(mqttpass, miscObj["MQTTPASS"] | "", maxPassSign);
  mqttport = miscObj["MQTTPORT"] | 1883;
  DEBUG_MSG("MQTT server IP: %s Port: %d User: %s Pass: %s\n", mqtthost, mqttport, mqttuser, mqttpass);
  DEBUG_MSG("%s\n", "------ loadConfig finished ------");
  
  configFile.close();
  if (numberOfSensors > 0)
    TickerSen.start();
  if (useDisplay)
    TickerDisp.start();

  DEBUG_MSG("Config file size %d\n", size);
  size_t len = measureJson(doc);
  DEBUG_MSG("JSON config length: %d\n", len);
  int memoryUsed = doc.memoryUsage();
  DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);

  if (startBuzzer)
    sendAlarm(ALARM_ON);

  return true;
}

void saveConfigCallback()
{

  if (LittleFS.begin())
  {
    saveConfig();
    shouldSaveConfig = true;
  }
  else
  {
    Serial.println("*** SYSINFO: WiFiManager failed to save MQTT broker IP");
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
    actorsObj["PIN"] = PinToString(actors[i].pin_actor);
    actorsObj["NAME"] = actors[i].name_actor;
    actorsObj["SCRIPT"] = actors[i].argument_actor;
    actorsObj["INV"] = (int)actors[i].isInverted;
    actorsObj["SW"] = (int)actors[i].switchable;
    DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %d SW: %d\n", (i + 1), actors[i].name_actor.c_str(), actors[i].argument_actor.c_str(), PinToString(actors[i].pin_actor).c_str(), actors[i].isInverted, actors[i].switchable);
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
    sensorsObj["NAME"] = sensors[i].getName();
    sensorsObj["OFFSET"] = sensors[i].getOffset();
    sensorsObj["SCRIPT"] = sensors[i].getTopic();
    sensorsObj["CBPIID"] = sensors[i].getId();
    sensorsObj["SW"] = (int)sensors[i].getSw();
    DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset: %f SW: %d\n", (i + 1), sensors[i].getName().c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].getTopic().c_str(), sensors[i].getId().c_str(), sensors[i].getOffset(), sensors[i].getSw());
  }

  DEBUG_MSG("%s\n", "--------------------");

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.isEnabled)
  {
    inductionStatus = 1;
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
    indObj["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
    indObj["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
    indObj["TOPIC"] = inductionCooker.mqtttopic;
    indObj["DELAY"] = inductionCooker.delayAfteroff;
    indObj["ENABLED"] = (int)inductionCooker.isEnabled;
    indObj["PL"] = inductionCooker.powerLevelOnError;
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d\n", inductionCooker.isEnabled, inductionCooker.mqtttopic.c_str(), PinToString(inductionCooker.PIN_WHITE).c_str(), PinToString(inductionCooker.PIN_YELLOW).c_str(), PinToString(inductionCooker.PIN_INTERRUPT).c_str(), (inductionCooker.delayAfteroff / 1000), inductionCooker.powerLevelOnError);
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionCooker.isEnabled);
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
  miscObj["display"] = (int)useDisplay;
  miscObj["devbranch"] = (int)devBranch;
  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["MQTTPORT"] = mqttport;
  miscObj["MQTTUSER"] = mqttuser;
  miscObj["MQTTPASS"] = mqttpass;
  DEBUG_MSG("MQTT broker IP: %s Port: %d User: %s Pass: %s\n", mqtthost, mqttport, mqttuser, mqttpass);

  size_t len = measureJson(doc);
  int memoryUsed = doc.memoryUsage();

  if (len > 2048 || memoryUsed > 2500)
  {
    DEBUG_MSG("JSON config length: %d\n", len);
    DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);
    DEBUG_MSG("%s\n", "Failed to write config file - config too large");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  File configFile = LittleFS.open("/config.txt", "w");
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
  DEBUG_MSG("%s\n", "------ saveConfig finished ------");

  if (numberOfSensors > 0)
    TickerSen.start();
  else
    TickerSen.stop();

  if (useDisplay)
    TickerDisp.start();
  else
    TickerDisp.stop();

  String Network = WiFi.SSID();
  DEBUG_MSG("ESP8266 device IP Address: %s\n", WiFi.localIP().toString().c_str());
  DEBUG_MSG("Configured WLAN SSID: %s\n", Network.c_str());
  DEBUG_MSG("%s\n", "---------------------------------");
  if (startBuzzer)
    sendAlarm(ALARM_ON);
  return true;
}
