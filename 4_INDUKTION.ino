class induction
{
  int8_t PIN_WHITE = 13;     // D7 Relay white
  int8_t PIN_YELLOW = 12;    // D6 Command channel yellow AUSGABE AN PLATTE
  int8_t PIN_INTERRUPT = 14; // D5 Back channel blue EINGABE VON PLATTE
  uint8_t power = 0;
  uint8_t newPower = 0;
  uint8_t oldPower = 0;
  int8_t CMD_CUR = 0;           // Aktueller Befehl
  boolean isRelayon = false;    // Systemstatus: ist das Relais in der Platte an?
  boolean oldisRelayon = false; // Systemstatus: ist das Relais in der Platte an?
  boolean isInduon = false;     // Systemstatus: ist Power > 0?
  boolean oldisInduon = false;
  boolean isPower = false;
  String mqtttopic = "";
  boolean isEnabled = false;
  uint8_t powerLevelOnError = 100;   // 100% schaltet das Event handling für Induktion aus
  uint8_t powerLevelBeforeError = 0; // in error event save last power state
  bool induction_state = true;       // Error state induction

  unsigned long timeTurnedoff;
  unsigned long lastInterrupt;
  unsigned long lastCommand;
  bool inputStarted = false;
  unsigned char inputCurrent = 0;
  unsigned char inputBuffer[33];
  long powerSampletime = 20000;
  unsigned long powerLast;
  long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
  long powerLow = 0;
  // Induktion Signallaufzeiten
  const int16_t SIGNAL_HIGH = 5120;
  const int16_t SIGNAL_HIGH_TOL = 1500;
  const int16_t SIGNAL_LOW = 1280;
  const int16_t SIGNAL_LOW_TOL = 500;
  const int16_t SIGNAL_START = 25;
  const int16_t SIGNAL_START_TOL = 10;
  const int16_t SIGNAL_WAIT = 10;
  const int16_t SIGNAL_WAIT_TOL = 5;

  /*  Binäre Signale für Induktionsplatte */
  int16_t CMD[6][33] = {
      {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
  uint8_t PWR_STEPS[6] = {0, 20, 40, 60, 80, 100};

public:
  
  induction()
  {
    setupCommands();
  }

  void change(const int8_t &pinwhite, const int8_t &pinyellow, const int8_t &pinblue, const String &topic, const bool &is_enabled, const uint8_t &powerLevel)
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
        // Interrupt deaktivert
        // detachInterrupt(PIN_INTERRUPT);
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
    powerLevelOnError = powerLevel;
    induction_state = true;
    isEnabled = is_enabled;
    if (isEnabled)
    {
      // neue PINS aktiveren
      if (isPin(PIN_WHITE))
      {
        pinMode(PIN_WHITE, OUTPUT);
        digitalWrite(PIN_WHITE, HIGH);
        pins_used[PIN_WHITE] = true;
      }

      if (isPin(PIN_YELLOW))
      {
        pinMode(PIN_YELLOW, OUTPUT);
        digitalWrite(PIN_YELLOW, HIGH);
        pins_used[PIN_YELLOW] = true;
      }

      if (isPin(PIN_INTERRUPT)) // D7
      {
        // Interrupt deaktivert
        // attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), readInputWrap, CHANGE);
        pinMode(PIN_INTERRUPT, INPUT_PULLUP);
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

  void handlemqtt(char *payload)
  {
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      DEBUG_MSG("Ind: handlemqtt deserialize Json error %s\n", error.c_str());
      return;
    }
    if (doc["state"] == "off")
      newPower = 0;
    else
      newPower = doc["power"];
  }

  // Interrupt deaktivert
  // unsigned char getPinInterrupt()
  // {
  //   return PIN_INTERRUPT;
  // }

  void setupCommands()
  {
    for (uint8_t i = 0; i < 33; i++)
    {
      for (uint8_t j = 0; j < 6; j++)
      {
        if (CMD[j][i] == 1)
          CMD[j][i] = SIGNAL_HIGH;
        else
          CMD[j][i] = SIGNAL_LOW;
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
      if (millis() > timeTurnedoff + DEF_DELAY_IND)
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

  void inductionNewPower(int16_t val)
  {
    newPower = constrain(val, 0, 100);
  }

  void updatePower()
  {
    if (power != newPower) // Neuer Befehl empfangen
    {
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
        for (uint8_t i = 1; i < 7; i++)
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

  void sendCommand(int16_t command[33])
  {
    digitalWrite(PIN_YELLOW, HIGH);
    millis2wait(SIGNAL_START);
    digitalWrite(PIN_YELLOW, LOW);
    millis2wait(SIGNAL_WAIT);

    // PIN_YELLOW := Ausgabe an IDS2
    for (uint8_t i = 0; i < 33; i++)
    {
      digitalWrite(PIN_YELLOW, HIGH);
      micros2wait(command[i]);
      digitalWrite(PIN_YELLOW, LOW);
      micros2wait(SIGNAL_LOW);
    }
  }

  // Interrupt deaktivert
  /*
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
  */

  void indERR()
  {
    if (isInduon && powerLevelOnError < 100 && induction_state) // powerlevelonerror == 100 -> kein event handling
    {
      powerLevelBeforeError = power;
      DEBUG_MSG("IND ERR: Induktion powerlevel: %d reduce power to: %d\n", power, powerLevelOnError);
      if (powerLevelOnError == 0)
        isInduon = false;
      else
        newPower = powerLevelOnError;

      newPower = powerLevelOnError;
      induction_state = false;
      Update();
    }
  }
    int8_t getPinWhite()
  {
    return PIN_WHITE;
  }
  int8_t getPinYellow()
  {
    return PIN_YELLOW;
  }
  int8_t getPinInterrupt()
  {
    return PIN_INTERRUPT;
  }
  void setPinWhite(int8_t val)
  {
    PIN_WHITE = val;
  }
  void setPinYellow(int8_t val)
  {
    PIN_YELLOW = val;
  }
  void setPinInterrupt(int8_t val)
  {
    PIN_INTERRUPT = val;
  }
  String getTopic()
  {
    return mqtttopic;
  }
  uint8_t getPower()
  {
    return power;
  }
  uint8_t getOldPower()
  {
    return oldPower;
  }
  void setOldPower()
  {
    oldPower = power;
  }
  int8_t getCMD_CUR()
  {
    return CMD_CUR;
  }
  uint8_t getNewPower()
  {
    return newPower;
  }
  void setNewPower(int16_t val)
  {
    newPower = constrain(val, 0, 100);
  }
  bool getisRelayon()
  {
    return isRelayon;
  }
  bool getoldisRelayon()
  {
    return oldisRelayon;
  }
  void setoldisRelayon()
  {
    oldisRelayon = isRelayon;
  }
  bool getisInduon()
  {
    return isInduon;
  }
  void setisInduon(bool val)
  {
    isInduon = val;
  }
  bool getoldisInduon()
  {
    return oldisInduon;
  }
  void setoldisInduon()
  {
    oldisInduon = isInduon;
  }
  bool getisPower()
  {
    return isPower;
  }
  uint8_t getIsEnabled()
  {
    return isEnabled;
  }
  void setIsEnabled(uint8_t val)
  {
    isEnabled = val;
  }
  bool getInductionState()
  {
    return induction_state;
  }
  void setInductionState(bool val)
  {
    induction_state = val;
  }
  uint8_t getPowerLevelOnError()
  {
    return powerLevelOnError;
  }
  uint8_t getPowerLevelBeforeError()
  {
    return powerLevelBeforeError;
  }
};

induction inductionCooker = induction();

// Interrupt deaktivert
// ICACHE_RAM_ATTR void readInputWrap()
// {
//   inductionCooker.readInput();
// }

void handleInduction()
{
  inductionCooker.Update();
}

void handleRequestInduction()
{
  DynamicJsonDocument doc(386);
  doc["enabled"] = inductionCooker.getIsEnabled();
  doc["power"] = 0;
  if (inductionCooker.getIsEnabled())
  {
    doc["relayOn"] = inductionCooker.getisRelayon();
    doc["power"] = inductionCooker.getPower();
    doc["state"] = inductionCooker.getInductionState();
    if (inductionCooker.getisPower())
    {
      doc["powerLevel"] = inductionCooker.getCMD_CUR();
    }
    else
    {
      doc["powerLevel"] = max(0, inductionCooker.getCMD_CUR() - 1);
    }
  }

  doc["topic"] = inductionCooker.getTopic();
  doc["pl"] = inductionCooker.getPowerLevelOnError();
  String response;
  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response.c_str());
  // size_t len = measureJson(doc);
  // int memoryUsed = doc.memoryUsage();
  // DEBUG_MSG("Ind JSON config length: %d\n", len);
  // DEBUG_MSG("Ind JSON memory usage: %d\n", memoryUsed);
}

void handleRequestIndu()
{
  String request = server.arg(0);
  String message;

  if (request == "pins")
  {
    int8_t id = server.arg(1).toInt();
    unsigned char pinswitched;
    switch (id)
    {
    case 0:
      pinswitched = inductionCooker.getPinWhite();
      break;
    case 1:
      pinswitched = inductionCooker.getPinYellow();
      break;
    case 2:
      pinswitched = inductionCooker.getPinInterrupt();
      break;
    }
    if (isPin(pinswitched))
    {
      message += F("<option>");
      message += PinToString(pinswitched);
      message += F("</option><option disabled>──────────</option>");
    }
    const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};
    for (uint8_t i = 0; i < NUMBEROFPINS; i++)
    {
      if (pins_used[pins[i]] == false)
      {
        message += F("<option>");
        message += pin_names[i];
        message += F("</option>");
      }
      yield();
    }
  }
  server.send(200, FPSTR("text/plain"), message.c_str());
}

void handleSetIndu()
{
  int8_t pin_white = inductionCooker.getPinWhite();
  int8_t pin_blue = inductionCooker.getPinInterrupt();
  int8_t pin_yellow = inductionCooker.getPinYellow();
  bool is_enabled = inductionCooker.getIsEnabled();
  String topic = inductionCooker.getTopic();
  int8_t pl = inductionCooker.getPowerLevelOnError();

  for (uint8_t i = 0; i < server.args(); i++)
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
    if (server.argName(i) == "pl")
    {
      if (isValidInt(server.arg(i)))
        pl = constrain(server.arg(i).toInt(), 0, 100);
      else
        pl = 100;
    }
    yield();
  }

  inductionCooker.change(pin_white, pin_yellow, pin_blue, topic, is_enabled, pl);
  saveConfig();
  server.send(200, FPSTR("text/plain"), "ok");
  inductionSSE(true);
}
