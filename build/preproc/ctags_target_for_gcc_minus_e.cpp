# 1 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino"
//    Erstellt:	2021
//    Author:	Innuendo

//    Sketch für ESP8266
//    Kommunikation via MQTT mit CraftBeerPi v3 und v4

//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Visulaisierung über Grafana
//    Unterstützung für OLED Display 126x64 I2C SH1106

# 15 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 16 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 17 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 18 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 19 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 20 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2

# 22 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 23 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 24 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 25 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 26 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 27 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 28 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 29 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 30 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 31 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 32 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 33 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
# 34 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2

extern "C"
{
# 38 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
}
# 48 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino"
// Version


// Definiere Pausen




// OneWire

OneWire oneWire(D3);
DallasTemperature DS18B20(&oneWire);

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient pubsubClient(espClient);
ESP8266HTTPUpdateServer httpUpdate;
MDNSResponder mdns;

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
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, // Aus
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0}, // P1
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // P2
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, // P3
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, // P4
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
unsigned char PWR_STEPS[] = {0, 20, 40, 60, 80, 100}; // Prozentuale Abstufung zwischen den Stufen

bool pins_used[17];
const unsigned char numberOfPins = 9;
const unsigned char pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

// Variablen
unsigned char numberOfSensors = 0; // Gesamtzahl der Sensoren

unsigned char addressesFound[6 /* Maximale Anzahl an Sensoren*/][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0; // Gesamtzahl der Aktoren

char mqtthost[16]; // MQTT Server
char mqtt_clientid[16]; // AP-Mode und Gerätename
bool alertState = false; // WebUpdate Status

// Zeitserver Einstellungen



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org" /* NTP Server*/, 60 * 60 /* Offset Winterzeit in Sekunden*/, 60 * 60 * 1000 /* Aktualisierung NTP in ms*/);

// EventManager
EventManager gEM; //  Eventmanager Objekt Queues

// System Fehler Events



// System Events



// Loop Events
# 137 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino"
// Event für Sensoren, Aktor und Induktion
# 148 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino"
// Event handling Status Variablen
bool StopOnWLANError = false; // Event handling für WLAN Fehler
bool StopOnMQTTError = false; // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
unsigned long wlanconnectlasttry; // Zeitstempel bei Fehler WLAN
bool mqtt_state = true; // Status MQTT
bool wlan_state = true; // Status WLAN

// Event handling Zeitintervall für Reconnects WLAN und MQTT



// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000; // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_error_wlan = 120000; // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000; // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerAct;
InnuTicker TickerInd;
InnuTicker TickerDisp;
InnuTicker TickerMQTT;
InnuTicker TickerWLAN;
InnuTicker TickerNTP;

// Update Intervalle für Ticker Objekte
int SEN_UPDATE = 5000; //  sensors update delay loop
int ACT_UPDATE = 5000; //  actors update delay loop
int IND_UPDATE = 5000; //  induction update delay loop
int DISP_UPDATE = 5000; //  NTP and display update

// Systemstart
bool startMDNS = true; // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[16] = "MQTTDevice";
bool shouldSaveConfig = false; // WiFiManager

unsigned long lastSenAct = 0; // Timestap actors on sensor error
unsigned long lastSenInd = 0; // Timestamp induction on sensor error

int sensorsStatus = 0;
int actorsStatus = 0;
int inductionStatus = 0;

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file

// OLED Display (optional)


bool useDisplay = false;



const int address[2] = {0x3C, 0x3D};

# 206 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
// Display mit SH1106 Chip:
//
// Wichtig: Für Displays mit SH1106 wird folgende lib benötigt
// https://github.com/kferrari/Adafruit_SH1106

# 212 "c:\\Arduino\\git\\MQTTDevice4\\MQTTDevice4.ino" 2
Adafruit_SH1106 display(2 /* D4*/);

// Display mit SSD1306 Chip
//
// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #define SCREEN_WIDTH 128       // OLED display width, in pixels
// #define SCREEN_HEIGHT 64       // OLED display height, in pixels
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);






const int PIN_BUZZER = D8; // Buzzer
bool startBuzzer = false; // Aktiviere Buzzer

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\0_SETUP.ino"
void setup()
{
  Serial.begin(115200);
// Debug Ausgaben prüfen




  Serial.println();
  Serial.println();
  // Setze Namen für das MQTTDevice
  snprintf(mqtt_clientid, 16, "ESP8266-%08X", ESP.getChipId());
  Serial.printf("*** SYSINFO: Starte MQTTDevice %s\n", mqtt_clientid);

  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter cstm_mqtthost("host", "MQTT Server IP (CBPi)", mqtthost, 16);
  WiFiManagerParameter p_hint("<small>*Sobald das MQTTDevice mit deinem WLAN verbunden ist, öffne im Browser http://mqttdevice </small>");
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.addParameter(&p_hint);
  wifiManager.autoConnect(mqtt_clientid);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // Lade Dateisystem
  if (LittleFS.begin())
  {
    Serial.printf("*** SYSINFO Starte Setup LittleFS Free Heap: %d\n", ESP.getFreeHeap());

    // Prüfe WebUpdate
    updateSys();

    // Erstelle Ticker Objekte
    setTicker();

    // Starte NTP
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();
    TickerNTP.start();

    if (shouldSaveConfig) // WiFiManager
    {
      strcpy(mqtthost, cstm_mqtthost.getValue());
      saveConfig();
    }

    if (LittleFS.exists("/config.txt")) // Lade Konfiguration
      loadConfig();
    else
      Serial.println("*** SYSINFO: Konfigurationsdatei config.txt nicht vorhanden. Setze Standardwerte ...");
  }
  else
    Serial.println("*** SYSINFO: Fehler - Dateisystem LittleFS konnte nicht eingebunden werden!");

  // Lege Event Queues an
  gEM.addListener(EventManager::kEventUser0, listenerSystem);
  gEM.addListener(EventManager::kEventUser1, listenerSensors);
  gEM.addListener(EventManager::kEventUser2, listenerActors);
  gEM.addListener(EventManager::kEventUser3, listenerInduction);

  // Starte Webserver
  setupServer();
  // Pinbelegung
  pins_used[D3] = true;
  if (useDisplay)
  {
    pins_used[D1] = true;
    pins_used[D2] = true;
  }
  // Starte Sensoren
  DS18B20.begin();

  // Starte mDNS
  if (startMDNS)
    cbpiEventSystem(26);
  else
  {
    Serial.printf("*** SYSINFO: ESP8266 IP Addresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
  }
  // Starte OLED Display
  dispStartScreen();
  if (startBuzzer)
  {
    pins_used[PIN_BUZZER] = true;
    pinMode(PIN_BUZZER, 0x01);
    digitalWrite(PIN_BUZZER, 0x0);
  }

  // Starte MQTT
  cbpiEventSystem(27); // MQTT Verbindung
  cbpiEventSystem(28); // MQTT Subscribe

  cbpiEventSystem(35); // webUpdate log

  // Verarbeite alle Events Setup
  gEM.processAllEvents();
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/setupActor", handleSetActor); // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor); // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors); // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);
  server.on("/reqIndu", handleRequestIndu); // Infos der Indu für WebConfig
  server.on("/setSensor", handleSetSensor); // Sensor ändern
  server.on("/setActor", handleSetActor); // Aktor ändern
  server.on("/setIndu", handleSetIndu); // Indu ändern
  server.on("/delSensor", handleDelSensor); // Sensor löschen
  server.on("/delActor", handleDelActor); // Aktor löschen
  server.on("/reboot", rebootDevice); // reboots the whole Device
  server.on("/reqDisplay", handleRequestDisplay);
  server.on("/reqDisp", handleRequestDisp); // Infos Display für WebConfig
  server.on("/setDisp", handleSetDisp); // Display ändern
  server.on("/reqMisc", handleRequestMisc); // Misc Infos für WebConfig
  server.on("/reqFirm", handleRequestFirm);
  server.on("/setMisc", handleSetMisc); // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate); // Firmware WebUpdate

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList); // Verzeichnisinhalt
  server.on("/edit", HTTP_GET, []() { // Lade Editor
    if (!handleFileRead("/edit.htm"))
    {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate); // Datei erstellen
  server.on("/edit", HTTP_DELETE, handleFileDelete); // Datei löschen
  server.on(
      "/edit", HTTP_POST, []()
      { server.send(200, "text/plain", ""); },
      handleFileUpload);

  server.onNotFound(handleWebRequests); // Sonstiges

  httpUpdate.setup(&server);
  server.begin();
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\1_LOOP.ino"
void loop()
{
  server.handleClient(); // Webserver handle
  cbpiEventSystem(20); // Überprüfe WLAN
  cbpiEventSystem(22); // Überprüfe MQTT
  if (startMDNS) // MDNS handle
    cbpiEventSystem(24);

  gEM.processAllEvents();

  if (numberOfSensors > 0) // Sensoren Ticker
    TickerSen.update();
  if (numberOfActors > 0) // Aktoren Ticker
    TickerAct.update();
  if (inductionStatus > 0) // Induktion Ticker
    TickerInd.update();
  if (useDisplay) // Display Ticker
    TickerDisp.update();
  // if (startDB && startVis)  // InfluxDB Ticker
  //   TickerInfluxDB.update();

  TickerNTP.update(); // NTP Ticker
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
class TemperatureSensor
{
  int sens_err = 0;
  bool sens_sw = false; // Events aktivieren
  bool sens_state = true; // Fehlerstatus ensor
  bool sens_isConnected; // ist der Sensor verbunden
  float sens_offset = 0.0; // Offset - Temp kalibrieren
  float sens_value = -127.0; // Aktueller Wert
  String sens_name; // Name für Anzeige auf Website
  unsigned char sens_address[8]; // 1-Wire Adresse
  char sens_mqtttopic[50]; // Für MQTT Kommunikation

public:
  // moved to private and get methods. change as set method

  String getSens_adress_string()
  {
    return SensorAddressToString(sens_address);
  }

  TemperatureSensor(String new_address, String new_mqtttopic, String new_name, float new_offset, bool new_sw)
  {
    change(new_address, new_mqtttopic, new_name, new_offset, new_sw);
  }

  void Update()
  {
    DS18B20.requestTemperatures(); // new conversion to get recent temperatures
    sens_isConnected = DS18B20.isConnected(sens_address); // attempt to determine if the device at the given address is connected to the bus
    sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;

    if (!sens_isConnected && sens_address[0] != 0xFF && sens_address[0] != 0x00) // double check on !sens_isConnected. Billig Tempfühler ist manchmal für 1-2 loops nicht connected. 0xFF default address. 0x00 virtual test device (adress 00 00 00 00 00)
    {
      millis2wait(750); // Wartezeit ca 750ms bevor Lesen vom Sensor wiederholt wird (Init Zeit)
      sens_isConnected = DS18B20.isConnected(sens_address); // hat der Sensor ene Adresse und ist am Bus verbunden?
      sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;
    }

    if (sens_value == 85.0)
    { // 85 Grad ist Standard Temp Default Reset. Wenn das Kabel zu lang ist, kommt als Fehler 85 Grad
      millis2wait(750); // Wartezeit 750ms vor einer erneuten Sensorabfrage
      DS18B20.requestTemperatures();
    }
    sensorsStatus = 0;
    sens_state = true;
    if (OneWire::crc8(sens_address, 7) != sens_address[7])
    {
      sensorsStatus = 1 /* Sensor CRC failed*/;
      sens_state = false;
    }
    else if (sens_value == -127.00 || sens_value == 85.00)
    {
      if (sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor connected AND sensor address exists (not default FF)
        sensorsStatus = 2 /* Sensor device error*/;
        sens_state = false;
      }
      else if (!sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor with valid address not connected
        sensorsStatus = 3 /* Sensor unplugged*/;
        sens_state = false;
      }
      else // not connected and unvalid address
      {
        sensorsStatus = 4 /* Sensor all errors*/;
        sens_state = false;
      }
    } // sens_value -127 || +85
    else
    {
      sensorsStatus = 0 /* Normal mode*/;
      sens_state = true;
    }
    sens_err = sensorsStatus;
    publishmqtt();
  } // void Update

  void change(const String &new_address, const String &new_mqtttopic, const String &new_name, float new_offset, const bool &new_sw)
  {
    new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
    sens_name = new_name;
    sens_offset = new_offset;
    sens_sw = new_sw;

    if (new_address.length() == 16)
    {
      char address_char[16];

      new_address.toCharArray(address_char, 17);

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
      }
    }
    DS18B20.setResolution(sens_address, 10);
  }

  void publishmqtt()
  {
    if (pubsubClient.connected())
    {
      StaticJsonDocument<256> doc;
      JsonObject sensorsObj = doc.createNestedObject("Sensor");
      sensorsObj["Name"] = sens_name;
      if (sensorsStatus == 0)
      {
        sensorsObj["Value"] = ((int)((sens_value + sens_offset)*10)) / 10.0;
      }
      else
      {
        sensorsObj["Value"] = sens_value;
      }
      sensorsObj["Type"] = "1-wire";
      char jsonMessage[100];
      serializeJson(doc, jsonMessage);
      pubsubClient.publish(sens_mqtttopic, jsonMessage);
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
  float getOffset()
  {
    return sens_offset;
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

  char buf[5];
  char *getValueString()
  {
    // char buf[5];
    dtostrf(sens_value, 2, 1, buf);
    return buf;
  }
};

// Initialisierung des Arrays -> max 6 Sensoren
TemperatureSensor sensors[6 /* Maximale Anzahl an Sensoren*/] = {
    TemperatureSensor("", "", "", 0.0, false),
    TemperatureSensor("", "", "", 0.0, false),
    TemperatureSensor("", "", "", 0.0, false),
    TemperatureSensor("", "", "", 0.0, false),
    TemperatureSensor("", "", "", 0.0, false),
    TemperatureSensor("", "", "", 0.0, false)};

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
    if (numberOfSensors >= 6 /* Maximale Anzahl an Sensoren*/)
      return;
  }

  String new_mqtttopic = sensors[id].getTopic();
  String new_name = sensors[id].getName();
  String new_address = sensors[id].getSens_adress_string();
  float new_offset = sensors[id].getOffset();
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
    if (server.argName(i) == "offset")
    {
      new_offset = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "sw")
    {
      new_sw = checkBool(server.arg(i));
    }
    yield();
  }

  sensors[id].change(new_address, new_mqtttopic, new_name, new_offset, new_sw);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelSensor()
{
  int id = server.arg(0).toInt();

  //  Alle einen nach vorne schieben
  for (int i = id; i < numberOfSensors; i++)
  {
    if (i == (6 /* Maximale Anzahl an Sensoren*/ - 1)) // 5 - Array von 0 bis (numberOfSensorsMax-1)
    {
      sensors[i].change("", "", "", 0.0, false);
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].getTopic(), sensors[i + 1].getName(), sensors[i + 1].getOffset(), sensors[i + 1].getSw());

    yield();
  }

  // den letzten löschen
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
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 306 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "2_SENSOREN.ino" "." "306" "." "295" "\", \"aSM\", @progbits, 1 #"))) = (
# 306 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              "<option>"
# 306 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              ); &__pstr__[0];}))
# 306 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              )));
    // message += SensorAddressToString(sensors[id].sens_address);
    message += sensors[id].getSens_adress_string();
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 309 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "2_SENSOREN.ino" "." "309" "." "296" "\", \"aSM\", @progbits, 1 #"))) = (
# 309 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              "</option><option disabled>──────────</option>"
# 309 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              ); &__pstr__[0];}))
# 309 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              )));
  }
  for (int i = 0; i < numberOfSensorsFound; i++)
  {
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 313 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "2_SENSOREN.ino" "." "313" "." "297" "\", \"aSM\", @progbits, 1 #"))) = (
# 313 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              "<option>"
# 313 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              ); &__pstr__[0];}))
# 313 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              )));
    message += SensorAddressToString(addressesFound[i]);
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 315 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "2_SENSOREN.ino" "." "315" "." "298" "\", \"aSM\", @progbits, 1 #"))) = (
# 315 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              "</option>"
# 315 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino" 3
              ); &__pstr__[0];}))
# 315 "c:\\Arduino\\git\\MQTTDevice4\\2_SENSOREN.ino"
              )));
    yield();
  }
  server.send(200, "text/html", message);
}

void handleRequestSensors()
{
  int id = server.arg(0).toInt();
  StaticJsonDocument<1024> doc;

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
      sensorsObj["offset"] = sensors[i].getOffset();
      sensorsObj["sw"] = sensors[i].getSw();
      sensorsObj["state"] = sensors[i].getState();
      if (sensors[i].getValue() != -127.0)
        sensorsObj["value"] = sensors[i].getValueString();
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
      yield();
    }
  }
  else // get single sensor by id
  {
    doc["name"] = sensors[id].getName();
    doc["offset"] = sensors[id].getOffset();
    doc["sw"] = sensors[id].getSw();
    doc["script"] = sensors[id].getTopic();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
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
  bool isOn;
  bool isInverted = false;
  bool switchable; // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true; // Error state actor
  bool setGrafana = false;

  // MQTT Publish
  char actor_mqtttopic[50]; // Für MQTT Kommunikation

  Actor(String pin, String argument, String aname, bool ainverted, bool aswitchable, bool agrafana)
  {
    change(pin, argument, aname, ainverted, aswitchable, agrafana);
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

  void change(const String &pin, const String &argument, const String &aname, const bool &ainverted, const bool &aswitchable, const bool &agrafana)
  {
    // Set PIN
    if (isPin(pin_actor))
    {
      digitalWrite(pin_actor, 0x1);
      pins_used[pin_actor] = false;
      millis2wait(10);
    }

    pin_actor = StringToPin(pin);
    if (isPin(pin_actor))
    {
      pinMode(pin_actor, 0x01);
      digitalWrite(pin_actor, 0x1);
      pins_used[pin_actor] = true;
    }

    isOn = false;
    name_actor = aname;
    if (argument_actor != argument)
    {
      mqtt_unsubscribe();
      argument_actor = argument;
      mqtt_subscribe();

      // MQTT Publish
      // argument.toCharArray(actor_mqtttopic, argument.length() + 1);
    }
    if (ainverted)
    {
      isInverted = true;
      ON = 0x1;
      OFF = 0x0;
    }
    else
    {
      isInverted = false;
      ON = 0x0;
      OFF = 0x1;
    }
    switchable = aswitchable;
    actor_state = true;
    isOnBeforeError = false;
    setGrafana = agrafana;
  }

  /* MQTT Publish

  void publishmqtt() {

    if (client.connected()) {

      StaticJsonDocument<256> doc;

      JsonObject actorObj = doc.createNestedObject("Actor");

      if (isOn) {

        doc["State"] = "on";

        doc["power"] = String(power_actor);

      }

      else

        doc["State"] = "off";



      char jsonMessage[100];

      serializeJson(doc, jsonMessage);

      char new_argument_actor[50];

      new_argument_actor.toCharArray(argument_actor, new_argument_actor.length() + 1);

      client.publish(new_argument_actor, jsonMessage);

    }

  }

  */
# 121 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
  void mqtt_subscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      ;
      pubsubClient.subscribe(subscribemsg);
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      ;
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  void handlemqtt(char *payload)
  {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      ;
      return;
    }
    String state = doc["state"];
    if (state == "off")
    {
      isOn = false;
      power_actor = 0;
      return;
    }
    if (state == "on")
    {
      int newpower = doc["power"];
      isOn = true;
      power_actor = min(100, newpower);
      power_actor = max(0, newpower);
      return;
    }
  }

  // bool getOn()
  // {
  //   return isOn;
  // }
  // bool getOnBefore()
  // {
  //   return isOnBeforeError;
  // }
  // bool getInverted()
  // {
  //   return isInverted;
  // }
  // bool getSw()
  // {
  //   return switchable;

  // }
  // bool getGrafana()
  // {
  //   return setGrafana;

  // }
  // bool getState()
  // {
  //   return actor_state;

  // }
};

// Initialisierung des Arrays max 8
Actor actors[8 /* Maximale Anzahl an Aktoren*/] = {
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false),
    Actor("", "", "", false, false, false)};

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
  StaticJsonDocument<1024> doc;
  if (id == -1) // fetch all sensors
  {
    JsonArray actorsArray = doc.to<JsonArray>();

    for (int i = 0; i < numberOfActors; i++)
    {
      JsonObject actorsObj = doc.createNestedObject();

      actorsObj["name"] = actors[i].name_actor;
      actorsObj["status"] = actors[i].isOn;
      actorsObj["power"] = actors[i].power_actor;
      actorsObj["mqtt"] = actors[i].argument_actor;
      actorsObj["pin"] = PinToString(actors[i].pin_actor);
      actorsObj["sw"] = actors[i].switchable;
      actorsObj["state"] = actors[i].actor_state;
      actorsObj["grafana"] = actors[i].setGrafana;
      yield();
    }
  }
  else
  {
    doc["name"] = actors[id].name_actor;
    doc["mqtt"] = actors[id].argument_actor;
    doc["sw"] = actors[id].switchable;
    doc["inv"] = actors[id].isInverted;
    doc["grafana"] = actors[id].setGrafana;
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
    if (numberOfActors >= 8 /* Maximale Anzahl an Aktoren*/)
      return;
  }

  String ac_pin = PinToString(actors[id].pin_actor);
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  bool ac_isinverted = actors[id].isInverted;
  bool ac_switchable = actors[id].switchable;
  bool ac_grafana = actors[id].setGrafana;

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
    if (server.argName(i) == "grafana")
    {
      ac_grafana = checkBool(server.arg(i));
    }
    yield();
  }
  actors[id].change(ac_pin, ac_argument, ac_name, ac_isinverted, ac_switchable, ac_grafana);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelActor()
{
  int id = server.arg(0).toInt();
  for (int i = id; i < numberOfActors; i++)
  {
    if (i == (8 /* Maximale Anzahl an Aktoren*/ - 1)) // 5 - Array von 0 bis (numberOfActorsMax-1)
    {
      actors[i].change("", "", "", false, false, false);
    }
    else
    {
      actors[i].change(PinToString(actors[i + 1].pin_actor), actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].isInverted, actors[i + 1].switchable, actors[i + 1].setGrafana);
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
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 337 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "3_AKTOREN.ino" "." "337" "." "299" "\", \"aSM\", @progbits, 1 #"))) = (
# 337 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
              "<option>"
# 337 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
              ); &__pstr__[0];}))
# 337 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
              )));
    message += PinToString(actors[id].pin_actor);
    message += ((reinterpret_cast<const __FlashStringHelper *>(
# 339 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
              (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "3_AKTOREN.ino" "." "339" "." "300" "\", \"aSM\", @progbits, 1 #"))) = (
# 339 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
              "</option><option disabled>──────────</option>"
# 339 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
              ); &__pstr__[0];}))
# 339 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
              )));
  }
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 345 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "3_AKTOREN.ino" "." "345" "." "301" "\", \"aSM\", @progbits, 1 #"))) = (
# 345 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
                "<option>"
# 345 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
                ); &__pstr__[0];}))
# 345 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
                )));
      message += pin_names[i];
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 347 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "3_AKTOREN.ino" "." "347" "." "302" "\", \"aSM\", @progbits, 1 #"))) = (
# 347 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
                "</option>"
# 347 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino" 3
                ); &__pstr__[0];}))
# 347 "c:\\Arduino\\git\\MQTTDevice4\\3_AKTOREN.ino"
                )));
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
  return 9;
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
      goto Ende;
    }
  }
Ende:
  return returnValue;
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
class induction
{
  unsigned long timeTurnedoff;

  long timeOutCommand = 5000; // TimeOut für Seriellen Befehl
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
  unsigned char PIN_WHITE = 14; // D5 RELAIS
  unsigned char PIN_YELLOW = 12; // D6 AUSGABE AN PLATTE
  unsigned char PIN_INTERRUPT = 13; // D7 EINGABE VON PLATTE
  int power = 0;
  int newPower = 0;
  unsigned char CMD_CUR = 0; // Aktueller Befehl
  boolean isRelayon = false; // Systemstatus: ist das Relais in der Platte an?
  boolean isInduon = false; // Systemstatus: ist Power > 0?
  boolean isPower = false;
  String mqtttopic = "";
  boolean isEnabled = false;
  long delayAfteroff = 120000;
  int powerLevelOnError = 100; // 100% schaltet das Event handling für Induktion aus
  int powerLevelBeforeError = 0; // in error event save last power state
  bool induction_state = true; // Error state induction
  bool setGrafana = false;

  // MQTT Publish
  // char induction_mqtttopic[50];      // Für MQTT Kommunikation

  induction()
  {
    setupCommands();
  }

  void change(unsigned char pinwhite, unsigned char pinyellow, unsigned char pinblue, String topic, long delayoff, bool is_enabled, int powerLevel, bool new_grafana)
  {
    if (isEnabled)
    {
      // aktuelle PINS deaktivieren
      if (isPin(PIN_WHITE))
      {
        digitalWrite(PIN_WHITE, 0x1);
        pins_used[PIN_WHITE] = false;
      }

      if (isPin(PIN_YELLOW))
      {
        digitalWrite(PIN_YELLOW, 0x1);
        pins_used[PIN_YELLOW] = false;
      }

      if (isPin(PIN_INTERRUPT))
      {
        detachInterrupt(PIN_INTERRUPT);
        pinMode(PIN_INTERRUPT, 0x01);

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
    setGrafana = new_grafana;

    // MQTT Publish
    //mqtttopic.toCharArray(induction_mqtttopic, mqtttopic.length() + 1);

    isEnabled = is_enabled;
    if (isEnabled)
    {
      // neue PINS aktiveren
      if (isPin(PIN_WHITE))
      {
        pinMode(PIN_WHITE, 0x01);
        digitalWrite(PIN_WHITE, 0x0);
        pins_used[PIN_WHITE] = true;
      }

      if (isPin(PIN_YELLOW))
      {
        pinMode(PIN_YELLOW, 0x01);
        digitalWrite(PIN_YELLOW, 0x0);
        pins_used[PIN_YELLOW] = true;
      }

      if (isPin(PIN_INTERRUPT))
      {
        attachInterrupt((((PIN_INTERRUPT) < 16)? (PIN_INTERRUPT) : -1), readInputWrap, 0x03);

        // pinMode(PIN_INTERRUPT, INPUT_PULLUP);
        pins_used[PIN_INTERRUPT] = true;
      }
      if (pubsubClient.connected())
      {
        mqtt_subscribe();
      }
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
        ;
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
      ;
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
# 166 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
  void handlemqtt(char *payload)
  {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      ;
      return;
    }
    String state = doc["state"];
    if (state == "off")
    {
      newPower = 0;
      return;
    }
    else
    {
      newPower = doc["power"];
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
      digitalWrite(PIN_WHITE, 0x1);
      return true;
    }

    if (isInduon == false && isRelayon == true)
    { /* Relais ausschalten */
      if (millis() > timeTurnedoff + delayAfteroff)
      {
        digitalWrite(PIN_WHITE, 0x0);
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

  void updatePower()
  {
    lastCommand = millis();

    if (power != newPower)
    { /* Neuer Befehl empfangen */

      if (newPower > 100)
      {
        newPower = 100; /* Nicht > 100 */
      }
      if (newPower < 0)
      {
        newPower = 0; /* Nicht < 0 */
      }
      power = newPower;

      timeTurnedoff = 0;
      isInduon = true;
      long difference = 0;

      if (power == 0)
      {
        CMD_CUR = 0;
        timeTurnedoff = millis();
        isInduon = false;
        difference = 0;
        goto setPowerLevel;
      }

      for (int i = 1; i < 7; i++)
      {
        if (power <= PWR_STEPS[i])
        {
          CMD_CUR = i;
          difference = PWR_STEPS[i] - power;
          goto setPowerLevel;
        }
      }

    setPowerLevel: /* Wie lange "HIGH" oder "LOW" */
      if (difference != 0)
      {
        powerLow = powerSampletime * difference / 20L;
        powerHigh = powerSampletime - powerLow;
      }
      else
      {
        powerHigh = powerSampletime;
        powerLow = 0;
      } // Test#1
    }
  }

  void sendCommand(int command[33])
  {
    digitalWrite(PIN_YELLOW, 0x1);
    millis2wait(SIGNAL_START);
    digitalWrite(PIN_YELLOW, 0x0);
    millis2wait(SIGNAL_WAIT);
    for (int i = 0; i < 33; i++)
    {
      digitalWrite(PIN_YELLOW, 0x1);
      delayMicroseconds(command[i]);
      digitalWrite(PIN_YELLOW, 0x0);
      delayMicroseconds(SIGNAL_LOW);
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

__attribute__((section("\".iram.text." "4_INDUKTION.ino" "." "384" "." "303" "\""))) __attribute__((deprecated("Use IRAM_ATTR in place of ICACHE_RAM_ATTR to move functions into IRAM"))) void readInputWrap()
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
  doc["grafana"] = inductionCooker.setGrafana;

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
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 447 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "4_INDUKTION.ino" "." "447" "." "304" "\", \"aSM\", @progbits, 1 #"))) = (
# 447 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                "<option>"
# 447 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                ); &__pstr__[0];}))
# 447 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                )));
      message += PinToString(pinswitched);
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 449 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "4_INDUKTION.ino" "." "449" "." "305" "\", \"aSM\", @progbits, 1 #"))) = (
# 449 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                "</option><option disabled>──────────</option>"
# 449 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                ); &__pstr__[0];}))
# 449 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                )));
    }

    for (int i = 0; i < numberOfPins; i++)
    {
      if (pins_used[pins[i]] == false)
      {
        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 456 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "4_INDUKTION.ino" "." "456" "." "306" "\", \"aSM\", @progbits, 1 #"))) = (
# 456 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                  "<option>"
# 456 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                  ); &__pstr__[0];}))
# 456 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                  )));
        message += pin_names[i];
        message += ((reinterpret_cast<const __FlashStringHelper *>(
# 458 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                  (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "4_INDUKTION.ino" "." "458" "." "307" "\", \"aSM\", @progbits, 1 #"))) = (
# 458 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                  "</option>"
# 458 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino" 3
                  ); &__pstr__[0];}))
# 458 "c:\\Arduino\\git\\MQTTDevice4\\4_INDUKTION.ino"
                  )));
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
  bool new_grafana = inductionCooker.setGrafana;

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
    if (server.argName(i) == "grafana")
    {
        new_grafana = checkBool(server.arg(i));
    }
    yield();
  }

  inductionCooker.change(pin_white, pin_yellow, pin_blue, topic, delayoff, is_enabled, pl, new_grafana);
  saveConfig();
  server.send(201, "text/plain", "created");
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
class oled
{
  unsigned long lastNTPupdate = 0;

public:
  bool dispEnabled = false;
  int address = 0x3C;

  bool senOK = true;
  bool actOK = true;
  bool indOK = true;
  bool wlanOK = true;
  bool mqttOK = false;
  int counter_sen = 0;
  int counter_act = 0;

  oled()
  {
  }

  void dispUpdate()
  {
    if (dispEnabled == 1)
    {
      showDispClear();
      showDispTime(timeClient.getFormattedTime());
      showDispIP(WiFi.localIP().toString());
      showDispWlan();
      showDispMqtt();
      showDispLines();
      showDispSen();
      showDispAct();
      showDispInd();
      showDispDisplay();
    }
  }

  void change(int dispAddress, bool is_enabled)
  {
    if (is_enabled == 1 && dispAddress != 0)
    {
      address = dispAddress;
      // Display mit SSD1306
      //display.begin(SSD1306_SWITCHCAPVCC, address);
      //display.ssd1306_command(SSD1306_DISPLAYON);

      // Display mit SH1106
      display.begin(0x2, address);
      display.SH1106_command(0xAF);
      display.clearDisplay();
      display.display();
      dispEnabled = is_enabled;
    }
    else
    {
      dispEnabled = is_enabled;
    }
  }
}

oledDisplay = oled();

void turnDisplay()
{
  if (!oledDisplay.dispEnabled)
  {
    if (oledDisplay.address != 0)
    {
      // Display mit SSD1306
      //display.ssd1306_command(SSD1306_DISPLAYOFF);

      // Display mit SH1106
      display.SH1106_command(0xAE);
      oledDisplay.dispEnabled = false;
      TickerDisp.stop();
    }
  }
  else
  {
    if (oledDisplay.address != 0)
    {
      // Display mit SSD1306
      //display.ssd1306_command(SSD1306_DISPLAYON);

      // Display mit SH1106
      display.SH1106_command(0xAF);
      oledDisplay.dispEnabled = true;
      TickerDisp.start();
    }
  }
}

void handleRequestDisplay()
{
  StaticJsonDocument<128> doc;
  doc["enabled"] = (int)oledDisplay.dispEnabled;
  doc["updisp"] = DISP_UPDATE / 1000;
  doc["displayOn"] = (int)oledDisplay.dispEnabled;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
void handleRequestDisp()
{
  String request = server.arg(0);
  String message;
  if (request == "address")
  {
    if (isAddress(oledDisplay.address))
    {
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 111 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "5_DISPLAY.ino" "." "111" "." "308" "\", \"aSM\", @progbits, 1 #"))) = (
# 111 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                "<option>"
# 111 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                ); &__pstr__[0];}))
# 111 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                )));
      message += String(decToHex(oledDisplay.address, 2));
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 113 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "5_DISPLAY.ino" "." "113" "." "309" "\", \"aSM\", @progbits, 1 #"))) = (
# 113 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                "</option><option disabled>──────────</option>"
# 113 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                ); &__pstr__[0];}))
# 113 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                )));
    }

    for (int i = 0; i < 2; i++)
    {
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 118 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "5_DISPLAY.ino" "." "118" "." "310" "\", \"aSM\", @progbits, 1 #"))) = (
# 118 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                "<option>"
# 118 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                ); &__pstr__[0];}))
# 118 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                )));
      message += String(decToHex(address[i], 2));
      message += ((reinterpret_cast<const __FlashStringHelper *>(
# 120 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "5_DISPLAY.ino" "." "120" "." "311" "\", \"aSM\", @progbits, 1 #"))) = (
# 120 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                "</option>"
# 120 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino" 3
                ); &__pstr__[0];}))
# 120 "c:\\Arduino\\git\\MQTTDevice4\\5_DISPLAY.ino"
                )));
    }
    goto SendMessage;
  }
SendMessage:
  server.send(200, "text/plain", message);
}

bool isAddress(int value)
{
  bool returnValue = false;
  for (int i = 0; i < 2; i++)
  {
    if (address[i] == value)
    {
      returnValue = true;
      goto Ende;
    }
  }
Ende:
  return returnValue;
}

void handleSetDisp()
{
  String dispAddress;
  int address;
  address = oledDisplay.address;
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "enabled")
    {
      oledDisplay.dispEnabled = checkBool(server.arg(i));
    }
    if (server.argName(i) == "address")
    {

      dispAddress = server.arg(i);
      dispAddress.remove(0, 2);
      char copy[4];
      dispAddress.toCharArray(copy, 4);
      address = strtol(copy, 0, 16);
    }
    if (server.argName(i) == "updisp")
    {
      int newdup = server.arg(i).toInt();
      if (newdup > 0)
      {
        DISP_UPDATE = newdup * 1000;
        TickerDisp.config(DISP_UPDATE, 0);
      }
    }
    yield();
  }
  oledDisplay.change(address, oledDisplay.dispEnabled);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void dispStartScreen() // Show Startscreen
{
  if (useDisplay)
  {
    if (oledDisplay.dispEnabled == 1 && oledDisplay.address != 0)
    {
      TickerDisp.start();
      showDispClear();
      showDispCbpi();
      showDispSTA();
      showDispDisplay();
    }
  }
}

void showDispClear() // Clear Display
{
  display.clearDisplay();
  display.display();
}

void showDispDisplay() // Show
{
  display.display();
}

void showDispVal(const String &value) // Display a String value
{
  display.print(value);
  display.display();
}

void showDispVal(const int value) // Display a Int value
{
  display.print(value);
  display.display();
}

void showDispWlan() // Show WLAN icon
{
  if (oledDisplay.wlanOK)
    display.drawBitmap(77, 3, wlan_logo, 20, 20, 1);
  else
  {
    showDispErr("WLAN ERROR");
    unsigned long val = 2 * wait_on_error_wlan - (millis() - wlanconnectlasttry);
    if (val > wait_on_error_wlan)
      return;
    showDispErr2(String(val / 1000));
  }
}
void showDispMqtt() // SHow MQTT icon
{
  if (oledDisplay.mqttOK)
    display.drawBitmap(102, 3, mqtt_logo, 20, 20, 1);
  else
  {
    showDispErr("MQTT ERROR");
    unsigned long val = 2 * wait_on_error_mqtt - (millis() - mqttconnectlasttry);
    if (val > wait_on_error_mqtt)
      return;
    showDispErr2(String(val / 1000));
  }
}
void showDispCbpi() // SHow CBPI icon
{
  display.clearDisplay();
  display.drawBitmap(41, 0, cbpi_logo, 50, 50, 1);
}

void showDispLines() // Draw lines in the bottom
{
  display.drawLine(0, 50, 128, 50, 1);
  display.drawLine(42, 50, 42, 64, 1);
  display.drawLine(84, 50, 84, 64, 1);
}
void showDispSen() // Show Sensor status on the left
{
  display.setTextSize(1);
  display.setCursor(3, 55);
  display.setTextColor(1);
  if (numberOfSensors == 0)
  {
    display.print("S off");
    return;
  }
  if (oledDisplay.counter_sen >= numberOfSensors)
    oledDisplay.counter_sen = 0;

  display.print("S");
  display.print(oledDisplay.counter_sen + 1);
  display.print(" ");
  if (sensors[oledDisplay.counter_sen].getErr() == 0)
    display.print((int)(sensors[oledDisplay.counter_sen].getOffset() + sensors[oledDisplay.counter_sen].getValue()));
  else
    display.print("Err");

  oledDisplay.counter_sen++;
}
void showDispAct() // Show actor status in the mid
{
  display.setCursor(45, 55);
  display.setTextColor(1);
  if (oledDisplay.counter_act >= numberOfActors)
    oledDisplay.counter_act = 0;

  display.print("A");
  display.print(oledDisplay.counter_act + 1);
  display.print(" ");
  if (!actors[oledDisplay.counter_act].isOn)
    display.print("off");
  else
    display.print(actors[oledDisplay.counter_act].power_actor);

  oledDisplay.counter_act++;
}
void showDispInd() // Show InductionCooker status on the right
{
  display.setTextSize(1);
  display.setCursor(87, 55);
  display.setTextColor(1);
  display.print("I ");
  if (inductionStatus == 1)
  {
    if (inductionCooker.isRelayon)
      display.print(inductionCooker.newPower);
    else
      display.print("off");
  }
  else if (inductionStatus == 0)
    display.print("off");
  else
    display.print("Err");
}

void showDispTime(const String &value) // Show time value in the upper left with fontsize 2
{
  // int l = value.length();
  display.setCursor(5, 5);
  display.setTextSize(2);
  display.setTextColor(1);
  display.print(value.substring(0, (value.length() - 3))); // substring w/o seconds
}

void showDispIP(const String &value) // Show IP address under time value with fontsize 1
{
  display.setCursor(5, 27);
  display.setTextSize(1);
  display.setTextColor(1);
  display.print("IP ");
  display.print(value);
}

void showDispErr(const String &value) // Show IP address under time value with fontsize 1
{
  display.setCursor(5, 39);
  display.setTextSize(1);
  display.setTextColor(1);
  display.print(value);
  display.display();
}

void showDispErr2(const String &value) // Show IP address under time value with fontsize 1
{
  display.setCursor(70, 39);
  display.setTextSize(1);
  display.setTextColor(1);
  display.print(" -");
  display.print(value);
  display.print("sec");
  display.display();
}

void showDispSet(const String &value) // Show current station mode
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(5, 30);
  display.print(value);
  display.display();
}

void showDispSet() // Show current station mode
{
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(1, 54);
  display.print("SET ");
  display.print(WiFi.localIP().toString());
  display.display();
}

void showDispSTA() // Show AP mode
{
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(8, 54);
  display.print("STA ");
  display.print(WiFi.localIP().toString());
  display.display();
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\7_WEB.ino"
void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  server.send(302, "text/plain", "");
}

void handleWebRequests()
{
  if (loadFromLittlefs(server.uri()))
  {
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

bool loadFromLittlefs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";

  if (!LittleFS.exists(path.c_str()))
  {
    return false;
  }
  File dataFile = LittleFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }
  dataFile.close();
  return true;
}

void mqttcallback(char *topic, unsigned char *payload, unsigned int length)
{
  ;
  // Serial.print("Web: Payload: ");
  // for (int i = 0; i < length; i++)
  // {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println(" ");
  char payload_msg[length];
  for (int i = 0; i < length; i++)
  {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic)
  {
    // if (inductionCooker.induction_state)
    //   {
      inductionCooker.handlemqtt(payload_msg);
      ;
    //   }
    // else
    //   DEBUG_MSG("%s\n", "*** Verwerfe MQTT wegen Status Induktion (Event handling)");
  }

  for (int i = 0; i < numberOfActors; i++)
  {
    if (actors[i].argument_actor == topic)
    {
      // if (actors[i].actor_state)
      //   {
        actors[i].handlemqtt(payload_msg);
      //   DEBUG_MSG("%s %s\n", "*** Handle MQTT Aktor", actors[i].name_actor);
      //   }
      // else
      //   DEBUG_MSG("%s %s\n", "*** Verwerfe MQTT zum Aktor", actors[i].name_actor);
      yield();
    }
  }
}

void handleRequestMisc()
{
  StaticJsonDocument<512> doc;
  doc["mqtthost"] = mqtthost;
  doc["mdns_name"] = nameMDNS;
  doc["mdns"] = startMDNS;
  doc["buzzer"] = startBuzzer;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["enable_wlan"] = StopOnWLANError;
  doc["delay_mqtt"] = wait_on_error_mqtt / 1000;
  doc["delay_wlan"] = wait_on_error_wlan / 1000;
  doc["del_sen_act"] = wait_on_Sensor_error_actor / 1000;
  doc["del_sen_ind"] = wait_on_Sensor_error_induction / 1000;
  doc["upsen"] = SEN_UPDATE / 1000;
  doc["upact"] = ACT_UPDATE / 1000;
  doc["upind"] = IND_UPDATE / 1000;
  doc["mqtt_state"] = oledDisplay.mqttOK; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["wlan_state"] = oledDisplay.wlanOK;
  doc["alertstate"] = alertState;
  if (alertState)
    alertState = false;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestFirm()
{
    String request = server.arg(0);
    String message;
    if (request == "firmware")
    {
        if (startMDNS)
        {
            message = nameMDNS;
            message += " V";
        }
        else
            message = "MQTTDevice4 V ";
        message += "2.60";
        goto SendMessage;
    }

SendMessage:
    server.send(200, "text/plain", message);
}

void handleSetMisc()
{
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "reset")
    {
      if (server.arg(i) == "true")
      {
        WiFi.disconnect();
        wifiManager.resetSettings();
        delay(2000);
        ESP.reset();
      }
    }
    if (server.argName(i) == "clear")
    {
      if (server.arg(i) == "true")
      {
        LittleFS.remove("/config.txt");
        delay(2000);
        ESP.reset();
      }
    }
    if (server.argName(i) == "MQTTHOST")
      server.arg(i).toCharArray(mqtthost, sizeof(mqtthost));

    if (server.argName(i) == "buzzer")
    {
      startBuzzer = checkBool(server.arg(i));
    }
    if (server.argName(i) == "mdns_name")
    {
      server.arg(i).toCharArray(nameMDNS, sizeof(nameMDNS));
      checkChars(nameMDNS);
    }
    if (server.argName(i) == "mdns")
    {
      startMDNS = checkBool(server.arg(i));
    }
    if (server.argName(i) == "enable_mqtt")
    {
      StopOnMQTTError = checkBool(server.arg(i));
    }
    if (server.argName(i) == "delay_mqtt")
      if (isValidInt(server.arg(i)))
      {
        wait_on_error_mqtt = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "enable_wlan")
    {
      StopOnWLANError = checkBool(server.arg(i));
    }
    if (server.argName(i) == "delay_wlan")
      if (isValidInt(server.arg(i)))
      {
        wait_on_error_wlan = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_act")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_ind")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_induction = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "upsen")
    {
      if (isValidInt(server.arg(i)))
      {
        int newsup = server.arg(i).toInt();
        if (newsup > 0)
          SEN_UPDATE = newsup * 1000;
      }
    }
    if (server.argName(i) == "upact")
    {
      if (isValidInt(server.arg(i)))
      {
        int newaup = server.arg(i).toInt();
        if (newaup > 0)
          ACT_UPDATE = newaup * 1000;
      }
    }
    if (server.argName(i) == "upind")
    {
      if (isValidInt(server.arg(i)))
      {
        int newiup = server.arg(i).toInt();
        if (newiup > 0)
          IND_UPDATE = newiup * 1000;
      }
    }
    yield();
  }
  saveConfig();
  server.send(201, "text/plain", "created");
}

// Some helper functions WebIf
void rebootDevice()
{
  server.send(205, "text/plain", "reboot");
  cbpiEventSystem(11);
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\8_CONFIGFILE.ino"
bool loadConfig()
{
  ;
  File configFile = LittleFS.open("/config.txt", "r");
  if (!configFile)
  {
    ;
    ;
    return false;
  }

  size_t size = configFile.size();
  if (size > 2048)
  {
    ;
    ;
    if (startBuzzer)
      sendAlarm(4);
    return false;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    ;
    if (startBuzzer)
      sendAlarm(4);
    return false;
  }

  JsonArray actorsArray = doc["actors"];
  numberOfActors = actorsArray.size();
  if (numberOfActors > 8 /* Maximale Anzahl an Aktoren*/)
    numberOfActors = 8 /* Maximale Anzahl an Aktoren*/;
  int i = 0;
  for (JsonObject actorObj : actorsArray)
  {
    if (i < numberOfActors)
    {
      String actorPin = actorObj["PIN"];
      String actorScript = actorObj["SCRIPT"];
      String actorName = actorObj["NAME"];
      bool actorInv = false;
      bool actorSwitch = false;
      bool actorGrafana = false;

      if (actorObj["INV"] || actorObj["INV"] == "1")
        actorInv = true;
      if (actorObj["SW"] || actorObj["SW"] == "1")
        actorSwitch = true;
      if (actorObj["GRAF"] || actorObj["GRAF"] == "1")
        actorGrafana = true;

      actors[i].change(actorPin, actorScript, actorName, actorInv, actorSwitch, actorGrafana);
      ;
      i++;
    }
  }

  if (numberOfActors == 0)
  {
    ;
  }
  ;

  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > 6 /* Maximale Anzahl an Sensoren*/)
    numberOfSensors = 6 /* Maximale Anzahl an Sensoren*/;
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      String sensorsAddress = sensorsObj["ADDRESS"];
      String sensorsScript = sensorsObj["SCRIPT"];
      String sensorsName = sensorsObj["NAME"];
      bool sensorsSwitch = false;
      float sensorsOffset = 0.0;
      if (sensorsObj["SW"] || sensorsObj["SW"] == "1")
        sensorsSwitch = true;
      if (sensorsObj.containsKey("OFFSET"))
        sensorsOffset = sensorsObj["OFFSET"];

      sensors[i].change(sensorsAddress, sensorsScript, sensorsName, sensorsOffset, sensorsSwitch);
      ;
      i++;
    }
    else
      sensors[i].change("", "", "", 0.0, false);
  }
  ;

  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  if (indObj.containsKey("ENABLED"))
  {
    inductionStatus = 1;
    bool indEnabled = true;
    bool indGrafana = false;
    String indPinWhite = indObj["PINWHITE"];
    String indPinYellow = indObj["PINYELLOW"];
    String indPinBlue = indObj["PINBLUE"];
    String indScript = indObj["TOPIC"];
    long indDelayOff = 120000 /* Standard Nachlaufzeit nach dem Ausschalten Induktionskochfeld*/; //default delay
    int indPowerLevel = 100;

    if (indObj["GRAF"] || indObj["GRAF"] == "1")
      indGrafana = true;
    if (indObj.containsKey("PL"))
      indPowerLevel = indObj["PL"];
    if (indObj.containsKey("DELAY"))
      indDelayOff = indObj["DELAY"];

    inductionCooker.change(StringToPin(indPinWhite), StringToPin(indPinYellow), StringToPin(indPinBlue), indScript, indDelayOff, indEnabled, indPowerLevel, indGrafana);
    ;
  }
  else
  {
    inductionStatus = 0;
    ;
  }
  ;
  JsonArray displayArray = doc["display"];
  JsonObject displayObj = displayArray[0];
  useDisplay = false;
  if (displayObj["ENABLED"] || displayObj["ENABLED"] == "1")
    useDisplay = true;

  if (useDisplay)
  {
    String dispAddress = displayObj["ADDRESS"];
    dispAddress.remove(0, 2);
    char copy[4];
    dispAddress.toCharArray(copy, 4);
    int address = strtol(copy, 0, 16);
    if (displayObj.containsKey("updisp"))
      DISP_UPDATE = displayObj["updisp"];

    oledDisplay.dispEnabled = true;
    oledDisplay.change(address, oledDisplay.dispEnabled);
    ;
    TickerDisp.config(DISP_UPDATE, 0);
    TickerDisp.start();
  }
  else
  {
    oledDisplay.dispEnabled = false;
    ;
    TickerDisp.stop();
  }
  ;

  // Misc Settings
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  if (miscObj.containsKey("del_sen_act"))
    wait_on_Sensor_error_actor = miscObj["del_sen_act"];
  if (miscObj.containsKey("del_sen_ind"))
    wait_on_Sensor_error_induction = miscObj["del_sen_ind"];
  ;
  ;

  StopOnMQTTError = false;
  if (miscObj["enable_mqtt"] || miscObj["enable_mqtt"] == "1")
    StopOnMQTTError = true;
  if (miscObj.containsKey("delay_mqtt"))
    wait_on_error_mqtt = miscObj["delay_mqtt"];
  ;

  StopOnWLANError = false;
  if (miscObj["enable_wlan"] || miscObj["enable_wlan"] == "1")
    StopOnWLANError = true;
  if (miscObj.containsKey("delay_wlan"))
    wait_on_error_wlan = miscObj["delay_wlan"];
  ;

  startBuzzer = false;
  if (miscObj["buzzer"] || miscObj["buzzer"] == "1")
    startBuzzer = true;
  ;

  if (miscObj.containsKey("mdns_name"))
    strlcpy(nameMDNS, miscObj["mdns_name"], sizeof(nameMDNS));
  startMDNS = false;
  if (miscObj["mdns"] || miscObj["mdns"] == "1")
    startMDNS = true;
  ;

  if (miscObj.containsKey("upsen"))
    SEN_UPDATE = miscObj["upsen"];
  if (miscObj.containsKey("upact"))
    ACT_UPDATE = miscObj["upact"];
  if (miscObj.containsKey("upind"))
    IND_UPDATE = miscObj["upind"];

  TickerSen.config(SEN_UPDATE, 0);
  TickerAct.config(ACT_UPDATE, 0);
  TickerInd.config(IND_UPDATE, 0);

  if (numberOfSensors > 0)
    TickerSen.start();
  if (numberOfActors > 0)
    TickerAct.start();
  if (inductionCooker.isEnabled)
    TickerInd.start();

  ;
  ;
  ;

  if (miscObj.containsKey("MQTTHOST"))
  {
    strlcpy(mqtthost, miscObj["MQTTHOST"], sizeof(mqtthost));
    ;
  }
  else
  {
    ;
  }
  ;
  configFile.close();
  ;
  size_t len = measureJson(doc);
  ;
  int memoryUsed = doc.memoryUsage();
  ;

  if (startBuzzer)
    sendAlarm(1);

  return true;
}

void saveConfigCallback()
{

  if (LittleFS.begin())
  {
    saveConfig();
    shouldSaveConfig = true;
  }
  else
  {
    Serial.println("*** SYSINFO: WiFiManager failed to save MQTT broker IP");
  }
}

bool saveConfig()
{
  ;
  DynamicJsonDocument doc(2500);

  // Write Actors
  JsonArray actorsArray = doc.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = actorsArray.createNestedObject();
    actorsObj["PIN"] = PinToString(actors[i].pin_actor);
    actorsObj["NAME"] = actors[i].name_actor;
    actorsObj["SCRIPT"] = actors[i].argument_actor;
    actorsObj["INV"] = (int)actors[i].isInverted;
    actorsObj["SW"] = (int)actors[i].switchable;
    actorsObj["GRAF"] = (int)actors[i].setGrafana;

    ;
  }
  if (numberOfActors == 0)
  {
    ;
  }
  ;

  // Write Sensors
  JsonArray sensorsArray = doc.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = sensorsArray.createNestedObject();
    sensorsObj["ADDRESS"] = sensors[i].getSens_adress_string();
    sensorsObj["NAME"] = sensors[i].getName();
    sensorsObj["OFFSET"] = sensors[i].getOffset();
    sensorsObj["SCRIPT"] = sensors[i].getTopic();
    sensorsObj["SW"] = (int)sensors[i].getSw();
    ;
  }

  ;

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.isEnabled)
  {
    inductionStatus = 1;
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
    indObj["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
    indObj["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
    indObj["TOPIC"] = inductionCooker.mqtttopic;
    indObj["DELAY"] = inductionCooker.delayAfteroff;
    indObj["ENABLED"] = (int)inductionCooker.isEnabled;
    indObj["PL"] = inductionCooker.powerLevelOnError;
    indObj["GRAF"] = (int)inductionCooker.setGrafana;
    ;
  }
  else
  {
    inductionStatus = 0;
    ;
  }
  ;

  // Write Display
  JsonArray displayArray = doc.createNestedArray("display");
  if (oledDisplay.dispEnabled)
  {
    JsonObject displayObj = displayArray.createNestedObject();
    displayObj["ENABLED"] = 1;
    displayObj["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
    displayObj["updisp"] = DISP_UPDATE;

    if (oledDisplay.address == 0x3C || oledDisplay.address == 0x3D)
    {
      // Display mit SDD1306 Chip:
      // display.ssd1306_command(SSD1306_DISPLAYON);

      // Display mit SH1106 Chip:
      display.SH1106_command(0xAF);
      cbpiEventSystem(30);
    }
    else
    {
      displayObj["ENABLED"] = 0;
      oledDisplay.dispEnabled = false;
      useDisplay = false;
    }
    ;
    TickerDisp.config(DISP_UPDATE, 0);
    TickerDisp.start();
  }
  else
  {
    // Display mit SSD1306 Chip:
    // display.ssd1306_command(SSD1306_DISPLAYOFF);

    // Display mit SH1106 Chip:
    display.SH1106_command(0xAE);
    ;
    TickerDisp.stop();
  }
  ;

  // Write Misc Stuff
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  ;
  ;

  miscObj["delay_mqtt"] = wait_on_error_mqtt;
  miscObj["enable_mqtt"] = (int)StopOnMQTTError;
  ;

  miscObj["delay_wlan"] = wait_on_error_wlan;
  miscObj["enable_wlan"] = (int)StopOnWLANError;
  ;

  miscObj["buzzer"] = (int)startBuzzer;
  miscObj["mdns_name"] = nameMDNS;
  miscObj["mdns"] = (int)startMDNS;
  miscObj["MQTTHOST"] = mqtthost;
  miscObj["upsen"] = SEN_UPDATE;
  miscObj["upact"] = ACT_UPDATE;
  miscObj["upind"] = IND_UPDATE;

  TickerSen.config(SEN_UPDATE, 0);
  TickerAct.config(ACT_UPDATE, 0);
  TickerInd.config(IND_UPDATE, 0);

  if (numberOfSensors > 0)
    TickerSen.start();
  else
    TickerSen.stop();
  if (numberOfActors > 0)
    TickerAct.start();
  else
    TickerAct.stop();
  if (inductionCooker.isEnabled)
    TickerInd.start();

  ;
  ;
  ;
  ;

  size_t len = measureJson(doc);
  int memoryUsed = doc.memoryUsage();

  if (len > 2048 || memoryUsed > 2500)
  {
    ;
    ;
    ;
    ;
    if (startBuzzer)
      sendAlarm(4);
    return false;
  }

  File configFile = LittleFS.open("/config.txt", "w");
  if (!configFile)
  {
    ;
    ;
    if (startBuzzer)
      sendAlarm(4);
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();
  ;
  String Network = WiFi.SSID();
  ;
  ;
  ;
  if (startBuzzer)
    sendAlarm(1);
  return true;
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\990_tickerCallback.ino"
void tickerSenCallback() // Timer Objekt Sensoren
{
  cbpiEventSensors(sensorsStatus);
}
void tickerActCallback() // Timer Objekt Aktoren
{
  cbpiEventActors(actorsStatus);
}
void tickerIndCallback() // Timer Objekt Induktion
{
  cbpiEventInduction(inductionStatus);
}
void tickerDispCallback() // Timer Objekt Display
{
  cbpiEventSystem(30);
}

void tickerMQTTCallback() // Ticker helper function calling Event MQTT Error
{
  if (TickerMQTT.counter() == 1)
  {
    switch (pubsubClient.state())
    {
    case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
      ;
      break;
    case -3: // MQTT_CONNECTION_LOST - the network connection was broken
      ;
      break;
    case -2: // MQTT_CONNECT_FAILED - the network connection failed
      ;
      break;
    case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
      ;
      break;
    case 0: // MQTT_CONNECTED - the client is connected
      pubsubClient.loop();
      break;
    case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
      ;
      break;
    case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
      ;
      break;
    case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
      ;
      break;
    case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
      ;
      break;
    case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
      ;
      break;
    default:
      break;
    }
  }
  cbpiEventSystem(2);
}

void tickerWLANCallback() // Ticker helper function calling Event WLAN Error
{
  ;
  if (TickerWLAN.counter() == 1)
  {
    switch (WiFi.status())
    {
    case 0: // WL_IDLE_STATUS
      ;
      break;
    case 1: // WL_NO_SSID_AVAIL
      ;
      break;
    case 2: // WL_SCAN_COMPLETED
      ;
      break;
    case 3: // WL_CONNECTED
      ;
      break;
    case 4: // WL_CONNECT_FAILED
      ;
      break;
    case 5: // WL_CONNECTION_LOST
      ;
      break;
    case 6: // WL_DISCONNECTED
      ;
      break;
    case 255: // WL_NO_SHIELD
      ;
      break;
    default:
      break;
    }
  }
  cbpiEventSystem(1);
}

void tickerNTPCallback() // Ticker helper function calling Event WLAN Error
{
  timeClient.update();
  Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
void upIn()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();

    HTTPClient https;

    if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/data/index.html"))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            // Serial.printf("*** SYSINFO: [HTTPS] GET index.html Antwort: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = https.getSize();

                // create buffer for read
                static uint8_t buff[128] = {0};

                // Open file for write
                fsUploadFile = LittleFS.open("/index.html", "w");
                if (!fsUploadFile)
                {
                    //Serial.printf( F("file open failed"));
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: Fehler beim Speichern index.html");
                    https.end();
                    return;
                }

                // read all data from server
                while (https.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = clientup->available();
                    //Serial.printf("*** SYSINFO: [HTTPS] index size avail: %d\n", size);

                    if (size)
                    {
                        //read up to 128 byte
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to file
                        fsUploadFile.write(buff, c);

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                Serial.println("*** SYSINFO: Index Update abgeschlossen.");
                fsUploadFile.close();
                LittleFS.remove("/update.txt");
                fsUploadFile = LittleFS.open("/update2.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Abbruch!");
            Serial.printf("*** SYSINFO: Update index.html Fehler: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upCerts()
{
    //    const uint8_t fingerprint[20] = {0xcc, 0xaa, 0x48, 0x48, 0x66, 0x46, 0x0e, 0x91, 0x53, 0x2c, 0x9c, 0x7c, 0x23, 0x2a, 0xb1, 0x74, 0x4d, 0x29, 0x9d, 0x33};
    std::unique_ptr<BearSSL::WiFiClientSecure> clientup(new BearSSL::WiFiClientSecure);
    //clientup->setFingerprint(fingerprint);
    clientup->setInsecure();
    HTTPClient https;

    if (https.begin(*clientup, "https://guest:guest:x-oauth-basic@raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/Info/ce.rts"))
    {
        int httpCode = https.GET();
        if (httpCode > 0)
        {
            // Serial.printf("*** SYSINFO: [HTTPS] GET certs.ar Antwort: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK)
            {
                int len = https.getSize();
                static uint8_t buff[128] = {0};
                fsUploadFile = LittleFS.open("/certs.ar", "w");
                if (!fsUploadFile)
                {
                    Serial.println("Abbruch!");
                    Serial.println("*** SYSINFO: Fehler beim Speichern certs.ar");
                    https.end();
                    return;
                }

                while (https.connected() && (len > 0 || len == -1))
                {
                    size_t size = clientup->available();
                    if (size)
                    {
                        int c = clientup->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        fsUploadFile.write(buff, c);
                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }
                fsUploadFile.close();
                Serial.println("*** SYSINFO: Certs Update abgeschlossen.");
                LittleFS.remove("/update2.txt");
                fsUploadFile = LittleFS.open("/update3.txt", "w");
                int bytesWritten = fsUploadFile.print("0");
                fsUploadFile.close();
            }
            else
                return;
        }
        else
        {
            Serial.println("Abbruch!");
            Serial.printf("*** SYSINFO: Update certs Fehler: %s\n", https.errorToString(httpCode).c_str());
            https.end();
            LittleFS.end(); // unmount LittleFS
            ESP.restart();
            return;
        }
        https.end();
        return;
    }
    return;
}

void upFirm()
{
    BearSSL::CertStore certStore;
    int numCerts = certStore.initCertStore(LittleFS, 
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "991_HTTPUpdate.ino" "." "156" "." "312" "\", \"aSM\", @progbits, 1 #"))) = (
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                                                    "/certs.idx"
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                                                    ); &__pstr__[0];}))
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                                                                      , 
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                                                                        (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "991_HTTPUpdate.ino" "." "156" "." "313" "\", \"aSM\", @progbits, 1 #"))) = (
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                                                                        "/certs.ar"
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                                                                        ); &__pstr__[0];}))
# 156 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                                                                                         );
    Serial.print(((reinterpret_cast<const __FlashStringHelper *>(
# 157 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "991_HTTPUpdate.ino" "." "157" "." "314" "\", \"aSM\", @progbits, 1 #"))) = (
# 157 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                "Number of CA certs read: "
# 157 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                ); &__pstr__[0];}))
# 157 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                ))));
    Serial.println(numCerts);
    if (numCerts == 0)
    {
        Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 161 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "991_HTTPUpdate.ino" "." "161" "." "315" "\", \"aSM\", @progbits, 1 #"))) = (
# 161 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                      "*** SYSINFO: No certs found. Did you run certs-from-mozill.py and upload the LittleFS directory before running?"
# 161 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino" 3
                      ); &__pstr__[0];}))
# 161 "c:\\Arduino\\git\\MQTTDevice4\\991_HTTPUpdate.ino"
                      ))));
        return; // Can't connect to anything w/o certs!
    }

    BearSSL::WiFiClientSecure clientFirm;
    clientFirm.setCertStore(&certStore);
    clientFirm.setInsecure();

    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    //ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret = ESPhttpUpdate.update(clientFirm, "https://raw.githubusercontent.com/InnuendoPi/MQTTDevice4/master/build/MQTTDevice4.ino.bin");

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("*** SYSINFO: HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("*** SYSINFO: HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("*** SYSINFO: HTTP_UPDATE_OK");
        break;
    }
    return;
}

//   // Stack dump
//   // https://github.com/esp8266/Arduino/blob/master/doc/Troubleshooting/stack_dump.md

void updateSys()
{
    if (LittleFS.exists("/update.txt"))
    {
        fsUploadFile = LittleFS.open("/update.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update.txt");
            Serial.println("*** SYSINFO: ERROR Index Update");
            return;
        }
        fsUploadFile = LittleFS.open("/update.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log1.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO Starte Index Update Free Heap: ");
        Serial.println(ESP.getFreeHeap());
        upIn();
    }
    if (LittleFS.exists("/update2.txt"))
    {
        fsUploadFile = LittleFS.open("/update2.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update2.txt");
            Serial.println("*** SYSINFO: ERROR Cert Update");
            return;
        }
        fsUploadFile = LittleFS.open("/update2.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log2.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();
        Serial.print("*** SYSINFO Starte Cert Update Free Heap: ");
        Serial.println(ESP.getFreeHeap());
        upCerts();
    }
    if (LittleFS.exists("/update3.txt"))
    {
        fsUploadFile = LittleFS.open("/update3.txt", "r");
        String line;
        while (fsUploadFile.available())
        {
            line = char(fsUploadFile.read());
        }
        fsUploadFile.close();
        int i = line.toInt();
        if (i > 3)
        {
            LittleFS.remove("/update3.txt");
            Serial.println("*** SYSINFO: ERROR Firmware Update");
            return;
        }
        fsUploadFile = LittleFS.open("/update3.txt", "w");
        i++;
        int bytesWritten = fsUploadFile.print(i);
        fsUploadFile.close();
        fsUploadFile = LittleFS.open("/log3.txt", "w");
        bytesWritten = fsUploadFile.print((i));
        fsUploadFile.close();

        Serial.print("*** SYSINFO Starte Firmware Update Free Heap: ");
        Serial.println(ESP.getFreeHeap());
        upFirm();
    }
}

void startHTTPUpdate()
{
    // Starte Updates
    fsUploadFile = LittleFS.open("/update.txt", "w");
    if (!fsUploadFile)
    {
        ;
        return;
    }
    else
    {
        int bytesWritten = fsUploadFile.print("0");
        fsUploadFile.close();
    }
    cbpiEventSystem(11);
}

void update_progress(int cur, int total)
{
    Serial.printf("*** SYSINFO:  Firmware Update %d von %d Bytes\n", cur, total);
}

void update_started()
{
    Serial.println("*** SYSINFO:  Firmware Update gestartet");
}

void update_finished()
{
    Serial.println("*** SYSINFO:  Firmware Update beendet");
    LittleFS.remove("/update3.txt");
}

void update_error(int err)
{
    Serial.printf("*** SYSINFO:  Firmware Update Fehler error code %d\n", err);
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\998_InfluxDB.ino"
/*

class DBServer

{

public:

    String kettle_id = "";            // Kettle ID

    String kettle_topic = "";         // Kettle Topic

    String kettle_heater_topic = "";  // Kettle Heater MQTT Topic

    float kettle_sensor_temp = 0.0;   // Kettle Sensor aktuelle Temperatur

    int kettle_target_temp = 0;       // Kettle Heater TargetTemp

    int kettle_heater_powerlevel = 0; // Kettle Heater aktueller Powerlevel

    int kettle_heater_state = 0;      // Kettle Heater Status (on/off)

    int dbEnabled = -1;               // Topic existiert nicht



    DBServer(String new_kettle_id, String new_kettle_topic)

    {

        kettle_id = new_kettle_id;

        kettle_topic = new_kettle_topic;

    }



    void mqtt_subscribe()

    {

        if (pubsubClient.connected())

        {

            char subscribemsg[50];

            kettle_topic.toCharArray(subscribemsg, 50);

            pubsubClient.subscribe(subscribemsg);

            if (!pubsubClient.subscribe(subscribemsg))

            {

                DEBUG_MSG("%s\n", "InfluxMQTT Fehler");

            }

            else

            {

                DEBUG_MSG("InfluxMQTT: Subscribing to %s\n", subscribemsg);

            }

        }

    }

    void mqtt_unsubscribe()

    {

        if (pubsubClient.connected())

        {

            char subscribemsg[50];

            kettle_topic.toCharArray(subscribemsg, 50);

            DEBUG_MSG("InfluxMQTT: Unsubscribing from %s\n", subscribemsg);

            pubsubClient.unsubscribe(subscribemsg);

        }

    }



    void handlemqtt(char *payload)

    {

        StaticJsonDocument<256> doc;

        DeserializationError error = deserializeJson(doc, (const char *)payload);

        if (error)

        {

            DEBUG_MSG("TCP: handlemqtt deserialize Json error %s\n", error.c_str());

            return;

        }

        kettle_id = doc["id"].as<String>();

        if (isValidInt(doc["tt"].as<String>()))

            kettle_target_temp = doc["tt"];

        else

            kettle_target_temp = 0;

        if (isValidFloat(doc["te"].as<String>()))

            kettle_sensor_temp = doc["te"];

        else

            kettle_sensor_temp = 0.0;

        kettle_heater_topic = doc["he"].as<String>();

        DEBUG_MSG("Influx handleMQTT dbEn: %d ID: %s State: %d Target: %d Temp: %f Power: %d Topic: %s\n", dbEnabled, kettle_id.c_str(), kettle_heater_state, kettle_target_temp, kettle_sensor_temp, kettle_heater_powerlevel, kettle_heater_topic.c_str());

        //if (dbEnabled == -1)

        if (dbEnabled != 0)

        {

            if (kettle_heater_topic == inductionCooker.mqtttopic)

            {

                if (inductionCooker.setGrafana)

                {

                    dbEnabled = 1;

                    kettle_heater_state = inductionCooker.isInduon;

                    kettle_heater_powerlevel = inductionCooker.power;

                    return;

                }

            }

            for (int j = 0; j < numberOfActors; j++) // Array Aktoren

            {

                if (kettle_heater_topic == actors[j].argument_actor)

                {

                    if (actors[j].setGrafana)

                    {

                        dbEnabled = 1;

                        kettle_heater_state = actors[j].actor_state;

                        kettle_heater_powerlevel = actors[j].power_actor;

                        return;

                    }

                }

            }

            if (dbEnabled == -1)

            {

                dbEnabled = 0;

                mqtt_unsubscribe();

            }

        }

    }

};



// Erstelle Array mit Kettle ID CBPi

DBServer dbInflux[numberOfDBMax] = {

    DBServer("1", "MQTTDevice/kettle/1"),

    DBServer("2", "MQTTDevice/kettle/2"),

    DBServer("3", "MQTTDevice/kettle/3")};



void sendData()

{

    for (int i = 0; i < numberOfDBMax; i++)

    {

        if (dbInflux[i].dbEnabled != 1)

            continue;



        Point dbData("mqttdevice_status");

        dbData.addTag("ID", dbInflux[i].kettle_id);

        if (dbVisTag[0] != '\0')

            dbData.addTag("Sud-ID", dbVisTag);

        dbData.addField("Temperatur", dbInflux[i].kettle_sensor_temp);

        dbData.addField("TargetTemp", dbInflux[i].kettle_target_temp);

        if (dbInflux[i].kettle_heater_state == 1)

        {

            // Test: Grafana powerlevel "no value"

            if (dbInflux[i].kettle_heater_powerlevel >= 0)

                dbData.addField("Powerlevel", dbInflux[i].kettle_heater_powerlevel);

            else

                dbData.addField("Powerlevel", 0);

        }

        else

            dbData.addField("Powerlevel", 0);

        DEBUG_MSG("Sende an InfluxDB: %s\n", dbData.toLineProtocol().c_str());



        if (!dbClient.writePoint(dbData))

        {

            DEBUG_MSG("InfluxDB Schreibfehler: %s\n", dbClient.getLastErrorMessage().c_str());

            sendAlarm(ALARM_ERROR2);

        }

    }

}



void setInfluxDB()

{

    // Setze Parameter

    dbClient.setConnectionParamsV1(dbServer, dbDatabase, dbUser, dbPass);

}



bool checkDBConnect()

{

    if (dbClient.validateConnection())

    {

        DEBUG_MSG("Verbunden mit InfluxDB: %s\n", dbClient.getServerUrl().c_str());

        return true;

    }

    else

    {

        DEBUG_MSG("Verbindung zu InfluxDB Datenbank fehlgeschlagen: %s\n", dbClient.getLastErrorMessage().c_str());

        sendAlarm(ALARM_ERROR2);

        return false;

    }

}

*/
# 1 "c:\\Arduino\\git\\MQTTDevice4\\999_TCP_Server.ino"
// Nicht merh in Verwendung: Anbindung TCPServer (Tozzi)
/*

class TCPServer

{

public:

    String kettle_id = "0";           // Kettle ID

    String kettle_heater_topic = "";  // Kettle Heater MQTT Topic

    String kettle_name = "";          // Kettle Name

    float kettle_sensor_temp = 0.0;   // Kettle Sensor aktuelle Temperatur

    int kettle_target_temp = 0.0;     // Kettle Heater TargetTemp

    int kettle_heater_powerlevel = 0; // Kettle Heater aktueller Powerlevel

    bool kettle_heater_state = false; // Kettle Heater Status (on/off)



    String topic = "";



    TCPServer(String new_kettle_id)

    {

        change(new_kettle_id);

    }



    void change(const String &new_kettle_id)

    {

        if (kettle_id != new_kettle_id)

        {

            mqtt_unsubscribe();

            kettle_id = new_kettle_id;

            topic = "MQTTDevice/kettle/" + kettle_id;

            // setTopic(kettle_id);

            mqtt_subscribe();

        }

    }

    void mqtt_subscribe()

    {

        if (pubsubClient.connected() && kettle_id != "0")

        {

            char subscribemsg[50];

            topic.toCharArray(subscribemsg, 50);

            DEBUG_MSG("TCP: Subscribing to %s\n", subscribemsg);

            if (!pubsubClient.subscribe(subscribemsg)) // Topic existiert nicht

                kettle_id = "0";

        }

    }

    void mqtt_unsubscribe()

    {

        if (pubsubClient.connected() && kettle_id != "0")

        {

            char subscribemsg[50];

            topic.toCharArray(subscribemsg, 50);

            DEBUG_MSG("TCP: Unsubscribing from %s\n", subscribemsg);

            pubsubClient.unsubscribe(subscribemsg);

        }

    }



    void handlemqtt(char *payload)

    {

        // StaticJsonDocument<256> doc;

        // DeserializationError error = deserializeJson(doc, (const char *)payload);

        // if (error)

        // {

        //     DEBUG_MSG("TCP: handlemqtt deserialize Json error %s\n", error.c_str());

        //     return;

        // }

        DynamicJsonDocument doc(256);

        DeserializationError error = deserializeJson(doc, (const char *)payload);

        if (error)

        {

            DEBUG_MSG("Conf: Error Json %s\n", error.c_str());

            return;

        }

        kettle_id = doc["id"].as<String>();

        kettle_name = doc["name"].as<String>();

        kettle_heater_state = doc["state"];

        kettle_target_temp = doc["target_temp"];

        kettle_sensor_temp = doc["temperatur"];

        kettle_heater_powerlevel = doc["power"];

        kettle_heater_topic = doc["heater"]["topic"].as<String>();

    }

};



// Erstelle Array mit ID - Anzahl maxSensoren == max Anzahl TCP

TCPServer tcpServer[numberOfSensorsMax] = {

    TCPServer("1"),

    TCPServer("2"),

    TCPServer("3"),

    TCPServer("4"),

    TCPServer("5"),

    TCPServer("6")};



void publishTCP()

{

    for (int i = 1; i < numberOfSensorsMax; i++)

    {

        if (tcpServer[i].kettle_id == "0")

            continue;

        // Daten sammeln

        dbData.clearFields();

        dbData.addField("ID", tcpServer[i].kettle_id);

        dbData.addField("Name", tcpServer[i].kettle_name);

        dbData.addField("Temperatur", tcpServer[i].kettle_sensor_temp);

        dbData.addField("TargetTemp", tcpServer[i].kettle_target_temp);

        if (tcpServer[i].kettle_heater_state)

            dbData.addField("Powerlevel", tcpServer[i].kettle_heater_powerlevel);

        else

            dbData.addField("Powerlevel", 0);

        DEBUG_MSG("Sende an InfluxDB: ", dbData.toLineProtocol().c_str());

        // Daten an InfluxDB senden

        if (!dbClient.writePoint(dbData))

        {

            DEBUG_MSG("InfluxDB Schreibfehler: %s\n", dbClient.getLastErrorMessage().c_str());

        }



        

        // Alte Visualisierung TCPServer    

        // tcpClient.connect(tcpHost, tcpPort);

        // if (tcpClient.connect(tcpHost, tcpPort))

        // {

        //     StaticJsonDocument<256> doc;

        //     doc["name"] = tcpServer[i].name;

        //     doc["ID"] = tcpServer[i].ID;

        //     doc["temperature"] = tcpServer[i].temperature;

        //     doc["temp_units"] = "C";

        //     doc["RSSI"] = WiFi.RSSI();

        //     doc["interval"] = TCP_UPDATE;

        //     // Send additional but sensless data to act as an iSpindle device

        //     // json from iSpindle:

        //     // Input Str is now:{"name":"iSpindle","ID":1234567,"angle":22.21945,"temperature":15.6875,

        //     // "temp_units":"C","battery":4.207508,"gravity":1.019531,"interval":900,"RSSI":-59}



        //     doc["angle"] = tcpServer[i].target_temp;

        //     doc["battery"] = tcpServer[i].powerlevel;

        //     doc["gravity"] = 0;

        //     char jsonMessage[256];

        //     serializeJson(doc, jsonMessage);

        //     tcpClient.write(jsonMessage);

        //     //DEBUG_MSG("TCP: TCP message %s", jsonMessage);

        // }

    }

}



void setTCPConfig()

{

    // Init TCP array

    // Das Array ist nicht lin aufsteigend sortiert

    // Das Array beginnt bei 1 mit der ersten ID aus MQTTPub (CBPi Plugin)

    // das Array Element 0 ist unbelegt

    

    

    // for (int i = 0; i < numberOfSensors; i++)

    // {

    //     if (sensors[i].kettle_id.toInt() > 0)

    //     {

    //         // Setze Kettle ID

    //         tcpServer[sensors[i].kettle_id.toInt()].kettle_id = sensors[i].kettle_id;

    //         // Setze MQTTTopic

    //         tcpServer[sensors[i].kettle_id.toInt()].tcpTopic = "MQTTDevice/kettle/" + sensors[i].kettle_id;

    //         tcpServer[sensors[i].kettle_id.toInt()].name = sensors[i].sens_name;

    //         tcpServer[sensors[i].kettle_id.toInt()].ID = SensorAddressToString(sensors[i].sens_address);

    //         tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);

    //     }

    // }







    // for (int i = 1; i < numberOfSensorsMax; i++)

    // {

    //     if (tcpServer[i].kettle_id == "0")

    //         break;

    //     //DEBUG_MSG("TCP Server: %s Topic %s Powerlevel %d", tcpServer[i].kettle_id.c_str(), tcpServer[i].tcpTopic.c_str(), tcpServer[i].powerlevel);

    // }

}



void setTopic(const String &id)

{

    tcpServer[id.toInt()].topic = "MQTTDevice/kettle/" + id;

    //DEBUG_MSG("TCP: topic %s set %s", id.c_str(), tcpServer[id.toInt()].tcpTopic.c_str());

}



// void setTCPTemp(const String &id, const float &temp)

// {

//     tcpServer[id.toInt()].temperature = temp;

// }



// int getTCPTargetTemp(const String &id)

// {

//     if (id != "0")

//         return tcpServer[id.toInt()].target_temp;

//     else

//         return 0;

// }



// void setTCPTemp()

// {

//     for (int i = 0; i < numberOfSensors; i++)

//     {

//         if (sensors[i].kettle_id.toInt() > 0)

//             tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);

//     }

// }



// void setTCPPowerAct(const String &id, const int &power)

// {

//     tcpServer[id.toInt()].powerlevel = power;

// }

// void setTCPPowerInd(const String &id, const int &power)

// {

//     tcpServer[id.toInt()].powerlevel = power;

// }

*/
# 1 "c:\\Arduino\\git\\MQTTDevice4\\99_PINMAP.ino"
/* PINMAP

  0   D3        OnewWire

  1

  2   D4

  3

  4   D2        OLED display (optional)

  5   D1        OLED display (optional)

  6

  7

  8

  9   UNUSED

  10

  11

  12  D6

  13  D7

  14  D5

  15  D8

  16  D0

*/
# 1 "c:\\Arduino\\git\\MQTTDevice4\\9_SYSTEM.ino"
void millis2wait(const int &value)
{
  unsigned long pause = millis();
  while (millis() < pause + value)
  {
    yield(); //wait approx. [period] ms
  }
}

// Prüfe WebIf Eingaben
float formatDOT(String str)
{
  str.replace(',', '.');
  if (isValidFloat(str))
    return str.toFloat();
  else
    return 0;
}

bool isValidInt(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (isdigit(str.charAt(i)))
      continue;
    if (str.charAt(i) == '.')
      return false;
    return false;
  }
  return true;
}

bool isValidFloat(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (i == 0)
    {
      if (str.charAt(i) == '-')
        continue;
    }
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

bool isValidDigit(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

bool checkBool(const String &value)
{
  if (value == "true")
    return true;
  else
    return false;
}

void checkChars(char *input)
{
  char *output = input;
  int j = 0;
  for (int i = 0; i < strlen(input); i++)
  {
    if (input[i] != ' ' && input[i] != '\n' && input[i] != '\r') // Suche nach Leerzeichen und CR LF
      output[j] = input[i];
    else
      j--;

    j++;
  }
  output[j] = '\0';
  *input = *output;
  return;
}

void setTicker()
{
  // Ticker Objekte
  TickerSen.config(tickerSenCallback, SEN_UPDATE, 0);
  TickerAct.config(tickerActCallback, ACT_UPDATE, 0);
  TickerInd.config(tickerIndCallback, IND_UPDATE, 0);
  TickerDisp.config(tickerDispCallback, DISP_UPDATE, 0);
  TickerMQTT.config(tickerMQTTCallback, 30000 /* für Ticker Objekt MQTT in ms*/, 0);
  TickerWLAN.config(tickerWLANCallback, 10000 /* für Ticker Objekt WLAN in ms*/, 0);
  TickerNTP.config(tickerNTPCallback, 60 * 60 * 1000 /* Aktualisierung NTP in ms*/, 0);
  TickerMQTT.stop();
  TickerWLAN.stop();
}

void checkSummerTime()
{
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  int year = ti->tm_year + 1900;
  int month = ti->tm_mon + 1;
  int day = ti->tm_mday;
  int hour = ti->tm_hour;
  int tzHours = 1; // UTC: 0 MEZ: 1
  int x1, x2, x3, lastyear;
  int lasttzHours;
  if (month < 3 || month > 10)
  {
    timeClient.setTimeOffset(3600);
    return;
  }
  if (month > 3 && month < 10)
  {
    timeClient.setTimeOffset(7200);
    return;
  }
  if (year != lastyear || tzHours != lasttzHours)
  {
    x1 = 1 + tzHours + 24 * (31 - (5 * year / 4 + 4) % 7);
    x2 = 1 + tzHours + 24 * (31 - (5 * year / 4 + 1) % 7);
    lastyear = year;
    lasttzHours = tzHours;
  }
  x3 = hour + 24 * day;
  if (month == 3 && x3 >= x1 || month == 10 && x3 < x2)
  {
    timeClient.setTimeOffset(7200);
    return;
  }
  else
  {
    timeClient.setTimeOffset(3600);
    return;
  }
}

String decToHex(unsigned char decValue, unsigned char desiredStringLength)
{
  String hexString = String(decValue, 16);
  while (hexString.length() < desiredStringLength)
    hexString = "0" + hexString;

  return "0x" + hexString;
}

unsigned char convertCharToHex(char ch)
{
  unsigned char returnType;
  switch (ch)
  {
  case '0':
    returnType = 0;
    break;
  case '1':
    returnType = 1;
    break;
  case '2':
    returnType = 2;
    break;
  case '3':
    returnType = 3;
    break;
  case '4':
    returnType = 4;
    break;
  case '5':
    returnType = 5;
    break;
  case '6':
    returnType = 6;
    break;
  case '7':
    returnType = 7;
    break;
  case '8':
    returnType = 8;
    break;
  case '9':
    returnType = 9;
    break;
  case 'A':
    returnType = 10;
    break;
  case 'B':
    returnType = 11;
    break;
  case 'C':
    returnType = 12;
    break;
  case 'D':
    returnType = 13;
    break;
  case 'E':
    returnType = 14;
    break;
  case 'F':
    returnType = 15;
    break;
  default:
    returnType = 0;
    break;
  }
  return returnType;
}

void sendAlarm(const uint8_t &setAlarm)
{
  if (!startBuzzer)
    return;
  switch (setAlarm)
  {
  case 1:
    tone(PIN_BUZZER, 440, 50);
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 880, 50);
    break;
  case 2:
    tone(PIN_BUZZER, 880, 50);
    delay(150);
    tone(PIN_BUZZER, 660, 50);
    delay(150);
    tone(PIN_BUZZER, 440, 50);
    break;
  case 3:
    digitalWrite(PIN_BUZZER, 0x1);
    delay(200);
    digitalWrite(PIN_BUZZER, 0x0);
    break;
  case 4:
    for (int i = 0; i < 20; i++)
    {
      tone(PIN_BUZZER, 880, 50);
      delay(150);
      tone(PIN_BUZZER, 440, 50);
      delay(150);
    }
    millis2wait(1000);
    break;
  case 5:
    for (int i = 0; i < 4; i++)
    {
      tone(PIN_BUZZER, 880, 50);
      delay(100);
      tone(PIN_BUZZER, 880, 50);
      delay(100);
      tone(PIN_BUZZER, 440, 50);
      delay(100);
      tone(PIN_BUZZER, 440, 50);
      delay(100);
    }
    break;
  default:
    break;
  }
}
# 1 "c:\\Arduino\\git\\MQTTDevice4\\EventManager.ino"
void listenerSystem(int event, int parm) // System event listener
{
  switch (parm)
  {
  case 0 /* Normal mode*/: // Normal mode
    break;
  // 1 - 9 Error events
  case 1: // WLAN error -> handling
                  //  Error Reihenfolge
                  //  1. WLAN connected?
                  //  2. MQTT connected
                  //  Wenn WiFi.status() != WL_CONNECTED (wlan_state false nach maxRetries und Delay) ist, ist ein check mqtt überflüssig

    oledDisplay.wlanOK = false;
    WiFi.reconnect();
    if (WiFi.status() == WL_CONNECTED)
    {
      wlan_state = true;
      oledDisplay.wlanOK = true;
      break;
    }
    ;
    if (millis() - wlanconnectlasttry >= wait_on_error_wlan) // Wait bevor Event handling
    {
      if (StopOnWLANError && wlan_state)
      {
        if (startBuzzer)
          sendAlarm(4);
        // if (useDisplay)
        //   showDispErr("WLAN ERROR")
        ;
        wlan_state = false;
        mqtt_state = false; // MQTT in error state - required to restore values
        cbpiEventActors(10 /* Bei Fehler Behandlung von Aktoren*/);
        cbpiEventInduction(10 /* Bei Fehler Behandlung Induktion*/);
      }
    }
    break;
  case 2: // MQTT Error -> handling
    oledDisplay.mqttOK = false;
    if (pubsubClient.connect(mqtt_clientid))
    {
      ;
      cbpiEventSystem(28); // MQTT subscribe
      cbpiEventSystem(10); // MQTT restore
      break;
    }
    if (millis() - mqttconnectlasttry >= wait_on_error_mqtt)
    {
      if (StopOnMQTTError && mqtt_state)
      {
        if (startBuzzer)
          sendAlarm(4);
        // if (useDisplay)
        //   showDispErr("MQTT ERROR")
        ;
        cbpiEventActors(10 /* Bei Fehler Behandlung von Aktoren*/);
        cbpiEventInduction(10 /* Bei Fehler Behandlung Induktion*/);
        mqtt_state = false; // MQTT in error state
      }
    }
    break;
  // 10-19 System triggered events
  case 10: // restore saved values after reconnect MQTT (10)
    if (pubsubClient.connected())
    {
      wlan_state = true;
      mqtt_state = true;
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state)
        {
          ;
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true; // Sensor ok
          actors[i].Update();
        }
        yield();
      }
      if (!inductionCooker.induction_state)
      {
        ;
        inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
        inductionCooker.isInduon = true;
        inductionCooker.induction_state = true; // Induction ok
        inductionCooker.Update();
        ;
      }
    }
    break;
  case 11: // Reboot ESP (11) - manual task
    // Stop actors
    cbpiEventActors(11 /* Aktor ausschalten*/);
    // Stop induction
    if (inductionCooker.isInduon)
      cbpiEventInduction(11 /* Induktion ausschalten*/);
    server.send(200, "text/plain", "rebooting...");
    LittleFS.end(); // unmount LittleFS
    ESP.restart();
    break;
  // System run & set events
  case 20: // check WLAN (20) and reconnect on error
    if (WiFi.status() == WL_CONNECTED)
    {
      oledDisplay.wlanOK = true;
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
  case 22: // check MQTT (22)
    if (!WiFi.status() == WL_CONNECTED)
      break;
    if (pubsubClient.connected())
    {
      oledDisplay.mqttOK = true;
      mqtt_state = true;
      pubsubClient.loop();
      if (TickerMQTT.state() == RUNNING)
        TickerMQTT.stop();
    }
    else //if (!pubsubClient.connected())
    {
      if (TickerMQTT.state() != RUNNING)
      {
        ;
        TickerMQTT.resume();
        mqttconnectlasttry = millis();
      }
      TickerMQTT.update();
    }
    break;
  case 27: // MQTT connect (27)
    if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
    {
      ;
      pubsubClient.setServer(mqtthost, 1883);
      pubsubClient.setCallback(mqttcallback);
      pubsubClient.connect(mqtt_clientid);
    }
    break;
  case 28: // MQTT subscribe (28)
    if (pubsubClient.connected())
    {
      ;
      for (int i = 0; i < numberOfActors; i++)
      {
        actors[i].mqtt_subscribe();
        yield();
      }
      if (inductionCooker.isEnabled)
        inductionCooker.mqtt_subscribe();
      oledDisplay.mqttOK = true; // Display MQTT
      mqtt_state = true; // MQTT state ok
      //if (TickerMQTT.state() == RUNNING)
      TickerMQTT.stop();
    }
    break;
  case 24: // check MDSN (24)
    mdns.update();
    break;
  case 29: // NTP Update (25)
    timeClient.begin();
    timeClient.forceUpdate();
    checkSummerTime();
    break;
  case 25: // NTP Update (25) -> In Ticker Objekt ausgelagert!
    timeClient.update();
    break;
  case 26: // MDNS setup (26)
    if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() == WL_CONNECTED)
    {
      if (mdns.begin(nameMDNS))
        Serial.printf("*** SYSINFO: mDNS gestartet als %s verbunden an %s Time: %s RSSI=%d\n", nameMDNS, WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
      else
        Serial.printf("*** SYSINFO: Fehler Start mDNS! IP Adresse: %s Time: %s RSSI: %d\n", WiFi.localIP().toString().c_str(), timeClient.getFormattedTime().c_str(), WiFi.RSSI());
    }
    break;
  case 30: // Display screen output update (30)
    if (oledDisplay.dispEnabled)
      oledDisplay.dispUpdate();
    break;
  case 35:
    if (LittleFS.exists("/log1.txt")) // WebUpdate Zertifikate
    {
      fsUploadFile = LittleFS.open("/log1.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update Index Anzahl Versuche %s\n", line.c_str());
      LittleFS.remove("/log1.txt");
    }
    if (LittleFS.exists("/log2.txt")) // WebUpdate Index
    {
      fsUploadFile = LittleFS.open("/log2.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      Serial.printf("*** SYSINFO: Update Zertifikate Anzahl Versuche %s\n", line.c_str());
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
      Serial.printf("*** SYSINFO: Update Firmware Anzahl Versuche %s\n", line.c_str());
      LittleFS.remove("/log3.txt");
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
  case 0 /* Normal mode*/:
    // all sensors ok
    lastSenInd = 0; // Delete induction timestamp after event
    lastSenAct = 0; // Delete actor timestamp after event

    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && wlan_state && mqtt_state)
    {
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state) // Sensor in normal mode: check actor in error state
        {
          ;
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true;
          actors[i].Update();
          lastSenAct = 0; // Delete actor timestamp after event
        }
        yield();
      }

      if (!inductionCooker.induction_state)
      {
        ;
        if (!inductionCooker.induction_state)
        {
          inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
          inductionCooker.isInduon = true;
          inductionCooker.induction_state = true;
          inductionCooker.Update();
          ;
          lastSenInd = 0; // Delete induction timestamp after event
        }
      }
    }
    break;
  case 1 /* Sensor CRC failed*/:
    // Sensor CRC ceck failed
  case 2 /* Sensor device error*/:
    // -127°C device error
  case 3 /* Sensor unplugged*/:
    // sensor unpluged
  case 4 /* Sensor all errors*/:
    // all other errors
    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && wlan_state && mqtt_state)
    {
      for (int i = 0; i < numberOfSensors; i++)
      {
        if (!sensors[i].getState())
        {
          switch (parm)
          {
          case 1 /* Sensor CRC failed*/:
            // Sensor CRC ceck failed
            ;
            break;
          case 2 /* Sensor device error*/:
            // -127°C device error
            ;
            break;
          case 3 /* Sensor unplugged*/:
            // sensor unpluged
            ;
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
            ;
          }
          if (lastSenInd == 0)
          {
            lastSenInd = millis(); // Timestamp on error
            ;
          }
          if (millis() - lastSenAct >= wait_on_Sensor_error_actor) // Wait bevor Event handling
            cbpiEventActors(10 /* Bei Fehler Behandlung von Aktoren*/);

          if (millis() - lastSenInd >= wait_on_Sensor_error_induction) // Wait bevor Event handling
          {
            if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
              cbpiEventInduction(10 /* Bei Fehler Behandlung Induktion*/);
          }
        } // Switchable
        yield();
      } // Iterate sensors
    } // wlan und mqtt state
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
  case 0 /* Normal mode*/:
    break;
  case 1:
    break;
  case 2:
    break;
  case 10 /* Bei Fehler Behandlung von Aktoren*/:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].switchable && actors[i].actor_state && actors[i].isOn)
      {
        actors[i].isOnBeforeError = actors[i].isOn;
        actors[i].isOn = false;
        actors[i].actor_state = false;
        actors[i].Update();
        ;
      }
      yield();
    }
    break;
  case 11 /* Aktor ausschalten*/:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].isOn)
      {
        actors[i].isOn = false;
        actors[i].Update();
        ;
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
  case 0 /* Normal mode*/: // Induction off
    break;
  case 1: // Induction on
    break;
  case 2:
    //DBG_PRINTLN("EM IND2: received induction event"); // bislang keine Verwendung
    break;
  case 10 /* Bei Fehler Behandlung Induktion*/:
    if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state) // powerlevelonerror == 100 -> kein event handling
    {
      inductionCooker.powerLevelBeforeError = inductionCooker.power;
      ;
      if (inductionCooker.powerLevelOnError == 0)
        inductionCooker.isInduon = false;
      else
        inductionCooker.newPower = inductionCooker.powerLevelOnError;

      inductionCooker.newPower = inductionCooker.powerLevelOnError;
      inductionCooker.induction_state = false;
      inductionCooker.Update();
    }
    break;
  case 11 /* Induktion ausschalten*/:
    if (inductionCooker.isInduon)
    {
      ;
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
# 1 "c:\\Arduino\\git\\MQTTDevice4\\FSBrowser.ino"
// format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}
// Datei editieren -> speichern CTRL+S
bool handleFileRead(String path) {
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
    if (LittleFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    ;
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    ;
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    ;
    loadConfig();
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  ;
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!LittleFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  LittleFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  ;
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (LittleFS.exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = LittleFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = server.arg("dir");
  Dir dir = LittleFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    // output += String(entry.name()).substring(1); //SPIFFS
    output += String(entry.name()).substring(0); // Änderung für LittleFS
    output += "\"}";
    entry.close();
  }
  output += "]";
  server.send(200, "text/json", output);
}
