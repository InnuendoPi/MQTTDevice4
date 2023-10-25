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

void setTicker()
{
  // Ticker Objekte
  TickerSen.config(tickerSenCallback, SEN_UPDATE, 0);
  TickerAct.config(tickerActCallback, ACT_UPDATE, 0);
  TickerInd.config(tickerIndCallback, IND_UPDATE, 0);
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
  if (!startBuzzer)
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
      Serial.printf("*** SYSINFO: Update development firmware retries count %d\n", anzahlSys);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      Serial.printf("*** SYSINFO: Update firmware retries count %d\n", anzahlSys);
    }
    if (anzahlSys > MAXVERSUCHE)
      alertState = 2;
    else
      alertState = 1;
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
      Serial.printf("*** SYSINFO: Update development tools retries count %d\n", anzahlTools);
      check = LittleFS.remove(DEVBRANCH);
    }
    else
    {
      Serial.printf("*** SYSINFO: Update tools retries count %d\n", anzahlTools);
    }
    if (anzahlTools > MAXVERSUCHE)
      alertState = 2;
  }
}

void EM_MDNSET() // MDNS setup
{
  if (startMDNS)
  {
    if (mdns.begin(nameMDNS))
      Serial.printf("*** SYSINFO: mDNS started as %s connected to %s Time: %s RSSI=%d\n", nameMDNS, WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
    else
      Serial.printf("*** SYSINFO: error start mDNS! IP Adresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }
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
      DEBUG_MSG("EM ACTER: Aktor: %s  switched off\n", actors[i].getActorName().c_str());
    }
    yield();
  }
  // Stop induction
  if (inductionCooker.getisInduon())
  {
    DEBUG_MSG("%s\n", "EM INDOFF: induction switched off");
    inductionCooker.setNewPower(0);
    inductionCooker.setisInduon(false);
    inductionCooker.Update();
  }
  server.send(205, FPSTR("text/plain"), "reboot");
  LittleFS.end(); // unmount LittleFS
  ESP.restart();
}

void EM_WIFICONNECT(const WiFiEventStationModeGotIP &event)
{
  if (wlanStatus > 0)
  {
    wifiManager.setConnectTimeout(30); // 30sek Timeout für WIFI_STA Modus. Anschließend AP Mode
    wlanStatus = 0;
  }
}

void EM_WIFIDISCONNECT(const WiFiEventStationModeDisconnected &event)
{
  if (wlanStatus == 0)
    wifiManager.setConnectTimeout(300); // 30sek Timeout für WIFI_STA Modus. Anschließend AP Mode
  wlanStatus++;
}

void EM_MQTTCONNECT() // MQTT connect
{
  if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
  {
    pubsubClient.setServer(mqtthost, mqttport);
    pubsubClient.setCallback(mqttcallback);
    pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass);
    DEBUG_MSG("Connecting MQTT broker %s with client-id: %s user: %s pass: %s\n", mqtthost, mqtt_clientid, mqttuser, mqttpass);
  }
}

void EM_MQTTSUBSCRIBE() // MQTT subscribe
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("%s\n", "MQTT connected! Subscribing...");
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
    }
    else if (mqttBuzzer) // mqttBuzzer only
      cbpi4notification_subscribe();

    TickerMQTT.stop();
  }
}

void EM_MQTTUPDATE()
{
  if (pubsubClient.connected())
  {
    mqtt_state = true;
    pubsubClient.loop();
    if (TickerMQTT.state() == RUNNING)
      TickerMQTT.stop();

    return;
  }
  else
  {
    if (TickerMQTT.state() != RUNNING)
    {
      DEBUG_MSG("%s\n", "MQTT not connected - TickerMQTT started");
      DEBUG_MSG("pubdubClient error rc=%d \n", pubsubClient.state());
      // mqtt_state = false;
      TickerMQTT.start();
      mqttconnectlasttry = millis();
      miscSSE();
    }
    TickerMQTT.update();
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
        DEBUG_MSG("EM MQTTRES: %s isOnBeforeError: %d Powerlevel: %d\n", actors[i].getActorName().c_str(), actors[i].getIsOnBeforeError(), actors[i].getActorPower());
        actors[i].setIsOn(actors[i].getIsOnBeforeError());
        actors[i].setActorState(true);
        actors[i].Update();
      }
      yield();
    }
    if (!inductionCooker.getInductionState())
    {
      DEBUG_MSG("EM MQTTRES: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d\n", inductionCooker.getPower(), inductionCooker.getPowerLevelOnError(), inductionCooker.getPowerLevelBeforeError());
      inductionCooker.setNewPower(inductionCooker.getPowerLevelBeforeError());
      inductionCooker.setisInduon(true);
      inductionCooker.setInductionState(true);
      inductionCooker.Update();
      DEBUG_MSG("EM MQTTRES: Induction restore old value: %d\n", inductionCooker.getNewPower());
    }
  }
}

void EM_MQTTERROR() // MQTT Error -> handling
{
  if (pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass))
  {
    DEBUG_MSG("%s", "MQTT auto reconnect successful. Subscribing..\n");
    EM_MQTTSUBSCRIBE();
    EM_MQTTRESTORE();
    miscSSE();
    return;
  }
  if (millis() - mqttconnectlasttry >= wait_on_error_mqtt)
  {
    if (StopOnMQTTError && mqtt_state)
    {
      mqtt_state = false; // MQTT in error state
      if (startBuzzer)
        sendAlarm(ALARM_ERROR);
      DEBUG_MSG("EM MQTTER: MQTT Broker %s not availible! StopOnMQTTError: %d mqtt_state: %d\n", mqtthost, StopOnMQTTError, mqtt_state);
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
        debLog.print(timeClient.getFormattedTime().c_str());
        debLog.print("\t");
        debLog.print(valText);
        debLog.print("\n");
        debLog.close();
    }
}
