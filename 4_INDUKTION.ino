class induction
{
  unsigned long timeTurnedoff;

  unsigned long lastInterrupt;
  unsigned long lastCommand;
  bool inputStarted = false;
  unsigned char inputCurrent = 0;
  unsigned char inputBuffer[33];
  unsigned char error = 0;
  long powerSampletime = 20000;
  unsigned long powerLast;
  long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
  long powerLow = 0;
  // Induktion Signallaufzeiten
  const int SIGNAL_HIGH = 5120;
  const int SIGNAL_HIGH_TOL = 1500;
  const int SIGNAL_LOW = 1280;
  const int SIGNAL_LOW_TOL = 500;
  const int SIGNAL_START = 25;
  const int SIGNAL_START_TOL = 10;
  const int SIGNAL_WAIT = 10;
  const int SIGNAL_WAIT_TOL = 5;

  /*  Binäre Signale für Induktionsplatte */
  int CMD[6][33] = {
      {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
  unsigned char PWR_STEPS[6] = {0, 20, 40, 60, 80, 100};

public:
  unsigned char PIN_WHITE = 13;     // D7 Relay white
  unsigned char PIN_YELLOW = 12;    // D6 Command channel yellow AUSGABE AN PLATTE
  unsigned char PIN_INTERRUPT = 14; // D5 Back channel blue EINGABE VON PLATTE
  int power = 0;
  int newPower = 0;
  unsigned char CMD_CUR = 0; // Aktueller Befehl
  boolean isRelayon = false; // Systemstatus: ist das Relais in der Platte an?
  boolean isInduon = false;  // Systemstatus: ist Power > 0?
  boolean isPower = false;
  String mqtttopic = "";
  boolean isEnabled = false;
  int powerLevelOnError = 100;   // 100% schaltet das Event handling für Induktion aus
  int powerLevelBeforeError = 0; // in error event save last power state
  bool induction_state = true;   // Error state induction

  induction()
  {
    setupCommands();
  }

  void change(unsigned char pinwhite, unsigned char pinyellow, unsigned char pinblue, String topic, bool is_enabled, int powerLevel)
  {
    if (isEnabled)
    {
      int type = pinType(PIN_WHITE);
      int pcf_pin = type - GPIOPINS;
      // aktuelle PINS deaktivieren
      if (isPin(PIN_WHITE))
      {
        if (type != -100 && type < GPIOPINS)
          digitalWrite(PIN_WHITE, HIGH);
        else
          pcf020.write(pcf_pin, HIGH);
        pins_used[PIN_WHITE] = false;
      }

      type = pinType(PIN_YELLOW);
      pcf_pin = type - GPIOPINS;

      if (isPin(PIN_YELLOW))
      {
        if (type != -100 && type < GPIOPINS)
          digitalWrite(PIN_YELLOW, HIGH);
        else
          pcf020.write(pcf_pin, HIGH);
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
    // delayAfteroff = delayoff;
    powerLevelOnError = powerLevel;
    induction_state = true;
    isEnabled = is_enabled;
    if (isEnabled)
    {
      int type = pinType(PIN_WHITE);
      int pcf_pin = type - GPIOPINS;
      // neue PINS aktiveren
      if (isPin(PIN_WHITE))
      {
        if (type != -100 && type < GPIOPINS)
        {
          pinMode(PIN_WHITE, OUTPUT);
          digitalWrite(PIN_WHITE, HIGH);
        }
        else if (type >= GPIOPINS)
        {
          pcf020.write(pcf_pin, HIGH);
        }
        pins_used[PIN_WHITE] = true;
      }

      type = pinType(PIN_YELLOW);
      pcf_pin = type - GPIOPINS;

      if (isPin(PIN_YELLOW))
      {
        if (type != -100 && type < GPIOPINS)
        {
          pinMode(PIN_YELLOW, OUTPUT);
          digitalWrite(PIN_YELLOW, HIGH);
        }
        else if (type >= GPIOPINS)
        {
          pcf020.write(pcf_pin, HIGH);
        }
        pins_used[PIN_YELLOW] = true;
      }

      if (isPin(PIN_INTERRUPT)) // D7
      {
        attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), readInputWrap, CHANGE);
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
    int type = pinType(PIN_WHITE);
    int pcf_pin = type - GPIOPINS;
    if (isInduon == true && isRelayon == false)
    { /* Relais einschalten */
      if (type != -100 && type < GPIOPINS)
      {
        digitalWrite(PIN_WHITE, HIGH);
        // DEBUG_MSG("IDS2 update relay GPIO PIN White %s isHigh: %d \n", PinToString(PIN_WHITE).c_str(), HIGH);
      }
      else if (type >= GPIOPINS)
      {
        pcf020.write(pcf_pin, HIGH);
        // DEBUG_MSG("IDS2 update relay PCF PIN White %s : %d isHigh: %d \n", PinToString(PIN_WHITE).c_str(), pcf_pin, HIGH);
      }
      // digitalWrite(PIN_WHITE, HIGH);
      return true;
    }

    if (isInduon == false && isRelayon == true)
    { /* Relais ausschalten */
      if (millis() > timeTurnedoff + DEF_DELAY_IND)
      {
        if (type != -100 && type < GPIOPINS)
        {
          digitalWrite(PIN_WHITE, LOW);
          // DEBUG_MSG("IDS2 update relay GPIO PIN White %s isLow: %d \n", PinToString(PIN_WHITE).c_str(), LOW);
        }
        else if (type >= GPIOPINS)
        {
          pcf020.write(pcf_pin, LOW);
          // DEBUG_MSG("IDS2 update relay PCF PIN White %s : %d isLow: %d \n", PinToString(PIN_WHITE).c_str(), pcf_pin, LOW);
        }
        // digitalWrite(PIN_WHITE, LOW);
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

  void inductionNewPower(int value)
  {
    newPower = min(100, value);
    newPower = max(0, newPower);
  }

  void updatePower()
  {
    if (power != newPower) // Neuer Befehl empfangen
    {
      // if (newPower > 100)
      //   newPower = 100; // Nicht > 100
      // if (newPower < 0)
      //   newPower = 0; // Nicht < 0
      // power = newPower;

      newPower = min(100, newPower);
      power = max(0, newPower);

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

  void sendCommand(int command[33])
  {
    int type = pinType(PIN_YELLOW);
    int pcf_pin = type - GPIOPINS;

    if (type != -100 && type < GPIOPINS)
    {
      digitalWrite(PIN_YELLOW, HIGH);
      // DEBUG_MSG("IDS2 sendCommand GPIO PIN Yellow %s isHigh: %d \n",  PinToString(PIN_YELLOW).c_str(), HIGH);
    }
    else if (type >= GPIOPINS)
    {
      pcf020.write(pcf_pin, HIGH);
      // DEBUG_MSG("IDS2 sendCommand PCF PIN Yellow %s : %d isHigh: %d \n",  PinToString(PIN_YELLOW).c_str(), pcf_pin, HIGH);
    }
    // digitalWrite(PIN_YELLOW, HIGH);
    millis2wait(SIGNAL_START);

    if (type != -100 && type < GPIOPINS)
    {
      digitalWrite(PIN_YELLOW, LOW);
      // DEBUG_MSG("IDS2 GsendCommand PIO PIN Yellow %s isLow\n",  PinToString(PIN_YELLOW).c_str());
    }
    else if (type >= GPIOPINS)
    {
      pcf020.write(pcf_pin, LOW);
      // DEBUG_MSG("IDS2 sendCommand PCF PIN Yellow %s : %d isLow\n",  PinToString(PIN_YELLOW).c_str(), pcf_pin);
    }
    // digitalWrite(PIN_YELLOW, LOW);
    millis2wait(SIGNAL_WAIT);

    // PIN_YELLOW := Ausgabe an IDS2
    for (int i = 0; i < 33; i++)
    {
      if (type != -100 && type < GPIOPINS)
      {
        digitalWrite(PIN_YELLOW, HIGH);
        // DEBUG_MSG("IDS2 sendCommand GPIO PIN Yellow %s isHigh\n",  PinToString(PIN_YELLOW).c_str());
      }
      else if (type >= 9)
      {
        pcf020.write(pcf_pin, HIGH);
        // DEBUG_MSG("IDS2 sendCommand PCF PIN Yellow %s : %d isHigh\n",  PinToString(PIN_YELLOW).c_str(), pcf_pin);
      }
      // digitalWrite(PIN_YELLOW, HIGH);
      micros2wait(command[i]);
      if (type != -100 && type < GPIOPINS)
      {
        digitalWrite(PIN_YELLOW, LOW);
        // DEBUG_MSG("IDS2 sendCommand GPIO PIN Yellow %s isLow\n",  PinToString(PIN_YELLOW).c_str());
      }
      else if (type >= 9)
      {
        pcf020.write(pcf_pin, LOW);
        // DEBUG_MSG("IDS2 sendCommand PCF PIN Yellow %s : %d isLow\n",  PinToString(PIN_YELLOW).c_str(), pcf_pin);
      }

      // digitalWrite(PIN_YELLOW, LOW);
      micros2wait(SIGNAL_LOW);
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
  DynamicJsonDocument doc(386);
  doc["enabled"] = inductionCooker.isEnabled;
  doc["power"] = 0;
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
  doc["pl"] = inductionCooker.powerLevelOnError;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
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
    // goto SendMessage;
  }

  // SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetIndu()
{
  unsigned char pin_white = inductionCooker.PIN_WHITE;
  unsigned char pin_blue = inductionCooker.PIN_INTERRUPT;
  unsigned char pin_yellow = inductionCooker.PIN_YELLOW;
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
  server.send(200, "text/plain", "ok");
}
