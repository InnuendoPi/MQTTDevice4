void initDisplay()
{
  // BrewPage:    max 4 Kettels mit Temepratur und Ziel
  // KettlePage:  Kessel an Sensor ID 0 (IDS2 MaischeSud)

  // register callback functions
  p0ForButton.touch(pageCallback1);         // BrewPage forward to KettlePage
  p0BackButton.touch(pageCallback2);        // BrewPage backward to InductionPage
  p1ForButton.touch(pageCallback2);         // KelltlePage forward to InductionPage
  p1BackButton.touch(pageCallback0);        // KettlePage backward to BrewPage
  p2ForButton.touch(pageCallback0);         // InductionPage forward to BrewPage
  p2BackButton.touch(pageCallback1);        // InductionPage backward to KettlePage
  powerButton.release(powerButtonCallback); // buttonBack auf induction page backward auf page 1

  activePage = startPage;
  tempPage = startPage;
  switch (startPage)
  {
  case 0:
    nextion.command("page 0");
    break;
  case 1:
    nextion.command("page 1");
    break;
  case 2:
    nextion.command("page 2");
    break;
  default:
    nextion.command("page 0");
    break;
  }
  nextion.command("doevents"); // Force immediate screen refresh and receive serial bytes to buffer
  // start display tikcer
  activePage = nextion.getCurrentPageID();
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

void BrewPage() // Seite 1
{
  currentStepName_text.attribute("txt", currentStepName);
  currentStepRemain_text.attribute("txt", currentStepRemain);
  nextStepRemain_text.attribute("txt", nextStepRemain);
  nextStepName_text.attribute("txt", nextStepName);

  if (strlen(structKettles[0].id) > 0)
  {
    kettleName1_text.attribute("txt", structKettles[0].name);
    if (sensors[0].getId() != "")
      kettleSoll1_text.attribute("txt", structKettles[0].target_temp);
    else
      kettleSoll1_text.attribute("txt", "na");
    kettleIst1_text.attribute("txt", structKettles[0].current_temp);
  }
  else
    dispPublishmqtt();

  if (strlen(structKettles[1].id) > 0)
  {
    kettleName2_text.attribute("txt", structKettles[1].name);
    if (sensors[1].getId() != "")
      kettleSoll2_text.attribute("txt", structKettles[1].target_temp);
    else
      kettleSoll2_text.attribute("txt", "na");
    kettleIst2_text.attribute("txt", structKettles[1].current_temp);
  }
  if (strlen(structKettles[2].id) > 0)
  {
    kettleName3_text.attribute("txt", structKettles[2].name);
    if (sensors[2].getId() != "")
      kettleSoll3_text.attribute("txt", structKettles[2].target_temp);
    else
      kettleSoll3_text.attribute("txt", "na");
    kettleIst3_text.attribute("txt", structKettles[2].current_temp);
  }
  if (strlen(structKettles[3].id) > 0)
  {
    kettleName4_text.attribute("txt", structKettles[3].name);
    if (sensors[3].getId() != "")
      kettleSoll4_text.attribute("txt", structKettles[3].target_temp);
    else
      kettleSoll4_text.attribute("txt", "na");
    kettleIst4_text.attribute("txt", structKettles[3].current_temp);
  }
  progress.value(sliderval);
  notification.attribute("txt", notify);
}

void KettlePage() // Seite 2
{
  if (strlen(structKettles[0].sensor) != 0)
  {
    p1temp_text.attribute("txt", structKettles[0].current_temp);
    p1target_text.attribute("txt", structKettles[0].target_temp);

    // Serial.printf("KettlePage sensor %s current temp: %s\n", structKettles[0].sensor, structKettles[0].current_temp);

    // if (sensors[0].getId() != "")
    // {
    //   for (uint8_t i = 0; i < maxKettles; i++)
    //   {
    //     if (strcmp(structKettles[i].sensor, sensors[0].getId().c_str()) == 0)
    //     {
    //       p1temp_text.attribute("txt", structKettles[i].current_temp);
    //       p1target_text.attribute("txt", structKettles[i].target_temp);
    //       break;
    //     }
    //   }
    // }
    // else
    // {
    //   p1temp_text.attribute("txt", structKettles[0].current_temp);
    //   p1target_text.attribute("txt", "na");
    // }
  }
  else
  {
    p1temp_text.attribute("txt", structKettles[0].current_temp);
    p1target_text.attribute("txt", "na");
  }

  p1current_text.attribute("txt", currentStepName);
  p1remain_text.attribute("txt", currentStepRemain);
  p1progress.value(sliderval);
  p1notification.attribute("txt", notify);
}

void InductionPage()
{
  // log_e("Disp: InductionPage activeBrew: %d kettleID0: %s", activeBrew, structKettles[0].id);
  // p2uhrzeit_text
  // p2slider
  // p2temp_text
  // 316 = 0°C - 360 = 44°C - 223 = 100°C -- 53,4 je 20°C

  int32_t aktSlider = p2slider.value();
  if (aktSlider >= 0 && aktSlider <= 100)
    inductionCooker.inductionNewPower(aktSlider); // inductionCooker.handleInductionPage(aktSlider);

  p2temp_text.attribute("txt", String(structKettles[0].current_temp).c_str());
  if (sensors[0].calcOffset() < 16.0)
    p2gauge.attribute("val", (int)(sensors[0].calcOffset() * 2.7 + 316));
  else
    p2gauge.attribute("val", (int)(sensors[0].calcOffset() * 2.7 - 44));
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
        kettleName1_text.attribute("txt", structKettles[i].name);
        kettleSoll1_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 1:
        kettleName2_text.attribute("txt", structKettles[i].name);
        kettleSoll2_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 2:
        kettleName3_text.attribute("txt", structKettles[i].name);
        kettleSoll3_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 3:
        kettleName4_text.attribute("txt", structKettles[i].name);
        kettleSoll4_text.attribute("txt", structKettles[i].target_temp);
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
          kettleSoll1_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 1:
          kettleSoll2_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 2:
          kettleSoll3_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 3:
          kettleSoll4_text.attribute("txt", structKettles[i].target_temp);
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
    // if (structSteps[i].id == doc["id"])
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
        // if (structSteps[i].name == doc["name"])
        if (strcmp(structSteps[i].name, doc["name"]) == 0)
        {
          if (stepsCounter >= i + 1)
          {
            strlcpy(nextStepName, structSteps[i + 1].name, maxStepSign);
            strlcpy(nextStepRemain, structSteps[i + 1].timer, maxRemainSign);
          }
          if (activePage == 0)
          {
            nextStepName_text.attribute("txt", nextStepName);
            nextStepRemain_text.attribute("txt", nextStepRemain);
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
        currentStepName_text.attribute("txt", currentStepName);
      if (activePage == 1)
        p1current_text.attribute("txt", currentStepName);
    }
    if (doc["state_text"] != 0 && doc["state_text"] != "Waiting for Target Temp")
    {
      strlcpy(currentStepRemain, doc["state_text"] | "", maxRemainSign);
      if (activePage == 0)
        currentStepRemain_text.attribute("txt", currentStepRemain);
      if (activePage == 1)
        p1remain_text.attribute("txt", currentStepRemain);

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
          progress.value(sliderval);
        if (activePage == 1)
          p1progress.value(sliderval);
      }
      else
      {
        sliderval = 0;
        if (activePage == 0)
          progress.value(0);
        if (activePage == 1)
          p1progress.value(0);
      }
    }
    else
    {
      // if (props.containsKey("Timer"))
      if (props["Timer"].is<int>())
      {
        int minutes = props["Timer"].as<int>();
        sprintf(currentStepRemain, "%02d:%02d", minutes, 0);
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        if (activePage == 1)
          p1remain_text.attribute("txt", currentStepRemain);
      }
      else
      {
        strcpy(currentStepRemain, "0:00");
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        if (activePage == 1)
          p1remain_text.attribute("txt", currentStepRemain);
      }
      sliderval = 0;
      if (activePage == 0)
        progress.value(sliderval);
      if (activePage == 1)
        p1progress.value(sliderval);
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

// void cbpi4fermenter_handlemqtt(unsigned char *payload, unsigned int length)
void cbpi4fermenter_handlemqtt(unsigned char *payload)
{
  // Serial.printf("Display ferm len: %d\n", length);
  // for (unsigned int i = 0; i < length; i++)
  // {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println();

  JsonDocument doc;
  JsonDocument filter;
  filter["id"] = true;
  filter["state"] = true;
  filter["name"] = true;
  filter["sensor"] = true;
  filter["target_temp"] = true;
  // filter["steps"]["name"] = true;
  DeserializationError error = deserializeJson(doc, (const char *)payload, DeserializationOption::Filter(filter));
  // DeserializationError error = deserializeJson(doc, (const char *)payload);
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

      // Serial.printf("#%d new kettle name %s kettle-id: %s sensor-id: %s target: %s update: %s\n", i, structKettles[i].name, structKettles[i].id, structKettles[i].sensor, structKettles[i].target_temp, sensorupdate);

      switch (i)
      {
      case 0:
        kettleName1_text.attribute("txt", structKettles[i].name);
        kettleSoll1_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 1:
        kettleName2_text.attribute("txt", structKettles[i].name);
        kettleSoll2_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 2:
        kettleName3_text.attribute("txt", structKettles[i].name);
        kettleSoll3_text.attribute("txt", structKettles[i].target_temp);
        break;
      case 3:
        kettleName4_text.attribute("txt", structKettles[i].name);
        kettleSoll4_text.attribute("txt", structKettles[i].target_temp);
        break;
      }
      break;
    }
    else // structKettle belegt
    {
      if (strcmp(structKettles[i].id, doc["id"]) == 0)
      {
      // Serial.printf("#%d old kettle name %s kettle-id: %s sensor-id: %s target: %s update: %s\n", i, structKettles[i].name, structKettles[i].id, structKettles[i].sensor, structKettles[i].target_temp, sensorupdate);
        dtostrf(doc["target_temp"], -1, 1, structKettles[i].target_temp);
        switch (i)
        {
        case 0:
          kettleSoll1_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 1:
          kettleSoll2_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 2:
          kettleSoll3_text.attribute("txt", structKettles[i].target_temp);
          break;
        case 3:
          kettleSoll4_text.attribute("txt", structKettles[i].target_temp);
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
    // sprintf(currentStepRemain, "%02d:%02d:%02d", doc["props"]["TimerD"].as<int>(), doc["props"]["TimerH"].as<int>(), doc["props"]["TimerM"].as<int>());
  }
  else
  {
    sprintf(currentStepName, "%s", "");
    sprintf(currentStepRemain, "%s", "");
  }
}
