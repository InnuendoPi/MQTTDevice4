void millis2wait(const int &value)
{
  unsigned long pause = millis();
  while (millis() < pause + value)
  {
    yield(); // wait approx. [period] milliseconds
  }
}
void micros2wait(const int &value)
{
  unsigned long pause = micros();
  while (micros() < pause + value)
  {
    yield(); // wait approx. [period] microsseconds
  }
}

void setTicker()
{
  TickerSen.config(tickerSenCallback, (SENCYLCE * SEN_UPDATE), 0);
  TickerAct.config(tickerActCallback, ACT_UPDATE, 0);
  TickerInd.config(tickerIndCallback, IND_UPDATE, 0);
  TickerPUBSUB.config(tickerPUBSUBCallback, tickerPUSUB, 0);
  TickerMQTT.config(tickerMQTTCallback, tickerMQTT, 0);
  TickerDisp.config(tickerDispCallback, DISP_UPDATE, 0);
  TickerTime.config(tickerTimeCallback, TIME_UPDATE, 0);
  TickerMQTT.stop();
}

void setupTime()
{
#ifdef ESP32
  configTzTime(ntpZone, ntpServer);
#elif ESP8266
  configTime(ntpZone, ntpServer);
#endif
  getLocalTime(&timeinfo);
  // 1 Minute vor Winterzeit 27.10.2024 2:59:00 1729994340
  // 1 Minute vor Sommerzeit 31.03.2024 1:59:00 1711846740

  // timeval epoch = {1711846740, 0};
  // settimeofday((const timeval*)&epoch, 0);

  strftime(zeit, sizeof(zeit), "%H:%M:%S", &timeinfo);
  TickerTime.start();
  // TickerTime.updatenow();
}

String decToHex(unsigned char decValue, unsigned char desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength)
    hexString = "0" + hexString;

  return "0x" + hexString;
}

String IPtoString(IPAddress ip)
{
  String res = "";
  for (uint8_t i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

unsigned char convertCharToHex(char ch)
{
  unsigned char returnType;
  switch (ch)
  {
  case '0':
    returnType = 0;
    break;
  case '1':
    returnType = 1;
    break;
  case '2':
    returnType = 2;
    break;
  case '3':
    returnType = 3;
    break;
  case '4':
    returnType = 4;
    break;
  case '5':
    returnType = 5;
    break;
  case '6':
    returnType = 6;
    break;
  case '7':
    returnType = 7;
    break;
  case '8':
    returnType = 8;
    break;
  case '9':
    returnType = 9;
    break;
  case 'A':
    returnType = 10;
    break;
  case 'B':
    returnType = 11;
    break;
  case 'C':
    returnType = 12;
    break;
  case 'D':
    returnType = 13;
    break;
  case 'E':
    returnType = 14;
    break;
  case 'F':
    returnType = 15;
    break;
  default:
    returnType = 0;
    break;
  }
  return returnType;
}

void sendAlarm(const uint8_t &setAlarm)
{
  if (PIN_BUZZER == -100)
    return;
  switch (setAlarm)
  {
  case ALARM_ON:
    tone(PIN_BUZZER, 440, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 660, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_OFF:
    tone(PIN_BUZZER, 880, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 660, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 440, 50);
    break;
  case ALARM_INFO:
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_SUCCESS:
    tone(PIN_BUZZER, 880, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 880, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_WARNING:
    tone(PIN_BUZZER, 660, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 660, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 660, 50);
    millis2wait(150);
    break;
  case ALARM_ERROR:
    tone(PIN_BUZZER, 440, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 440, 50);
    millis2wait(150);
    tone(PIN_BUZZER, 440, 50);
    millis2wait(150);
    break;
  default:
    break;
  }
}

void EM_LOG()
{
  if (LittleFS.exists(LOGUPDATESYS)) // WebUpdate Firmware
  {
    fsUploadFile = LittleFS.open(LOGUPDATESYS, "r");
    int anzahlSys = 0;
    if (fsUploadFile)
    {
      anzahlSys = char(fsUploadFile.read()) - '0';
    }
    fsUploadFile.close();
    bool check = LittleFS.remove(LOGUPDATESYS);
    if (LittleFS.exists(DEVBRANCH)) // WebUpdate Firmware
    {
      DEBUG_INFO("SYS", "*** SYSINFO: Firmware Update development Anzahl Versuche: %d", anzahlSys);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      DEBUG_INFO("SYS", "*** SYSINFO: Firmware Update Anzahl Versuche: %d", anzahlSys);
    }
    alertState = true;
  }

  if (LittleFS.exists(LOGUPDATETOOLS)) // WebUpdate Firmware
  {
    fsUploadFile = LittleFS.open(LOGUPDATETOOLS, "r");
    int anzahlTools = 0;
    if (fsUploadFile)
    {
      anzahlTools = char(fsUploadFile.read()) - '0';
    }
    fsUploadFile.close();

    bool check = LittleFS.remove(LOGUPDATETOOLS);
    if (LittleFS.exists(DEVBRANCH)) // WebUpdate Firmware
    {
      DEBUG_INFO("SYS", "*** SYSINFO: Framework Update development Anzahl der Versuche: %d", anzahlTools);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      DEBUG_INFO("SYS", "*** SYSINFO: Framework Update Anzahl der Versuche: %d", anzahlTools);
    }
    alertState = true;
  }
}

void EM_MDNSET() // MDNS setup
{
  uint8_t tmpLevel = getTagLevel("SYS");
  setTagLevel("SYS", INNU_INFO);
#ifdef ESP32
  if (MDNS.begin(nameMDNS))
  {
    MDNS.addService("http", "tcp", PORT);
    DEBUG_INFO("SYS", "*** SYSINFO: mDNS started as %s.local connected to %s Time: %s RSSI: %d", nameMDNS, WiFi.localIP().toString().c_str(), zeit, WiFi.RSSI());
  }
#elif ESP8266
  if (mdns.begin(nameMDNS))
  {
    DEBUG_INFO("SYS", "*** SYSINFO: mDNS started as %s.local connected to %s Time: %s RSSI: %d", nameMDNS, WiFi.localIP().toString().c_str(), zeit, WiFi.RSSI());
  }
#endif
  else
    DEBUG_INFO("SYS", "*** SYSINFO: error start mDNS! IP Adresse: %s Time: %s RSSI: %d", WiFi.localIP().toString().c_str(), zeit, WiFi.RSSI());
  setTagLevel("SYS", tmpLevel);
}

void EM_REBOOT() // Reboot ESP
{
  // Stop actors
  for (int i = 0; i < numberOfActors; i++)
  {
    if (actors[i].getIsOn())
    {
      actors[i].setIsOn(false);
      actors[i].Update();
    }
  }
  // Stop induction
  if (inductionCooker.getisInduon())
  {
    inductionCooker.setNewPower(0);
    inductionCooker.setisInduon(false);
    inductionCooker.Update();
  }
  server.sendHeader("Location", "/", true);
  replyReboot("reboot");
  // server.send(205, FPSTR("text/plain"), "reboot");
  LittleFS.end(); // unmount LittleFS
  ESP.restart();
}

void EM_MQTTCONNECT() // MQTT connect
{
  if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
  {
    // if (useFerm)
    //   pubsubClient.setBufferSize(2048); // extra large Buffersize to receive fermenterupdate
    // else
    //   pubsubClient.setBufferSize(512);  // large buffersize to receive all CBPi4 MQTT payload

    pubsubClient.setBufferSize(2048); // large buffersize to receive all CBPi4 MQTT payload
    pubsubClient.setServer(mqtthost, mqttport);
    pubsubClient.setCallback(mqttcallback);
    pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass);
    DEBUG_INFO("SYS", "Connecting MQTT broker %s with client-id: %s user: %s pass: %s", mqtthost, mqtt_clientid, mqttuser, mqttpass);
  }
}

void EM_MQTTSUBSCRIBE() // MQTT subscribe
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("SYS", "MQTT connected! Subscribing...");
    mqtt_state = true; // MQTT state ok
    for (int i = 0; i < numberOfActors; i++)
    {
      actors[i].mqtt_subscribe();
      yield();
    }
    if (inductionCooker.getIsEnabled())
      inductionCooker.mqtt_subscribe();
    if (useDisplay)
    {
      dispPublishmqtt();
      cbpi4kettle_subscribe();
      cbpi4steps_subscribe();
      cbpi4notification_subscribe();
      if (useFerm)
      {
        cbpi4fermenter_subscribe();
        cbpi4fermentersteps_subscribe();
      }
    }
    else if (mqttBuzzer) // mqttBuzzer only
      cbpi4notification_subscribe();

    TickerMQTT.stop();
  }
}

void EM_MQTTRESTORE() // restore saved values after reconnect MQTT
{
  if (pubsubClient.connected())
  {
    mqtt_state = true;
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].getActorSwitch() && !actors[i].getActorState())
      {
        DEBUG_INFO("SYS", "EM MQTTRES: %s isOnBeforeError: %d Powerlevel: %d", actors[i].getActorName().c_str(), actors[i].getIsOnBeforeError(), actors[i].getActorPower());
        actors[i].setIsOn(actors[i].getIsOnBeforeError());
        actors[i].setActorState(true);
        actors[i].Update();
      }
      yield();
    }
    if (!inductionCooker.getInductionState())
    {
      DEBUG_INFO("SYS", "EM MQTTRES: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d", inductionCooker.getPower(), inductionCooker.getPowerLevelOnError(), inductionCooker.getPowerLevelBeforeError());
      inductionCooker.setNewPower(inductionCooker.getPowerLevelBeforeError());
      inductionCooker.setisInduon(true);
      inductionCooker.setInductionState(true);
      inductionCooker.Update();
      DEBUG_INFO("SYS", "EM MQTTRES: Induction restore old value: %d", inductionCooker.getNewPower());
    }
  }
}

void EM_MQTTERROR() // MQTT Error -> handling
{
  if (pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass))
  {
    DEBUG_INFO("SYS", "MQTT auto reconnect successful. Subscribing..");
    EM_MQTTSUBSCRIBE();
    EM_MQTTRESTORE();
    miscSSE();
    return;
  }
  if (millis() - mqttconnectlasttry >= wait_on_error_mqtt)
  {
    if (StopOnMQTTError)
    {
      mqtt_state = false; // MQTT in error state
      if (PIN_BUZZER != -100)
        sendAlarm(ALARM_ERROR);
      DEBUG_ERROR("SYS", "EM MQTTERROR: MQTT Broker %s not availible! StopOnMQTTError: %d mqtt_state: %d", mqtthost, StopOnMQTTError, mqtt_state);
      actERR();
      inductionCooker.indERR();
    }
  }
}

void debugLog(String valFile, String valText)
{
  File debLog = LittleFS.open(valFile, "a");
  if (debLog)
  {
    debLog.print(zeit);
    debLog.print("\t");
    debLog.print(valText);
    debLog.print("\n");
    debLog.close();
  }
}

#ifdef ESP8266
void EM_WIFICONNECT(const WiFiEventStationModeGotIP &event)
{
  if (wlanStatus > 0)
  {
    wifiManager.setConnectTimeout(30); // 30sek Timeout für WIFI_STA Modus. Anschließend AP Mode
    wlanStatus = 0;
  }
}
#endif
#ifdef ESP8266
void EM_WIFIDISCONNECT(const WiFiEventStationModeDisconnected &event)
{
  if (wlanStatus == 0)
    wifiManager.setConnectTimeout(300); // 30sek Timeout für WIFI_STA Modus. Anschließend AP Mode
  wlanStatus++;
}
#endif

#ifdef ESP32
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_READY:
    DEBUG_VERBOSE("SYS", "WiFi interface ready");
    break;
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
    DEBUG_VERBOSE("SYS", "Completed scan for access points");
    break;
  case ARDUINO_EVENT_WIFI_STA_START:
    DEBUG_VERBOSE("SYS", "WiFi client started");
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    DEBUG_VERBOSE("SYS", "WiFi clients stopped");
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    DEBUG_VERBOSE("SYS", "Connected to access point");
    // wifiManager.setConnectTimeout(30);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    WiFi.reconnect();
    break;
  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    DEBUG_VERBOSE("SYS", "Authentication mode of access point has changed");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // wifiManager.setConnectTimeout(30);
    DEBUG_VERBOSE("SYS", "WiFiStationGotIP %s", WiFi.localIP().toString().c_str());
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    // wifiManager.setConnectTimeout(300);
    DEBUG_VERBOSE("SYS", "Lost IP address and IP address is reset to 0. Reconnect Timeout: 300s");
    WiFi.reconnect();
    break;
  case ARDUINO_EVENT_WPS_ER_SUCCESS:
    DEBUG_VERBOSE("SYS", "WiFi Protected Setup (WPS): succeeded in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_FAILED:
    DEBUG_VERBOSE("SYS", "WiFi Protected Setup (WPS): failed in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_TIMEOUT:
    DEBUG_VERBOSE("SYS", "WiFi Protected Setup (WPS): timeout in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_PIN:
    DEBUG_VERBOSE("SYS", "WiFi Protected Setup (WPS): pin code in enrollee mode");
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    // wifiManager.setConnectTimeout(300);
    DEBUG_VERBOSE("SYS", "WiFi access point started");
    break;
  case ARDUINO_EVENT_WIFI_AP_STOP:
    DEBUG_VERBOSE("SYS", "WiFi access point  stopped");
    // EM_REBOOT();
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    DEBUG_VERBOSE("SYS", "Client connected");
    break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    DEBUG_VERBOSE("SYS", "Client disconnected");
    break;
  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    DEBUG_VERBOSE("SYS", "Assigned IP address to client");
    break;
  case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
    DEBUG_VERBOSE("SYS", "Received probe request");
    break;
  case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
    DEBUG_VERBOSE("SYS", "AP IPv6 is preferred");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    DEBUG_VERBOSE("SYS", "STA IPv6 is preferred");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP6:
    DEBUG_VERBOSE("SYS", "Ethernet IPv6 is preferred");
    break;
  case ARDUINO_EVENT_ETH_START:
    DEBUG_VERBOSE("SYS", "Ethernet started");
    break;
  case ARDUINO_EVENT_ETH_STOP:
    DEBUG_VERBOSE("SYS", "Ethernet stopped");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    DEBUG_VERBOSE("SYS", "Ethernet connected");
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    DEBUG_VERBOSE("SYS", "Ethernet disconnected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    DEBUG_VERBOSE("SYS", "Obtained IP address");
    break;
  default:
    break;
  }
}
#endif
