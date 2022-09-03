class induction
{
  unsigned long timeTurnedoff;

  long timeOutCommand = 5000;  // TimeOut für Seriellen Befehl
  long timeOutReaction = 2000; // TimeOut für Induktionskochfeld
  unsigned long lastInterrupt;
  unsigned long lastCommand;
  bool inputStarted = false;
  unsigned char inputCurrent = 0;
  unsigned char inputBuffer[33];
  bool isError = false;
  unsigned char error = 0;
  long powerSampletime = 20000;
  unsigned long powerLast;
  long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
  long powerLow = 0;

public:
  unsigned char PIN_WHITE = 14;     // D5 RELAIS
  unsigned char PIN_YELLOW = 12;    // D6 AUSGABE AN PLATTE
  unsigned char PIN_INTERRUPT = 13; // D7 EINGABE VON PLATTE
  int power = 0;
  int newPower = 0;
  unsigned char CMD_CUR = 0; // Aktueller Befehl
  boolean isRelayon = false; // Systemstatus: ist das Relais in der Platte an?
  boolean isInduon = false;  // Systemstatus: ist Power > 0?
  boolean isPower = false;
  String mqtttopic = "";
  boolean isEnabled = false;
  long delayAfteroff = 120000;
  int powerLevelOnError = 100;   // 100% schaltet das Event handling für Induktion aus
  int powerLevelBeforeError = 0; // in error event save last power state
  bool induction_state = true;   // Error state induction

  // MQTT Publish
  // char induction_mqtttopic[50];      // Für MQTT Kommunikation
  induction()
  {
    setupCommands();
  }

  void change(unsigned char pinwhite, unsigned char pinyellow, unsigned char pinblue, String topic, long delayoff, bool is_enabled, int powerLevel)
  {
    if (isEnabled)
    {
      // aktuelle PINS deaktivieren
      if (isPin(PIN_WHITE))
      {
        digitalWrite(PIN_WHITE, HIGH);
        pins_used[PIN_WHITE] = false;
      }

      if (isPin(PIN_YELLOW))
      {
        digitalWrite(PIN_YELLOW, HIGH);
        pins_used[PIN_YELLOW] = false;
      }

      if (isPin(PIN_INTERRUPT))
      {
        detachInterrupt(PIN_INTERRUPT);
        pinMode(PIN_INTERRUPT, OUTPUT);

        // digitalWrite(PIN_INTERRUPT, HIGH);
        pins_used[PIN_INTERRUPT] = false;
      }
      mqtt_unsubscribe();
    }

    // Neue Variablen Speichern
    PIN_WHITE = pinwhite;
    PIN_YELLOW = pinyellow;
    PIN_INTERRUPT = pinblue;

    mqtttopic = topic;
    delayAfteroff = delayoff;
    powerLevelOnError = powerLevel;
    induction_state = true;

    // MQTT Publish
    // mqtttopic.toCharArray(induction_mqtttopic, mqtttopic.length() + 1);

    isEnabled = is_enabled;
    if (isEnabled)
    {
      // neue PINS aktiveren
      if (isPin(PIN_WHITE))
      {
        pinMode(PIN_WHITE, OUTPUT);
        digitalWrite(PIN_WHITE, LOW);
        pins_used[PIN_WHITE] = true;
      }

      if (isPin(PIN_YELLOW))
      {
        pinMode(PIN_YELLOW, OUTPUT);
        digitalWrite(PIN_YELLOW, LOW);
        pins_used[PIN_YELLOW] = true;
      }

      if (isPin(PIN_INTERRUPT))
      {
        attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), readInputWrap, CHANGE);

        // pinMode(PIN_INTERRUPT, INPUT_PULLUP);
        pins_used[PIN_INTERRUPT] = true;
      }
      mqtt_subscribe();
    }
  }

  void mqtt_subscribe()
  {
    if (isEnabled)
    {
      if (pubsubClient.connected())
      {
        char subscribemsg[50];
        mqtttopic.toCharArray(subscribemsg, 50);
        DEBUG_MSG("Ind: Subscribing to %s\n", subscribemsg);
        pubsubClient.subscribe(subscribemsg);
      }
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      mqtttopic.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Ind: Unsubscribing from %s\n", subscribemsg);
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  /*
          // MQTT Publish
          void publishmqtt() {
            if (client.connected()) {
              StaticJsonBuffer<256> jsonBuffer;
              JsonObject& json = jsonBuffer.createObject();
              if (isInduon) {
                json["state"] = "on";
                json["power"] = String(power);
              }
              else
                json["state"] = "off";

              char jsonMessage[100];
              json.printTo(jsonMessage);
              client.publish(induction_mqtttopic, jsonMessage);
              DBG_PRINT("MQTT pub message: ");
              DBG_PRINTLN(jsonMessage);
            }
          }
    */

  void handlemqtt(char *payload)
  {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      DEBUG_MSG("Ind: handlemqtt deserialize Json error %s\n", error.c_str());
      return;
    }
    if (doc["state"] == "off")
    {
      newPower = 0;
      return;
    }
    else
    {
      newPower = doc["power"];
    }
  }

  unsigned char getPinInterrupt()
  {
    return PIN_INTERRUPT;
  }

  void handleInductionPage(int value)
  {
    return;
    if (value > 0)
    {
      newPower = value;
    }
    else
    {
      newPower = 0;
    }
  }

  void inductionNewPower(int value)
  {
    if (value > 0)
    {
      newPower = value;
    }
    else
    {
      newPower = 0;
    }
  }

  void setupCommands()
  {
    for (int i = 0; i < 33; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        if (CMD[j][i] == 1)
        {
          CMD[j][i] = SIGNAL_HIGH;
        }
        else
        {
          CMD[j][i] = SIGNAL_LOW;
        }
      }
    }
  }

  bool updateRelay()
  {
    if (isInduon == true && isRelayon == false)
    { /* Relais einschalten */
      digitalWrite(PIN_WHITE, HIGH);
      return true;
    }

    if (isInduon == false && isRelayon == true)
    { /* Relais ausschalten */
      if (millis() > timeTurnedoff + delayAfteroff)
      {
        digitalWrite(PIN_WHITE, LOW);
        return false;
      }
    }

    if (isInduon == false && isRelayon == false)
    { /* Ist aus, bleibt aus. */
      return false;
    }

    return true; /* Ist an, bleibt an. */
  }

  void Update()
  {
    updatePower();

    isRelayon = updateRelay();

    if (isInduon && power > 0)
    {
      if (millis() > powerLast + powerSampletime)
      {
        powerLast = millis();
      }
      if (millis() > powerLast + powerHigh)
      {
        sendCommand(CMD[CMD_CUR - 1]);
        isPower = false;
      }
      else
      {
        sendCommand(CMD[CMD_CUR]);
        isPower = true;
      }
    }
    else if (isRelayon)
    {
      sendCommand(CMD[0]);
    }
  }

  // Test 20220903
  void updatePower()
  {
    if (power != newPower)  // Neuer Befehl empfangen
    {
      if (newPower > 100)
      {
        newPower = 100; // Nicht > 100
      }
      if (newPower < 0)
      {
        newPower = 0; // Nicht < 0
      }
      power = newPower;

      timeTurnedoff = 0;
      isInduon = true;
      if (power == 0)
      {
        CMD_CUR = 0;
        timeTurnedoff = millis();
        isInduon = false;
        /* Wie lange "HIGH" oder "LOW" */
        powerHigh = powerSampletime;
        powerLow = 0;
      }
      else
      {
        for (int i = 1; i < 7; i++)
        {
          if (power <= PWR_STEPS[i])
          {
            CMD_CUR = i;
            /* Wie lange "HIGH" oder "LOW" */
            powerLow = powerSampletime * (PWR_STEPS[i] - power) / 20L;
            powerHigh = powerSampletime - powerLow;
            return;
          }
        }
      }
    }
  }

  // ori
  // void updatePower()
  // {
  //   lastCommand = millis();
  //   if (power != newPower)
  //   { /* Neuer Befehl empfangen */

  //     if (newPower > 100)
  //     {
  //       newPower = 100; /* Nicht > 100 */
  //     }
  //     if (newPower < 0)
  //     {
  //       newPower = 0; /* Nicht < 0 */
  //     }
  //     power = newPower;

  //     timeTurnedoff = 0;
  //     isInduon = true;
  //     long difference = 0;

  //     if (power == 0)
  //     {
  //       CMD_CUR = 0;
  //       timeTurnedoff = millis();
  //       isInduon = false;
  //       difference = 0;
  //       goto setPowerLevel;
  //     }

  //     for (int i = 1; i < 7; i++)
  //     {
  //       if (power <= PWR_STEPS[i])
  //       {
  //         CMD_CUR = i;
  //         difference = PWR_STEPS[i] - power;
  //         goto setPowerLevel;
  //       }
  //     }

  //   setPowerLevel: /* Wie lange "HIGH" oder "LOW" */
  //     if (difference != 0)
  //     {
  //       powerLow = powerSampletime * difference / 20L;
  //       powerHigh = powerSampletime - powerLow;
  //     }
  //     else
  //     {
  //       powerHigh = powerSampletime;
  //       powerLow = 0;
  //     };
  //   }
  // }

  void sendCommand(int command[33])
  {
    digitalWrite(PIN_YELLOW, HIGH);
    // delay(SIGNAL_START);
    millis2wait(SIGNAL_START);
    digitalWrite(PIN_YELLOW, LOW);
    // delay(SIGNAL_WAIT);
    millis2wait(SIGNAL_WAIT);

    // PIN_YELLOW := Ausgabe an IDS2
    for (int i = 0; i < 33; i++)
    {
      digitalWrite(PIN_YELLOW, HIGH);
      // delayMicroseconds(command[i]);
      micros2wait(command[i]);
      digitalWrite(PIN_YELLOW, LOW);
      micros2wait(SIGNAL_LOW);
      // delayMicroseconds(SIGNAL_LOW);
    }
  }

  void readInput()
  {
    // Variablen sichern
    bool ishigh = digitalRead(PIN_INTERRUPT);
    unsigned long newInterrupt = micros();
    long signalTime = newInterrupt - lastInterrupt;

    // Glitch rausfiltern
    if (signalTime > 10)
    {
      if (ishigh)
      {
        lastInterrupt = newInterrupt; // PIN ist auf Rising, Bit senden hat gestartet :)
      }
      else
      { // Bit ist auf Falling, Bit Übertragung fertig. Auswerten.

        if (!inputStarted)
        { // suche noch nach StartBit.
          if (signalTime < 35000L && signalTime > 15000L)
          {
            inputStarted = true;
            inputCurrent = 0;
          }
        }
        else
        { // Hat Begonnen. Nehme auf.
          if (inputCurrent < 34)
          { // nur bis 33 aufnehmen.
            if (signalTime < (SIGNAL_HIGH + SIGNAL_HIGH_TOL) && signalTime > (SIGNAL_HIGH - SIGNAL_HIGH_TOL))
            {
              // HIGH BIT erkannt
              inputBuffer[inputCurrent] = 1;
              inputCurrent += 1;
            }
            if (signalTime < (SIGNAL_LOW + SIGNAL_LOW_TOL) && signalTime > (SIGNAL_LOW - SIGNAL_LOW_TOL))
            {
              // LOW BIT erkannt
              inputBuffer[inputCurrent] = 0;
              inputCurrent += 1;
            }
          }
          else
          { // Aufnahme vorbei.
            inputCurrent = 0;
            inputStarted = false;
          }
        }
      }
    }
  }
};

induction inductionCooker = induction();

ICACHE_RAM_ATTR void readInputWrap()
{
  inductionCooker.readInput();
}

void handleInduction()
{
  inductionCooker.Update();
}

void handleRequestInduction()
{
  StaticJsonDocument<256> doc;
  doc["enabled"] = inductionCooker.isEnabled;
  if (inductionCooker.isEnabled)
  {
    doc["relayOn"] = inductionCooker.isRelayon;
    doc["power"] = inductionCooker.power;
    doc["relayOn"] = inductionCooker.isRelayon;
    doc["state"] = inductionCooker.induction_state;

    if (inductionCooker.isPower)
    {
      doc["powerLevel"] = inductionCooker.CMD_CUR;
    }
    else
    {
      doc["powerLevel"] = max(0, inductionCooker.CMD_CUR - 1);
    }
  }
  doc["topic"] = inductionCooker.mqtttopic;
  doc["delay"] = inductionCooker.delayAfteroff / 1000;
  doc["pl"] = inductionCooker.powerLevelOnError;

  // PID mode values
  doc["tempvalue"] = sensors[0].getTotalValueString();
  if (autoTune)
  {
    doc["target"] = Setpoint;
    doc["step"] = "AutoTune";
  }
  else
  {
    doc["target"] = structPlan[actMashStep].temp;
    doc["step"] = structPlan[actMashStep].name;
  }
  if (autoTune)
  {
    if (statePower)
      doc["timer"] = "in progress";
    else
      doc["timer"] = "press power";
  }
  else if (TickerMash.state() == RUNNING)
  {
    unsigned long allSeconds = TickerMash.remaining() / 1000;
    int runHours = allSeconds / 3600;
    int secsRemaining = allSeconds % 3600;
    int runMinutes = secsRemaining / 60;
    int runSeconds = secsRemaining % 60;
    char buf[21];
    if (runHours > 0)
      runMinutes += runHours * 60;
    //   sprintf(buf, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);
    // else
    sprintf(buf, "%02d:%02d", runMinutes, runSeconds);
    doc["timer"] = buf;
    // Serial.println(buf);
  }
  else if (TickerMash.state() == STOPPED)
  {
    // if (!structPlan[actMashStep - 1].autonext && actMashStep > 0 && statePause) // check for last step autonext false?
    if (actMashStep > 0 && statePause) // check for last step autonext false?
    {
      doc["timer"] = "pause to continue";
    }
    else
    {
      // doc["timer"] = structPlan[actMashStep].duration;
      unsigned long allSeconds = structPlan[actMashStep].duration * 60;
      int runHours = allSeconds / 3600;
      int secsRemaining = allSeconds % 3600;
      int runMinutes = secsRemaining / 60;
      int runSeconds = secsRemaining % 60;
      char buf[21];
      if (runHours > 0)
        runMinutes += runHours * 60;
      //   sprintf(buf, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);
      // else
      sprintf(buf, "%02d:%02d", runMinutes, runSeconds);
      doc["timer"] = buf;
    }
  }
  else if (TickerMash.state() == PAUSED && TickerMash.counter() >= 1)
  {
    doc["timer"] = "click forward";
  }
  else if (TickerMash.state() == PAUSED)
    doc["timer"] = "paused";

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestIndu()
{
  String request = server.arg(0);
  String message;

  if (request == "pins")
  {
    int id = server.arg(1).toInt();
    unsigned char pinswitched;
    switch (id)
    {
    case 0:
      pinswitched = inductionCooker.PIN_WHITE;
      break;
    case 1:
      pinswitched = inductionCooker.PIN_YELLOW;
      break;
    case 2:
      pinswitched = inductionCooker.PIN_INTERRUPT;
      break;
    }
    if (isPin(pinswitched))
    {
      message += F("<option>");
      message += PinToString(pinswitched);
      message += F("</option><option disabled>──────────</option>");
    }

    for (int i = 0; i < numberOfPins; i++)
    {
      if (pins_used[pins[i]] == false)
      {
        message += F("<option>");
        message += pin_names[i];
        message += F("</option>");
      }
      yield();
    }
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetIndu()
{
  unsigned char pin_white = inductionCooker.PIN_WHITE;
  unsigned char pin_blue = inductionCooker.PIN_INTERRUPT;
  unsigned char pin_yellow = inductionCooker.PIN_YELLOW;
  long delayoff = inductionCooker.delayAfteroff;
  bool is_enabled = inductionCooker.isEnabled;
  String topic = inductionCooker.mqtttopic;
  int pl = inductionCooker.powerLevelOnError;

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "enabled")
    {
      is_enabled = checkBool(server.arg(i));
    }
    if (server.argName(i) == "topic")
    {
      topic = server.arg(i);
    }
    if (server.argName(i) == "pinwhite")
    {
      pin_white = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinyellow")
    {
      pin_yellow = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinblue")
    {
      pin_blue = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "delay")
    {
      delayoff = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "pl")
    {
      if (isValidInt(server.arg(i)))
        pl = server.arg(i).toInt();
      else
        pl = 100;
    }
    yield();
  }

  inductionCooker.change(pin_white, pin_yellow, pin_blue, topic, delayoff, is_enabled, pl);
  saveConfig();
  server.send(201, "text/plain", "created");
}
