void pageCallback0()
{
  if (PIN_BUZZER != -100)
    sendAlarm(ALARM_INFO);

  tempPage = 0;
  TickerDisp.updatenow();
}
void pageCallback1()
{
  if (PIN_BUZZER != -100)
    sendAlarm(ALARM_INFO);

  tempPage = 1;
  TickerDisp.updatenow();
}
void pageCallback2()
{
  if (PIN_BUZZER != -100)
    sendAlarm(ALARM_INFO);

  tempPage = 2;
  TickerDisp.updatenow();
}

void powerButtonCallback()
{
  inductionCooker.setInductionState(!inductionCooker.getInductionState());
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
    sprintf_P(ipMQTT, (PGM_P)F("http://%s.local"), nameMDNS);
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
      strlcpy(currentStepName, sensors[0].getSensorName().c_str(), maxStepSign);

    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);

    p1mqttDevice.attribute("txt", ipMQTT);
    p1uhrzeit_text.attribute("txt", uhrzeit);

    KettlePage();
    break;
  case 2: // Induction mode
    // log_e("Ticker: dispCallback InductionPage activePage: %d", activePage);
    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);
    p2uhrzeit_text.attribute("txt", uhrzeit);
    InductionPage();
    break;
  }
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
#ifdef ESP32
      log_e("%s", "Ticker PubSub Error: TickerMQTT started");
#endif
      TickerMQTT.start();
      mqttconnectlasttry = millis();
      mqtt_state = false; // MQTT in error state
      miscSSE();
    }
    TickerMQTT.update();
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
    // if (WiFi.status() == WL_CONNECTED && mqtt_state)
    if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
    {
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].getActorSwitch() && !actors[i].getActorState()) // Sensor in normal mode: check actor in error state
        {
#ifdef ESP32
          log_e("EM SenOK: %s isOnBeforeError: %d power level: %d", actors[i].getActorName().c_str(), actors[i].getIsOnBeforeError(), actors[i].getActorPower());
#endif
          actors[i].setIsOn(actors[i].getIsOnBeforeError());
          actors[i].setActorState(true);
          actors[i].Update();
          lastSenAct = 0; // Delete actor timestamp after event
        }
        yield();
      }

      if (!inductionCooker.getInductionState())
      {
#ifdef ESP32
        log_e("EM SenOK: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d", inductionCooker.getPower(), inductionCooker.getPowerLevelOnError(), inductionCooker.getPowerLevelBeforeError());
#endif
        if (!inductionCooker.getInductionState())
        {
          inductionCooker.setNewPower(inductionCooker.getPowerLevelBeforeError());
          inductionCooker.setisInduon(true);
          inductionCooker.setInductionState(true);
          inductionCooker.Update();
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
    // if (WiFi.status() == WL_CONNECTED && mqtt_state)
    if (WiFi.status() == WL_CONNECTED && TickerPUBSUB.state() == RUNNING && mqtt_state)
    {
      for (int i = 0; i < numberOfSensors; i++)
      {
        if (!sensors[i].getSensorState())
        {
          switch (sensorsStatus)
          {
          case EM_CRCER:
// Sensor CRC ceck failed
#ifdef ESP32
            log_e("EM CRCER: Sensor %s crc check failed", sensors[i].getSensorName().c_str());
#endif
            break;
          case EM_DEVER:
// -127°C device error
#ifdef ESP32
            log_e("EM DEVER: Sensor %s device error", sensors[i].getSensorName().c_str());
#endif
            break;
          case EM_UNPL:
// sensor unpluged
#ifdef ESP32
            log_e("EM UNPL: Sensor %s unplugged", sensors[i].getSensorName().c_str());
#endif
            break;
          default:
            break;
          }
        }

        if (sensors[i].getSensorSwitch() && !sensors[i].getSensorState())
        {
          if (lastSenAct == 0)
          {
            lastSenAct = millis(); // Timestamp on error
#ifdef ESP32
            log_e("EM SENER: timestamp actors due to sensor error: %l Wait on error actors: %d", lastSenAct, wait_on_Sensor_error_actor / 1000);
#endif
          }
          if (lastSenInd == 0)
          {
            lastSenInd = millis(); // Timestamp on error
#ifdef ESP32
            log_e("EM SENER: timestamp induction due to sensor error: %l Wait on error induction: %d", lastSenInd, wait_on_Sensor_error_induction / 1000);
#endif
          }
          if (millis() - lastSenAct >= wait_on_Sensor_error_actor) // Wait bevor Event handling
          {
            actERR();
          }
          if (millis() - lastSenInd >= wait_on_Sensor_error_induction) // Wait bevor Event handling
          {
            if (inductionCooker.getisInduon() && inductionCooker.getPowerLevelOnError() < 100 && inductionCooker.getInductionState())
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

void tickerMQTTCallback() // Ticker helper function calling Event MQTT Error
{
  if (TickerMQTT.counter() == 1)
  {
    switch (pubsubClient.state())
    {
    case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECTION_TIMEOUT", pubsubClient.state());
#endif
      break;
    case -3: // MQTT_CONNECTION_LOST - the network connection was broken
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECTION_LOST", pubsubClient.state());
#endif
      break;
    case -2: // MQTT_CONNECT_FAILED - the network connection failed
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_FAILED", pubsubClient.state());
#endif
      break;
    case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_DISCONNECTED", pubsubClient.state());
#endif
      break;
    case 0: // MQTT_CONNECTED - the client is connected
      pubsubClient.loop();
      break;
    case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_BAD_PROTOCOL", pubsubClient.state());
#endif
      break;
    case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_BAD_CLIENT_ID", pubsubClient.state());
#endif
      break;
    case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_UNAVAILABLE", pubsubClient.state());
#endif
      break;
    case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_BAD_CREDENTIALS", pubsubClient.state());
#endif
      break;
    case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
#ifdef ESP32
      log_e("MQTT status: error rc=%d MQTT_CONNECT_UNAUTHORIZED", pubsubClient.state());
#endif
      break;
    default:
      break;
    }
  }
  EM_MQTTERROR();
}
