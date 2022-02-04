void listenerSystem(int event, int parm) // System event listener
{
  switch (parm)
  {
  case EM_OK: // Normal mode
    break;
  // 1 - 9 Error events
  case EM_WLANER:
    WiFi.reconnect();
    if (WiFi.status() == WL_CONNECTED)
      break;

    DEBUG_MSG("%s", "EM WLAN: WLAN error ... try to reconnectn\n");
    break;
  case EM_MQTTER: // MQTT Error -> handling
    if (pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass))
    {
      DEBUG_MSG("%s", "MQTT auto reconnect successful. Subscribing..\n");
      cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
      cbpiEventSystem(EM_MQTTRES); // MQTT restore
      break;
    }
    if (millis() - mqttconnectlasttry >= wait_on_error_mqtt)
    {
      if (StopOnMQTTError && mqtt_state)
      {
        mqtt_state = false; // MQTT in error state
        if (startBuzzer)
          sendAlarm(ALARM_ERROR);
        DEBUG_MSG("EM MQTTER: MQTT Broker %s not availible! StopOnMQTTError: %d mqtt_state: %d\n", mqtthost, StopOnMQTTError, mqtt_state);
        cbpiEventActors(EM_ACTER);
        cbpiEventInduction(EM_INDER);
      }
    }
    break;
  // 10-19 System triggered events
  case EM_MQTTRES: // restore saved values after reconnect MQTT (10)
    if (pubsubClient.connected())
    {
      // wlan_state = true;
      mqtt_state = true;
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state)
        {
          DEBUG_MSG("EM MQTTRES: %s isOnBeforeError: %d Powerlevel: %d\n", actors[i].name_actor.c_str(), actors[i].isOnBeforeError, actors[i].power_actor);
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true; // Sensor ok
          actors[i].Update();
        }
        yield();
      }
      if (!inductionCooker.induction_state)
      {
        DEBUG_MSG("EM MQTTRES: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError, inductionCooker.powerLevelBeforeError);
        inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
        inductionCooker.isInduon = true;
        inductionCooker.induction_state = true; // Induction ok
        inductionCooker.Update();
        DEBUG_MSG("EM MQTTRES: Induction restore old value: %d\n", inductionCooker.newPower);
      }
    }
    break;
  case EM_REBOOT: // Reboot ESP (11) - manual task
    // Stop actors
    cbpiEventActors(EM_ACTOFF);
    // Stop induction
    if (inductionCooker.isInduon)
      cbpiEventInduction(EM_INDOFF);
    server.send(200, "text/plain", "rebooting...");
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
    break;
  // System run & set events
  case EM_WLAN: // check WLAN (20) and reconnect on error
    if (WiFi.status() == WL_CONNECTED)
    {
      if (TickerWLAN.state() == RUNNING)
        TickerWLAN.stop();
    }
    else
    {
      if (TickerWLAN.state() != RUNNING)
      {
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        TickerWLAN.resume();
        wlanconnectlasttry = millis();
      }
      TickerWLAN.update();
    }
    break;
  case EM_WEB:             // (21)
    server.handleClient(); // Webserver handle
    break;
  case EM_MQTT: // check MQTT (22)
    if (!WiFi.status() == WL_CONNECTED)
      break;
    if (TickerMQTT.state() != RUNNING)
    {
      if (pubsubClient.connected())
      {
        mqtt_state = true;
        pubsubClient.loop();
        if (TickerMQTT.state() == RUNNING)
          TickerMQTT.stop();
      }
      else //if (!pubsubClient.connected())
      {
        if (TickerMQTT.state() != RUNNING)
        {
          DEBUG_MSG("%s\n", "MQTT Error: TickerMQTT started");
          mqtt_state = false;
          TickerMQTT.resume();
          mqttconnectlasttry = millis();
        }
        TickerMQTT.update();
      }
    }
    else
      TickerMQTT.update();

    break;
  case EM_MQTTCON:                     // MQTT connect (27)
    if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
    {
      pubsubClient.setServer(mqtthost, mqttport);
      pubsubClient.setCallback(mqttcallback);
      pubsubClient.connect(mqtt_clientid, mqttuser, mqttpass);
      DEBUG_MSG("Connecting MQTT broker %s with client-id: %s user: %s pass: %s\n", mqtthost, mqtt_clientid, mqttuser, mqttpass);
    }
    break;
  case EM_MQTTSUB: // MQTT subscribe (28)
    if (pubsubClient.connected())
    {
      DEBUG_MSG("%s\n", "MQTT connected! Subscribing...");
      mqtt_state = true; // MQTT state ok
      for (int i = 0; i < numberOfActors; i++)
      {
        actors[i].mqtt_subscribe();
        yield();
      }
      if (inductionCooker.isEnabled)
        inductionCooker.mqtt_subscribe();
      if (useDisplay)
      {
        dispPublishmqtt();
        cbpi4kettle_subscribe();
        cbpi4steps_subscribe();
        cbpi4notification_subscribe();
      }
      TickerMQTT.stop();
    }
    break;
  case EM_MDNS: // check MDSN (24)
    mdns.update();
    break;
  case EM_SETNTP: // NTP Update (25)
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();
    break;
  case EM_NTP: // NTP Update (25) -> In Ticker Objekt ausgelagert!
    timeClient.update();
    break;
  case EM_MDNSET: // MDNS setup (26)
    if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() == WL_CONNECTED)
    {
      if (mdns.begin(nameMDNS))
        Serial.printf("*** SYSINFO: mDNS started as %s connected to %s Time: %s RSSI=%d\n", nameMDNS, WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
      else
        Serial.printf("*** SYSINFO: error start mDNS! IP Adresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
    }
    break;
  case EM_LOG:
    if (LittleFS.exists("/log.txt")) // WebUpdate Zertifikate
    {
      fsUploadFile = LittleFS.open("/log.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update index retries count %s\n", line.c_str());
      LittleFS.remove("/log.txt");
    }
    if (LittleFS.exists("/log1.txt")) // css Zertifikate
    {
      fsUploadFile = LittleFS.open("/log1.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update css retries count %s\n", line.c_str());
      LittleFS.remove("/log1.txt");
    }
    if (LittleFS.exists("/log11.txt")) // js Zertifikate
    {
      fsUploadFile = LittleFS.open("/log11.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update JS retries count %s\n", line.c_str());
      LittleFS.remove("/log11.txt");
    }
    if (LittleFS.exists("/log111.txt")) // qj Zertifikate
    {
      fsUploadFile = LittleFS.open("/log111.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update JQuery retries count %s\n", line.c_str());
      LittleFS.remove("/log111.txt");
    }
    if (LittleFS.exists("/log2.txt")) // certs
    {
      fsUploadFile = LittleFS.open("/log2.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update certificate retries count %s\n", line.c_str());
      LittleFS.remove("/log2.txt");
    }
    if (LittleFS.exists("/log3.txt")) // WebUpdate Firmware
    {
      fsUploadFile = LittleFS.open("/log3.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      LittleFS.remove("/log3.txt");
      if (LittleFS.exists("/dev.txt")) // WebUpdate Firmware
      {
        Serial.printf("*** SYSINFO: Update development firmware retries count %s\n", line.c_str());
        LittleFS.remove("/dev.txt");
      }
      else
      {
        Serial.printf("*** SYSINFO: Update firmware retries count %s\n", line.c_str());
      }
      alertState = true;
    }
    break;
  default:
    break;
  }
}

void listenerSensors(int event, int parm) // Sensor event listener
{
  // 1:= Sensor on Err
  switch (parm)
  {
  case EM_OK:
    // all sensors ok
    lastSenInd = 0; // Delete induction timestamp after event
    lastSenAct = 0; // Delete actor timestamp after event
    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && mqtt_state)
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
    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && mqtt_state)
    {
      for (int i = 0; i < numberOfSensors; i++)
      {
        if (!sensors[i].getState())
        {
          switch (parm)
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
            cbpiEventActors(EM_ACTER);

          if (millis() - lastSenInd >= wait_on_Sensor_error_induction) // Wait bevor Event handling
          {
            if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
              cbpiEventInduction(EM_INDER);
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

void listenerActors(int event, int parm) // Actor event listener
{
  switch (parm)
  {
  case EM_OK:
    break;
  case 1:
    break;
  case 2:
    break;
  case EM_ACTER:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].switchable && actors[i].actor_state && actors[i].isOn)
      {
        actors[i].isOnBeforeError = actors[i].isOn;
        actors[i].isOn = false;
        actors[i].actor_state = false;
        actors[i].Update();
        DEBUG_MSG("EM ACTER: Aktor: %s : %d isOnBeforeError: %d\n", actors[i].name_actor.c_str(), actors[i].actor_state, actors[i].isOnBeforeError);
      }
      yield();
    }
    break;
  case EM_ACTOFF:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].isOn)
      {
        actors[i].isOn = false;
        actors[i].Update();
        DEBUG_MSG("EM ACTER: Aktor: %s  switched off\n", actors[i].name_actor.c_str());
      }
      yield();
    }
    break;
  default:
    break;
  }
  handleActors();
}
void listenerInduction(int event, int parm) // Induction event listener
{
  switch (parm)
  {
  case EM_OK: // Induction off
    break;
  case 1: // Induction on
    break;
  case 2:
    //DBG_PRINTLN("EM IND2: received induction event"); // bislang keine Verwendung
    break;
  case EM_INDER:
    if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state) // powerlevelonerror == 100 -> kein event handling
    {
      inductionCooker.powerLevelBeforeError = inductionCooker.power;
      DEBUG_MSG("EM INDER: Induktion powerlevel: %d reduce power to: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError);
      if (inductionCooker.powerLevelOnError == 0)
        inductionCooker.isInduon = false;
      else
        inductionCooker.newPower = inductionCooker.powerLevelOnError;

      inductionCooker.newPower = inductionCooker.powerLevelOnError;
      inductionCooker.induction_state = false;
      inductionCooker.Update();
    }
    break;
  case EM_INDOFF:
    if (inductionCooker.isInduon)
    {
      DEBUG_MSG("%s\n", "EM INDOFF: induction switched off");
      inductionCooker.newPower = 0;
      inductionCooker.isInduon = false;
      inductionCooker.Update();
    }
    break;
  default:
    break;
  }
  handleInduction();
}

void cbpiEventSystem(int parm) // System events
{
  gEM.queueEvent(EventManager::kEventUser0, parm);
}

void cbpiEventSensors(int parm) // Sensor events
{
  gEM.queueEvent(EventManager::kEventUser1, parm);
}
void cbpiEventActors(int parm) // Actor events
{
  gEM.queueEvent(EventManager::kEventUser2, parm);
}
void cbpiEventInduction(int parm) // Induction events
{
  gEM.queueEvent(EventManager::kEventUser3, parm);
}
