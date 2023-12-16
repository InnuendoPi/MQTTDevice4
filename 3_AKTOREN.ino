class Actor
{
  unsigned long powerLast; // Zeitmessung für High oder Low
  unsigned char OFF;
  unsigned char ON;
  int8_t pin_actor = -100; // the number of the LED pin
  String argument_actor;
  String name_actor;
  uint8_t power_actor;
  bool isInverted = false;
  bool switchable;              // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true;      // Error state actor
  bool isOn;
  bool old_isOn;

public:
  Actor(int8_t pin, String argument, String aname, bool ainverted, bool aswitchable)
  {
    change(pin, argument, aname, ainverted, aswitchable);
  }

  void Update()
  {
    if (pin_actor == -100)
      return;
    if (isPin(pin_actor))
    {
      if (isOn && power_actor > 0)
      {
        if (millis() > powerLast + DUTYCYLCE)
        {
          powerLast = millis();
        }
        if (millis() > powerLast + (DUTYCYLCE * power_actor / 100L))
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
#ifdef ESP32
      log_e("Act: Subscribing to %s", subscribemsg);
#endif
      pubsubClient.subscribe(subscribemsg);
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
#ifdef ESP32
      log_e("Act: Unsubscribing from %s", subscribemsg);
#endif
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  void handlemqtt(char *payload)
  {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
#ifdef ESP32
      log_e("Act: handlemqtt deserialize Json error %s", error.c_str());
#endif
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
  String getActorName()
  {
    return name_actor;
  }
  String getActorTopic()
  {
    return argument_actor;
  }
  bool getInverted()
  {
    return isInverted;
  }
  bool getActorSwitch()
  {
    return switchable;
  }
  bool getIsOn()
  {
    return isOn;
  }
  void setIsOn(bool val)
  {
    isOn = val;
  }
  bool getOldIsOn()
  {
    return old_isOn;
  }
  bool getActorState()
  {
    return actor_state;
  }
  void setActorState(bool val)
  {
    actor_state = val;
  }
  bool getIsOnBeforeError()
  {
    return isOnBeforeError;
  }
  void setIsOnBeforeError(bool val)
  {
    isOnBeforeError = val;
  }
  int8_t getPinActor()
  {
    return pin_actor;
  }
  uint8_t getActorPower()
  {
    return power_actor;
  }
  void setOldIsOn()
  {
    old_isOn = isOn;
  }
};

#ifdef ESP32
// Initialisierung des Arrays max 15
Actor actors[NUMBEROFACTORSMAX] = {
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false)};
#elif ESP8266
// Initialisierung des Arrays max 10
Actor actors[NUMBEROFACTORSMAX] = {
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false),
    Actor(-100, "", "", false, false)};
#endif
// Funktionen für Loop im Timer Objekt
void handleActors(bool checkAct)
{
  // checkAct true: init
  // checkAct false: only updates

  DynamicJsonDocument ssedoc(768);
  JsonArray sseArray = ssedoc.to<JsonArray>();
  for (uint8_t i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    if (actors[i].getOldIsOn() != actors[i].getIsOn())
    {
      actors[i].setOldIsOn();
      checkAct = true;
    }
    JsonObject sseObj = ssedoc.createNestedObject();
    sseObj["name"] = actors[i].getActorName();
    sseObj["ison"] = actors[i].getIsOn();
    sseObj["power"] = actors[i].getActorPower();
    sseObj["mqtt"] = actors[i].getActorTopic();
    sseObj["state"] = actors[i].getActorState();
    sseObj["pin"] = PinToString(actors[i].getPinActor());
    yield();
  }

  if (checkAct)
  {
    String jsonValue = "";
    serializeJson(ssedoc, jsonValue);
    // if (measureJson(ssedoc) > 5)
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

      actorsObj["name"] = actors[i].getActorName();
      actorsObj["ison"] = actors[i].getIsOn();
      actorsObj["power"] = actors[i].getActorPower();
      actorsObj["mqtt"] = actors[i].getActorTopic();
      actorsObj["pin"] = actors[i].getPinActor();
      actorsObj["sw"] = actors[i].getActorSwitch();
      actorsObj["state"] = actors[i].getActorState();
      yield();
    }
  }
  else
  {
    doc["name"] = actors[id].getActorName();
    doc["mqtt"] = actors[id].getActorTopic();
    doc["sw"] = actors[id].getActorSwitch();
    doc["inv"] = actors[id].getInverted();
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
    if (numberOfActors >= NUMBEROFACTORSMAX) // maximale Anzahl Aktoren erreicht?
    {
      server.send(204, FPSTR("text/plain"), "err");
      return;
    }
    id = numberOfActors;
    numberOfActors++;
  }

  int8_t ac_pin = actors[id].getPinActor();
  String ac_argument = actors[id].getActorTopic();
  String ac_name = actors[id].getActorName();
  bool ac_isinverted = actors[id].getInverted();
  bool ac_switchable = actors[id].getActorSwitch();

  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      ac_name = checkName(server.arg(i), 15, false);
    }
    if (server.argName(i) == "pin")
    {
      ac_pin = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "script")
    {
      ac_argument = checkName(server.arg(i), 20, true);
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
  actors[id].setIsOn(false);
  pins_used[actors[id].getPinActor()] = false;

  for (uint8_t i = id; i < numberOfActors; i++)
  {
    if (i == (NUMBEROFACTORSMAX - 1)) // 5 - Array von 0 bis (NUMBEROFACTORSMAX-1)
    {
      actors[i].change(-100, "", "", false, false);
    }
    else
    {
      actors[i].change(actors[i + 1].getPinActor(), actors[i + 1].getActorTopic(), actors[i + 1].getActorName(), actors[i + 1].getInverted(), actors[i + 1].getActorSwitch());
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
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(actors[id].getPinActor());
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += F("<option>");
      message += pin_names[i];
      message += F("</option>");
    }
    yield();
  }
  server.send_P(200, "text/plain", message.c_str());
}

int8_t StringToPin(String pinstring)
{
  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pin_names[i] == pinstring)
      return pins[i];
  }
  return -100;
}

String PinToString(int8_t pinbyte)
{
  for (int i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins[i] == pinbyte)
      return pin_names[i];
  }
  return "-100";
}

bool isPin(int8_t pinbyte)
{
  if (pinbyte == -100)
    return false;

  for (uint8_t i = 0; i < NUMBEROFPINS; i++)
  {
    if (pins[i] == pinbyte)
    {
      return true;
    }
  }
  return false;
}

void actERR()
{
  for (uint8_t i = 0; i < numberOfActors; i++)
  {
    if (actors[i].getActorSwitch() && actors[i].getActorState() && actors[i].getIsOn())
    {
      actors[i].setIsOnBeforeError(actors[i].getIsOn());
      actors[i].setIsOn(false);
      actors[i].setActorState(false);
      actors[i].Update();
#ifdef ESP32
      log_e("ACT MQTT event handling - actor: %s state: %d isOnBeforeError: %d", actors[i].getActorName().c_str(), actors[i].getActorState(), actors[i].getIsOnBeforeError());
#endif
    }
    yield();
  }
}