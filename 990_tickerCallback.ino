void pageCallback()
{
  if (startBuzzer)
    sendAlarm(ALARM_INFO);

  // currentPageID bei Event Touch nicht aktuell
  // activePage = nextion.currentPageID;

  TickerDisp.updatenow();
}

void powerButtonCallback()
{
  inductionCooker.induction_state = !inductionCooker.induction_state;
}

void tickerDispCallback()
{
  nextion.update();

  char ipMQTT[50];
  sprintf_P(uhrzeit, (PGM_P)F("%02d:%02d"), timeClient.getHours(), timeClient.getMinutes());
  if (startMDNS)
    sprintf_P(ipMQTT, (PGM_P)F("http://%s - %s"), nameMDNS, WiFi.localIP().toString().c_str());
  else
    sprintf_P(ipMQTT, (PGM_P)F("http://%s"), WiFi.localIP().toString().c_str());

  activePage = nextion.currentPageID;
  switch (activePage)
  {
  case 0: // BrewPage
    if (mqttoff)
    {
      // DEBUG_MSG("Ticker: dispCallback BrewPage activePage: %d\n", activePage);
      currentStepName_text.attribute("txt", structPlan[actMashStep].name.c_str());
      currentStepRemain_text.attribute("txt", calcRemaining().c_str());
      if (actMashStep < maxActMashSteps)
      {
        nextStepRemain_text.attribute("txt", String(structPlan[actMashStep + 1].duration).c_str());
        nextStepName_text.attribute("txt", structPlan[actMashStep + 1].name.c_str());
      }
      else
      {
        nextStepRemain_text.attribute("txt", "");
        nextStepName_text.attribute("txt", "");
      }
      if (inductionStatus)
      {
        kettleName1_text.attribute("txt", "GGM IDS2");
        if (ids2Setpoint > 0)
          kettleSoll1_text.attribute("txt", String(int(ids2Setpoint * 10) / 10.0).c_str());
        else
          kettleSoll1_text.attribute("txt", String(int(structPlan[actMashStep].temp * 10) / 10.0).c_str());

        // kettleIst1_text.attribute("txt", String(int(ids2Input * 10) / 10.0).c_str());
        kettleIst1_text.attribute("txt", sensors[0].getTotalValueString());
        if (hltStatus)
        {
          kettleName2_text.attribute("txt", "HLT sparge");
          kettleSoll2_text.attribute("txt", String(int(hltSetpoint * 10) / 10.0).c_str());
          kettleIst2_text.attribute("txt", sensors[kettleHLT.senid].getTotalValueString()); // String(int(hltInput * 10) / 10.0).c_str());
        }
      }
      else if (hltStatus)
      {
        kettleName1_text.attribute("txt", "HLT sparge");
        kettleSoll1_text.attribute("txt", String(int(hltSetpoint * 10) / 10.0).c_str());
        kettleIst1_text.attribute("txt", sensors[kettleHLT.senid].getTotalValueString()); // String(int(hltInput * 10) / 10.0).c_str());
      }

      uhrzeit_text.attribute("txt", uhrzeit);
      mqttDevice.attribute("txt", ipMQTT);
      int sliderval = 100;
      if (TickerMash.state() == RUNNING)
        sliderval = TickerMash.remaining() / 1000 * 100 / structPlan[actMashStep].duration;

      progress.value(sliderval);
    }
    else
    {
      if (!activeBrew) // aktiver Step vorhanden?
        strlcpy(currentStepName, "BrewPage", maxStepSign);

      uhrzeit_text.attribute("txt", uhrzeit);
      mqttDevice.attribute("txt", ipMQTT);

      BrewPage();
    }
    break;
  case 1: // KettlePage
    if (mqttoff)
    {
      char buf[5];
      sprintf(buf, "%s", "0.0");

      // DEBUG_MSG("Ticker: dispCallback KettlePage activePage: %d\n", activePage);
      if (inductionStatus)
      {
        // p1temp_text.attribute("txt", String(int(ids2Input * 10) / 10.0).c_str());
        p1temp_text.attribute("txt", sensors[0].getTotalValueString());
        if (ids2AutoTune)
        {
          dtostrf(ids2Setpoint, 2, 1, buf);
          p1target_text.attribute("txt", buf);
          // p1target_text.attribute("txt", String(int(ids2Setpoint)).c_str());
        }
        else
        {
          dtostrf(structPlan[actMashStep].temp, 2, 1, buf);
          p1target_text.attribute("txt", buf);
          // p1target_text.attribute("txt", String(int(structPlan[actMashStep].temp * 100) / 100.0).c_str());
        }

        // p1current_text.attribute("txt", structPlan[actMashStep].name.c_str());
        // p1remain_text.attribute("txt", calcRemaining().c_str());
        // p1mqttDevice.attribute("txt", ipMQTT);
        // p1uhrzeit_text.attribute("txt", uhrzeit);

        // unsigned long allSeconds = TickerMash.remaining() / 1000;
        // int secsRemaining = allSeconds % 3600;

        // int sliderval = 100;
        // if (TickerMash.state() == RUNNING)
        //   p1slider.value(secsRemaining * 100 / structPlan[actMashStep].duration / 60);
        // else
        //   p1slider.value(100);
      }
      else if (hltStatus)
      {
        p1temp_text.attribute("txt", sensors[kettleHLT.senid].getTotalValueString());
        dtostrf(hltSetpoint, 2, 1, buf);
        p1target_text.attribute("txt", buf);
        // p1target_text.attribute("txt", String(int(hltSetpoint)).c_str());
      }

      if (ids2AutoTune)
      {
        p1current_text.attribute("txt", "AutoTune IDS2");
        p1remain_text.attribute("txt", "");
      }
      else if (hltAutoTune)
      {
        p1current_text.attribute("txt", "AutoTune HLT");
        p1remain_text.attribute("txt", "");
      }
      else
      {
        p1current_text.attribute("txt", structPlan[actMashStep].name.c_str());
        p1remain_text.attribute("txt", calcRemaining().c_str());
      }
      p1mqttDevice.attribute("txt", ipMQTT);
      p1uhrzeit_text.attribute("txt", uhrzeit);

      unsigned long allSeconds = TickerMash.remaining() / 1000;
      int secsRemaining = allSeconds % 3600;

      int sliderval = 100;
      if (TickerMash.state() == RUNNING)
        sliderval = secsRemaining * 100 / structPlan[actMashStep].duration / 60;

      p1progress.value(sliderval);
      break;
    }
    else
    {
      if (!activeBrew) // aktiver Step vorhanden?
        strlcpy(currentStepName, sensors[0].getName().c_str(), maxStepSign);

      strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);

      p1mqttDevice.attribute("txt", ipMQTT);
      p1uhrzeit_text.attribute("txt", uhrzeit);

      KettlePage();
    }
    break;
  case 2: // Induction mode
    // DEBUG_MSG("Ticker: dispCallback InductionPage activePage: %d\n", activePage);
    strlcpy(structKettles[0].current_temp, sensors[0].getTotalValueString(), maxTempSign);
    p2uhrzeit_text.attribute("txt", uhrzeit);
    InductionPage();
    break;
  }
  nextion.update();
}

void tickerSenCallback() // Timer Objekt Sensoren
{
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
  handleSensors();
}

void tickerActCallback() // Timer Objekt Sensoren
{
  handleActors();
}

void tickerIndCallback() // Timer Objekt Sensoren
{
  handleInduction();
}
void tickerHltCallback() // Timer Objekt Sensoren
{
  handleHLT();
}

void tickerPUBSUBCallback() // Timer Objekt Sensoren
{
  if (TickerMQTT.state() != RUNNING)
  {
    if (pubsubClient.connected())
    {
      mqtt_state = true;
      pubsubClient.loop();
      if (TickerMQTT.state() == RUNNING)
        TickerMQTT.stop();
      return;
    }
    if (!pubsubClient.connected()) // if (!pubsubClient.connected())
    {
      if (TickerMQTT.state() != RUNNING)
      {
        // DEBUG_MSG("%s\n", "Ticker PubSub Error: TickerMQTT started");
        // DEBUG_MSG("Ticker PubSub error rc=%d \n", pubsubClient.state());
        // mqtt_state = false;
        TickerMQTT.resume();
        mqttconnectlasttry = millis();
      }
      TickerMQTT.update();
    }
  }
  else
    TickerMQTT.update();
}

void tickerNTPCallback() // Ticker helper function calling Event WLAN Error
{
  timeClient.update();
  Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
}

void tickerMashCallback() // Ticker helper function calling Event WLAN Error
{
  handleMash();
}
void tickerPIDCallback()
{
  if (ids2AutoTune)
  {
    runAutoTune();
    return;
  }
  if (hltAutoTune)
  {
    runHltAutoTune();
    return;
  }
  if (TickerHlt.state() == RUNNING)
  {
    hltInput = sensors[kettleHLT.senid].getTotalValueDouble();
    hltPID.Compute();
    kettleHLT.newPower(int(hltOutput));
  }
  if (pidMode)
  {
    ids2Input = sensors[0].getTotalValueDouble();
    ids2PID.Compute();
    inductionCooker.inductionNewPower(int(ids2Output));
    // Serial.printf("Ticker: ids2Input %.03f\tdiff: %.03f\tids2Output: %.03f\n", ids2Input, (ids2Input-ids2Setpoint), ids2Output);
    if (TickerMash.state() != RUNNING && TickerMash.state() != PAUSED)
    {
      checkTemp(); // check piddelta
    }
  }
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
