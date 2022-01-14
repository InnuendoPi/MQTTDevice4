void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  server.send(302, "text/plain", "");
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
}

void handleRequestMisc2()
{
  StaticJsonDocument<256> doc;
  doc["mqtthost"] = mqtthost;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["mqtt_state"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["alertstate"] = alertState;
  if (alertState)
    alertState = false;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestMisc()
{
  StaticJsonDocument<384> doc;
  doc["mqtthost"] = mqtthost;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["buzzer"] = startBuzzer;
  doc["display"] = useDisplay;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["delay_mqtt"] = wait_on_error_mqtt / 1000;
  doc["del_sen_act"] = wait_on_Sensor_error_actor / 1000;
  doc["del_sen_ind"] = wait_on_Sensor_error_induction / 1000;
  doc["mqtt_state"] = mqtt_state; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  // doc["alertstate"] = alertState;
  // if (alertState)
  //   alertState = false;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
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
      message = "MQTTDevice4 V ";
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
    if (server.argName(i) == "MQTTHOST")
      server.arg(i).toCharArray(mqtthost, sizeof(mqtthost));

    if (server.argName(i) == "buzzer")
    {
      startBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "display")
    {
      useDisplay = checkBool(server.arg(i));
    }
    if (server.argName(i) == "mdns_name")
    {
      server.arg(i).toCharArray(nameMDNS, sizeof(nameMDNS));
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
        wait_on_error_mqtt = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_act")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_ind")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_induction = server.arg(i).toInt() * 1000;
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
