class TemperatureSensor
{
  int8_t sens_err = 0;
  bool sens_sw = false;          // Events aktivieren
  bool sens_state = true;        // Fehlerstatus ensor
  bool sens_isConnected;         // ist der Sensor verbunden
  float sens_offset1 = 0.0;      // Offset - Temp kalibrieren
  float sens_offset2 = 0.0;      // Offset - Temp kalibrieren
  float sens_value = -127.0;     // Aktueller Wert
  float old_value = 0.0;         // Aktueller Wert
  String sens_name;              // Name für Anzeige auf Website
  char sens_mqtttopic[50];       // Für MQTT Kommunikation
  unsigned char sens_address[8]; // 1-Wire Adresse
  String sens_id;
  char buf[8];
  uint8_t sens_type = 0; // 0 := DS18B20, 1 := PT100, 2 := PT1000
  uint8_t sens_pin = 0;  // 0 := 2-Leiter, 1 := 3-Leiter, 2 := 4-Leiter
  int8_t sens_ptid = -1;

public:
  TemperatureSensor(String new_address, String new_mqtttopic, String new_name, String new_id, float new_offset1, float new_offset2, bool new_sw, uint8_t new_type, uint8_t new_pin)
  {
    change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw, new_type, new_pin);
  }

  void Update()
  {
    if (sens_type == 0)
    {
      sens_value = DS18B20.getTempC(sens_address);
      // sensorsStatus = 0;
      // sens_state = true;

      if (OneWire::crc8(sens_address, 7) != sens_address[7])
      {
        sensorsStatus = EM_CRCER;
        sens_state = false;
      }
      else if (sens_value <= -127.0)
      {
        if (sens_isConnected && sens_address[0] != 0xFF) // Sensor connected AND sensor address exists (not default FF)
        {
          sensorsStatus = EM_DEVER;
          sens_state = false;
        }
        else if (!sens_isConnected && sens_address[0] != 0xFF) // Sensor with valid address not connected
        {
          sensorsStatus = EM_UNPL;
          sens_state = false;
        }
        else // not connected and unvalid address
        {
          sensorsStatus = EM_SENER;
          sens_state = false;
        }
      } // sens_value -127
      else
      {
        sensorsStatus = EM_OK;
        sens_state = true;
      }
      sens_err = sensorsStatus;
    }
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

      if (sens_value > 200.0)
      {
        sensorsStatus = EM_SENER;
        sens_state = false;
      }
      sens_err = sensorsStatus;
      // Serial.printf("Sen Update value: %.03f type: %d id: %d\n", sens_value, sens_type, sens_ptid);
    }
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

      if (sens_value > 200.0)
      {
        sensorsStatus = EM_SENER;
        sens_state = false;
      }
      sens_err = sensorsStatus;
      // Serial.printf("Sen Update value: %.03f type: %d id: %d\n", sens_value, sens_type, sens_ptid);
    }

    if (TickerPUBSUB.state() == RUNNING && TickerMQTT.state() != RUNNING)
      publishmqtt();
  } // void Update

  void change(const String &new_address, const String &new_mqtttopic, const String &new_name, const String &new_id, float new_offset1, float new_offset2, const bool &new_sw, const uint8_t &new_type, const uint8_t &new_pin)
  {
    new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
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
        char hexbyte[2];
        int32_t octets[8];
        for (uint8_t d = 0; d < 16; d += 2)
        {
          // Assemble a digit pair into the hexbyte string
          hexbyte[0] = address_char[d];
          hexbyte[1] = address_char[d + 1];

          // Convert the hex pair to an integer
          sscanf(hexbyte, "%x", &octets[d / 2]);
          // yield();
        }
        for (uint8_t i = 0; i < 8; i++)
        {
          sens_address[i] = octets[i];
        }
        if (senRes)
          DS18B20.setResolution(sens_address, RESOLUTION_HIGH);
        else
          DS18B20.setResolution(sens_address, RESOLUTION);
      }
    }
  }

  void publishmqtt()
  {
    if (pubsubClient.connected())
    {
      DynamicJsonDocument doc(256);
      JsonObject sensorsObj = doc.createNestedObject("Sensor");
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
    if (sens_value == -127.00 || sens_value > 200.0)
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
  // int8_t getSensPTid()
  // {
  //   return sens_ptid;
  // }
};

// Initialisierung des Arrays -> max 6 Sensoren
TemperatureSensor sensors[NUMBEROFSENSORSMAX] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    // TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    // TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    // TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false, 0, 0)};

// Funktion für Loop im Timer Objekt
void handleSensors(bool checkSen)
{
  // checkSen true: init
  // checkSen false: only updates

  DynamicJsonDocument ssedoc(512);
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
    JsonObject sseObj = ssedoc.createNestedObject();
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
    String jsonValue = "";
    serializeJson(ssedoc, jsonValue);
    SSEBroadcastJson(jsonValue.c_str(), 0);
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

// Sensor wird geändert
void handleSetSensor()
{
  int8_t id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfSensors;
    numberOfSensors++;
    if (numberOfSensors > NUMBEROFSENSORSMAX)
      return;
  }

  String new_mqtttopic = sensors[id].getSensorTopic();
  String new_name = sensors[id].getSensorName();
  String new_address = sensors[id].getSens_adress_string();
  String new_id = sensors[id].getId();
  float new_offset1 = sensors[id].getOffset1();
  float new_offset2 = sensors[id].getOffset2();
  bool new_sw = sensors[id].getSensorSwitch();
  uint8_t new_type = sensors[id].getSensType();
  uint8_t new_pin = sensors[id].getSensPin();

  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      new_name = server.arg(i);
    }
    if (server.argName(i) == "topic")
    {
      new_mqtttopic = server.arg(i);
    }
    if (server.argName(i) == "address")
    {
      new_address = server.arg(i);
    }
    if (server.argName(i) == "offset1")
    {
      new_offset1 = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "offset2")
    {
      new_offset2 = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "sw")
    {
      new_sw = checkBool(server.arg(i));
    }
    if (server.argName(i) == "cbpiid")
    {
      new_id = server.arg(i);
    }
    if (server.argName(i) == "type")
    {
      if (server.arg(i) == "DS18B20")
        new_type = 0;
      else if (server.arg(i) == "PT100")
        new_type = 1;
      else if (server.arg(i) == "PT1000")
        new_type = 2;
    }
    if (server.argName(i) == "pin")
    {
      if (server.arg(i) == "2-cable")
        new_pin = 0;
      else if (server.arg(i) == "3-cable")
        new_pin = 1;
      else if (server.arg(i) == "4-cable")
        new_pin = 2;
    }
    yield();
  }
  server.send(200, FPSTR("text/plain"), "ok");
  sensors[id].change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw, new_type, new_pin);
  saveConfig();
  setupPT();
  handleSensors(true);
}

void handleDelSensor()
{
  int8_t id = server.arg(0).toInt();
  for (uint8_t i = id; i < numberOfSensors; i++)
  {
    if (i == (NUMBEROFSENSORSMAX - 1)) // 5 - Array von 0 bis (NUMBEROFSENSORSMAX-1)
    {
      sensors[i].change("", "", "", "", 0.0, 0.0, false, 0, 0);
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].getSensorTopic(), sensors[i + 1].getSensorName(), sensors[i + 1].getId(), sensors[i + 1].getOffset1(), sensors[i + 1].getOffset2(), sensors[i + 1].getSensorSwitch(), sensors[i + 1].getSensType(), sensors[i + 1].getSensPin());
  }
  if (numberOfSensors > 0)
    numberOfSensors--;
  else
    numberOfSensors = 0;
  saveConfig();
  server.send(200, FPSTR("text/plain"), "ok");
  handleSensors(true);
}

void handleRequestSensorAddresses()
{
  uint8_t numberOfSensorsFound = searchSensors();
  int8_t id = server.arg(0).toInt();
  String message;
  if (id != -1)
  {
    message += F("<option>");
    message += sensors[id].getSens_adress_string();
    message += F("</option><option disabled>──────────</option>");
  }
  for (uint8_t i = 0; i < numberOfSensorsFound; i++)
  {
    message += F("<option>");
    message += SensorAddressToString(addressesFound[i]);
    message += F("</option>");
    yield();
  }
  server.send(200, FPSTR("text/html"), message.c_str());
}

void handleRequestSensors()
{
  int8_t id = server.arg(0).toInt();
  DynamicJsonDocument doc(1024);

  if (id == -1) // fetch all sensors
  {
    JsonArray sensorsArray = doc.to<JsonArray>();
    for (uint8_t i = 0; i < numberOfSensors; i++)
    {
      JsonObject sensorsObj = doc.createNestedObject();
      sensorsObj["name"] = sensors[i].getSensorName();
      sensorsObj["type"] = sensors[id].getSensType();
      String str = sensors[i].getSensorName();
      str.replace(" ", "%20"); // Erstze Leerzeichen für URL Charts
      sensorsObj["namehtml"] = str;
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

      sensorsObj["mqtt"] = sensors[i].getSensorTopic();
      sensorsObj["cbpiid"] = sensors[i].getId();
      doc["type"] = sensors[id].getSensType();
      doc["pin"] = sensors[id].getSensPin();
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

  String response;
  serializeJson(doc, response);
  server.send(200, FPSTR("application/json"), response.c_str());
}

void handleRequestSensorType()
{
  int8_t id = server.arg(0).toInt();
  String message = "";
  const String type_options[3] = {"DS18B20", "PT100", "PT1000"};
  if (id != -1)
  {
    message = F("<option>");
    message += type_options[sensors[id].getSensType()];
    message += F("</option><option disabled>──────────</option>");
  }
  for (uint8_t i = 0; i < 3; i++)
  {
    if (id == -1) // neuer sensor
    {
      message += F("<option>");
      message += type_options[i];
      message += F("</option>");
    }
    else if (i != sensors[id].getSensType()) // vorhandener sensor
    {
      message += F("<option>");
      message += type_options[i];
      message += F("</option>");
    }
  }

  server.send(200, FPSTR("text/html"), message.c_str());
}

void handlereqSenPins()
{
  int8_t id = server.arg(0).toInt();
  String message = "";
  const String pin_options[3] = {"2-cable", "3-cable", "4-cable"};
  if (id != -1)
  {
    message = F("<option>");
    message += pin_options[sensors[id].getSensPin()];
    message += F("</option><option disabled>──────────</option>");
  }
  for (uint8_t i = 0; i < 3; i++)
  {
    if (id == -1) // neuer sensor
    {
      message += F("<option>");
      message += pin_options[i];
      message += F("</option>");
    }
    else if (i != sensors[id].getSensPin()) // vorhandener sensor
    {
      message += F("<option>");
      message += pin_options[i];
      message += F("</option>");
    }
  }

  server.send(200, FPSTR("text/html"), message.c_str());
}

void setupPT()
{
  // startSPI false := Aus, true := starte Max31865
  // sens_type 0 := DS18B20, 1 := PT100, 2 := PT1000
  // sens_pin 0 := 2-Leiter, 1 := 3-Leiter, 2 := 4-Leiter
  // Serial.printf("MOSI: %d/%s MISO: %d/%s CLK: %d/%s\n", SPI_MOSI, PinToString(SPI_MOSI).c_str(), SPI_MISO, PinToString(SPI_MISO).c_str(), SPI_CLK, PinToString(SPI_CLK).c_str());
  // Serial.printf("MOSI: %d MISO: %d CLK: %d SS: %d\n", MOSI, MISO, SCK, SS);

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
        // Serial.printf("Pin 0: 2-cable Sensor: %d %s type: %d\n", i, sensors[i].getSensorName().c_str(), sensors[i].getSensType());
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
        break;
      case 1: // 3-cable
        // Serial.printf("Pin 1: 3-cable Sensor: %d %s type: %d\n", i, sensors[i].getSensorName().c_str(), sensors[i].getSensType());
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
        break;
      case 2: // 4-cable
        // Serial.printf("Pin 2: 4-cable Sensor: %d %s type: %d\n", i, sensors[i].getSensorName().c_str(), sensors[i].getSensType());
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
        break;
      }
    }
    // else
      // Serial.printf("Dallas Sensor: %d %s type: %d\n", i, sensors[i].getSensorName().c_str(), sensors[i].getSensType());

    pins_used[CS0] = activePT_0;
    pins_used[CS1] = activePT_1;
    pins_used[CS2] = activePT_2;

    // Serial.printf("active PT100x Sensors: 0:%d 1:%d 2:%d\n", activePT_0, activePT_1, activePT_2);
  }
}
