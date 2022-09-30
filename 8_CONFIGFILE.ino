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
  // DEBUG_MSG("Buzzer: %d\n", startBuzzer);
  if (startBuzzer)
    mqttBuzzer = miscObj["mqbuz"] | 0;
  else
    mqttBuzzer = false;
  DEBUG_MSG("Buzzer: %d mqttBuzzer: %d\n", startBuzzer, mqttBuzzer);

  useDisplay = miscObj["display"] | 0;
  startPage = miscObj["page"] | 0;
  devBranch = miscObj["devbranch"] | 0;

  DEBUG_MSG("Display: %d startPage: %d\n", useDisplay, startPage);

  strlcpy(nameMDNS, miscObj["mdns_name"] | "", maxHostSign);
  startMDNS = miscObj["mdns"] | 0;
  useI2C = miscObj["i2c"] | 0;
  DEBUG_MSG("I2C: %d mDNS: %d name: %s\n", useI2C, startMDNS, nameMDNS);
  strlcpy(mqtthost, miscObj["MQTTHOST"] | "", maxHostSign);
  strlcpy(mqttuser, miscObj["MQTTUSER"] | "", maxUserSign);
  strlcpy(mqttpass, miscObj["MQTTPASS"] | "", maxPassSign);
  mqttport = miscObj["MQTTPORT"] | 1883;
  mqttoff = miscObj["MQTTOFF"] | 0;
  if (!mqttoff)
    DEBUG_MSG("MQTT server IP: %s Port: %d User: %s Pass: %s Off: %d\n", mqtthost, mqttport, mqttuser, mqttpass, mqttoff);

  if (useI2C)
    numberOfPins = ALLPINS;
  else
    numberOfPins = GPIOPINS;
  DEBUG_MSG("%s\n", "--------------------");

  // read actors
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

  // read sensors
  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > numberOfSensorsMax)
    numberOfSensors = numberOfSensorsMax;
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      sensors[i].change(sensorsObj["ADDRESS"] | "", sensorsObj["SCRIPT"] | "", sensorsObj["NAME"] | "", sensorsObj["CBPIID"] | "", sensorsObj["OFFSET1"] | 0.0, sensorsObj["OFFSET2"] | 0.0, sensorsObj["SW"] | 0);
      DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %.02f Offset2: %.02f SW: %d\n", (i + 1), sensorsObj["NAME"].as<const char *>(), sensorsObj["ADDRESS"].as<const char *>(), sensorsObj["SCRIPT"].as<const char *>(), sensorsObj["CBPIID"].as<const char *>(), sensorsObj["OFFSET1"].as<float>(), sensorsObj["OFFSET2"].as<float>(), sensorsObj["SW"].as<int>());
      i++;
    }
    else
      sensors[i].change("", "", "", "", 0.0, 0.0, false);
  }
  DEBUG_MSG("%s\n", "--------------------");

  // read induction
  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  if (indObj.containsKey("ENABLED"))
  {
    inductionStatus = 1;
    inductionCooker.change(StringToPin(indObj["PINWHITE"]), StringToPin(indObj["PINYELLOW"]), StringToPin(indObj["PINBLUE"]), indObj["TOPIC"] | "", indObj["DELAY"] | DEF_DELAY_IND, true, indObj["PL"] | 100);
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d\n", inductionStatus, indObj["TOPIC"].as<const char *>(), indObj["PINWHITE"].as<const char *>(), indObj["PINYELLOW"].as<const char *>(), indObj["PINBLUE"].as<const char *>(), indObj["DELAY"].as<int>(), indObj["PL"].as<int>());
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionStatus);
  }
  DEBUG_MSG("%s\n", "--------------------");

  // read HLT
  JsonArray hltArray = doc["hlt"];
  JsonObject hltObj = hltArray[0];
  kettleHLT.isEnabled = hltObj["ENABLED"] | 0;
  hltStatus = kettleHLT.isEnabled;
  DEBUG_MSG("HLT: hltStatus %d\n", hltStatus);

  if (kettleHLT.isEnabled)
  {
    hltStatus = 1;
    hltKp = hltObj["kp"] | 2.0;
    hltKi = hltObj["ki"] | 0.5;
    hltKd = hltObj["kd"] | 1.0;
    hltSetpoint = hltObj["SETP"] | 78.0;

    kettleHLT.change(hltObj["ENABLED"] | 0, hltObj["PIN"] | "", hltObj["INV"] | 0, hltObj["SENID"] | 1);
    DEBUG_MSG("HLT: %d PIN: %s invert GPIO: %d SenID: %d\n", hltStatus, hltObj["PIN"].as<const char *>(), hltObj["INV"].as<int>(), hltObj["SENID"].as<int>());
  }
  else
  {
    hltStatus = 0;
    DEBUG_MSG("HLT: %d\n", hltStatus);
  }
  DEBUG_MSG("HLT: Kp %.02f Ki %.02f Kd %.02f Setpoint %.02f\n", hltKp, hltKi, hltKd, hltSetpoint);
  DEBUG_MSG("%s\n", "--------------------");

  // PID stuff
  JsonArray pidArray = doc["pid"];
  JsonObject pidObj = pidArray[0];

  ids2Kp = pidObj["kp"] | 2.0;
  ids2Ki = pidObj["ki"] | 0.5;
  ids2Kd = pidObj["kd"] | 1.0;
  pidDelta = pidObj["piddelta"] | 0.0;
  DEBUG_MSG("PIDs: Kp: %.03f Ki: %.03f Kd: %.03f delta: %.03f\n", ids2Kp, ids2Ki, ids2Kd, pidDelta);

  DEBUG_MSG("%s\n", "------ loadConfig finished ------");

  configFile.close();
  if (numberOfSensors > 0) // Ticker Sensors
    TickerSen.start();

  if (numberOfActors > 0) // Ticker Sensors
    TickerAct.start();

  if (inductionStatus > 0) // Induktion
    TickerInd.start();

  if (hltStatus > 0) // Ticker HLT
    TickerHlt.start();

  DEBUG_MSG("Config file size %d\n", size);
  size_t len = measureJson(doc);
  DEBUG_MSG("JSON config length: %d\n", len);
  int memoryUsed = doc.memoryUsage();
  DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);
  DEBUG_MSG("%s\n", "--------------------");

  if (useI2C)
    numberOfPins = ALLPINS;
  else
    numberOfPins = GPIOPINS;

  if (LittleFS.exists("/mashplan.json")) // Lade Maischeplan
    readMash();

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

  // if (LittleFS.begin())
  // {
  //   // saveConfig();
  //   shouldSaveConfig = true;
  // }
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
    sensorsObj["OFFSET1"] = (int(sensors[i].getOffset1() * 100)) / 100.0;
    sensorsObj["OFFSET2"] = (int(sensors[i].getOffset2() * 100)) / 100.0;
    sensorsObj["SCRIPT"] = sensors[i].getTopic();
    sensorsObj["CBPIID"] = sensors[i].getId();
    sensorsObj["SW"] = (int)sensors[i].getSw();
    DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s CBPi-ID: %s Offset1: %f Offset2: %f SW: %d\n", (i + 1), sensors[i].getName().c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].getTopic().c_str(), sensors[i].getId().c_str(), sensors[i].getOffset1(), sensors[i].getOffset2(), sensors[i].getSw());
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
  // Write PID Stuff
  JsonArray pidArray = doc.createNestedArray("pid");
  JsonObject pidObj = pidArray.createNestedObject();

  pidObj["kp"] = ids2Kp;
  pidObj["ki"] = ids2Ki;
  pidObj["kd"] = ids2Kd;
  pidObj["piddelta"] = (int(pidDelta * 100)) / 100.0;

  DEBUG_MSG("PID IDS2: Kp: %.03f Ki: %.03f Kd: %.03f pidDelta: %.01f ids2AutoTune: %d Setpoint: %.01f\n", ids2Kp, ids2Ki, ids2Kd, pidDelta, ids2AutoTune, ids2Setpoint);
  DEBUG_MSG("%s\n", "--------------------");

  // Write HLT
  JsonArray hltArray = doc.createNestedArray("hlt");
  JsonObject hltObj = hltArray.createNestedObject();
  hltObj["ENABLED"] = (int)kettleHLT.isEnabled;

  if (kettleHLT.isEnabled)
  {
    hltStatus = 1;
    hltObj["PIN"] = PinToString(kettleHLT.pin_hlt);
    hltObj["INV"] = (int)kettleHLT.isInverted;
    hltObj["SENID"] = kettleHLT.senid;
    hltObj["SETP"] = int(hltSetpoint);
    hltObj["kp"] = hltKp;
    hltObj["ki"] = hltKi;
    hltObj["kd"] = hltKd;
    DEBUG_MSG("HLT: %d PIN: %s Invert: %d SenID: %d\n", kettleHLT.isEnabled, PinToString(kettleHLT.pin_hlt).c_str(), kettleHLT.isInverted, kettleHLT.senid);
  }
  else
  {
    hltStatus = 0;
    DEBUG_MSG("HLT: %d\n", kettleHLT.isEnabled);
  }
  DEBUG_MSG("PID HLT: Kp %.02f Ki %.02f Kd %.02f Setpoint %.02f\n", hltKp, hltKi, hltKd, hltSetpoint);
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

  miscObj["display"] = (int)useDisplay;
  miscObj["page"] = startPage;
  miscObj["devbranch"] = (int)devBranch;
  DEBUG_MSG("Display: %d startPage: %d\n", useDisplay, startPage);
  DEBUG_MSG("DevBranch: %d\n", devBranch);

  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["i2c"] = (int)useI2C;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["MQTTPORT"] = mqttport;
  miscObj["MQTTUSER"] = mqttuser;
  miscObj["MQTTPASS"] = mqttpass;
  miscObj["MQTTOFF"] = (int)mqttoff;

  DEBUG_MSG("MQTT broker IP: %s Port: %d User: %s Pass: %s Off: %d\n", mqtthost, mqttport, mqttuser, mqttpass, mqttoff);
  DEBUG_MSG("%s\n", "--------------------");

  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();

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

  DEBUG_MSG("Config file size %d\n", configFile.size());
  DEBUG_MSG("JSON config length: %d\n", measureJson(doc));
  DEBUG_MSG("JSON memory usage: %d\n", doc.memoryUsage());
  DEBUG_MSG("%s\n", "------ saveConfig finished ------");

  if (useI2C)
    numberOfPins = ALLPINS;
  else
    numberOfPins = GPIOPINS;
  DEBUG_MSG("Maximum number of pins: %d\n", numberOfPins);
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

  if (hltStatus > 0) // Ticker HLT
    TickerHlt.start();
  else
    TickerHlt.stop();

  if (!useDisplay) // Ticker Display
    TickerDisp.stop();
  else
  {
    if (TickerDisp.state() != RUNNING)
    {
      if (!softSerial)
        softSerial.begin(9600, SWSERIAL_8N1, D1, D2, false);

      DEBUG_MSG("SoftwareSerial init %d\n", int(softSerial));
      pins_used[D1] = true;
      pins_used[D2] = true;
      nextion.begin(softSerial);
      initDisplay();
      TickerDisp.start();
    }
  }

  if (mqttoff)
  {
    if (TickerPUBSUB.state() == RUNNING)
      TickerPUBSUB.stop();
    if (TickerMQTT.state() == RUNNING)
      TickerMQTT.stop();
  }
  else
  {
    TickerPUBSUB.start();
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
