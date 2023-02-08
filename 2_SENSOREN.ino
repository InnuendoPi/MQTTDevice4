// 2827c59d0d0000b1
class TemperatureSensor
{
  int sens_err = 0;
  bool sens_sw = false;           // Events aktivieren
  bool sens_state = true;         // Fehlerstatus ensor
  bool sens_isConnected;          // ist der Sensor verbunden
  float sens_offset1 = 0.0;       // Offset - Temp kalibrieren
  float sens_offset2 = 0.0;       // Offset - Temp kalibrieren
  float sens_value = -127.0;      // Aktueller Wert
  String sens_name;               // Name für Anzeige auf Website
  char sens_mqtttopic[50];        // Für MQTT Kommunikation
  unsigned char sens_address[8];  // 1-Wire Adresse
  String sens_id;

public:
  TemperatureSensor(String new_address, String new_mqtttopic, String new_name, String new_id, float new_offset1, float new_offset2, bool new_sw)
  {
    change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw);
  }

  void Update()
  {
    DS18B20.requestTemperatures();                        // new conversion to get recent temperatures
    sens_isConnected = DS18B20.isConnected(sens_address); // attempt to determine if the device at the given address is connected to the bus
    sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;
    sensorsStatus = 0;
    sens_state = true;

    if (OneWire::crc8(sens_address, 7) != sens_address[7])
    {
      sensorsStatus = EM_CRCER;
      sens_state = false;
    }
    else if (sens_value == -127.00 || sens_value == 85.00)
    {
      if (sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor connected AND sensor address exists (not default FF)
        sensorsStatus = EM_DEVER;
        sens_state = false;
      }
      else if (!sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor with valid address not connected
        sensorsStatus = EM_UNPL;
        sens_state = false;
      }
      else // not connected and unvalid address
      {
        sensorsStatus = EM_SENER;
        sens_state = false;
      }
    } // sens_value -127 || +85
    else
    {
      sensorsStatus = EM_OK;
      sens_state = true;
    }
    sens_err = sensorsStatus;
    if (TickerPUBSUB.state() == RUNNING && TickerMQTT.state() != RUNNING)
      publishmqtt();
  } // void Update

  void change(const String &new_address, const String &new_mqtttopic, const String &new_name, const String &new_id, float new_offset1, float new_offset2, const bool &new_sw)
  {
    new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
    sens_id = new_id;
    sens_name = new_name;
    sens_offset1 = new_offset1;
    sens_offset2 = new_offset2;
    sens_sw = new_sw;
    if (new_address.length() == 16)
    {
      char address_char[20];
      new_address.toCharArray(address_char, new_address.length() + 1);
      char hexbyte[2];
      int octets[8];
      for (int d = 0; d < 16; d += 2)
      {
        // Assemble a digit pair into the hexbyte string
        hexbyte[0] = address_char[d];
        hexbyte[1] = address_char[d + 1];

        // Convert the hex pair to an integer
        sscanf(hexbyte, "%x", &octets[d / 2]);
        yield();
      }
      for (int i = 0; i < 8; i++)
      {
        sens_address[i] = octets[i];
        // Serial.printf("%x", sens_address[i]);
      }
    }
    DS18B20.setResolution(sens_address, RESOLUTION);
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
        // sensorsObj["Value"] = round((sens_value + sens_offset + 0.05) * 10) / 10.0;
        // sensorsObj["Value"] = round((calcOffset() + 0.05) * 10) / 10.0;
        sensorsObj["Value"] = getTotalValueFloat() * 10 / 10.0;
      }
      else
      {
        sensorsObj["Value"] = sens_value;
      }
      sensorsObj["Type"] = "1-wire";
      char jsonMessage[100];
      serializeJson(doc, jsonMessage);
      pubsubClient.publish(sens_mqtttopic, jsonMessage);
      // DEBUG_MSG("SEN publish %f\n", getTotalValueFloat());
    }
  }
  int getErr()
  {
    return sens_err;
  }
  bool getSw()
  {
    return sens_sw;
  }
  bool getState()
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
  String getName()
  {
    return sens_name;
  }
  String getTopic()
  {
    return sens_mqtttopic;
  }
  String getId()
  {
    return sens_id;
  }
  char buf[10];
  char *getValueString()
  {
    // char buf[5];
    dtostrf(sens_value, 2, 1, buf);
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
      dtostrf((round((calcOffset() - 0.05) * 10) / 10.0), 2, 1, buf);
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
      // float calc_value = m * sens_value + b;
      // DEBUG_MSG("sens_value: %.2f calc_value: %.2f sensoffset1: %.2f sensoffset2: %.2f m: %.5f b: %.5f\n", sens_value, calc_value, sens_offset1, sens_offset2, m, b);
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
};

// Initialisierung des Arrays -> max 6 Sensoren
TemperatureSensor sensors[numberOfSensorsMax] = {
    TemperatureSensor("", "", "", "", 0.0, 0.0, false),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false),
    TemperatureSensor("", "", "", "", 0.0, 0.0, false)};

// Funktion für Loop im Timer Objekt
void handleSensors()
{
  int max_status = 0;
  for (int i = 0; i < numberOfSensors; i++)
  {
    sensors[i].Update();

    // get max sensorstatus
    if (sensors[i].getSw() && max_status < sensors[i].getErr())
      max_status = sensors[i].getErr();

    yield();
  }
  sensorsStatus = max_status;
}

unsigned char searchSensors()
{
  unsigned char i;
  unsigned char n = 0;
  unsigned char addr[8];

  while (oneWire.search(addr))
  {

    if (OneWire::crc8(addr, 7) == addr[7])
    {
      for (i = 0; i < 8; i++)
      {
        addressesFound[n][i] = addr[i];
      }
      n += 1;
    }
    yield();
  }
  return n;
  oneWire.reset_search();
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
  int id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfSensors;
    numberOfSensors += 1;
    if (numberOfSensors >= numberOfSensorsMax)
      return;
  }

  String new_mqtttopic = sensors[id].getTopic();
  String new_name = sensors[id].getName();
  String new_address = sensors[id].getSens_adress_string();
  String new_id = sensors[id].getId();
  float new_offset1 = sensors[id].getOffset1();
  float new_offset2 = sensors[id].getOffset2();
  bool new_sw = sensors[id].getSw();

  for (int i = 0; i < server.args(); i++)
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
    yield();
  }
  sensors[id].change(new_address, new_mqtttopic, new_name, new_id, new_offset1, new_offset2, new_sw);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelSensor()
{
  int id = server.arg(0).toInt();
  for (int i = id; i < numberOfSensors; i++)
  {
    if (i == (numberOfSensorsMax - 1)) // 5 - Array von 0 bis (numberOfSensorsMax-1)
    {
      sensors[i].change("", "", "", "", 0.0, 0.0, false);
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].getTopic(), sensors[i + 1].getName(), sensors[i + 1].getId(), sensors[i + 1].getOffset1(), sensors[i + 1].getOffset2(), sensors[i + 1].getSw());

    yield();
  }
  numberOfSensors--;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handleRequestSensorAddresses()
{
  numberOfSensorsFound = searchSensors();
  int id = server.arg(0).toInt();
  String message;
  if (id != -1)
  {
    message += F("<option>");
    // message += SensorAddressToString(sensors[id].sens_address);
    message += sensors[id].getSens_adress_string();
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfSensorsFound; i++)
  {
    message += F("<option>");
    message += SensorAddressToString(addressesFound[i]);
    message += F("</option>");
    yield();
  }
  server.send(200, "text/html", message);
}

void handleRequestSensors()
{
  int id = server.arg(0).toInt();
  DynamicJsonDocument doc(1024);

  if (id == -1) // fetch all sensors
  {
    JsonArray sensorsArray = doc.to<JsonArray>();
    for (int i = 0; i < numberOfSensors; i++)
    {
      JsonObject sensorsObj = doc.createNestedObject();
      sensorsObj["name"] = sensors[i].getName();
      String str = sensors[i].getName();
      str.replace(" ", "%20"); // Erstze Leerzeichen für URL Charts
      sensorsObj["namehtml"] = str;
      sensorsObj["offset1"] = sensors[i].getOffset1();
      sensorsObj["offset2"] = sensors[i].getOffset2();
      sensorsObj["sw"] = sensors[i].getSw();
      sensorsObj["state"] = sensors[i].getState();
      if (sensors[i].getValue() != -127.0)
        sensorsObj["value"] = sensors[i].getTotalValueString();
      else
      {
        if (sensors[i].getErr() == 1)
          sensorsObj["value"] = "CRC";
        if (sensors[i].getErr() == 2)
          sensorsObj["value"] = "DER";
        if (sensors[i].getErr() == 3)
          sensorsObj["value"] = "UNP";
        else
          sensorsObj["value"] = "ERR";
      }
      sensorsObj["mqtt"] = sensors[i].getTopic();
      sensorsObj["cbpiid"] = sensors[i].getId();
      yield();
    }
  }
  else // get single sensor by id
  {
    doc["name"] = sensors[id].getName();
    doc["offset1"] = sensors[id].getOffset1();
    doc["offset2"] = sensors[id].getOffset2();
    doc["sw"] = sensors[id].getSw();
    doc["script"] = sensors[id].getTopic();
    doc["cbpiid"] = sensors[id].getId();
    // doc["value"] = sensors[id].getTotalValueString();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
