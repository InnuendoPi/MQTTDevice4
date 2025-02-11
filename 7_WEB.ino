void handleRoot()
{
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", index_htm_gz, index_htm_gz_len);
}

bool loadFromLittlefs(String path)
{
  if (path.endsWith("/undefined"))
  {
    DEBUG_ERROR("SYS", "Web loadFromLittlefs path error: %s", path.c_str());
    return false;
  }
  else if (path.endsWith("/"))
    path += "index.htm";

  String contentType;
  if (server.hasArg("download"))
    contentType = F("application/octet-stream");
  else
    contentType = getContentType(path); // FSBrowser

  if (LittleFS.exists(path.c_str()))
  {
    File dataFile = LittleFS.open(path.c_str(), "r");
    if (dataFile)
    {
      int32_t fsize = dataFile.size();
      server.sendHeader("Content-Length", (String)fsize);
      size_t sent = server.streamFile(dataFile, contentType);
      dataFile.close();
      return true;
    }
    else
      return false;
  }
  return false;
}

void mqttcallback(char *topic, unsigned char *payload, unsigned int length)
{
  // Uncomment for debug output received MQTT payloads
  // log_e("Web: Received MQTT Topic with char payload: %s\n", topic);
  // Serial.print("Web: Payload: ");
  // for (unsigned int i = 0; i < length; i++)
  // {
  // Serial.print((char)payload[i]);
  // }
  // Serial.println(" ");

  // char payload_msg[length];

  if (inductionCooker.getTopic() == topic)
  {
    inductionCooker.handlemqtt(payload, length);
    return;
  }

  if (numberOfActors > 0)
  {
    for (uint8_t i = 0; i < numberOfActors; i++)
    {
      if (actors[i].getActorTopic() == topic)
      {
        actors[i].handlemqtt(payload);
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
    const char *fermenterupdate = "cbpi/fermenterupdate/";
    const char *fermenterstepupdate = "cbpi/fermenterstepupdate/";

    p = strstr(topic, kettleupdate);
    if (p)
    {
      cbpi4kettle_handlemqtt(payload);
      return;
    }
    p = strstr(topic, stepupdate);
    if (p)
    {
      cbpi4steps_handlemqtt(payload);
      return;
    }
    p = strstr(topic, notificationupdate);
    if (p)
    {
      cbpi4notification_handlemqtt(payload);
      return;
    }
    p = strstr(topic, sensorupdate);
    if (p)
    {
      cbpi4sensor_handlemqtt(payload);
      return;
    }
    p = strstr(topic, fermenterupdate);
    if (p)
    {
      cbpi4fermenter_handlemqtt(payload);
      return;
    }
    p = strstr(topic, fermenterstepupdate);
    if (p)
    {
      cbpi4fermentersteps_handlemqtt(payload);
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
      cbpi4notification_handlemqtt(payload);
      return;
    }
  }
}

void handleRequestMisc2()
{
  JsonDocument doc;
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["s_mqtt"] = mqtt_state;
  doc["display"] = useDisplay;
  if (startMDNS)
    doc["mdns"] = nameMDNS;
  else
    doc["mdns"] = 0;

  char response[measureJson(doc) + 2];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void handleRequestMiscAlert()
{
  if (alertState)
  {
    replyResponse("1");
    alertState = false;
  }
  else
    replyResponse("0");
}

void handleRequestMisc()
{
  JsonDocument doc;
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["user"] = mqttuser;
  doc["pass"] = mqttpass;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["mqbuz"] = mqttBuzzer;
  doc["res"] = senRes;
  doc["display"] = useDisplay;
  doc["ferm"] = useFerm;
  doc["page"] = startPage;
  doc["dev"] = devBranch;
  doc["e_mqtt"] = StopOnMQTTError;
  doc["d_mqtt"] = wait_on_error_mqtt / 1000;
  doc["dsa"] = wait_on_Sensor_error_actor / 1000;
  doc["dsi"] = wait_on_Sensor_error_induction / 1000;
  doc["s_mqtt"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["spi"] = startSPI;
  doc["ntp"] = ntpServer;
  doc["zone"] = ntpZone;
  doc["duty"] = DUTYCYLCE;
  doc["sen"] = SENCYLCE;

  doc["logCFG"] = getTagLevel("CFG");
  doc["logSen"] = getTagLevel("SEN");
  doc["logAct"] = getTagLevel("ACT");
  doc["logInd"] = getTagLevel("IND");
  doc["logSys"] = getTagLevel("SYS");
  if (getTagLevel("DIS") > 0)
    doc["logDis"] = 1;
  else
    doc["logDis"] = 0;

  String message = "";
  if (isPin(PIN_BUZZER))
  {
    message += OPTIONSTART;
    message += PinToString(PIN_BUZZER);
    message += OPTIONDISABLED;
  }
  else
  {
    message += OPTIONSTART;
    message += "-";
    message += OPTIONDISABLED;
  }

  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += OPTIONSTART;
      message += pin_names[i];
      message += OPTIONEND;
    }
  }
  doc["buz"] = message;

  char response[measureJson(doc) + 2];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void handleSetMiscLang()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
    DEBUG_ERROR("SYS", "error deserializeJson %s", error.c_str());
    replyServerError("Server error set language");
    return;
  }
  selLang = doc["lang"];
  saveConfig();
  replyOK();
}

void handleRequestFirm()
{
  String request = server.arg(0);
  String message;
#ifdef ESP32
  message = F("MQTTDevice32 V ");
#elif ESP8266
  message = F("MQTTDevice V ");
#endif
  message += Version;
  if (devBranch == 1)
    message += F(" dev");

  replyResponse(message.c_str());
}

void handleGetTitle()
{
#ifdef ESP32
  replyResponse("MQTTDevice32 ");
#elif ESP8266
  replyResponse("MQTTDevice4 ");
#endif
}

void handleReqSys()
{
  JsonDocument doc;
  String message;
#ifdef ESP32
  message = F("MQTTDevice32 V");
  doc["title"] = "MQTTDevice32";
#elif ESP8266
  message = F("MQTTDevice4 V");
  doc["title"] = "MQTTDevice4";
#endif
  message += Version;
  if (devBranch == 1)
    message += F(" dev");

  doc["firm"] = message;

  char response[measureJson(doc) + 2];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void handleSetMisc()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
    DEBUG_ERROR("SYS", "error deserializeJson %s", error.c_str());
    replyServerError("Server error deserialize misc settings");
    return;
  }

  if (doc["reset"] && doc["clear"])
  {
    LittleFS.remove(CONFIG);
    LittleFS.remove(LOG_CFG);
    WiFi.disconnect();
    wifiManager.resetSettings();
    delay(PAUSE1SEC);
    EM_REBOOT();
  }
  if (doc["reset"])
  {
    WiFi.disconnect();
    wifiManager.resetSettings();
    delay(PAUSE1SEC);
    ESP.restart();
  }
  if (doc["clear"])
  {
    LittleFS.remove(CONFIG);
    LittleFS.remove(LOG_CFG);
    EM_REBOOT();
  }

  PIN_BUZZER = StringToPin(doc["buz"]);
  pins_used[PIN_BUZZER] = true;

  mqttport = doc["mqttport"];
  strlcpy(mqtthost, doc["mqtthost"] | "", maxHostSign);
  strlcpy(mqttuser, doc["mqttuser"] | "", maxUserSign);
  strlcpy(mqttpass, doc["mqttpass"] | "", maxPassSign);

  mqttBuzzer = doc["mqbuz"];
  senRes = doc["res"];
  useDisplay = doc["display"];
  startPage = doc["page"];
  useFerm = doc["ferm"];
  devBranch = doc["dev"];
  strlcpy(nameMDNS, doc["mdns_name"] | "", maxHostSign);
  startMDNS = doc["mdns"];
  StopOnMQTTError = doc["e_mqtt"];
  wait_on_error_mqtt = doc["d_mqtt"].as<int>() * 1000;
  wait_on_Sensor_error_actor = doc["dsa"].as<int>() * 1000;
  wait_on_Sensor_error_induction = doc["dsi"].as<int>() * 1000;

  strlcpy(ntpServer, doc["ntp"] | NTP_ADDRESS, maxNTPSign);
  strlcpy(ntpZone, doc["zone"] | NTP_ZONE, maxNTPSign);
  startSPI = doc["spi"];
  DUTYCYLCE = doc["duty"];
  SENCYLCE = doc["sen"];
  setTagLevel("CFG", doc["logCFG"]);
  setTagLevel("SEN", doc["logSen"]);
  setTagLevel("ACT", doc["logAct"]);
  setTagLevel("IND", doc["logInd"]);
  setTagLevel("SYS", doc["logSys"]);
  if (doc["logDis"].as<int>() > 0)
  {
    setTagLevel("DIS", 3);
    nextion.setDebug(true);
  }
  else
  {
    setTagLevel("DIS", 0);
    nextion.setDebug(false);
  }
  saveConfig();
  replyOK();
  miscSSE();
}

void handleRestore()
{
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = "config.txt"; // upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    fsUploadFile = LittleFS.open(filename, "w"); // Open the file for writing in LittleFS (create if it doesn't exist)
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {                       // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      EM_REBOOT();
    }
  }
}

void handleGetLanguage()
{
  char response[3];
  sprintf_P(response, PSTR("%d"), selLang);
  replyResponse(response);
}
