// void pageCallback()
// {
//   if (startBuzzer)
//     sendAlarm(ALARM_INFO);

//   // currentPageID bei Event Touch nicht aktuell
//   // activePage = nextion.currentPageID;

//   TickerDisp.updatenow();
// }

void pageCallback0()
{
  if (startBuzzer)
    sendAlarm(ALARM_INFO);

  tempPage = 0;
  TickerDisp.updatenow();
}
void pageCallback1()
{
  if (startBuzzer)
    sendAlarm(ALARM_INFO);

  tempPage = 1;
  TickerDisp.updatenow();
}
void pageCallback2()
{
  if (startBuzzer)
    sendAlarm(ALARM_INFO);

  tempPage = 2;
  TickerDisp.updatenow();
}

void powerButtonCallback()
{
  inductionCooker.induction_state = !inductionCooker.induction_state;
}

void tickerDispCallback()
{
  if (tempPage < 0)
    activePage = nextion.getCurrentPageID();
  else
  {
    activePage = tempPage;
    tempPage = -1;
  }

  char ipMQTT[50];
  sprintf_P(uhrzeit, (PGM_P)F("%02d:%02d"), timeClient.getHours(), timeClient.getMinutes());
  if (startMDNS)
    sprintf_P(ipMQTT, (PGM_P)F("http://%s - %s"), nameMDNS, WiFi.localIP().toString().c_str());
  else
    sprintf_P(ipMQTT, (PGM_P)F("http://%s"), WiFi.localIP().toString().c_str());

  activePage = nextion.getCurrentPageID();
  switch (activePage)
  {
  case 0:            // BrewPage
    if (!activeBrew) // aktiver Step vorhanden?
      strlcpy(currentStepName, "BrewPage", maxStepSign);

    uhrzeit_text.attribute("txt", uhrzeit);
    mqttDevice.attribute("txt", ipMQTT);

    BrewPage();
    break;
  case 1:            // KettlePage
    if (!activeBrew) // aktiver Step vorhanden?
      strlcpy(currentStepName, sensors[0].getName().c_str(), maxStepSign);

    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);

    p1mqttDevice.attribute("txt", ipMQTT);
    p1uhrzeit_text.attribute("txt", uhrzeit);

    KettlePage();
    break;
  case 2: // Induction mode
    // DEBUG_MSG("Ticker: dispCallback InductionPage activePage: %d\n", activePage);
    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);
    p2uhrzeit_text.attribute("txt", uhrzeit);
    InductionPage();
    break;
  }
}

void tickerSenCallback() // Timer Objekt Sensoren
{
  DS18B20.requestTemperatures();
  switch (sensorsStatus)
  {
  case EM_OK:
    // all sensors ok
    lastSenInd = 0; // Delete induction timestamp after event
    lastSenAct = 0; // Delete actor timestamp after event
    // if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && mqtt_state)
    if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
    // if (WiFi.status() == WL_CONNECTED && !mqttoff && mqtt_state)
    {
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state) // Sensor in normal mode: check actor in error state
        {
          DEBUG_MSG("EM SenOK: %s isOnBeforeError: %d power level: %d\n", actors[i].name_actor.c_str(), actors[i].isOnBeforeError, actors[i].power_actor);
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true;
          actors[i].Update();
          lastSenAct = 0; // Delete actor timestamp after event
        }
        yield();
      }

      if (!inductionCooker.induction_state)
      {
        DEBUG_MSG("EM SenOK: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError, inductionCooker.powerLevelBeforeError);
        if (!inductionCooker.induction_state)
        {
          inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
          inductionCooker.isInduon = true;
          inductionCooker.induction_state = true;
          inductionCooker.Update();
          DEBUG_MSG("EM SenOK: Induction restore old value: %d\n", inductionCooker.newPower);
          lastSenInd = 0; // Delete induction timestamp after event
        }
      }
    }
    break;
  case EM_CRCER:
    // Sensor CRC ceck failed
  case EM_DEVER:
    // -127°C device error
  case EM_UNPL:
    // sensor unpluged
  case EM_SENER:
    // all other errors
    // if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && mqtt_state)
    if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
    // if (WiFi.status() == WL_CONNECTED && !mqttoff && mqtt_state)
    {
      for (int i = 0; i < numberOfSensors; i++)
      {
        if (!sensors[i].getState())
        {
          switch (sensorsStatus)
          {
          case EM_CRCER:
            // Sensor CRC ceck failed
            DEBUG_MSG("EM CRCER: Sensor %s crc check failed\n", sensors[i].getName().c_str());
            break;
          case EM_DEVER:
            // -127°C device error
            DEBUG_MSG("EM DEVER: Sensor %s device error\n", sensors[i].getName().c_str());
            break;
          case EM_UNPL:
            // sensor unpluged
            DEBUG_MSG("EM UNPL: Sensor %s unplugged\n", sensors[i].getName().c_str());
            break;
          default:
            break;
          }
        }

        if (sensors[i].getSw() && !sensors[i].getState())
        {
          if (lastSenAct == 0)
          {
            lastSenAct = millis(); // Timestamp on error
            DEBUG_MSG("EM SENER: timestamp actors due to sensor error: %l Wait on error actors: %d\n", lastSenAct, wait_on_Sensor_error_actor / 1000);
          }
          if (lastSenInd == 0)
          {
            lastSenInd = millis(); // Timestamp on error
            DEBUG_MSG("EM SENER: timestamp induction due to sensor error: %l Wait on error induction: %d\n", lastSenInd, wait_on_Sensor_error_induction / 1000);
          }
          if (millis() - lastSenAct >= wait_on_Sensor_error_actor) // Wait bevor Event handling
          {
            actERR();
          }
          if (millis() - lastSenInd >= wait_on_Sensor_error_induction) // Wait bevor Event handling
          {
            if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
            {
              inductionCooker.indERR();
            }
          }
        } // Switchable
        yield();
      } // Iterate sensors
    }   // wlan und mqtt state
    break;
  default:
    break;
  }
  handleSensors(false);
}

void tickerActCallback() // Timer Objekt Sensoren
{
  handleActors(false);
}

void tickerIndCallback() // Timer Objekt Sensoren
{
  handleInduction();
  inductionSSE(false);
}

void tickerPUBSUBCallback() // Timer Objekt Sensoren
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
      DEBUG_MSG("%s\n", "Ticker PubSub Error: TickerMQTT started");
      DEBUG_MSG("Ticker PubSub error rc=%d \n", pubsubClient.state());
      mqtt_state = false;
      TickerMQTT.start();
      mqttconnectlasttry = millis();
      miscSSE();
    }
    TickerMQTT.update();
  }
}

void tickerNTPCallback() // Ticker helper function calling Event WLAN Error
{
  timeClient.update();
  Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
}

void tickerMQTTCallback() // Ticker helper function calling Event MQTT Error
{
  if (TickerMQTT.counter() == 1)
  {
    switch (pubsubClient.state())
    {
    case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECTION_TIMEOUT\n", pubsubClient.state());
      break;
    case -3: // MQTT_CONNECTION_LOST - the network connection was broken
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECTION_LOST\n", pubsubClient.state());
      break;
    case -2: // MQTT_CONNECT_FAILED - the network connection failed
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_FAILED\n", pubsubClient.state());
      break;
    case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
      DEBUG_MSG("MQTT status: error rc=%d MQTT_DISCONNECTED\n", pubsubClient.state());
      break;
    case 0: // MQTT_CONNECTED - the client is connected
      pubsubClient.loop();
      break;
    case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_BAD_PROTOCOL\n", pubsubClient.state());
      break;
    case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_BAD_CLIENT_ID\n", pubsubClient.state());
      break;
    case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_UNAVAILABLE\n", pubsubClient.state());
      break;
    case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_BAD_CREDENTIALS\n", pubsubClient.state());
      break;
    case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
      DEBUG_MSG("MQTT status: error rc=%d MQTT_CONNECT_UNAUTHORIZED\n", pubsubClient.state());
      break;
    default:
      break;
    }
  }
  EM_MQTTER();
}

void tickerWLANCallback() // Ticker helper function calling Event WLAN Error
{
  DEBUG_MSG("%s", "tickerWLAN: callback\n");
  if (TickerWLAN.counter() == 1)
  {
    switch (WiFi.status())
    {
    case 0: // WL_IDLE_STATUS
      DEBUG_MSG("WiFi status: error rc: %d WL_IDLE_STATUS");
      break;
    case 1: // WL_NO_SSID_AVAIL
      DEBUG_MSG("WiFi status: error rc: %d WL_NO_SSID_AVAIL");
      break;
    case 2: // WL_SCAN_COMPLETED
      DEBUG_MSG("WiFi status: error rc: %d WL_SCAN_COMPLETED");
      break;
    case 3: // WL_CONNECTED
      DEBUG_MSG("WiFi status: error rc: %d WL_CONNECTED");
      break;
    case 4: // WL_CONNECT_FAILED
      DEBUG_MSG("WiFi status: error rc: %d WL_CONNECT_FAILED");
      break;
    case 5: // WL_CONNECTION_LOST
      DEBUG_MSG("WiFi status: error rc: %d WL_CONNECTION_LOST");
      break;
    case 6: // WL_DISCONNECTED
      DEBUG_MSG("WiFi status: error rc: %d WL_DISCONNECTED");
      break;
    case 255: // WL_NO_SHIELD
      DEBUG_MSG("WiFi status: error rc: %d WL_NO_SHIELD");
      break;
    default:
      break;
    }
  }
  WiFi.reconnect();
}
