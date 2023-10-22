class Actor
{
  unsigned long powerLast; // Zeitmessung für High oder Low
  int16_t dutycycle_actor = 5000;
  unsigned char OFF;
  unsigned char ON;

public:
  int8_t pin_actor = 9; // the number of the LED pin
  String argument_actor;
  String name_actor;
  uint8_t power_actor;
  bool isOn;
  bool old_isOn;
  bool isInverted = false;
  bool switchable;              // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true;      // Error state actor

  Actor(int8_t pin, String argument, String aname, bool ainverted, bool aswitchable)
  {
    change(pin, argument, aname, ainverted, aswitchable);
  }

  void Update()
  {
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
          digitalWrite(pin_actor, OFF);
        }
        else
        {
          digitalWrite(pin_actor, ON);
        }
      }
      else
      {
        digitalWrite(pin_actor, OFF);
      }
    }
  }

  void change(const int8_t &pin, const String &argument, const String &aname, const bool &ainverted, const bool &aswitchable)
  {
    if (isPin(pin_actor))
    {
      digitalWrite(pin_actor, OFF);
      pins_used[pin_actor] = false;
      millis2wait(10);
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
    // pin_actor = StringToPin(pin);
    pin_actor = pin;

    if (isPin(pin_actor))
    {
      pinMode(pin_actor, OUTPUT);
      digitalWrite(pin_actor, OFF);
      pins_used[pin_actor] = true;
    }

    isOn = false;
    old_isOn = false;
    name_actor = aname;
    if (argument_actor != argument)
    {
      mqtt_unsubscribe();
      argument_actor = argument;
      mqtt_subscribe();
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
      int16_t newPower = doc["power"];
      power_actor = constrain(newPower, 0, 100);
      return;
    }
  }
};

// Initialisierung des Arrays max 10
Actor actors[numberOfActorsMax] = {
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false),
    Actor(9, "", "", false, false)};

// Funktionen für Loop im Timer Objekt
void handleActors(bool checkAct)
{
  // checkAct true: init
  // checkAct false: only updates

  DynamicJsonDocument ssedoc(768);
  JsonArray sseArray = ssedoc.to<JsonArray>();
  // bool checkAct = false;
  for (uint8_t i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    if (actors[i].old_isOn != actors[i].isOn)
    {
      actors[i].old_isOn = actors[i].isOn;
      checkAct = true;
    }

    JsonObject sseObj = ssedoc.createNestedObject();
    sseObj["name"] = actors[i].name_actor;
    sseObj["ison"] = actors[i].isOn;
    sseObj["power"] = actors[i].power_actor;
    sseObj["mqtt"] = actors[i].argument_actor;
    // sseObj["sw"] = actors[i].switchable;
    sseObj["state"] = actors[i].actor_state;
    sseObj["pin"] = PinToString(actors[i].pin_actor);
    yield();
  }

  if (checkAct)
  {
    String jsonValue = "";
    serializeJson(ssedoc, jsonValue);
    if (measureJson(ssedoc) > 5)
      SSEBroadcastJson(jsonValue.c_str(), 1);
  }
}

/* Funktionen für Web */
void handleRequestActors()
{
  int8_t id = server.arg(0).toInt();
  DynamicJsonDocument doc(1024);
  if (id == -1) // fetch all sensors
  {
    JsonArray actorsArray = doc.to<JsonArray>();

    for (uint8_t i = 0; i < numberOfActors; i++)
    {
      JsonObject actorsObj = doc.createNestedObject();

      actorsObj["name"] = actors[i].name_actor;
      actorsObj["ison"] = actors[i].isOn;
      actorsObj["power"] = actors[i].power_actor;
      actorsObj["mqtt"] = actors[i].argument_actor;
      actorsObj["pin"] = PinToString(actors[i].pin_actor);
      actorsObj["sw"] = actors[i].switchable;
      actorsObj["state"] = actors[i].actor_state;
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
  server.send(200, FPSTR("application/json"), response.c_str());
}

void handleSetActor()
{
  int8_t id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfActors;
    numberOfActors += 1;
    if (numberOfActors >= numberOfActorsMax)
      return;
  }

  int8_t ac_pin = actors[id].pin_actor;
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  bool ac_isinverted = actors[id].isInverted;
  bool ac_switchable = actors[id].switchable;

  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      ac_name = server.arg(i);
    }
    if (server.argName(i) == "pin")
    {
      ac_pin = StringToPin(server.arg(i));
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
  server.send(200, FPSTR("text/plain"), "ok");
  handleActors(true);
}

void handleDelActor()
{
  int8_t id = server.arg(0).toInt();
  if (id < 0 || id > numberOfActors)
  {
    server.send(200, FPSTR("text/plain"), "ok");
    return;
  }
  actors[id].isOn = false;
  pins_used[actors[id].pin_actor] = false;

  for (uint8_t i = id; i < numberOfActors; i++)
  {
    if (i == (numberOfActorsMax - 1)) // 5 - Array von 0 bis (numberOfActorsMax-1)
    {
      actors[i].change(9, "", "", false, false);
    }
    else
    {
      actors[i].change(actors[i + 1].pin_actor, actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].isInverted, actors[i + 1].switchable);
    }
    yield();
  }

  if (numberOfActors > 0)
    numberOfActors--;
  else
    numberOfActors = 0;
  saveConfig();
  server.send(200, FPSTR("text/plain"), "ok");
  handleActors(true);
}

void handlereqPins()
{
  const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};
  int8_t id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(actors[id].pin_actor);
    message += F("</option><option disabled>──────────</option>");
  }
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
  server.send(200, FPSTR("text/plain"), message.c_str());
}

unsigned char StringToPin(String pinstring)
{
  const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};
  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pin_names[i] == pinstring)
    {
      return pins[i];
    }
  }
  return 9;
}

String PinToString(unsigned char pinbyte)
{
  const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};
  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
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
  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins[i] == pinbyte)
    {
      returnValue = true;
      break;
    }
  }
  return returnValue;
}

void actERR()
{
  for (uint8_t i = 0; i < numberOfActors; i++)
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
