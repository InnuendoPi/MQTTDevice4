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

// Prüfe WebIf Eingaben
float formatDOT(String str)
{
  str.trim();
  str.replace(',', '.');
  if (isValidFloat(str))
    return str.toFloat();
  else
    return 0;
}

bool isValidInt(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (isdigit(str.charAt(i)))
      continue;
    if (str.charAt(i) == '.')
      return false;
    return false;
  }
  return true;
}

bool isValidFloat(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (i == 0)
    {
      if (str.charAt(i) == '-')
        continue;
    }
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

bool isValidDigit(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

bool checkBool(const String &value)
{
  if (value == "true")
    return true;
  else
    return false;
}

void checkChars(char *input)
{
  char *output = input;
  int j = 0;
  for (int i = 0; i < strlen(input); i++)
  {
    if (input[i] != ' ' && input[i] != '\n' && input[i] != '\r') // Suche nach Leerzeichen und CR LF
      output[j] = input[i];
    else
      j--;

    j++;
  }
  output[j] = '\0';
  *input = *output;
  return;
}

String checkName(String tmpName, int laenge, bool leer)
{
  // max Länge im Display 20 Zeichen
  tmpName = convertUmlaute(tmpName, leer);
  if (tmpName.length() > laenge)
    tmpName = tmpName.substring(0, laenge);
  tmpName.trim();
  return tmpName;
}

String convertUmlaute(String val, bool space)
{
  if (space)
    val.replace(" ", "_");
  val.replace("ü", "ue");
  val.replace("ä", "ae");
  val.replace("ö", "oe");
  val.replace("Ü", "Ue");
  val.replace("Ä", "Ae");
  val.replace("Ö", "Oe");
  val.replace("ß", "ss");
  return val;
  // "Ö" "\326" // ASCII 214
  // "ö" "\366" // ASCII 246
  // "Ä" "\304" // ASCII 196
  // "ä" "\344" // ASCII 228
  // "Ü" "\334" // ASCII 220
  // "ü" "\365" // ASCII 252
  // ä	\u00e4
  // Ä	\u00c4
  // ö	\u00f6
  // Ö	\u00d6
  // ü	\u00fc
  // Ü	\u00dc
  // ß	\u00df
}

void setTicker()
{
  // Ticker Objekte
  uint8_t senFaktor = 1; // dyn update intervall sensors
  if (numberOfSensors >= 2)
    senFaktor = 2;
  else if (numberOfSensors >= 4)
    senFaktor = 3;

  TickerSen.config(tickerSenCallback, (senFaktor * SEN_UPDATE), 0);
  TickerAct.config(tickerActCallback, ACT_UPDATE, 0);
  TickerInd.config(tickerIndCallback, IND_UPDATE, 0);
  TickerPUBSUB.config(tickerPUBSUBCallback, tickerPUSUB, 0);
  TickerMQTT.config(tickerMQTTCallback, tickerMQTT, 0);
  TickerDisp.config(tickerDispCallback, DISP_UPDATE, 0);
  TickerMQTT.stop();
}

void checkSummerTime()
{
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  int year = ti->tm_year + 1900;
  int month = ti->tm_mon + 1;
  int day = ti->tm_mday;
  int hour = ti->tm_hour;
  int tzHours = 1; // UTC: 0 MEZ: 1
  int x1, x2, x3, lastyear;
  int lasttzHours;
  if (month < 3 || month > 10)
  {
    timeClient.setTimeOffset(3600); // +1h
    return;
  }
  if (month > 3 && month < 10)
  {
    timeClient.setTimeOffset(7200); // +2h
    return;
  }
  if (year != lastyear || tzHours != lasttzHours)
  {
    x1 = 1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7);
    x2 = 1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7);
    lastyear = year;
    lasttzHours = tzHours;
  }
  x3 = hour + 24 * day;
  if (month == 3 && x3 >= x1 || month == 10 && x3 < x2)
  {
    timeClient.setTimeOffset(7200); // +2h
    return;
  }
  else
  {
    timeClient.setTimeOffset(3600); // +1h
    return;
  }
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
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_OFF:
    tone(PIN_BUZZER, 880, 50);
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 440, 50);
    break;
  case ALARM_INFO:
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_SUCCESS:
    tone(PIN_BUZZER, 880, 50);
    delay(150);
    tone(PIN_BUZZER, 880, 50);
    delay(150);
    tone(PIN_BUZZER, 880, 50);
    break;
  case ALARM_WARNING:
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    break;
  case ALARM_ERROR:
    tone(PIN_BUZZER, 440, 50);
    delay(150);
    tone(PIN_BUZZER, 440, 50);
    delay(150);
    tone(PIN_BUZZER, 440, 50);
    delay(150);
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
      Serial.printf("*** SYSINFO: Firmware Update development Anzahl Versuche: %d\n", anzahlSys);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      Serial.printf("*** SYSINFO: Firmware Update Anzahl Versuche: %d\n", anzahlSys);
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
      Serial.printf("*** SYSINFO: Framework Update development Anzahl der Versuche: %d\n", anzahlTools);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      Serial.printf("*** SYSINFO: Framework Update Anzahl der Versuche: %d\n", anzahlTools);
    }
    alertState = true;
  }
}

void EM_MDNSET() // MDNS setup
{
#ifdef ESP32
  if (MDNS.begin(nameMDNS))
  {
    MDNS.addService("http", "tcp", PORT);
    Serial.printf("*** SYSINFO: mDNS started as %s.local connected to %s Time: %s RSSI: %d\n", nameMDNS, WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }
#elif ESP8266
  if (mdns.begin(nameMDNS))
  {
    Serial.printf("*** SYSINFO: mDNS started as %s.local connected to %s Time: %s RSSI: %d\n", nameMDNS, WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }
#endif
  else
    Serial.printf("*** SYSINFO: error start mDNS! IP Adresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
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
  server.send(205, FPSTR("text/plain"), "reboot");
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

    pubsubClient.setBufferSize(3096); // large buffersize to receive all CBPi4 MQTT payload
    pubsubClient.setServer(mqtthost, mqttport);
    pubsubClient.setCallback(mqttcallback);
    pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass);
#ifdef ESP32
    log_e("Connecting MQTT broker %s with client-id: %s user: %s pass: %s", mqtthost, mqtt_clientid, mqttuser, mqttpass);
#endif
  }
}

void EM_MQTTSUBSCRIBE() // MQTT subscribe
{
  if (pubsubClient.connected())
  {
#ifdef ESP32
    log_e("%s", "MQTT connected! Subscribing...");
#endif
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
#ifdef ESP32
        log_e("EM MQTTRES: %s isOnBeforeError: %d Powerlevel: %d", actors[i].getActorName().c_str(), actors[i].getIsOnBeforeError(), actors[i].getActorPower());
#endif
        actors[i].setIsOn(actors[i].getIsOnBeforeError());
        actors[i].setActorState(true);
        actors[i].Update();
      }
      yield();
    }
    if (!inductionCooker.getInductionState())
    {
#ifdef ESP32
      log_e("EM MQTTRES: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d", inductionCooker.getPower(), inductionCooker.getPowerLevelOnError(), inductionCooker.getPowerLevelBeforeError());
#endif
      inductionCooker.setNewPower(inductionCooker.getPowerLevelBeforeError());
      inductionCooker.setisInduon(true);
      inductionCooker.setInductionState(true);
      inductionCooker.Update();
#ifdef ESP32
      log_e("EM MQTTRES: Induction restore old value: %d", inductionCooker.getNewPower());
#endif
    }
  }
}

void EM_MQTTERROR() // MQTT Error -> handling
{
  if (pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass))
  {
#ifdef ESP32
    log_e("%s", "MQTT auto reconnect successful. Subscribing..");
#endif
    EM_MQTTSUBSCRIBE();
    EM_MQTTRESTORE();
    miscSSE();
    return;
  }
  if (millis() - mqttconnectlasttry >= wait_on_error_mqtt)
  {
    // if (StopOnMQTTError && mqtt_state)
    if (StopOnMQTTError)
    {
      mqtt_state = false; // MQTT in error state
      if (PIN_BUZZER != -100)
        sendAlarm(ALARM_ERROR);
#ifdef ESP32
      log_e("EM MQTTERROR: MQTT Broker %s not availible! StopOnMQTTError: %d mqtt_state: %d", mqtthost, StopOnMQTTError, mqtt_state);
#endif
      actERR();
      inductionCooker.indERR();
      // miscSSE();
    }
  }
}

void debugLog(String valFile, String valText)
{
  File debLog = LittleFS.open(valFile, "a");
  if (debLog)
  {
    debLog.print(timeClient.getFormattedTime().c_str());
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
    log_e("WiFi interface ready");
    break;
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
    log_e("Completed scan for access points");
    break;
  case ARDUINO_EVENT_WIFI_STA_START:
    log_e("WiFi client started");
    break;
  case ARDUINO_EVENT_WIFI_STA_STOP:
    log_e("WiFi clients stopped");
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    log_e("Connected to access point");
    // wifiManager.setConnectTimeout(30);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    WiFi.reconnect();
    break;
  case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    log_e("Authentication mode of access point has changed");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // wifiManager.setConnectTimeout(30);
    log_e("WiFiStationGotIP %s", WiFi.localIP().toString().c_str());
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    // wifiManager.setConnectTimeout(300);
    log_e("Lost IP address and IP address is reset to 0. Reconnect Timeout: 300s");
    WiFi.reconnect();
    break;
  case ARDUINO_EVENT_WPS_ER_SUCCESS:
    log_e("WiFi Protected Setup (WPS): succeeded in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_FAILED:
    log_e("WiFi Protected Setup (WPS): failed in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_TIMEOUT:
    log_e("WiFi Protected Setup (WPS): timeout in enrollee mode");
    break;
  case ARDUINO_EVENT_WPS_ER_PIN:
    log_e("WiFi Protected Setup (WPS): pin code in enrollee mode");
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    // wifiManager.setConnectTimeout(300);
    log_e("WiFi access point started");
    break;
  case ARDUINO_EVENT_WIFI_AP_STOP:
    log_e("WiFi access point  stopped");
    // EM_REBOOT();
    break;
  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    log_e("Client connected");
    break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    log_e("Client disconnected");
    break;
  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    log_e("Assigned IP address to client");
    break;
  case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
    log_e("Received probe request");
    break;
  case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
    log_e("AP IPv6 is preferred");
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    log_e("STA IPv6 is preferred");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP6:
    log_e("Ethernet IPv6 is preferred");
    break;
  case ARDUINO_EVENT_ETH_START:
    log_e("Ethernet started");
    break;
  case ARDUINO_EVENT_ETH_STOP:
    log_e("Ethernet stopped");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    log_e("Ethernet connected");
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    log_e("Ethernet disconnected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    log_e("Obtained IP address");
    break;
  default:
    break;
  }
}
#endif