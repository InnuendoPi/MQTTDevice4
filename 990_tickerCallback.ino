void pageCallback()
{
  // activePage = nextion.currentPageID;
  if (startBuzzer)
    sendAlarm(ALARM_INFO);

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
  // Serial.printf("Ticker Disp currentPageID: %d lastPageID: %d\n", nextion.currentPageID, nextion.lastPageID);
  
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
      // DEBUG_MSG("Ticker: dispCallback KettlePage activePage: %d\n", activePage);
      if (inductionStatus)
      {
        p1temp_text.attribute("txt", String(int(ids2Input * 10) / 10.0).c_str());
        if (ids2Setpoint > 0)
          p1target_text.attribute("txt", String(int(ids2Setpoint * 10) / 10.0).c_str());
        else
          p1target_text.attribute("txt", String(int(structPlan[actMashStep].temp * 10) / 10.0).c_str());

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
        p1target_text.attribute("txt", String(int(hltSetpoint * 10) / 10.0).c_str());
      }

      p1current_text.attribute("txt", structPlan[actMashStep].name.c_str());
      p1remain_text.attribute("txt", calcRemaining().c_str());
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
  cbpiEventSensors(sensorsStatus);
}

void tickerActCallback() // Timer Objekt Sensoren
{
  cbpiEventActors(actorsStatus);
}

void tickerIndCallback() // Timer Objekt Sensoren
{
  cbpiEventInduction(inductionStatus);
}
void tickerHltCallback() // Timer Objekt Sensoren
{
  kettleHLT.Update();
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
        mqtt_state = false;
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

void tickerPIDCallback() // Ticker helper function calling Event WLAN Error
{
  sensors[0].Update();
  float val = sensors[0].getTotalValueFloat();
  if (val != -127.00)
    ids2Input = val;

  if (!isnan(ids2Input))
  {
    ids2Output = ids2PID.Run(ids2Input);
    inductionCooker.inductionNewPower(int(ids2Output));
    // handleInduction();
    TickerInd.updatenow();
    // DEBUG_MSG("Ticker PID ids2Input: %.02f ids2Output: %.02f intOutput %d ids2Setpoint: %.02f\n", ids2Input, ids2Output, int(ids2Output), ids2Setpoint);
    // Serial.printf("Ticker PID ids2Input: %.02f ids2Output: %.02f intOutput %d ids2Setpoint: %.02f\n", ids2Input, ids2Output, int(ids2Output), ids2Setpoint);
  }

  if (ids2AutoTune)
    return;

  if (TickerMash.state() != RUNNING && TickerMash.state() != PAUSED)
  {
    checkTemp();
  }
  // printPID();
}

void tickerHltPIDCallback() // Ticker helper function calling Event WLAN Error
{
  sensors[kettleHLT.senid].Update();
  float val = sensors[kettleHLT.senid].getTotalValueFloat();
  if (val != -127.00)
    hltInput = val;

  if (!isnan(hltInput))
  {
    hltOutput = hltPID.Run(hltInput);
    kettleHLT.newPower(int(hltOutput));
    TickerHlt.updatenow();
    // DEBUG_MSG("Ticker hltPID hltInput: %.02f hltOutput: %.02f intOutput %d hltSetpoint: %.02f\n", hltInput, hltOutput, int(hltOutput), hltSetpoint);
    // Serial.printf("Ticker HltPID hltInput: %.02f hltOutput: %.02f intOutput %d hltSetpoint: %.02f\n", hltInput, hltOutput, int(hltOutput), hltSetpoint);
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
  cbpiEventSystem(EM_MQTTER);
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
  cbpiEventSystem(EM_WLANER);
}
