class TemperatureSensor
{
  int8_t sens_err = 0;
  bool sens_sw = false;          // Events aktivieren
  bool sens_state = true;        // Fehlerstatus ensor
  bool old_state = true;         // Fehlerstatus ensor
  bool sens_isConnected;         // ist der Sensor verbunden
  float sens_offset1 = 0.0;      // Offset - Temp kalibrieren
  float sens_offset2 = 0.0;      // Offset - Temp kalibrieren
  float sens_value = 0.0;        // Aktueller Wert
  float old_value = 0.0;         // Aktueller Wert
  String sens_name;              // Name für Anzeige auf Website
  char sens_mqtttopic[50];       // Für MQTT Kommunikation
  unsigned char sens_address[8]; // 1-Wire Adresse
  String sens_id;
  char buf[8];
  uint8_t sens_type = 0; // 0 := DS18B20, 1 := PT100, 2 := PT1000
  uint8_t sens_pin = 0;  // 0 := 2-cable, 1 := 3-cable, 2 := 4-cable
  int8_t sens_ptid = -1; // PT-Sensors ID

public:
  TemperatureSensor(String new_address, String new_mqtttopic, String new_name, String new_id, float new_offset1, float new_offset2, bool new_sw, uint8_t new_type, uint8_t new_pin)
  {
    change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw, new_type, new_pin);
  }

  void Update()
  {
    sens_value = DS18B20.getTempC(sens_address);
    sensorsStatus = 0;
    sens_state = true;
    if (simErr == 1) // simulate sensor error http://mqttdevice.local/setSenErr?err=1
    {
      sens_value = -127.0;
    }
    if (sens_type == 0)
    {
      if (OneWire::crc8(sens_address, 7) != sens_address[7])
      {
        if (old_state)
          DEBUG_ERROR("SEN", "sensor address CRC %s %s %s %s %s %s %s %s", sens_address[0], sens_address[1], sens_address[2], sens_address[3], sens_address[4], sens_address[5], sens_address[6], sens_address[7]);
        sensorsStatus = EM_CRCER;
        sens_state = false;
        old_state = sens_state;
      }
      else if (sens_value <= -127.0)
      {
        if (sens_isConnected && sens_address[0] != 0xFF) // Sensor connected AND sensor address exists (not default FF)
        {
          sensorsStatus = EM_DEVER;
          if (old_state)
            DEBUG_ERROR("SEN", "sensor address device error %s %s %s %s %s %s %s %s", sens_address[0], sens_address[1], sens_address[2], sens_address[3], sens_address[4], sens_address[5], sens_address[6], sens_address[7]);
          sens_state = false;
          old_state = sens_state;
        }
        else if (!sens_isConnected && sens_address[0] != 0xFF) // Sensor with valid address not connected
        {
          sensorsStatus = EM_UNPL;
          if (old_state)
            DEBUG_ERROR("SEN", "sensor address unplugged");
          sens_state = false;
          old_state = sens_state;
        }
        else // not connected and unvalid address
        {
          sensorsStatus = EM_SENER;
          if (old_state)
            DEBUG_ERROR("SEN", "sensor error");
          sens_state = false;
          old_state = sens_state;
        }
      } // sens_value -127
      else
      {
        sensorsStatus = EM_OK;
        sens_state = true;
      }
      sens_err = sensorsStatus;
    } // if senstyp DS18B20
    else if (sens_type == 1)
    {
      sensorsStatus = 0;
      sens_state = true;

      if (sens_ptid == 0)
        sens_value = pt_0.temperature(RNOMINAL100, RREF100);
      else if (sens_ptid == 1)
        sens_value = pt_1.temperature(RNOMINAL100, RREF100);
      else if (sens_ptid == 2)
        sens_value = pt_2.temperature(RNOMINAL100, RREF100);
#ifdef ESP32
      else if (sens_ptid == 3)
        sens_value = pt_3.temperature(RNOMINAL100, RREF100);
      else if (sens_ptid == 4)
        sens_value = pt_4.temperature(RNOMINAL100, RREF100);
      else if (sens_ptid == 5)
        sens_value = pt_5.temperature(RNOMINAL100, RREF100);
#endif

      if (sens_value > 120.0 || sens_value < -100.0) // außerhalb realistischer Messbereich
      {
        sensorsStatus = EM_SENER;
        if (old_state)
          DEBUG_ERROR("SEN", "PT100 sen error");
        sens_state = false;
        old_state = sens_state;
      }
      sens_err = sensorsStatus;
      // DEBUG_VERBOSE("SEN", "Update value: %.03f type: %d id: %d", sens_value, sens_type, sens_ptid);
    } // if senstyp pt100
    else if (sens_type == 2)
    {
      sensorsStatus = 0;
      sens_state = true;
      if (sens_ptid == 0)
        sens_value = pt_0.temperature(RNOMINAL1000, RREF1000);
      else if (sens_ptid == 1)
        sens_value = pt_1.temperature(RNOMINAL1000, RREF1000);
      else if (sens_ptid == 2)
        sens_value = pt_2.temperature(RNOMINAL1000, RREF1000);
#ifdef ESP32
      else if (sens_ptid == 3)
        sens_value = pt_3.temperature(RNOMINAL1000, RREF1000);
      else if (sens_ptid == 4)
        sens_value = pt_4.temperature(RNOMINAL1000, RREF1000);
      else if (sens_ptid == 5)
        sens_value = pt_5.temperature(RNOMINAL1000, RREF1000);
#endif
      if (sens_value > 120.0 || sens_value < -100.0) // außerhalb realistischer Messbereich
      {
        sensorsStatus = EM_SENER;
        if (old_state)
          DEBUG_ERROR("SEN", "PT1000 sen error");
        sens_state = false;
        old_state = sens_state;
      }
      sens_err = sensorsStatus;
    } // if senstyp pt1000

    DEBUG_VERBOSE("SEN", "%s\t%s\t%s\t%g\t%g/%g", sens_name.c_str(), sens_type == 0 ? "DS18B20" : "PT100x", sens_mqtttopic, sens_value, sens_offset1, sens_offset2);
    if (TickerPUBSUB.state() == RUNNING && TickerMQTT.state() != RUNNING)
      publishmqtt();
  } // void Update

  void change(const String &new_address, const String &new_mqtttopic, const String &new_name, const String &new_id, float new_offset1, float new_offset2, const bool &new_sw, const uint8_t &new_type, const uint8_t &new_pin)
  {
    new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
    // strlcpy(sens_mqtttopic, new_mqtttopic.c_str(), 49);
    // sprintf(sens_mqtttopic, "%s", new_mqtttopic.c_str());
    sens_id = new_id;
    sens_name = new_name;
    sens_offset1 = new_offset1;
    sens_offset2 = new_offset2;
    sens_sw = new_sw;
    sens_type = new_type;
    sens_pin = new_pin;

    if (sens_type == 0)
    {
      sens_ptid = -1;
      if (new_address.length() == 16)
      {
        char address_char[20];
        new_address.toCharArray(address_char, new_address.length() + 1);
        // strlcpy(address_char, new_address.c_str(), 19);
        // sprintf(address_char, "%s", new_address.c_str());
        char hexbyte[2];
        int32_t octets[8];
        for (uint8_t d = 0; d < 16; d += 2)
        {
          // Assemble a digit pair into the hexbyte string
          hexbyte[0] = address_char[d];
          hexbyte[1] = address_char[d + 1];

          // Convert the hex pair to an integer
          sscanf(hexbyte, "%x", &octets[d / 2]);
          yield();
        }
        for (uint8_t i = 0; i < 8; i++)
        {
          sens_address[i] = octets[i];
        }
      }
      if (senRes)
      {
        DS18B20.setResolution(sens_address, RESOLUTION_HIGH);
        DEBUG_INFO("SEN", "senor: %s set resolution high", sens_name.c_str());
      }
      else
      {
        DS18B20.setResolution(sens_address, RESOLUTION);
        DEBUG_INFO("SEN", "senor: %s set resolution normal", sens_name.c_str());
      }
    }
  }

  void publishmqtt()
  {
    if (pubsubClient.connected())
    {
      JsonDocument doc;
      JsonObject sensorsObj = doc["Sensor"].to<JsonObject>();
      sensorsObj["Name"] = sens_name;
      if (sensorsStatus == 0)
      {
        sensorsObj["Value"] = calcOffset();
      }
      else
      {
        sensorsObj["Value"] = sens_value;
      }
      if (sens_type == 1)
        sensorsObj["Type"] = "PT100";
      else if (sens_type == 2)
        sensorsObj["Type"] = "PT1000";
      else
        sensorsObj["Type"] = "1-wire";
      char jsonMessage[100];
      serializeJson(doc, jsonMessage);
      pubsubClient.publish(sens_mqtttopic, jsonMessage);
    }
  }
  int8_t getErr()
  {
    return sens_err;
  }
  bool getSensorSwitch()
  {
    return sens_sw;
  }
  bool getSensorState()
  {
    return sens_state;
  }
  float getOffset1()
  {
    return sens_offset1;
  }
  float getOffset2()
  {
    return sens_offset2;
  }
  float getValue()
  {
    return sens_value;
  }
  float oldValue()
  {
    return old_value;
  }
  void setOldValue()
  {
    old_value = sens_value;
  }
  String getSensorName()
  {
    return sens_name;
  }
  String getSensorTopic()
  {
    return sens_mqtttopic;
  }
  String getId()
  {
    return sens_id;
  }
  char *getValueString()
  {
    dtostrf(sens_value, -1, 1, buf);
    return buf;
  }

  float getTotalValueFloat()
  {
    return round((calcOffset() - 0.05) * 10) / 10.0;
  }

  char *getTotalValueString()
  {
    if (sens_value == -127.00)
      sprintf(buf, "%s", "-127.0");
    else
    {
      sprintf(buf, "%s", "0.0");
      dtostrf((round((calcOffset() - 0.04) * 10) / 10.0), -1, 1, buf);
    }
    return buf;
  }

  float calcOffset()
  {
    if (sens_value == -127.00)
      return sens_value;
    if (sens_offset1 == 0.0 && sens_offset2 == 0.0) // keine Kalibrierung
    {
      return sens_value;
    }
    else if ((sens_offset1 != 0.0 && sens_offset2 != 0.0) || (sens_offset1 == 0.0 && sens_offset2 != 0.0)) // 2-Punkte-Kalibrierung
    {
      float m = (TEMP_OFFSET2 - TEMP_OFFSET1) / ((TEMP_OFFSET2 + sens_offset2) - (TEMP_OFFSET1 + sens_offset1));
      float b = ((TEMP_OFFSET2 + sens_offset2) * TEMP_OFFSET1 - ((TEMP_OFFSET1 + sens_offset1) * TEMP_OFFSET2)) / ((TEMP_OFFSET2 + sens_offset2) - (TEMP_OFFSET1 + sens_offset1));
      return m * sens_value + b;
    }
    else if (sens_offset1 != 0.0 && sens_offset2 == 0.0) // 1-Punkt-Kalibrierung
    {
      return sens_value + sens_offset1;
    }
    return sens_value;
  }

  String getSens_adress_string()
  {
    return SensorAddressToString(sens_address);
  }
  uint8_t getSensType()
  {
    return sens_type;
  }
  void setSensType(uint8_t val)
  {
    sens_type = val;
  }
  uint8_t getSensPin()
  {
    return sens_pin;
  }
  void setSensPTid(int8_t val)
  {
    sens_ptid = val;
  }
};

// Initialisierung des Arrays -> max 6 Sensoren
#ifdef ESP32
TemperatureSensor sensors[NUMBEROFSENSORSMAX] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0)};
#elif ESP8266
// Initialisierung des Arrays -> max 3 Sensoren
TemperatureSensor sensors[NUMBEROFSENSORSMAX] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0)};
#endif

void handleSensors(bool checkSen)
{
  // checkSen true: init
  // checkSen false: only updates

  JsonDocument ssedoc;
  JsonArray sseArray = ssedoc.to<JsonArray>();
  int8_t max_status = 0;
  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    sensors[i].Update();

    // get max sensorstatus
    if (sensors[i].getSensorSwitch() && max_status < sensors[i].getErr())
      max_status = sensors[i].getErr();
    if (sensors[i].getValue() != sensors[i].oldValue())
    {
      sensors[i].setOldValue();
      checkSen = true;
    }
    JsonObject sseObj = ssedoc.add<JsonObject>();
    sseObj["name"] = sensors[i].getSensorName();
    sseObj["typ"] = sensors[i].getSensType();
    if (sensors[i].getErr() == EM_OK)
      sseObj["value"] = sensors[i].getTotalValueString();
    else if (sensors[i].getErr() == EM_CRCER)
      sseObj["value"] = "CRC";
    else if (sensors[i].getErr() == EM_DEVER)
      sseObj["value"] = "DER";
    else if (sensors[i].getErr() == EM_UNPL)
      sseObj["value"] = "UNP";
    else if (sensors[i].getErr() == EM_SENER)
      sseObj["value"] = "ERR";
    yield();
  }
  sensorsStatus = max_status;
  if (checkSen)
  {
    char response[measureJson(ssedoc) + 2];
    serializeJson(ssedoc, response, sizeof(response));
    SSEBroadcastJson(response, 0);
  }
}

uint8_t searchSensors()
{
  uint8_t n = 0;
  unsigned char addr[8];

  while (oneWire.search(addr))
  {

    if (OneWire::crc8(addr, 7) == addr[7])
    {
      for (uint8_t i = 0; i < 8; i++)
      {
        addressesFound[n][i] = addr[i];
      }
      n++;
    }
  }
  oneWire.reset_search();
  return n;
}

String SensorAddressToString(unsigned char addr[8])
{
  char charbuffer[50];
  sprintf(charbuffer, "%02x%02x%02x%02x%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
  return charbuffer;
}

void handleSetSensor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
    DEBUG_ERROR("SEN", "error deserializeJson %s", error.c_str());
    replyServerError("Server error deserialize sensor json");
    return;
  }
  int8_t id = doc["id"];
  if (id == -1) // new sensor
  {
    if (numberOfSensors >= NUMBEROFSENSORSMAX)
    {
      replyServerError("Server error max number sensors");
      return;
    }
    id = numberOfSensors;
    numberOfSensors++;
  }

  sensors[id].change(doc["address"], doc["script"], doc["name"], doc["cbpiid"], doc["offset1"], doc["offset2"], doc["sw"], doc["type"], doc["pin"]);
  replyOK();
  saveConfig();
  setupPT();
  handleSensors(true);
  TickerSen.setLastTime(millis()); // requiered for async mode
}

void handleDelSensor()
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg(0));
  if (error)
  {
    DEBUG_ERROR("SEN", "error deserializeJson %s", error.c_str());
    replyServerError("Server error sensor delete");
    return;
  }
  int8_t id = doc["id"];
  for (uint8_t i = id; i < numberOfSensors; i++)
  {
    if (i == (NUMBEROFSENSORSMAX - 1)) // 5 - Array von 0 bis (NUMBEROFSENSORSMAX-1)
    {
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].getSensorTopic(), sensors[i + 1].getSensorName(), sensors[i + 1].getId(), sensors[i + 1].getOffset1(), sensors[i + 1].getOffset2(), sensors[i + 1].getSensorSwitch(), sensors[i + 1].getSensType(), sensors[i + 1].getSensPin());

    yield();
  }
  if (numberOfSensors > 0)
    numberOfSensors--;
  else
    numberOfSensors = 0;
  saveConfig();
  replyOK();
  handleSensors(true);
  TickerSen.setLastTime(millis());
}

void handleRequestSensorAddresses()
{
  uint8_t numberOfSensorsFound = searchSensors();
  int8_t id = server.arg(0).toInt();
  String message;
  if (id != -1)
  {
    message += OPTIONSTART;
    message += sensors[id].getSens_adress_string();
    message += OPTIONDISABLED;
  }
  for (uint8_t i = 0; i < numberOfSensorsFound; i++)
  {
    message += OPTIONSTART;
    message += SensorAddressToString(addressesFound[i]);
    message += OPTIONEND;
    yield();
  }
  replyResponse(message.c_str());
}

void handleRequestSensors()
{
  int8_t id = server.arg(0).toInt();
  JsonDocument doc;
  if (id == -1) // fetch all sensors
  {
    JsonArray sensorsArray = doc.to<JsonArray>();
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      JsonObject sensorsObj = doc.add<JsonObject>();
      sensorsObj["name"] = sensors[i].getSensorName();
      sensorsObj["type"] = sensors[id].getSensType();
      // String str = sensors[i].getSensorName();
      // str.replace(" ", "%20"); // Erstze Leerzeichen für URL Charts
      // sensorsObj["namehtml"] = str;
      sensorsObj["offset1"] = sensors[i].getOffset1();
      sensorsObj["offset2"] = sensors[i].getOffset2();
      sensorsObj["sw"] = sensors[i].getSensorSwitch();
      sensorsObj["state"] = sensors[i].getSensorState();

      if (sensors[i].getErr() == EM_OK)
        sensorsObj["value"] = sensors[i].getTotalValueString();
      else if (sensors[i].getErr() == EM_CRCER)
        sensorsObj["value"] = "CRC";
      else if (sensors[i].getErr() == EM_DEVER)
        sensorsObj["value"] = "DER";
      else if (sensors[i].getErr() == EM_UNPL)
        sensorsObj["value"] = "UNP";
      else
        sensorsObj["value"] = "ERR";

      sensorsObj["script"] = sensors[i].getSensorTopic();
      sensorsObj["cbpiid"] = sensors[i].getId();
      sensorsObj["type"] = sensors[id].getSensType();
      sensorsObj["pin"] = sensors[id].getSensPin();
      yield();
    }
  }
  else // get single sensor by id
  {
    doc["name"] = sensors[id].getSensorName();
    doc["offset1"] = sensors[id].getOffset1();
    doc["offset2"] = sensors[id].getOffset2();
    doc["sw"] = sensors[id].getSensorSwitch();
    doc["script"] = sensors[id].getSensorTopic();
    doc["cbpiid"] = sensors[id].getId();
    doc["type"] = sensors[id].getSensType();
    doc["pin"] = sensors[id].getSensPin();
  }

  char response[measureJson(doc) + 2];
  serializeJson(doc, response, sizeof(response));
  replyResponse(response);
}

void setupPT()
{
  // startSPI false := Aus, true := starte Max31865
  // sens_type 0 := DS18B20, 1 := PT100, 2 := PT1000
  // sens_pin 0 := 2-Leiter, 1 := 3-Leiter, 2 := 4-Leiter
  pins_used[SPI_MOSI] = true; // MAX31865
  pins_used[SPI_MISO] = true; // MAX31865
  pins_used[SPI_CLK] = true;  // MAX31865
                              // pins_used[CS0] = true;      // MAX31865

  for (uint8_t i = 0; i < numberOfSensors; i++)
  {
    if (sensors[i].getSensType() > 0) // PT100x?
    {
      switch (sensors[i].getSensPin())
      {
      case 0: // 2-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(2);
        }
#ifdef ESP32
        else if (!activePT_3)
        {
          activePT_3 = pt_3.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(3);
        }
        else if (!activePT_4)
        {
          activePT_4 = pt_4.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(4);
        }
        else if (!activePT_5)
        {
          activePT_5 = pt_5.begin(MAX31865_2WIRE);
          sensors[i].setSensPTid(5);
        }
#endif
        break;
      case 1: // 3-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(2);
        }
#ifdef ESP32
        else if (!activePT_3)
        {
          activePT_3 = pt_3.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(3);
        }
        else if (!activePT_4)
        {
          activePT_4 = pt_4.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(4);
        }
        else if (!activePT_5)
        {
          activePT_5 = pt_5.begin(MAX31865_3WIRE);
          sensors[i].setSensPTid(5);
        }
#endif
        break;
      case 2: // 4-cable
        if (!activePT_0)
        {
          activePT_0 = pt_0.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(0);
        }
        else if (!activePT_1)
        {
          activePT_1 = pt_1.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(1);
        }
        else if (!activePT_2)
        {
          activePT_2 = pt_2.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(2);
        }
#ifdef ESP32
        else if (!activePT_3)
        {
          activePT_3 = pt_3.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(3);
        }
        else if (!activePT_4)
        {
          activePT_4 = pt_4.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(4);
        }
        else if (!activePT_5)
        {
          activePT_5 = pt_5.begin(MAX31865_4WIRE);
          sensors[i].setSensPTid(5);
        }
#endif
        break;
      }
    }
    pins_used[CS0] = activePT_0;
    pins_used[CS1] = activePT_1;
    pins_used[CS2] = activePT_2;
#ifdef ESP32
    pins_used[CS3] = activePT_3;
    pins_used[CS3] = activePT_4;
    pins_used[CS5] = activePT_5;
#endif
  }
}

void handleSetSenErr() // simulate sensor err
{
  simErr = server.arg(0).toInt();
  DEBUG_ERROR("SEN", "simulate %d", simErr);
  replyOK();
}