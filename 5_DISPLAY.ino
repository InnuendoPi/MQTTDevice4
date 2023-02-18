void initDisplay()
{

  // register callback functions
  p0ForButton.touch(pageCallback);  // BrewPage forward to KettlePage
  p0BackButton.touch(pageCallback); // BrewPage backward to InductionPage
  p1ForButton.touch(pageCallback);  // KelltlePage forward to InductionPage
  p1BackButton.touch(pageCallback); // KettlePage backward to BrewPage
  p2ForButton.touch(pageCallback);  // InductionPage forward to BrewPage
  p2BackButton.touch(pageCallback); // InductionPage backward to KettlePage

  // p0ForButton.release(pageCallback);          // BrewPage forward to KettlePage
  // p0BackButton.release(pageCallback);         // BrewPage backward to InductionPage
  // p1ForButton.release(pageCallback);          // KelltlePage forward to InductionPage
  // p1BackButton.release(pageCallback);         // KettlePage backward to BrewPage
  // p2ForButton.release(pageCallback);          // InductionPage forward to BrewPage
  // p2BackButton.release(pageCallback);         // InductionPage backward to KettlePage
  powerButton.release(powerButtonCallback); // buttonBack auf induction page backward auf page 1

  activePage = startPage;
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
  TickerDisp.start();
}

void dispPublishmqtt()
{
  if (pubsubClient.connected())
  {
    DynamicJsonDocument doc(128);
    char jsonMessage[48];
    serializeJson(doc, jsonMessage);
    // DEBUG_MSG("%s\n", "Disp: Request CBPi4 configuration");
    pubsubClient.publish("cbpi/updatekettle", jsonMessage);
    pubsubClient.publish("cbpi/updateactor", jsonMessage);
    pubsubClient.publish("cbpi/updatesensor", jsonMessage);
    // pubsubClient.publish("cbpi/updatefermenter", jsonMessage);
  }
}

void BrewPage()
{
  // DEBUG_MSG("Disp: BrewPage activeBrew: %d kettleID0: %s\n", activeBrew, structKettles[0].id);
  currentStepName_text.attribute("txt", currentStepName);
  currentStepRemain_text.attribute("txt", currentStepRemain);
  nextStepRemain_text.attribute("txt", nextStepRemain);
  nextStepName_text.attribute("txt", nextStepName);

  if (strlen(structKettles[0].id) > 0)
  {
    kettleName1_text.attribute("txt", structKettles[0].name);
    kettleSoll1_text.attribute("txt", structKettles[0].target_temp);
    kettleIst1_text.attribute("txt", structKettles[0].current_temp);
    // DEBUG_MSG("BP Kettle1: %s Target: %s Temp: %s \n", structKettles[0].name, structKettles[0].target_temp, structKettles[0].current_temp);
  }
  else
    dispPublishmqtt();

  if (strlen(structKettles[1].id) > 0)
  {
    kettleName2_text.attribute("txt", structKettles[1].name);
    kettleSoll2_text.attribute("txt", structKettles[1].target_temp);
    kettleIst2_text.attribute("txt", structKettles[1].current_temp);
    // DEBUG_MSG("BP Kettle2: %s Target: %s Temp: %s \n", structKettles[1].name, structKettles[1].target_temp, structKettles[1].current_temp);
  }
  if (strlen(structKettles[2].id) > 0)
  {
    kettleName3_text.attribute("txt", structKettles[2].name);
    kettleSoll3_text.attribute("txt", structKettles[2].target_temp);
    kettleIst3_text.attribute("txt", structKettles[2].current_temp);
  }
  if (strlen(structKettles[3].id) > 0)
  {
    kettleName4_text.attribute("txt", structKettles[3].name);
    kettleSoll4_text.attribute("txt", structKettles[3].target_temp);
    kettleIst4_text.attribute("txt", structKettles[3].current_temp);
  }
  progress.value(sliderval);
  notification.attribute("txt", notify);
}

void KettlePage()
{
  if (strlen(structKettles[0].sensor) != 0)
  {
    for (int i = 0; i < maxKettles; i++)
    {
      DEBUG_MSG("structKettleID %s - sensorID: %s\n", structKettles[i].sensor, sensors[0].getId().c_str());
      if (strcmp(structKettles[i].sensor, sensors[0].getId().c_str()) == 0)
      {
        p1temp_text.attribute("txt", structKettles[i].current_temp);
        p1target_text.attribute("txt", structKettles[i].target_temp);
        DEBUG_MSG("KP Kettle %d: %s Current: %s Target: %s \n", i, structKettles[i].name, sensors[0].getTotalValueString(), structKettles[i].target_temp);
        break;
      }
    }
  }
  else
  {
    p1temp_text.attribute("txt", structKettles[0].current_temp);
    p1target_text.attribute("txt", "0");
  }

  p1current_text.attribute("txt", currentStepName);
  p1remain_text.attribute("txt", currentStepRemain);
  p1progress.value(sliderval);
  p1notification.attribute("txt", notify);
}

void InductionPage()
{
  // DEBUG_MSG("Disp: InductionPage activeBrew: %d kettleID0: %s\n", activeBrew, structKettles[0].id);
  // p2uhrzeit_text
  // p2slider
  // p2temp_text
  // 316 = 0°C - 360 = 44°C - 223 = 100°C -- 53,4 je 20°C

  int32_t aktSlider = p2slider.value();
  if (aktSlider >= 0 && aktSlider <= 100)
    inductionCooker.inductionNewPower(aktSlider); // inductionCooker.handleInductionPage(aktSlider);

  // String aktTemp = strcat(structKettles[0].current_temp, "°C");
  p2temp_text.attribute("txt", String(structKettles[0].current_temp).c_str());
  if (sensors[0].calcOffset() < 16.0)
  {
    // p2gauge.attribute("val", (int)((sensors[0].getValue() + sensors[0].getOffset1()) * 2.7 + 316));
    p2gauge.attribute("val", (int)(sensors[0].calcOffset() * 2.7 + 316));
  }
  else
  {
    p2gauge.attribute("val", (int)(sensors[0].calcOffset() * 2.7 - 44));
  }

  // p2temp_text.attribute("txt", strcat(structKettles[0].current_temp, "°C"));
}

void cbpi4kettle_subscribe()
{
  if (pubsubClient.connected())
  {
    // DEBUG_MSG("Disp: Subscribing to %s\n", cbpi4kettle_topic);
    pubsubClient.subscribe(cbpi4kettle_topic);
    pubsubClient.loop();
  }
}

void cbpi4kettle_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Unsubscribing from %s\n", cbpi4kettle_topic);
    pubsubClient.unsubscribe(cbpi4steps_topic);
  }
}

void cbpi4steps_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Subscribing to %s\n", cbpi4steps_topic);
    pubsubClient.subscribe(cbpi4steps_topic);
    pubsubClient.loop();
  }
}

void cbpi4steps_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Unsubscribing from %s\n", cbpi4steps_topic);
    pubsubClient.unsubscribe(cbpi4steps_topic);
  }
}

void cbpi4notification_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Subscribing to %s\n", cbpi4notification_topic);
    pubsubClient.subscribe(cbpi4notification_topic);
    pubsubClient.loop();
  }
}
void cbpi4notification_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Unsubscribing from %s\n", cbpi4notification_topic);
    pubsubClient.unsubscribe(cbpi4notification_topic);
  }
}

void cbpi4kettle_handlemqtt(char *payload)
{
  // StaticJsonDocument<1024> doc;
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);
    return;
  }
  for (int i = 0; i < maxKettles; i++)
  {
    if (strlen(structKettles[i].id) == 0) // structKettle unbelegt
    {
      // DEBUG_MSG("New Kettle setup start %d ID: %s Name: %s Current: %s Target: %s Sensor: %s activepage %d\n", i, structKettles[i].id, structKettles[i].name, structKettles[i].current_temp, structKettles[i].target_temp, structKettles[i].sensor, activePage);
      strlcpy(structKettles[i].id, doc["id"], maxIdSign);
      strlcpy(structKettles[i].name, doc["name"], maxKettleSign);
      dtostrf(doc["target_temp"], 1, 1, structKettles[i].target_temp);
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
      // DEBUG_MSG("New Kettle Setup end %d ID: %s Name: %s Current: %s Target: %s Sensor: %s\n", i, structKettles[i].id, structKettles[i].name, structKettles[i].current_temp, structKettles[i].target_temp, structKettles[i].sensor);
      break;
    }
    else // structKettle belegt
    {
      // DEBUG_MSG("++ OLD kettle ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      if (structKettles[i].id == doc["id"])
      {
        dtostrf(doc["target_temp"], 1, 1, structKettles[i].target_temp);
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
    // DEBUG_MSG("OLD kettle ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
  }
  // DEBUG_MSG("Disp: kettle_handlemqtt %s %s\n", sensorID.c_str(), target_temp.c_str());
}

void cbpi4sensor_handlemqtt(char *payload)
{
  // StaticJsonDocument<256> doc;
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);

    return;
  }
  for (int i = 0; i < maxKettles; i++)
  {
    if (structKettles[i].sensor == doc["id"])
    {
      // DEBUG_MSG("Sensor value PRE %d Sensor-ID: %s Kettle-Sensor: %s value: %s activepage: %d\n", i, structKettles[i].id, structKettles[i].sensor, structKettles[i].current_temp, activePage);
      if (activePage == 0)
      {
        dtostrf(doc["value"], 1, 1, structKettles[i].current_temp);
        switch (i)
        {
        case 0:
          kettleIst1_text.attribute("txt", structKettles[i].current_temp);
          break;
        case 1:
          kettleIst2_text.attribute("txt", structKettles[i].current_temp);
          break;
        case 2:
          kettleIst3_text.attribute("txt", structKettles[i].current_temp);
          break;
        case 3:
          kettleIst4_text.attribute("txt", structKettles[i].current_temp);
          break;
        }
      }
      if (activePage == 1)
        p1temp_text.attribute("txt", structKettles[0].current_temp);
      // DEBUG_MSG("Sensor value POST %d Sensor-ID: %s Kettle-Sensor: %s value: %s activepage %d\n", i, structKettles[i].id, structKettles[i].sensor, structKettles[i].current_temp, activePage);
      break;
    }
  }
  return;
}

void cbpi4steps_handlemqtt(char *payload)
{
  // StaticJsonDocument<1024> doc;
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);

    return;
  }
  if (doc["status"] == "D") // ignore solved steps
    return;

  JsonObject props = doc["props"];
  bool newStep = true;

  for (int i = 0; i < stepsCounter; i++)
  {
    if (structSteps[i].id == doc["id"])
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
      for (int i = 0; i < stepsCounter; i++)
      {
        if (structSteps[i].name == doc["name"])
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

      // DEBUG_MSG("DISP stephandle 1 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      if (activePage == 0)
        currentStepName_text.attribute("txt", currentStepName);
      if (activePage == 1)
        p1current_text.attribute("txt", currentStepName);
    }
    // DEBUG_MSG("DISP stephandle 2 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    if (doc["state_text"] != 0 && doc["state_text"] != "Waiting for Target Temp")
    {
      strlcpy(currentStepRemain, doc["state_text"] | "", maxRemainSign);
      if (activePage == 0)
        currentStepRemain_text.attribute("txt", currentStepRemain);
      if (activePage == 1)
        p1remain_text.attribute("txt", currentStepRemain);
      // DEBUG_MSG("DISP stephandle 3 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));

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
      // DEBUG_MSG("DISP stephandle 4 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
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
      // DEBUG_MSG("DISP stephandle 5 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      /*
      min = min * 60;
      int akt = min + sec;
      valTimer = valTimer * 60;
      int rest = valTimer - akt;
      rest = rest * 100 / valTimer;
      slider.value(rest);
      */
    }
    else
    {
      // DEBUG_MSG("DISP stephandle 6 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      if (props.containsKey("Timer"))
      {
        int minutes = props["Timer"].as<int>();
        sprintf(currentStepRemain, "%02d:%02d", minutes, 0);
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        if (activePage == 1)
          p1remain_text.attribute("txt", currentStepRemain);
        // DEBUG_MSG("DISP stephandle 7 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      }
      else
      {
        strcpy(currentStepRemain, "0:00");
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        if (activePage == 1)
          p1remain_text.attribute("txt", currentStepRemain);
        // DEBUG_MSG("DISP stephandle 8 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      }
      sliderval = 0;
      if (activePage == 0)
        progress.value(sliderval);
      if (activePage == 1)
        p1progress.value(sliderval);
      // DEBUG_MSG("DISP stephandle 9 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    }
    return;
  }
}

void cbpi4notification_handlemqtt(char *payload)
{
  // StaticJsonDocument<384> doc;
  DynamicJsonDocument doc(384);
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);
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
      strlcpy(currentStepName, sensors[0].getName().c_str(), maxStepSign);
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
