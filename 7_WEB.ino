void handleRoot()
{
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send_P(200, "text/html", index_htm_gz, index_htm_gz_len);
}

bool loadFromLittlefs(String path)
{
  if (path.endsWith("/undefined"))
  {
#ifdef ESP32
    log_e("Web loadFromLittlefs path error: %s", path.c_str());
#endif
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
  // for (int i = 0; i < length; i++)
  // {
  // Serial.print((char)payload[i]);
  // }
  // Serial.println(" ");

  char payload_msg[length];
  for (int16_t i = 0; i < length; i++)
  {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.getTopic() == topic)
  {
    inductionCooker.handlemqtt(payload_msg);
    return;
  }

  if (numberOfActors > 0)
  {
    for (uint8_t i = 0; i < numberOfActors; i++)
    {
      if (actors[i].getActorTopic() == topic)
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
      cbpi4kettle_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, stepupdate);
    if (p)
    {
      cbpi4steps_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, notificationupdate);
    if (p)
    {
      cbpi4notification_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, sensorupdate);
    if (p)
    {
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
      cbpi4notification_handlemqtt(payload_msg);
      return;
    }
  }
}

void handleRequestMisc2()
{
  DynamicJsonDocument doc(256);
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["s_mqtt"] = mqtt_state;
  doc["display"] = useDisplay;
  if (startMDNS)
    doc["mdns"] = nameMDNS;
  else
    doc["mdns"] = 0;
  String response;

  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response.c_str());
}

void handleRequestMiscAlert()
{
  DynamicJsonDocument doc(32);
  doc["alert"] = alertState;
  if (alertState > 0)
    alertState = 0;
  char response[33];
  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response);
  doc.clear();
}

void handleRequestMisc()
{
  DynamicJsonDocument doc(1024);
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["user"] = mqttuser;
  doc["pass"] = mqttpass;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["buzzer"] = startBuzzer;
  doc["mqbuz"] = mqttBuzzer;
  doc["res"] = senRes;
  doc["display"] = useDisplay;
  doc["page"] = startPage;
  doc["dev"] = devBranch;
  doc["e_mqtt"] = StopOnMQTTError;
  doc["d_mqtt"] = wait_on_error_mqtt / 1000;
  doc["dsa"] = wait_on_Sensor_error_actor / 1000;
  doc["dsi"] = wait_on_Sensor_error_induction / 1000;
  doc["s_mqtt"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["spi"] = startSPI;
  doc["ntp"] = ntpServer;
  String response;
  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response.c_str());
}

void handleRequestFirm()
{
  String request = server.arg(0);
  String message;
  if (startMDNS)
  {
    message = nameMDNS;
    message += F(" V");
  }
  else
  {
#ifdef ESP32
    message = F("MQTTDevice32 V ");
#elif ESP8266
    message = F("MQTTDevice V ");
#endif
  }
  message += Version;
  if (devBranch == 1)
    message += F(" dev");

  server.send(200, FPSTR("text/plain"), message.c_str());
}

void handleGetTitle()
{
#ifdef ESP32
  server.send(200, FPSTR("text/plain"), "MQTTDevice32");
#elif ESP8266
  server.send(200, FPSTR("text/plain"), "MQTTDevice4");
#endif
}

void handleReqSys()
{
  DynamicJsonDocument doc(256);
  String message;
  if (startMDNS)
  {
    message = nameMDNS;
    message += F(" V");
  }
  else
  {
#ifdef ESP32
    message = F("MQTTDevice32 V ");
#elif ESP8266
    message = F("MQTTDevice4 V ");
#endif
  }
  message += Version;
  if (devBranch == 1)
    message += F(" dev");

  doc["firm"] = message;
#ifdef ESP32
  doc["title"] = "MQTTDevice32";
#elif ESP8266
  doc["title"] = "MQTTDevice4";
#endif
  doc["lang"] = selLang;
  String response;
  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response.c_str());
}

void handleSetMisc()
{
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "reset")
    {
      if (server.arg(i) == "true")
      {
        WiFi.disconnect();
        wifiManager.resetSettings();
        millis2wait(PAUSE2SEC);
        ESP.restart();
      }
    }
    if (server.argName(i) == "clear")
    {
      if (server.arg(i) == "true")
      {
        LittleFS.remove("/config.txt");
        delay(PAUSE2SEC);
        ESP.restart();
      }
    }
    if (server.argName(i) == "all")
    {

      int8_t val = 0;
      if (isValidDigit(server.arg(i)))
        val = server.arg(i).toInt();
      if (val == 1)
      {
        LittleFS.remove(CONFIG);
        WiFi.disconnect();
        wifiManager.resetSettings();
        delay(PAUSE1SEC);
        EM_REBOOT();
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
    if (server.argName(i) == "buzzer")
    {
      startBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "mqbuz")
    {
      mqttBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "res")
    {
      senRes = checkBool(server.arg(i));
    }
    if (server.argName(i) == "display")
    {
      useDisplay = checkBool(server.arg(i));
    }
    if (server.argName(i) == "page")
    {
      if (isValidDigit(server.arg(i)))
        startPage = server.arg(i).toInt();
      else
        startPage = 1;
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
    if (server.argName(i) == "enable_mqtt")
    {
      StopOnMQTTError = checkBool(server.arg(i));
    }
    if (server.argName(i) == "delay_mqtt")
      if (isValidInt(server.arg(i)))
      {
        int16_t tmpVal = server.arg(i).toInt();
        wait_on_error_mqtt = constrain(tmpVal, 1, 600) * 1000;
      }
    if (server.argName(i) == "del_sen_act")
    {
      if (isValidInt(server.arg(i)))
      {
        int16_t tmpVal = server.arg(i).toInt();
        wait_on_Sensor_error_actor = constrain(tmpVal, 1, 600) * 1000;
      }
    }
    if (server.argName(i) == "del_sen_ind")
    {
      if (isValidInt(server.arg(i)))
      {
        int16_t tmpVal = server.arg(i).toInt();
        wait_on_Sensor_error_induction = constrain(tmpVal, 1, 600) * 1000;
      }
    }
    if (server.argName(i) == "ntp")
    {
      server.arg(i).toCharArray(ntpServer, maxHostSign);
      checkChars(ntpServer);
    }
    if (server.argName(i) == "spi")
    {
      startSPI = checkBool(server.arg(i));
    }
    if (server.argName(i) == "lang")
    {
      int8_t temp = -1;
      if (isValidDigit(server.arg(i)))
      {
        temp = server.arg(i).toInt();
      }
      if (temp != selLang && temp >= 0)
      {
        selLang = temp;
      }
    }
    yield();
  }
  saveConfig();
  // server.sendHeader("Location", "/", true);
  server.send(200, FPSTR("text/plain"), "ok");
  miscSSE();
}

// Some helper functions WebIf
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
      // server.sendHeader("Location", "/", true);
      // server.send(302, FPSTR("text/plain"), "Restore config");
      // LittleFS.end(); // unmount LittleFS
      // ESP.restart();
    }
  }
}

void handleGetLanguage()
{
  String response = "";
  response += selLang;
  server.send_P(200, "text/plain", response.c_str());
}