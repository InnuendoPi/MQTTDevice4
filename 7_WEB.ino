void handleRoot()
{
  // server.sendHeader("Location", "/index.html", true); // Redirect to our html web page
  // server.send(302, "text/plain", "");
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, sizeof(index_htm_gz));
}

void handleGetMash()
{
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", mash_htm_gz, sizeof(mash_htm_gz));
}
void handleWebRequests()
{
  if (loadFromLittlefs(server.uri()))
  {
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

bool loadFromLittlefs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";

  if (!LittleFS.exists(path.c_str()))
  {
    return false;
  }
  File dataFile = LittleFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }
  dataFile.close();
  return true;
}

void mqttcallback(char *topic, unsigned char *payload, unsigned int length)
{
  /*
  DEBUG_MSG("Web: Received MQTT Topic with char payload: %s\n", topic);
  Serial.print("Web: Payload: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println(" ");
  */

  // prüfen - Abfrage sollte überflüssig sein, wenn Tickerobjekt TickerSUBPUB gestoppt ist
  // if (mqttoff)
  //   return;

  if (mqttoff || TickerPUBSUB.state() != RUNNING)
  {
    DEBUG_MSG("WEB: mqttcallback1 ticker pusub %d ticker mqtt %d mqttoff %d\n", TickerPUBSUB.state(), TickerMQTT.state(), mqttoff);
    return;
  }
  // else
  // {
  //   DEBUG_MSG("WEB: mqttcallback2 ticker pusub %d ticker mqtt %d mqttoff %d\n", TickerPUBSUB.state(), TickerMQTT.state(), mqttoff );
  // }

  char payload_msg[length];
  for (int i = 0; i < length; i++)
  {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic)
  {
    inductionCooker.handlemqtt(payload_msg);
    return;
  }

  if (numberOfActors > 0)
  {
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].argument_actor == topic)
      {
        actors[i].handlemqtt(payload_msg);
        return;
      }
    }
  }

  if (useDisplay)
  {
    char *p;
    const char *kettleupdate = "cbpi/kettleupdate/";
    const char *stepupdate = "cbpi/stepupdate/";
    const char *sensorupdate = "cbpi/sensordata/";
    const char *notificationupdate = "cbpi/notification";

    p = strstr(topic, kettleupdate);
    if (p)
    {
      // DEBUG_MSG("%s\n", "Web: kettleupdate");
      cbpi4kettle_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, stepupdate);
    if (p)
    {
      // DEBUG_MSG("%s\n", "Web: stepsupdate");
      cbpi4steps_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, notificationupdate);
    if (p)
    {
      // DEBUG_MSG("%s\n", "Web: notificationupdate");
      cbpi4notification_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, sensorupdate);
    if (p)
    {
      // DEBUG_MSG("%s\n", "Web: sensorupdate");
      cbpi4sensor_handlemqtt(payload_msg);
      return;
    }
  }
  else if (mqttBuzzer)
  {
    char *p;
    const char *notificationupdate = "cbpi/notification";
    p = strstr(topic, notificationupdate);
    if (p)
    {
      // DEBUG_MSG("%s\n", "Web: notificationupdate mqttBuzzer");
      cbpi4notification_handlemqtt(payload_msg);
      return;
    }
  }
}

void handleRequestMisc2()
{
  // StaticJsonDocument<512> doc;
  DynamicJsonDocument doc(256);
  doc["mqtthost"] = mqtthost;
  doc["mqttport"] = mqttport;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["mqtt_state"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["buzzer"] = startBuzzer;
  doc["mqbuz"] = mqttBuzzer;
  doc["display"] = useDisplay;
  doc["i2c"] = useI2C;
  if (startMDNS)
    doc["mdns"] = nameMDNS;
  else
    doc["mdns"] = 0;
  doc["alertstate"] = alertState;
  // doc["statePower"] = statePower;
  // doc["statePause"] = statePause;
  // doc["statePlay"] = statePlay;
  doc["mqttoff"] = mqttoff;
  if (alertState)
    alertState = false;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();
  // DEBUG_MSG("WEB Misc2 JSON config length: %d\n", len);
  // DEBUG_MSG("WEB Misc2 JSON memory usage: %d\n", memoryUsed);
}

void handleRequestMisc3()
{
  // StaticJsonDocument<256> doc;
  DynamicJsonDocument doc(128);
  if (hltAutoTune)
    doc["statePower"] = kettleHLT.isOn;
  else
    doc["statePower"] = statePower;
  doc["statePause"] = statePause;
  doc["statePlay"] = statePlay;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();
  // DEBUG_MSG("WEB Misc3 JSON config length: %d\n", len);
  // DEBUG_MSG("WEB Misc3 JSON memory usage: %d\n", memoryUsed);
}

void handleRequestMisc()
{
  // StaticJsonDocument<768> doc;
  DynamicJsonDocument doc(768);
  doc["mqtthost"] = mqtthost;
  doc["mqttport"] = mqttport;
  doc["mqttuser"] = mqttuser;
  doc["mqttpass"] = mqttpass;
  doc["mqttoff"] = mqttoff;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["i2c"] = useI2C;
  doc["buzzer"] = startBuzzer;
  doc["mqbuz"] = mqttBuzzer;
  doc["display"] = useDisplay;
  doc["page"] = startPage;
  doc["devbranch"] = devBranch;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["delay_mqtt"] = wait_on_error_mqtt / 1000;
  doc["del_sen_act"] = wait_on_Sensor_error_actor / 1000;
  doc["del_sen_ind"] = wait_on_Sensor_error_induction / 1000;
  doc["mqtt_state"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  // doc["alertstate"] = alertState;
  // if (alertState)
  //   alertState = false;
  doc["pidmode"] = pidMode;
  doc["autotune"] = ids2AutoTune;
  doc["Setpoint"] = int(ids2Setpoint);
  doc["kp"] = ids2Kp;
  doc["ki"] = ids2Ki;
  doc["kd"] = ids2Kd;
  doc["piddelta"] = pidDelta;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();
  // DEBUG_MSG("WEB Misc JSON config length: %d\n", len);
  // DEBUG_MSG("WEB Misc JSON memory usage: %d\n", memoryUsed);
}

void handleRequestFirm()
{
  String request = server.arg(0);
  String message;
  if (request == "firmware")
  {
    if (startMDNS)
    {
      message = nameMDNS;
      message += " V";
    }
    else
    {
      message = "MQTTDevice4 V ";
    }
    message += Version;
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetMisc()
{
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "reset")
    {
      if (server.arg(i) == "true")
      {
        WiFi.disconnect();
        wifiManager.resetSettings();
        delay(PAUSE2SEC);
        ESP.reset();
      }
    }
    if (server.argName(i) == "clear")
    {
      if (server.arg(i) == "true")
      {
        LittleFS.remove("/config.txt");
        delay(PAUSE2SEC);
        ESP.reset();
      }
    }
    if (server.argName(i) == "mqtthost")
    {
      server.arg(i).toCharArray(mqtthost, maxHostSign);
    }
    if (server.argName(i) == "mqttport")
    {
      if (isValidInt(server.arg(i)))
      {
        mqttport = server.arg(i).toInt();
      }
    }
    if (server.argName(i) == "mqttuser")
    {
      server.arg(i).toCharArray(mqttuser, maxUserSign);
    }
    if (server.argName(i) == "mqttpass")
    {
      server.arg(i).toCharArray(mqttpass, maxPassSign);
    }
    if (server.argName(i) == "mqttoff")
    {
      mqttoff = checkBool(server.arg(i));
    }
    if (server.argName(i) == "buzzer")
    {
      startBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "mqbuz")
    {
      mqttBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "display")
    {
      useDisplay = checkBool(server.arg(i));
    }
    if (server.argName(i) == "page")
    {
      if (server.arg(i) == "BrewPage")
      {
        startPage = 0;
      }
      else if (server.arg(i) == "KettlePage")
      {
        startPage = 1;
      }
      else if (server.arg(i) == "InductionPage")
      {
        startPage = 2;
      }
      else
      {
        startPage = 1;
      }
    }
    if (server.argName(i) == "devbranch")
    {
      devBranch = checkBool(server.arg(i));
    }
    if (server.argName(i) == "mdns_name")
    {
      server.arg(i).toCharArray(nameMDNS, maxHostSign);
      checkChars(nameMDNS);
    }
    if (server.argName(i) == "mdns")
    {
      startMDNS = checkBool(server.arg(i));
    }
    if (server.argName(i) == "i2c")
    {
      useI2C = checkBool(server.arg(i));
    }
    if (server.argName(i) == "enable_mqtt")
    {
      StopOnMQTTError = checkBool(server.arg(i));
    }
    if (server.argName(i) == "delay_mqtt")
      if (isValidInt(server.arg(i)))
      {
        wait_on_error_mqtt = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_act")
    {
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
      }
    }
    if (server.argName(i) == "del_sen_ind")
    {
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_induction = server.arg(i).toInt() * 1000;
      }
    }
    if (server.argName(i) == "pidmode")
    {
      pidMode = checkBool(server.arg(i));
    }
    if (server.argName(i) == "autotune")
    {
      ids2AutoTune = checkBool(server.arg(i));
    }
    if (server.argName(i) == "setpoint")
    {
      if (isValidDigit(server.arg(i)))
      {
        ids2Setpoint = server.arg(i).toInt();
        DEBUG_MSG("WEB: ids2Setpoint %.02f\n", ids2Setpoint);
      }
      else
      {
        DEBUG_MSG("WEB: error ids2Setpoint %.02f\n", ids2Setpoint);
      }

    }
    if (server.argName(i) == "kp")
    {
      ids2Kp = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "ki")
    {
      ids2Ki = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "kd")
    {
      ids2Kd = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "piddelta")
    {
      pidDelta = formatDOT(server.arg(i));
    }

    yield();
  }
  saveConfig();
  server.send(201, "text/plain", "created");
}

// Some helper functions WebIf
void rebootDevice()
{
  server.send(205, "text/plain", "reboot");
  cbpiEventSystem(EM_REBOOT);
}

void handleRequestPages()
{
  int id = server.arg(0).toInt();
  String message;
  message += F("<option>");
  message += page_names[startPage];
  message += F("</option><option disabled>──────────</option>");

  for (int i = 0; i < numberOfPages; i++)
  {
    if (i != startPage)
    {
      message += F("<option>");
      message += page_names[i];
      message += F("</option>");
    }
  }
  server.send(200, "text/plain", message);
}

void handleRequestMash()
{

  File mashFile = LittleFS.open("/mashplan.json", "r");
  if (mashFile)
  {
    DynamicJsonDocument docIn(sizeRezeptMax);
    DynamicJsonDocument docOut(sizeRezeptMax);
    DeserializationError error = deserializeJson(docIn, mashFile);
    JsonArray mashArray = docIn.as<JsonArray>();
    int anzahlSchritte = mashArray.size();
    if (anzahlSchritte > maxSchritte)
      anzahlSchritte = maxSchritte;

    int i = 0;
    for (JsonObject mashObj : mashArray)
    {
      if (i < anzahlSchritte)
      {
        JsonObject responseObj = docOut.createNestedObject();
        responseObj["name"] = mashObj["step name"].as<String>();
        responseObj["temperature"] = mashObj["temperature"].as<unsigned short>() | 0;
        responseObj["duration"] = mashObj["duration"].as<unsigned short>() | 0;

        if (mashObj["autonext"] != "")
          responseObj["autonext"] = mashObj["autonext"].as<bool>();
        else
          responseObj["autonext"] = "false";

        i++;
      }
    }

    planResponse = "";
    serializeJson(docOut, planResponse);
    mashFile.close();
  } // read file
  // DEBUG_MSG("Web: reqMash %s\n", planResponse.c_str());
  server.send(200, "application/json", planResponse);
  return;
}

void handleSetMash()
{
  DynamicJsonDocument doc(sizeRezeptMax);
  // Serial.print("SetMash server arg: ");
  // Serial.println(server.arg(0));

  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
    DEBUG_MSG("Mash: deserialize Json error %s\n", error.c_str());
    return;
  }
  File mashFile = LittleFS.open("/mashplan.json", "w");
  if (mashFile)
  {
    serializeJson(doc, mashFile);
    mashFile.close();
    initMashPlan();
    readMash();
    server.send(201, "text/plain", "JSON successful");
    if (startBuzzer)
      sendAlarm(ALARM_ON);
  }
  else
  {
    server.send(500, "text/plain", "Server error");
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
  }
}

void handleRezeptUp()
{
  // DEBUG_MSG("%s\n", "Rezept Import gestartet");
  int typeRezept = -1;
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = "upRezept.json"; // upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    DEBUG_MSG("WEB Import recipe handleFileUpload Name: %s\n", filename.c_str());
    fsUploadFile = LittleFS.open(filename, "w"); // Open the file for writing in LittleFS (create if it doesn't exist)
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
                                                          // DEBUG_MSG("%s\n", "Rezept wird gespeichert");
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {                       // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      DEBUG_MSG("WEB Import recipe handleFileUpload Size: %d\n", upload.totalSize);
      server.sendHeader("Location", "/mash.html"); // Redirect the client to the success page
      server.send(303);                            // Antwort für Post und Put

      fsUploadFile = LittleFS.open("/upRezept.json", "r");
      DynamicJsonDocument testDoc(sizeImportMax);
      DeserializationError error = deserializeJson(testDoc, fsUploadFile);
      fsUploadFile.close();

      if (error)
      {
        DEBUG_MSG("WEB: handleRezeptUp Error Json %s\n", error.c_str());
        if (startBuzzer)
          sendAlarm(ALARM_ERROR);
        return;
      }

      if (testDoc.containsKey("Global")) // Datenbankversion KBH2
      {
        typeRezept = 1;
      }
      else
      {
        JsonArray testArray = testDoc.as<JsonArray>();
        JsonObject testObj = testArray[0];
        if (testObj.containsKey("autonext")) // MQTTDevice
          typeRezept = 0;
        else if (testObj.containsKey("Sorte")) // MMum
          typeRezept = 2;
      }
      server.send(201, "text/plain", "Upload successful");
    }
    else
    {
      server.send(500, "text/plain", "500: couldn't create file");
    }

    if (typeRezept == 0)
    {
      bool check = false;
      if (LittleFS.exists("/mashplan.json"))
        check = LittleFS.remove("/mashplan.json");

      check = LittleFS.rename("/upRezept.json", "/mashplan.json");
      initMashPlan();
      readMash();
      return;
    }
    else if (typeRezept == 1)
      BtnImportKBH2();
    else if (typeRezept == 2)
      BtnImportMMUM();

    LittleFS.remove("/upRezept.json");
  }
}

void handleBtnPower()
{
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "statePower")
    {
      if (server.arg(i) == "true")
      {
        if (hltAutoTune)
        {
          startHltAutoTune();
          // kettleHLT.isOn = true;
        }
        else if (ids2AutoTune)
        {
          startAutoTune();
          statePower = true;
          DEBUG_MSG("WEB: PowerButton on ids2AutoTune: %d ids2Setpoint: %.01f\n", ids2AutoTune, ids2Setpoint);
        }
        else
        {
          statePower = true;
          actMashStep = 0;
          pidMode = true;

          ids2PID.SetTunings(ids2Kp, ids2Ki, ids2Kd, PID::P_On::Error);
          ids2PID.SetOutputLimits(0, 100);
          ids2PID.SetSampleTime(RUN_PID);
          // turn the PID on
          resetPID();

          if (TickerPUBSUB.state() == RUNNING)
            TickerPUBSUB.stop();
          if (TickerMQTT.state() == RUNNING)
            TickerMQTT.stop();
          if (TickerInd.state() == RUNNING)
            TickerInd.stop();

          if (structPlan[actMashStep].duration > 0)
          {
            ids2Setpoint = structPlan[actMashStep].temp;
            ids2PID.Start(ids2Input, 0, ids2Setpoint);
            inductionCooker.inductionNewPower(0);
            TickerMash.stop(); // stop mash ticker and configure temp and duration
            TickerMash.config(tickerMashCallback, structPlan[actMashStep].duration * 60 * 1000, 1);
            DEBUG_MSG("WEB: PowerButton on aktMashStep: %d duration: %lu\n", actMashStep, (structPlan[actMashStep].duration * 60 * 1000));
          }

          TickerPID.start();
          if (startBuzzer)
            sendAlarm(ALARM_ON);
        }
      }
      else
      {
        if (hltAutoTune)
        {
          hltAutoTune = false;

          kettleHLT.isOn = false;
          statePause = false;
          statePlay = false;
          TickerHltPID.stop();
          kettleHLT.newPower(0);
          TickerHlt.updatenow();
        }
        else if (ids2AutoTune)
        {
          statePower = false;
          ids2AutoTune = false;
          statePause = false;
          statePlay = false;
          TickerMash.stop();
          TickerPID.stop();
          inductionCooker.inductionNewPower(0);
          // handleInduction();
          TickerInd.updatenow();
          if (!mqttoff)
            TickerPUBSUB.start();

          TickerInd.start();
          DEBUG_MSG("WEB: PowerButton off ids2AutoTune: %d\n", ids2AutoTune);
        }
        else
        {
          pidMode = false;
          statePower = false;
          statePause = false;
          statePlay = false;
          actMashStep = 0;
          ids2Setpoint = 0.0;
          ids2PID.Start(ids2Input, 0, ids2Setpoint);

          if (!mqttoff)
            TickerPUBSUB.start();

          TickerInd.start();

          TickerMash.stop();
          TickerPID.stop();
          inductionCooker.inductionNewPower(0);
          // handleInduction();
          TickerInd.updatenow();
          DEBUG_MSG("WEB: PowerButton off aktMashStep: %d duration: %lu\n", actMashStep, (structPlan[actMashStep].duration * 60 * 1000));
          if (startBuzzer)
            sendAlarm(ALARM_OFF);
        }
      }
    }
  }
  server.send(201, "text/plain", "created");
}
void handleBtnPlay()
{
  // Änderung play
  // if (pidMode && statePower && !statePause)
  // {
  //   DEBUG_MSG("WEB: Play Button ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
  //   TickerMash.start();
  // }

  // Änderung play
  if (pidMode && statePower && !statePause && !statePlay && TickerMash.state() == STOPPED)
  {
    DEBUG_MSG("WEB: Play Button ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
    TickerMash.start();
  }
  if (TickerMash.state() == STOPPED && actMashStep > 0) // check for last step autonext false?
  {
    if (!structPlan[actMashStep - 1].autonext && statePlay)
    {
      DEBUG_MSG("WEB: PlayButton1 ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
      ids2Setpoint = structPlan[actMashStep].temp;
      ids2PID.Start(ids2Input, 0, ids2Setpoint);
      // handleInduction(); -> Funktion geändert!
      statePlay = false;
    }
    else if (actMashStep > 0 && !statePlay)
    {
      ids2Setpoint = ids2Input;
      ids2PID.Start(ids2Input, 0, ids2Setpoint);
      // handleInduction(); -> Funktion geändert!
      statePlay = true;
      DEBUG_MSG("WEB: PlayButton2 ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
    }
  }
  else if (TickerMash.state() == RUNNING && actMashStep > 0)
  {
    server.send(201, "text/plain", "created");
    return;
  }
  else if (TickerMash.state() == PAUSED && actMashStep > 0)
  {
    server.send(201, "text/plain", "created");
    return;
  }
  server.send(201, "text/plain", "created");
}
void handleBtnPause()
{
  if (!pidMode)
  {
    server.send(201, "text/plain", "created");
    statePause = false;
    return;
  }

  if (TickerMash.state() == RUNNING)
  {
    DEBUG_MSG("WEB: PauseButton on aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
    statePause = true;
    TickerMash.pause();
  }
  else if (TickerMash.state() == PAUSED)
  {
    DEBUG_MSG("WEB: PauseButton off aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
    statePause = false;
    TickerMash.resume();
  }
  // Änderung play
  // else if (TickerMash.state() == STOPPED) // check for last step autonext false?
  // {
  //   if (!structPlan[actMashStep - 1].autonext && actMashStep > 0 && statePause)
  //   {
  //     DEBUG_MSG("WEB: PauseButton1 ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
  //     ids2Setpoint = structPlan[actMashStep].temp;
  //     ids2PID.Start(ids2Input, 0, ids2Setpoint);
  //     // handleInduction();

  //     statePause = false; // set btn-primary
  //   }
  //   else if (actMashStep > 0 && !statePause)
  //   {
  //     ids2Setpoint = ids2Input;
  //     ids2PID.Start(ids2Input, 0, ids2Setpoint);
  //     // handleInduction();
  //     statePause = true;
  //     DEBUG_MSG("WEB: PauseButton2 ids2Setpoint: %.02f ids2Input: %.02f\n", ids2Setpoint, ids2Input);
  //   }
  // }
  server.send(201, "text/plain", "created");
}

void handleBtnNextStep()
{
  if (TickerPID.state() != RUNNING || statePause)
  {
    server.send(201, "text/plain", "created");
    return;
  }

  if (!structPlan[actMashStep].autonext && TickerMash.state() == PAUSED && TickerMash.counter() >= 1)
  {
    if (actMashStep < maxActMashSteps)
      actMashStep++;

    DEBUG_MSG("WEB: handleBtnNextStep after autonext false aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
    TickerMash.stop();
    TickerMash.config(tickerMashCallback, structPlan[actMashStep].duration * 60 * 1000, 1);
    DEBUG_MSG("WEB: handleBtnNextStep new actMashStep: aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
    server.send(201, "text/plain", "created");
    return;
  }

  if (actMashStep < maxActMashSteps)
    actMashStep++;

  if (structPlan[actMashStep].duration >= 0 || structPlan[actMashStep].temp > 0)
  {
    DEBUG_MSG("WEB: handleBtnNextStep duration aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
    TickerMash.stop();
    TickerMash.config(tickerMashCallback, structPlan[actMashStep].duration * 60 * 1000, 1);
    ids2Setpoint = structPlan[actMashStep].temp;
    inductionCooker.inductionNewPower(int(ids2Output));
    // handleInduction();
    TickerInd.updatenow();
    ids2PID.Start(ids2Input, ids2Output, ids2Setpoint);
  }
  else // last mash step finished
  {
    statePower = false;
    pidMode = false;
    actMashStep = 0;
    ids2Setpoint = 0.0;
    ids2PID.Start(ids2Input, 0, ids2Setpoint);
    TickerMash.stop();
    TickerPID.stop();
    inductionCooker.inductionNewPower(0);
    // handleInduction();
    TickerInd.updatenow();
    if (startBuzzer)
      sendAlarm(ALARM_OFF);
    if (!mqttoff)
      TickerPUBSUB.start();

    TickerInd.start();
    DEBUG_MSG("WEB: handleBtnNextStep end aktMashStep: %d elapsed: %lu remaining: %lu counter: %d\n", actMashStep, TickerMash.elapsed(), TickerMash.remaining(), TickerMash.counter());
  }

  server.send(201, "text/plain", "created");
}

void handleActorPower()
{
  int id = server.arg(0).toInt();
  if (id < 0 || id > numberOfActorsMax)
  {
    server.send(201, "text/plain", "created");
    return;
  }

  actors[id].isOn = !actors[id].isOn;
  if (actors[id].isOn)
    actors[id].power_actor = actors[id].pwm;
  else
    actors[id].power_actor = 0;
    
  TickerAct.updatenow();
  DEBUG_MSG("Actor ID %d Pin %s switched to %d\n", id, PinToString(actors[id].pin_actor).c_str(), actors[id].isOn);
  server.send(201, "text/plain", "created");
}