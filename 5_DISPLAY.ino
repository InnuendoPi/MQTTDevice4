void initDisplay()
{
  nextion.command("rest");
  // nextion.command("doevents");
  millis2wait(2000); // wait short delay after reset befor passing new commands to display
  sprintf(structKettles[0].target_temp, "%s", "0");
  sprintf(structKettles[0].current_temp, "%s", "0.0");
  brewButton.touch(kettleCallback);
  kettleButton.touch(brewCallback);
  if (startPage == 0)
  {
    activePage = 0;
    nextion.command("page brewpage");
    BrewPage();
  }
  else
  {
    activePage = 1;
    nextion.command("page kettlepage");
    KettlePage();
  }
}

void BrewPage()
{
  uhrzeit_text.attribute("txt", uhrzeit);
  // DEBUG_MSG("+BrewPage ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
  if (strlen(structKettles[0].id) == 0 && !activeBrew)
  {
    // DEBUG_MSG("Disp: BrewPage kettle#ID0 not init %s\n", structKettles[0].id);
    currentStepName_text.attribute("txt", "BrewPage");
    notification.attribute("txt", notify);
  }
  else
  {
    currentStepName_text.attribute("txt", currentStepName);
    currentStepRemain_text.attribute("txt", currentStepRemain);
    nextStepRemain_text.attribute("txt", nextStepRemain);
    nextStepName_text.attribute("txt", nextStepName);
    kettleName1_text.attribute("txt", structKettles[0].name);
    kettleSoll1_text.attribute("txt", structKettles[0].target_temp);
    kettleIst1_text.attribute("txt", structKettles[0].current_temp);
    kettleName2_text.attribute("txt", structKettles[1].name);
    kettleSoll2_text.attribute("txt", structKettles[1].target_temp);
    kettleIst2_text.attribute("txt", structKettles[1].current_temp);
    kettleName3_text.attribute("txt", structKettles[2].name);
    kettleSoll3_text.attribute("txt", structKettles[2].target_temp);
    kettleIst3_text.attribute("txt", structKettles[2].current_temp);
    kettleName4_text.attribute("txt", structKettles[3].name);
    kettleSoll4_text.attribute("txt", structKettles[3].target_temp);
    kettleIst4_text.attribute("txt", structKettles[3].current_temp);
    slider.value(sliderval);
    notification.attribute("txt", notify);
  }
}

void KettlePage()
{
  p1uhrzeit_text.attribute("txt", uhrzeit);
  // DEBUG_MSG("-KettlePage ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
  // DEBUG_MSG("--- Calling KettlePage ID: %s strlen: %d \n", structKettles[0].id, strlen(structKettles[0].name));
  if (strlen(structKettles[0].id) == 0 && !activeBrew)
  {
    p1temp_text.attribute("txt", sensors[0].getTotalValueString());
    p1target_text.attribute("txt", structKettles[0].target_temp);
    p1current_text.attribute("txt", sensors[0].getName().c_str());
    // p1notification.attribute("txt", notify);
  }
  else
  {
    p1current_text.attribute("txt", currentStepName);
    p1remain_text.attribute("txt", currentStepRemain);
    p1temp_text.attribute("txt", structKettles[0].current_temp);
    p1target_text.attribute("txt", structKettles[0].target_temp);
    p1slider.value(sliderval);
    p1notification.attribute("txt", notify);
  }
}

/*
void cbpi4sensor_subscribe()
{
  return;
  if (pubsubClient.connected())
  {

    for (int i = 0; i < maxKettles; i++)
    {
      // char *p;
      // p = strstr(topic, structKettles[i].sensor);
      char sensorupdate[45];
      // = "cbpi/sensordata/" + structKettles[i].sensor;
      sprintf(sensorupdate, "%s%s", "cbpi/sensordata/", structKettles[i].sensor);
      DEBUG_MSG("Disp: Subscribing to %s\n", sensorupdate);
      pubsubClient.subscribe(sensorupdate);

      // if (strstr(cbpi4sensor_topic, structKettles[i].sensor))
      // {
      //   DEBUG_MSG("Disp: Subscribing to %s\n", cbpi4sensor_topic);
      //   pubsubClient.subscribe(cbpi4sensor_topic);
      // }
    }
  }
}

void cbpi4sensor_unsubscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Unsubscribing from %s\n", cbpi4sensor_topic);
    pubsubClient.unsubscribe(cbpi4sensor_topic);
  }
}
*/

void cbpi4kettle_subscribe()
{
  if (pubsubClient.connected())
  {
    DEBUG_MSG("Disp: Subscribing to %s\n", cbpi4kettle_topic);
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
  StaticJsonDocument<768> doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);

    return;
  }
  for (int i = 0; i < maxKettles; i++)
  {
    if (strlen(structKettles[i].id) == 0) //structKettle unbelegt
    {
      // DEBUG_MSG("New Kettle setup start %d ID: %s Name: %s Current: %s Target: %s Sensor: %s activepage %d\n", i, structKettles[i].id, structKettles[i].name, structKettles[i].current_temp, structKettles[i].target_temp, structKettles[i].sensor, activePage);
      strcpy(structKettles[i].id, doc["id"]);
      strcpy(structKettles[i].name, doc["name"]);
      dtostrf(doc["target_temp"], 1, 1, structKettles[i].target_temp);
      strcpy(structKettles[i].sensor, doc["sensor"]);
      char sensorupdate[45];
      // sprintf(sensorupdate, "%s%s", "cbpi/sensordata/", structKettles[i].sensor);
      sprintf(sensorupdate, "%s%s", cbpi4sensor_topic, structKettles[i].sensor);
      // DEBUG_MSG("Disp: Subscribing to %s\n", sensorupdate);
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
  }
  // DEBUG_MSG("Disp: kettle_handlemqtt %s %s\n", sensorID.c_str(), target_temp.c_str());
}

void cbpi4sensor_handlemqtt(char *payload)
{
  StaticJsonDocument<128> doc;
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
      
      // structKettles[i].current_temp = doc["value"].as<const char *>());

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
      else
        p1temp_text.attribute("txt", sensors[0].getTotalValueString() );
      // DEBUG_MSG("Sensor value POST %d Sensor-ID: %s Kettle-Sensor: %s value: %s activepage %d\n", i, structKettles[i].id, structKettles[i].sensor, structKettles[i].current_temp, activePage);
      break;
    }
  }
  return;
}

void cbpi4steps_handlemqtt(char *payload)
{
  StaticJsonDocument<768> doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);

    return;
  }
  if (doc["status"] == "A")
  {
    current_step = true;
    // const char *payload_remaining = doc["state_text"];
    // const char *payload_timer;
    int timer = 0;
    int min = 0;
    int sec = 0;
    JsonObject props = doc["props"];
    // if (props.containsKey("Timer"))
    // payload_timer = props["Timer"];

    if (currentStepName != doc["name"]) // New active step
    {
      if (doc["type"] == "MashStep")
      {
        strcpy(notify, "Waiting for Target Temp");
      }
      else if (doc["type"] == "ToggleStep")
      {
        strcpy(notify, "");
      }
      else if (doc["type"] == "NotificationStep")
      {
        strcpy(notify, props["Notification"] | "");
      }
      else if (doc["type"] == "WaitStep")
      {
        strcpy(notify, "WaitStep");
      }
      else if (doc["type"] == "TargetStep")
      {
        strcpy(notify, "");
      }
      else if (doc["type"] == "BoilStep")
      {
        strcpy(notify, "BoilStep");
      }
      // else if (strcmp(payload_type, "MashInStep") == 0)
      else if (doc["type"] == "MashInStep")
      {
        strcpy(notify, "Waiting for Target Temp");
      }
      else if (doc["type"] == "CoolDownStep")
      {
        strcpy(notify, "CoolDownStep");
      }
      else if (doc["type"] == "ActorStep")
      {
        strcpy(notify, "ActorStep");
      }
      else if (doc["type"] == "ClearLogStep")
      {
        strcpy(notify, "");
      }
      else
      {
        strcpy(notify, "");
      }

      if (activePage == 0)
        notification.attribute("txt", notify);
      else
        p1notification.attribute("txt", notify);
    }

    strcpy(currentStepName, doc["name"] | "");
    // DEBUG_MSG("DISP stephandle 1 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    if (activePage == 0)
      currentStepName_text.attribute("txt", currentStepName);
    else
      p1current_text.attribute("txt", currentStepName);

    // DEBUG_MSG("DISP stephandle 2 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    // if ((strcmp(payload_remaining, "\0") != 0) && (strcmp(payload_remaining, "Waiting for Target Temp") != 0))
    if (doc["state_text"] != 0 && doc["state_text"] != "Waiting for Target Temp")
    {
      strcpy(currentStepRemain, doc["state_text"] | "");
      if (activePage == 0)
        currentStepRemain_text.attribute("txt", currentStepRemain);
      else
        p1remain_text.attribute("txt", currentStepRemain);
      // DEBUG_MSG("DISP stephandle 3 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));

      if (doc["type"] == "MashStep" && doc["state_text"] != "")
      {
        char delimiter[] = ":";
        char buf[10];
        strcpy(buf, doc["state_text"].as<const char *>());
        char *hours = strtok(buf, delimiter); // Das erste Zeichen muss ein # sein
        char *minutes = strtok(NULL, delimiter);
        char *seconds = strtok(NULL, delimiter);
        // if (props.containsKey("Timer"))
        timer = props["Timer"].as<int>();
        // else
        //   timer = 0;

        min = atoi(minutes);
        sec = atoi(seconds);
      }
      // DEBUG_MSG("DISP stephandle 4 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      if (timer > 0 && min > 0)
      {
        sliderval = (timer * 60 - (min * 60 + sec)) * 100 / (timer * 60);
        // slider.value((timer * 60 - (min * 60 + sec)) * 100 / (timer * 60));
        if (activePage == 0)
          slider.value(sliderval);
        else
          p1slider.value(sliderval);
      }
      else
      {
        sliderval = 0;
        if (activePage == 0)
          slider.value(0);
        else
          p1slider.value(0);
      }
      // DEBUG_MSG("DISP stephandle 5 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      /*
      min = min * 60;
      int akt = min + sec;
      timer = timer * 60;
      int rest = timer - akt;
      rest = rest * 100 / timer;
      slider.value(rest);
      */
    }
    else
    {
      // DEBUG_MSG("DISP stephandle 6 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      if (props.containsKey("Timer"))
      {
        // int minutes = atoi(props["Timer"]);
        int minutes = props["Timer"].as<int>();
        sprintf(currentStepRemain, "%02d:%02d", minutes, 0);
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        else
          p1remain_text.attribute("txt", currentStepRemain);
        // DEBUG_MSG("DISP stephandle 7 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      }
      else
      {
        // strncpy(currentStepRemain, "0:00", 10);
        strcpy(currentStepRemain, "0:00");
        if (activePage == 0)
          currentStepRemain_text.attribute("txt", currentStepRemain);
        else
          p1remain_text.attribute("txt", currentStepRemain);
        // DEBUG_MSG("DISP stephandle 8 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
      }
      sliderval = 0;
      if (activePage == 0)
        slider.value(sliderval);
      else
        p1slider.value(sliderval);
      // DEBUG_MSG("DISP stephandle 9 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    }
    return;
  }

  /*
{
  "id": "cgsR5F5VHGwwPv98aU6Nnf", 
  "name": "Nachguss off", 
  "state_text": "",
   "type": "ToggleStep", 
   "status": "D", 
   "props": 
   {
     "Actor": "h8aWHihm4vMQxk43tpF8x8", 
     "toggle_type": "Off"
     }
  }
*/

  if (doc["status"] == "I" && current_step)
  {
    current_step = false;
    // const char *payload_name_next = doc["name"];
    // DEBUG_MSG("DISP stephandle NEXT 1 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    JsonObject props = doc["props"];
    // if (props.containsKey("Timer"))
    // payload_timer_next = props["Timer"];
    // if (strcmp(nextStepName, payload_name_next) == 0)
    //   return;
    // strcpy(nextStepName, payload_name_next);
    // int laenge = max(maxStepSign, (int)strlen(payload_name_next));
    // strncpy(nextStepName, payload_name_next, laenge);
    strcpy(nextStepName, doc["name"] | "");
    // DEBUG_MSG("DISP stephandle NEXT 2 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
    if (activePage == 0)
      nextStepName_text.attribute("txt", nextStepName);

    int minutes = 0;
    if (props.containsKey("Timer"))
      minutes = props["Timer"].as<int>();
    sprintf(nextStepRemain, "%02d:%02d", minutes, 0);

    // DEBUG_MSG("DISP stephandle NEXT 3 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));

    if (activePage == 0)
      nextStepRemain_text.attribute("txt", nextStepRemain);
  }
  return;
}

void cbpi4notification_handlemqtt(char *payload)
{
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, (const char *)payload);
  if (error)
  {
    int memoryUsed = doc.memoryUsage();
    DEBUG_MSG("Disp: handlemqtt notification deserialize Json error %s MemoryUsage %d\n", error.c_str(), memoryUsed);
    return;
  }
  if (doc["title"] == "Stop" || doc["title"] == "Brewing completed")
  {
    activeBrew = false;
  }
  if (doc["title"] == "Start" || doc["title"] == "Resume")
    activeBrew = true;

  strcpy(notify, doc["message"] | "");
  if (activePage == 0)
    notification.attribute("txt", notify);
  else
    p1notification.attribute("txt", notify);
  // DEBUG_MSG("DISP notifyhandle5 ActivePage: %d ID: %s Name: %s Sensor: %s strlen: %d\n", activePage, structKettles[0].id, structKettles[0].name, structKettles[0].sensor, strlen(structKettles[0].id));
}
