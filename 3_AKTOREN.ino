class Actor
{
  unsigned long powerLast; // Zeitmessung für High oder Low
  int dutycycle_actor = 5000;
  unsigned char OFF;
  unsigned char ON;

public:
  unsigned char pin_actor = 9; // the number of the LED pin
  String argument_actor;
  String name_actor;
  unsigned char power_actor;
  unsigned char pwm = 100;
  bool isOn;
  bool isInverted = false;
  bool switchable;              // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true;      // Error state actor
  
  Actor(String pin, String argument, String aname, bool ainverted, bool aswitchable)
  {
    change(pin, argument, aname, ainverted, aswitchable);
  }

  void Update()
  {
    int type = pinType(pin_actor);
    int pcf_actor = type - GPIOPINS;

    // GPIO 0-16 Wemos PINs
    // GPIO 17-25 PCF PINs -> pin_actor to pcf_actor transform
    // ACT ON Update isPin 1 pin_actor 17 pcf_actor 0 pinType 9 power 100  -> D9 => P0
    // ACT ON Update isPin 1 pin_actor 18 pcf_actor 1 pinType 10 power 100 -> D10 => P1

    if (isPin(pin_actor))
    {
      if (isOn && power_actor > 0)
      {
        if (millis() > powerLast + dutycycle_actor)
        {
          powerLast = millis();
        }
        if (millis() > powerLast + (dutycycle_actor * power_actor / 100L))
        {
          if (type != -100 && type < GPIOPINS)
          {
            digitalWrite(pin_actor, OFF);
            // DEBUG_MSG("Actor GPIO PIN %s isOFF\n", PinToString(pin_actor).c_str());
          }
          else if (type >= GPIOPINS)
          {
            pcf020.write(pcf_actor, OFF);
            // DEBUG_MSG("Actor PCF PIN %d isOFF\n", pcf_actor);
          }
        }
        else
        {
          if (type != -100 && type < GPIOPINS)
          {
            digitalWrite(pin_actor, ON);
            // DEBUG_MSG("Actor GPIO PIN %s isON\n", PinToString(pin_actor).c_str());
          }
          else if (type >= GPIOPINS)
          {
            pcf020.write(pcf_actor, ON);
            // DEBUG_MSG("Actor PCF PIN %d isON\n", pcf_actor);
          }
        }
      }
      else
      {
        if (type != -100 && type < GPIOPINS)
        {
          digitalWrite(pin_actor, OFF);
          // DEBUG_MSG("Actor3 GPIO PIN %s isOFF\n", PinToString(pin_actor).c_str());
        }
        else if (type >= GPIOPINS)
        {
          pcf020.write(pcf_actor, OFF);
          // DEBUG_MSG("Actor3 PCF PIN %d isOFF\n", pcf_actor);
        }
      }
    }
  }

  void change(const String &pin, const String &argument, const String &aname, const bool &ainverted, const bool &aswitchable)
  {
    // Set PIN
    int type = pinType(pin_actor);
    int pcf_actor = type - GPIOPINS;

    if (isPin(pin_actor))
    {
      if (type != -100 && type < GPIOPINS)
        digitalWrite(pin_actor, HIGH);
      else if (type >= GPIOPINS) // PCF PIN
        pcf020.write(pcf_actor, HIGH);

      pins_used[pin_actor] = false;
      millis2wait(10);
    }

    pin_actor = StringToPin(pin);

    type = pinType(pin_actor);
    pcf_actor = type - GPIOPINS;

    if (isPin(pin_actor))
    {
      if (type != -100 && type < GPIOPINS)
      {
        pinMode(pin_actor, OUTPUT);
        digitalWrite(pin_actor, HIGH);
      }
      else if (type >= GPIOPINS)
      {
        pcf020.write(pcf_actor, HIGH);
      }
      pins_used[pin_actor] = true;
    }

    isOn = false;
    name_actor = aname;
    if (argument_actor != argument)
    {
      mqtt_unsubscribe();
      argument_actor = argument;
      mqtt_subscribe();
    }
    if (ainverted)
    {
      isInverted = true;
      ON = HIGH;
      OFF = LOW;
    }
    else
    {
      isInverted = false;
      ON = LOW;
      OFF = HIGH;
    }
    switchable = aswitchable;
    actor_state = true;
    isOnBeforeError = false;
  }

  void mqtt_subscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Act: Subscribing to %s\n", subscribemsg);
      pubsubClient.subscribe(subscribemsg);
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Act: Unsubscribing from %s\n", subscribemsg);
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  void handlemqtt(char *payload)
  {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      DEBUG_MSG("Act: handlemqtt deserialize Json error %s\n", error.c_str());
      return;
    }
    if (doc["state"] == "off")
    {
      isOn = false;
      power_actor = 0;
      return;
    }
    if (doc["state"] == "on")
    {
      isOn = true;
      int newPower = doc["power"];
      if (newPower > 100)
        newPower = 100; // Nicht > 100
      if (newPower < 0)
        newPower = 0; // Nicht < 0
      power_actor = newPower;
      return;
    }
  }
  void handlePWM(int newPower)
  {
    if (newPower > 100)
      newPower = 100; // Nicht > 100
    if (newPower < 0)
      newPower = 0; // Nicht < 0
    pwm = newPower;
    if (isOn)
      power_actor = pwm;
    return;
  }
};

// Initialisierung des Arrays max 10
Actor actors[numberOfActorsMax] = {
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false),
    Actor("", "", "", false, false)};

// Funktionen für Loop im Timer Objekt
void handleActors()
{
  for (int i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    yield();
  }
}

/* Funktionen für Web */
void handleRequestActors()
{
  int id = server.arg(0).toInt();
  DynamicJsonDocument doc(1024);
  if (id == -1) // fetch all sensors
  {
    JsonArray actorsArray = doc.to<JsonArray>();

    for (int i = 0; i < numberOfActors; i++)
    {
      JsonObject actorsObj = doc.createNestedObject();

      actorsObj["name"] = actors[i].name_actor;
      actorsObj["ison"] = actors[i].isOn;
      actorsObj["power"] = actors[i].power_actor;
      actorsObj["mqtt"] = actors[i].argument_actor;
      actorsObj["pin"] = PinToString(actors[i].pin_actor);
      actorsObj["sw"] = actors[i].switchable;
      actorsObj["state"] = actors[i].actor_state;
      actorsObj["pwm"] = actors[i].pwm;
      yield();
    }
  }
  else
  {
    doc["name"] = actors[id].name_actor;
    doc["mqtt"] = actors[id].argument_actor;
    doc["sw"] = actors[id].switchable;
    doc["inv"] = actors[id].isInverted;
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetActor()
{
  int id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfActors;
    numberOfActors += 1;
    if (numberOfActors >= numberOfActorsMax)
      return;
  }

  String ac_pin = PinToString(actors[id].pin_actor);
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  bool ac_isinverted = actors[id].isInverted;
  bool ac_switchable = actors[id].switchable;

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      ac_name = server.arg(i);
    }
    if (server.argName(i) == "pin")
    {
      ac_pin = server.arg(i);
    }
    if (server.argName(i) == "script")
    {
      ac_argument = server.arg(i);
    }
    if (server.argName(i) == "inv")
    {
      ac_isinverted = checkBool(server.arg(i));
    }
    if (server.argName(i) == "sw")
    {
      ac_switchable = checkBool(server.arg(i));
    }
    yield();
  }
  actors[id].change(ac_pin, ac_argument, ac_name, ac_isinverted, ac_switchable);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleSetPWM()
{
  int id = server.arg(0).toInt();

  if (id == -1)
    return;

  if (server.argName(1) == "pwm")
  {
     if (isValidDigit(server.arg(1)))
    {
      actors[id].handlePWM(server.arg(1).toInt());
    }
  }
  DEBUG_MSG("ACT: id: %d pwm: %d\n", id, actors[id].pwm);
  server.send(201, "text/plain", "created");
}

void handleDelActor()
{
  int id = server.arg(0).toInt();
  for (int i = id; i < numberOfActors; i++)
  {
    if (i == (numberOfActorsMax - 1)) // 5 - Array von 0 bis (numberOfActorsMax-1)
    {
      actors[i].change("", "", "", false, false);
    }
    else
    {
      actors[i].change(PinToString(actors[i + 1].pin_actor), actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].isInverted, actors[i + 1].switchable);
    }
    yield();
  }

  numberOfActors -= 1;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handlereqPins()
{
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(actors[id].pin_actor);
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
  server.send(200, "text/plain", message);
}

unsigned char StringToPin(String pinstring)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pin_names[i] == pinstring)
    {
      return pins[i];
    }
  }
  return -100;
}

String PinToString(unsigned char pinbyte)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      return pin_names[i];
    }
  }
  return "NaN";
}

bool isPin(unsigned char pinbyte)
{
  bool returnValue = false;
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      returnValue = true;
      break;
    }
  }
  return returnValue;
}

int pinType(unsigned char pinbyte)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      return i;
      break;
    }
  }
  return -100;
}

void actERR()
{
  for (int i = 0; i < numberOfActors; i++)
  {
    if (actors[i].switchable && actors[i].actor_state && actors[i].isOn)
    {
      actors[i].isOnBeforeError = actors[i].isOn;
      actors[i].isOn = false;
      actors[i].actor_state = false;
      actors[i].Update();
      DEBUG_MSG("ACT ERR Aktor: %s : %d isOnBeforeError: %d\n", actors[i].name_actor.c_str(), actors[i].actor_state, actors[i].isOnBeforeError);
    }
    yield();
  }
}