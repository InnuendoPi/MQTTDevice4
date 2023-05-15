void handleRoot()
{
  // server.sendHeader("Location", "/index.html", true); // Redirect to our html web page
  // server.send(302, "text/plain", "");
  // server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.sendHeader(PSTR("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, sizeof(index_htm_gz));
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
  
  int fsize = dataFile.size();
  server.sendHeader("Content-Length", (String)(fsize) );
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  
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
  // Uncomment for debug output received MQTT payloads
  // DEBUG_MSG("Web: Received MQTT Topic with char payload: %s\n", topic);
  // Serial.print("Web: Payload: ");
  // for (int i = 0; i < length; i++)
  // {
    // Serial.print((char)payload[i]);
  // }
  // Serial.println(" ");
  
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
        // Serial.printf("Actor payload received %s\n", actors[i].argument_actor.c_str());
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
      DEBUG_MSG("%s\n", "Web: kettleupdate");
      cbpi4kettle_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, stepupdate);
    if (p)
    {
      DEBUG_MSG("%s\n", "Web: stepsupdate");
      cbpi4steps_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, notificationupdate);
    if (p)
    {
      DEBUG_MSG("%s\n", "Web: notificationupdate");
      cbpi4notification_handlemqtt(payload_msg);
      return;
    }
    p = strstr(topic, sensorupdate);
    if (p)
    {
      DEBUG_MSG("%s\n", "Web: sensorupdate");
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
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["s_mqtt"] = mqtt_state;
  doc["display"] = useDisplay;
  doc["i2c"] = useI2C;
  if (startMDNS)
    doc["mdns"] = nameMDNS;
  else
    doc["mdns"] = 0;
  doc["alert"] = alertState;
  if (alertState)
    alertState = false;
  String response;
  
  serializeJson(doc, response);
  server.send(200, "application/json", response.c_str());
  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();
  // DEBUG_MSG("WEB Misc2 JSON config length: %d\n", len);
  // DEBUG_MSG("WEB Misc2 JSON memory usage: %d\n", memoryUsed);
}

void handleRequestMisc()
{
  // StaticJsonDocument<768> doc;
  DynamicJsonDocument doc(1024);
  doc["host"] = mqtthost;
  doc["port"] = mqttport;
  doc["user"] = mqttuser;
  doc["pass"] = mqttpass;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["i2c"] = useI2C;
  doc["buzzer"] = startBuzzer;
  doc["mqbuz"] = mqttBuzzer;
  doc["res"] = senRes;
  doc["display"] = useDisplay;
  // doc["page"] = startPage;
  doc["dev"] = devBranch;
  doc["e_mqtt"] = StopOnMQTTError;
  doc["d_mqtt"] = wait_on_error_mqtt / 1000;
  doc["dsa"] = wait_on_Sensor_error_actor / 1000;
  doc["dsi"] = wait_on_Sensor_error_induction / 1000;
  doc["s_mqtt"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
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
        wait_on_error_mqtt = constrain(server.arg(i).toInt(), 1, 600) * 1000;

      }
    if (server.argName(i) == "del_sen_act")
    {
      if (isValidInt(server.arg(i)))
      {
        // wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
        wait_on_Sensor_error_actor = constrain(server.arg(i).toInt(), 1, 600) * 1000;
      }
    }
    if (server.argName(i) == "del_sen_ind")
    {
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_induction = constrain(server.arg(i).toInt(), 1, 600) * 1000;
      }
    }
    yield();
  }
  saveConfig();
  server.send(200, "text/plain", "ok");
}

// Some helper functions WebIf
void rebootDevice()
{
  server.send(205, "text/plain", "reboot");
  EM_REBOOT();
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

void handleRestore()
{
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = "config.txt"; // upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    DEBUG_MSG("WEB restore config file: %s\n", filename.c_str());
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
      DEBUG_MSG("WEB restore configuration Size: %d\n", upload.totalSize);
      server.sendHeader("Location", "/index.html"); // Redirect the client to the success page
      server.send(303);
      EM_REBOOT();
    }
  }
}
