void readCustomCommand()
{
  int tempID = nextion.cmdGroup;
  int tempPage = nextion.cmdLength;

  if (tempID == 102) // 0x66 -> page command empfangen
  {
    activePage = nextion.currentPageId;
    if (PIN_BUZZER != -100)
      sendAlarm(ALARM_INFO);
    tickerDispCallback();
  }
  else // 0x65 -> component id empfangen
  {
    // Trigger Buttons manueller Modus
    if (activePage == 2) // Induction page
    {
      uint8_t manStatus = nextion.readNum(powerButton);
      uint8_t manPower = nextion.readNum(p2slider);
      if (manStatus && manPower > 0)
        inductionCooker.setNewPower(manPower);
      else
        inductionCooker.setNewPower(0);
    }
  }
}

void initDisplay()
{
  activePage = startPage;
  switch (startPage)
  {
  case 0:
    nextion.writeStr("page 0");
    nextion.currentPageId = 0;
    nextion.lastCurrentPageId = 1;
    break;
  case 1:
    nextion.writeStr("page 1");
    nextion.currentPageId = 1; // setze currentPageId wenn Startseite nicht 0
    nextion.lastCurrentPageId = 0;
    break;
  case 2:
    nextion.writeStr("page 2");
    nextion.currentPageId = 2;
    nextion.lastCurrentPageId = 0;
    break;
  default:
    startPage = 0;
    nextion.writeStr("page 0");
    nextion.currentPageId = 0;
    nextion.lastCurrentPageId = 1;
    break;
  }

  if (nextion.getDebug())
    DEBUG_INFO("CFG", "activePage: %d startPage: %d currentPage: %d lastcurrent: %d", activePage, startPage, nextion.currentPageId, nextion.lastCurrentPageId);
  tickerDispCallback();
}

void dispPublishmqtt()
{
  if (pubsubClient.connected())
  {
    JsonDocument doc;
    char jsonMessage[32];
    serializeJson(doc, jsonMessage);
    if (useFerm)
      pubsubClient.publish("cbpi/updatefermenter", jsonMessage);
    pubsubClient.publish("cbpi/updatekettle", jsonMessage);
    pubsubClient.publish("cbpi/updateactor", jsonMessage);
    pubsubClient.publish("cbpi/updatesensor", jsonMessage);
  }
}

void KettlePage() // Seite 1
{
  nextion.writeStr(currentStepName_text, currentStepName);
  nextion.writeStr(currentStepRemain_text, currentStepRemain);
  nextion.writeStr(nextStepRemain_text, nextStepRemain);
  nextion.writeStr(nextStepName_text, nextStepName);

  if (strlen(structKettles[0].id) > 0)
  {
    nextion.writeStr(kettleName1_text, structKettles[0].name);
    if (sensors[0].getId() != "")
      nextion.writeStr(kettleSoll1_text, structKettles[0].target_temp);
    else
      nextion.writeStr(kettleSoll1_text, "na");
    nextion.writeStr(kettleIst1_text, structKettles[0].current_temp);
  }
  else
    dispPublishmqtt();

  if (strlen(structKettles[1].id) > 0)
  {
    nextion.writeStr(kettleName2_text, structKettles[1].name);
    if (sensors[1].getId() != "")
      nextion.writeStr(kettleSoll2_text, structKettles[1].target_temp);
    else
      nextion.writeStr(kettleSoll2_text, "na");
    nextion.writeStr(kettleIst2_text, structKettles[1].current_temp);
  }
  if (strlen(structKettles[2].id) > 0)
  {
    nextion.writeStr(kettleName3_text, structKettles[2].name);
    if (sensors[2].getId() != "")
      nextion.writeStr(kettleSoll3_text, structKettles[2].target_temp);
    else
      nextion.writeStr(kettleSoll3_text, "na");
    nextion.writeStr(kettleIst3_text, structKettles[2].current_temp);
  }
  if (strlen(structKettles[3].id) > 0)
  {
    nextion.writeStr(kettleName4_text, structKettles[3].name);
    if (sensors[3].getId() != "")
      nextion.writeStr(kettleSoll4_text, structKettles[3].target_temp);
    else
      nextion.writeStr(kettleSoll4_text, "na");
    nextion.writeStr(kettleIst4_text, structKettles[3].current_temp);
  }
  nextion.writeNum(progress, sliderval);
  nextion.writeStr(notification, notify);
}

void BrewPage() // Seite 2
{
  if (strlen(structKettles[0].sensor) != 0)
  {
    nextion.writeStr(p1temp_text, structKettles[0].current_temp);
    nextion.writeStr(p1target_text, structKettles[0].target_temp);
  }
  else
  {
    nextion.writeStr(p1temp_text, structKettles[0].current_temp);
    nextion.writeStr(p1target_text, "na");
  }

  nextion.writeStr(p1current_text, currentStepName);
  nextion.writeStr(p1remain_text, currentStepRemain);
  nextion.writeNum(p1progress, sliderval);
  nextion.writeStr(p1notification, notify);
}

void InductionPage()
{
  // 316 = 0°C - 360 = 44°C - 223 = 100°C -- 53,4 je 20°C

  int32_t aktSlider = nextion.readNum(p2slider);
  if (aktSlider >= 0 && aktSlider <= 100)
    inductionCooker.inductionNewPower(aktSlider);

  nextion.writeStr(p2temp_text, String(structKettles[0].current_temp).c_str());
  if (sensors[0].calcOffset() < 16.0)
    nextion.writeNum(p2gauge, (int)(sensors[0].calcOffset() * 2.7 + 316));
  else
    nextion.writeNum(p2gauge, (int)(sensors[0].calcOffset() * 2.7 - 44));
}

void cbpi4kettle_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Subscribing to %s", cbpi4kettle_topic);
    pubsubClient.subscribe(cbpi4kettle_topic);
    pubsubClient.loop();
  }
}

void cbpi4kettle_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Unsubscribing from %s", cbpi4kettle_topic);
    pubsubClient.unsubscribe(cbpi4steps_topic);
  }
}

void cbpi4steps_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Subscribing to %s", cbpi4steps_topic);
    pubsubClient.subscribe(cbpi4steps_topic);
    pubsubClient.loop();
  }
}

void cbpi4steps_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Unsubscribing from %s", cbpi4steps_topic);
    pubsubClient.unsubscribe(cbpi4steps_topic);
  }
}

void cbpi4notification_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Subscribing to %s", cbpi4notification_topic);
    pubsubClient.subscribe(cbpi4notification_topic);
    pubsubClient.loop();
  }
}
void cbpi4notification_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Unsubscribing from %s", cbpi4notification_topic);
    pubsubClient.unsubscribe(cbpi4notification_topic);
  }
}

void cbpi4fermenter_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Subscribing to %s", cbpi4fermenter_topic);
    pubsubClient.subscribe(cbpi4fermenter_topic);
    pubsubClient.loop();
  }
}
void cbpi4fermenter_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Unsubscribing from %s", cbpi4fermenter_topic);
    pubsubClient.unsubscribe(cbpi4fermenter_topic);
  }
}

void cbpi4fermentersteps_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Subscribing to %s", cbpi4fermentersteps_topic);
    pubsubClient.subscribe(cbpi4fermentersteps_topic);
    pubsubClient.loop();
  }
}

void cbpi4fermentersteps_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_VERBOSE("DIS", "Unsubscribing from %s", cbpi4fermentersteps_topic);
    pubsubClient.unsubscribe(cbpi4fermentersteps_topic);
  }
}

void cbpi4kettle_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt notification deserialize Json error %s MemoryUsage %d", error.c_str());
    return;
  }
  for (uint8_t i = 0; i < maxKettles; i++)
  {
    if (strlen(structKettles[i].id) == 0) // structKettle unbelegt
    {
      strlcpy(structKettles[i].id, doc["id"], maxIdSign);
      strlcpy(structKettles[i].name, doc["name"], maxKettleSign);
      dtostrf(doc["target_temp"], -1, 1, structKettles[i].target_temp);
      strlcpy(structKettles[i].sensor, doc["sensor"], maxSensorSign);
      char sensorupdate[45];
      sprintf(sensorupdate, "%s%s", cbpi4sensor_topic, structKettles[i].sensor);
      pubsubClient.subscribe(sensorupdate);
      switch (i)
      {
      case 0:
        nextion.writeStr(kettleName1_text, structKettles[i].name);
        nextion.writeStr(kettleSoll1_text, structKettles[i].target_temp);
        break;
      case 1:
        nextion.writeStr(kettleName2_text, structKettles[i].name);
        nextion.writeStr(kettleSoll2_text, structKettles[i].target_temp);
        break;
      case 2:
        nextion.writeStr(kettleName3_text, structKettles[i].name);
        nextion.writeStr(kettleSoll3_text, structKettles[i].target_temp);
        break;
      case 3:
        nextion.writeStr(kettleName4_text, structKettles[i].name);
        nextion.writeStr(kettleSoll4_text, structKettles[i].target_temp);
        break;
      }
      break;
    }
    else // structKettle belegt
    {
      if (strcmp(structKettles[i].id, doc["id"]) == 0)
      {
        dtostrf(doc["target_temp"], -1, 1, structKettles[i].target_temp);
        switch (i)
        {
        case 0:
          nextion.writeStr(kettleSoll1_text, structKettles[i].target_temp);
          break;
        case 1:
          nextion.writeStr(kettleSoll2_text, structKettles[i].target_temp);
          break;
        case 2:
          nextion.writeStr(kettleSoll3_text, structKettles[i].target_temp);
          break;
        case 3:
          nextion.writeStr(kettleSoll4_text, structKettles[i].target_temp);
          break;
        }
        break;
      }
    }
  }
}

void cbpi4sensor_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt notification deserialize Json error %s", error.c_str());
    return;
  }
  for (uint8_t i = 0; i < maxKettles; i++)
  {
    if (strcmp(structKettles[i].sensor, doc["id"]) == 0)
    {
      dtostrf(doc["value"], -1, 1, structKettles[i].current_temp);
      break;
    }
  }
}

void cbpi4steps_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt notification deserialize Json error %s", error.c_str());
    return;
  }
  if (doc["status"] == "D") // ignore solved steps
    return;

  JsonObject props = doc["props"];
  bool newStep = true;

  for (uint8_t i = 0; i < stepsCounter; i++)
  {
    if (strcmp(structSteps[i].id, doc["id"]) == 0)
    {
      int minutes = props["Timer"].as<int>() | 0;
      sprintf(structSteps[i].timer, "%02d:%02d", minutes, 0);
      newStep = false;
      break;
    }
  }

  if (newStep == true)
  {
    strlcpy(structSteps[stepsCounter].id, doc["id"], maxIdSign);
    strlcpy(structSteps[stepsCounter].name, doc["name"], maxStepSign);
    int minutes = props["Timer"].as<int>() | 0;
    sprintf(structSteps[stepsCounter].timer, "%02d:%02d", minutes, 0);
    stepsCounter++;
  }

  if (doc["status"] == "A")
  {
    if (!activeBrew) // aktiver Step vorhanden?
      activeBrew = true;

    current_step = true;
    int valTimer = 0;
    int min = 0;
    int sec = 0;

    if (currentStepName != doc["name"]) // New active step
    {
      strlcpy(currentStepName, doc["name"] | "", maxStepSign);
      current_step = true;
      for (uint8_t i = 0; i < stepsCounter; i++)
      {
        if (strcmp(structSteps[i].name, doc["name"]) == 0)
        {
          if (stepsCounter >= i + 1)
          {
            strlcpy(nextStepName, structSteps[i + 1].name, maxStepSign);
            strlcpy(nextStepRemain, structSteps[i + 1].timer, maxRemainSign);
          }
          if (activePage == 0)
          {
            nextion.writeStr(nextStepName_text, nextStepName);
            nextion.writeStr(nextStepRemain_text, nextStepRemain);
          }
          break;
        }
      }

      if (doc["type"] == "MashStep")
      {
        strlcpy(notify, "Waiting for Target Temp", maxNotifySign);
      }
      else if (doc["type"] == "ToggleStep")
      {
        strlcpy(notify, "", maxNotifySign);
      }
      else if (doc["type"] == "NotificationStep")
      {
        strlcpy(notify, props["Notification"] | "", maxNotifySign);
      }
      else if (doc["type"] == "WaitStep")
      {
        strlcpy(notify, "WaitStep", maxNotifySign);
      }
      else if (doc["type"] == "TargetStep")
      {
        strlcpy(notify, "", maxNotifySign);
      }
      else if (doc["type"] == "BoilStep")
      {
        strlcpy(notify, "BoilStep", maxNotifySign);
      }
      else if (doc["type"] == "MashInStep")
      {
        strlcpy(notify, "Waiting for Target Temp", maxNotifySign);
      }
      else if (doc["type"] == "CoolDownStep")
      {
        strlcpy(notify, "CoolDownStep", maxNotifySign);
      }
      else if (doc["type"] == "ActorStep")
      {
        strlcpy(notify, "ActorStep", maxNotifySign);
      }
      else if (doc["type"] == "ClearLogStep")
      {
        strlcpy(notify, "", maxNotifySign);
      }
      else
      {
        strlcpy(notify, "", maxNotifySign);
      }
      if (activePage == 0)
        nextion.writeStr(currentStepName_text, currentStepName);
      if (activePage == 1)
        nextion.writeStr(p1current_text, currentStepName);
    }
    // 
    // if (doc["state_text"] != 0 && doc["state_text"] != "Waiting for Target Temp")
    if (doc["state_text"].as<int>() != 0 && doc["state_text"] != "Waiting for Target Temp")
    {
      strlcpy(currentStepRemain, doc["state_text"] | "", maxRemainSign);
      if (activePage == 0)
        nextion.writeStr(currentStepRemain_text, currentStepRemain);
      if (activePage == 1)
        nextion.writeStr(p1remain_text, currentStepRemain);

      if (doc["type"] == "MashStep" && doc["state_text"] != "")
      {
        char delimiter[] = ":";
        char buf[10];
        strcpy(buf, doc["state_text"].as<const char *>());
        char *hours = strtok(buf, delimiter);
        char *minutes = strtok(NULL, delimiter);
        char *seconds = strtok(NULL, delimiter);
        valTimer = props["Timer"].as<int>() | 0;
        min = atoi(minutes);
        sec = atoi(seconds);
      }
      if (valTimer > 0 && min > 0)
      {
        sliderval = (valTimer * 60 - (min * 60 + sec)) * 100 / (valTimer * 60);
        if (activePage == 0)
          nextion.writeNum(progress, sliderval);
        if (activePage == 1)
          nextion.writeNum(p1progress, sliderval);
      }
      else
      {
        sliderval = 0;
        if (activePage == 0)
          nextion.writeNum(progress, 0);
        if (activePage == 1)
          nextion.writeNum(p1progress, 0);
      }
    }
    else
    {
      if (props["Timer"].is<int>())
      {
        int minutes = props["Timer"].as<int>();
        sprintf(currentStepRemain, "%02d:%02d", minutes, 0);
        if (activePage == 0)
          nextion.writeStr(currentStepRemain_text, currentStepRemain);
        if (activePage == 1)
          nextion.writeStr(p1remain_text, currentStepRemain);
      }
      else
      {
        strcpy(currentStepRemain, "0:00");
        if (activePage == 0)
          nextion.writeStr(currentStepRemain_text, currentStepRemain);
        if (activePage == 1)
          nextion.writeStr(p1remain_text, currentStepRemain);
      }
      sliderval = 0;
      if (activePage == 0)
        nextion.writeNum(progress, sliderval);
      if (activePage == 1)
        nextion.writeNum(p1progress, sliderval);
    }
    return;
  }
}

void cbpi4notification_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt notification deserialize Json error %s", error.c_str());
    return;
  }

  if (mqttBuzzer) // MQTTBuzzer
  {
    if (doc["type"] == "success")
    {
      sendAlarm(ALARM_SUCCESS);
    }
    if (doc["type"] == "info")
    {
      sendAlarm(ALARM_INFO);
    }
    if (doc["type"] == "warning")
    {
      sendAlarm(ALARM_WARNING);
    }
    if (doc["type"] == "error")
    {
      sendAlarm(ALARM_ERROR);
    }
  }
  if (!useDisplay)
    return; // mqqtBuzzer only

  if (doc["title"] == "Stop")
  {
    activeBrew = false;
    if (activePage == 0)
    {
      strlcpy(currentStepName, "BrewPage", maxStepSign);
      strlcpy(currentStepRemain, "", maxRemainSign);
      strlcpy(nextStepName, "", maxStepSign);
      strlcpy(nextStepRemain, "", maxRemainSign);
    }
    if (activePage == 1)
    {
      strlcpy(currentStepName, sensors[0].getSensorName().c_str(), maxStepSign); // xxx
    }
    strlcpy(notify, "", maxNotifySign);
    return;
  }
  if (doc["title"] == "Brewing completed")
  {
    activeBrew = false;
    strlcpy(notify, "Brewing completed", maxNotifySign);
    return;
  }
  if (doc["title"] == "Start" || doc["title"] == "Resume")
    activeBrew = true;

  strlcpy(notify, doc["message"] | "", maxNotifySign);
}

void cbpi4fermenter_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  JsonDocument filter;
  filter["id"] = true;
  filter["state"] = true;
  filter["name"] = true;
  filter["sensor"] = true;
  filter["target_temp"] = true;
  // filter["steps"]["name"] = true;
  DeserializationError error = deserializeJson(doc, (const char *)payload, DeserializationOption::Filter(filter));
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt fermenter deserialize Json error %s MemoryUsage %d", error.c_str());
    return;
  }

  fermenterStatus = doc["state"] | false;

  for (uint8_t i = 0; i < maxKettles; i++)
  {
    if (strlen(structKettles[i].id) == 0) // structKettle unbelegt
    {
      strlcpy(structKettles[i].id, doc["id"].as<const char *>(), maxIdSign);
      strlcpy(structKettles[i].name, doc["name"].as<const char *>(), maxKettleSign);
      dtostrf(doc["target_temp"], -1, 1, structKettles[i].target_temp);
      strlcpy(structKettles[i].sensor, doc["sensor"].as<const char *>(), maxSensorSign);
      char sensorupdate[45];
      sprintf(sensorupdate, "%s%s", cbpi4sensor_topic, structKettles[i].sensor);
      pubsubClient.subscribe(sensorupdate);
      switch (i)
      {
      case 0:
        nextion.writeStr(kettleName1_text, structKettles[i].name);
        nextion.writeStr(kettleSoll1_text, structKettles[i].target_temp);
        break;
      case 1:
        nextion.writeStr(kettleName2_text, structKettles[i].name);
        nextion.writeStr(kettleSoll2_text, structKettles[i].target_temp);
        break;
      case 2:
        nextion.writeStr(kettleName3_text, structKettles[i].name);
        nextion.writeStr(kettleSoll3_text, structKettles[i].target_temp);
        break;
      case 3:
        nextion.writeStr(kettleName4_text, structKettles[i].name);
        nextion.writeStr(kettleSoll4_text, structKettles[i].target_temp);
        break;
      }
      break;
    }
    else // structKettle belegt
    {
      if (strcmp(structKettles[i].id, doc["id"]) == 0)
      {
        dtostrf(doc["target_temp"], -1, 1, structKettles[i].target_temp);
        switch (i)
        {
        case 0:
          nextion.writeStr(kettleSoll1_text, structKettles[i].target_temp);
          break;
        case 1:
          nextion.writeStr(kettleSoll2_text, structKettles[i].target_temp);
          break;
        case 2:
          nextion.writeStr(kettleSoll3_text, structKettles[i].target_temp);
          break;
        case 3:
          nextion.writeStr(kettleSoll4_text, structKettles[i].target_temp);
          break;
        }
        break;
      }
    }
  }
}

void cbpi4fermentersteps_handlemqtt(unsigned char *payload)
{
  JsonDocument doc;
  JsonDocument filter;
  filter["name"] = true;
  filter["props"]["Sensor"] = true;
  DeserializationError error = deserializeJson(doc, (const char *)payload, DeserializationOption::Filter(filter));
  if (error)
  {
    DEBUG_ERROR("DIS", "handlemqtt notification deserialize Json error %s MemoryUsage %d", error.c_str());
    return;
  }

  if (fermenterStatus)
  {
    strlcpy(currentStepName, doc["name"] | "", maxStepSign);
    sprintf(currentStepRemain, "%s", "");
  }
  else
  {
    sprintf(currentStepName, "%s", "");
    sprintf(currentStepRemain, "%s", "");
  }
}
