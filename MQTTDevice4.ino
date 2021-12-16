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

#include <OneWire.h>            // OneWire Bus Kommunikation
#include <DallasTemperature.h>  // Vereinfachte Benutzung der DS18B20 Sensoren
#include <ESP8266WiFi.h>        // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>   // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>        // WiFiManager zur Einrichtung
#include <DNSServer.h>          // Benötigt für WiFiManager
#include "LittleFS.h"
#include <ArduinoJson.h>        // Lesen und schreiben von JSON Dateien 6.18
#include <ESP8266mDNS.h>        // mDNS
#include <WiFiUdp.h>            // WiFi
#include <EventManager.h>       // Eventmanager
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include "InnuTicker.h"         // Bibliothek für Hintergrund Aufgaben (Tasks)
#include <PubSubClient.h>       // MQTT Kommunikation 2.8.0
#include <CertStoreBearSSL.h>   // WebUpdate

extern "C"
{
#include "user_interface.h"
}

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...)                                                   \
    DEBUG_ESP_PORT.printf("%s ", timeClient.getFormattedTime().c_str()); \
    DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

// Version
#define Version "2.60"

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
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
#define DEF_DELAY_IND 120000 // Standard Nachlaufzeit nach dem Ausschalten Induktionskochfeld

/*  Binäre Signale für Induktionsplatte */
int CMD[6][33] = {
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
unsigned char PWR_STEPS[] = {0, 20, 40, 60, 80, 100};                                                     // Prozentuale Abstufung zwischen den Stufen

bool pins_used[17];
const unsigned char numberOfPins = 9;
const unsigned char pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

// Variablen
unsigned char numberOfSensors = 0; // Gesamtzahl der Sensoren
#define numberOfSensorsMax 6       // Maximale Anzahl an Sensoren
unsigned char addressesFound[numberOfSensorsMax][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0; // Gesamtzahl der Aktoren
#define numberOfActorsMax 8       // Maximale Anzahl an Aktoren
char mqtthost[16];                // MQTT Server
char mqtt_clientid[16];           // AP-Mode und Gerätename
bool alertState = false;          // WebUpdate Status

// Zeitserver Einstellungen
#define NTP_OFFSET 60 * 60                // Offset Winterzeit in Sekunden
#define NTP_INTERVAL 60 * 60 * 1000       // Aktualisierung NTP in ms
#define NTP_ADDRESS "europe.pool.ntp.org" // NTP Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// EventManager
EventManager gEM; //  Eventmanager Objekt Queues

// System Fehler Events
#define EM_WLANER 1
#define EM_MQTTER 2

// System Events
#define EM_MQTTRES 10
#define EM_REBOOT 11

// Loop Events
#define EM_WLAN 20
#define EM_OTA 21
#define EM_MQTT 22
#define EM_MDNS 24
#define EM_NTP 25
#define EM_MDNSET 26
#define EM_MQTTCON 27
#define EM_MQTTSUB 28
#define EM_SETNTP 29
#define EM_DISPUP 30
#define EM_LOG 35

// Event für Sensoren, Aktor und Induktion
#define EM_OK 0      // Normal mode
#define EM_CRCER 1   // Sensor CRC failed
#define EM_DEVER 2   // Sensor device error
#define EM_UNPL 3    // Sensor unplugged
#define EM_SENER 4   // Sensor all errors
#define EM_ACTER 10  // Bei Fehler Behandlung von Aktoren
#define EM_INDER 10  // Bei Fehler Behandlung Induktion
#define EM_ACTOFF 11 // Aktor ausschalten
#define EM_INDOFF 11 // Induktion ausschalten

// Event handling Status Variablen
bool StopOnWLANError = false;     // Event handling für WLAN Fehler
bool StopOnMQTTError = false;     // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
unsigned long wlanconnectlasttry; // Zeitstempel bei Fehler WLAN
bool mqtt_state = true;           // Status MQTT
bool wlan_state = true;           // Status WLAN

// Event handling Zeitintervall für Reconnects WLAN und MQTT
#define tickerWLAN 10000 // für Ticker Objekt WLAN in ms
#define tickerMQTT 30000 // für Ticker Objekt MQTT in ms

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_error_wlan = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
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
int SEN_UPDATE = 5000;  //  sensors update delay loop
int ACT_UPDATE = 5000;  //  actors update delay loop
int IND_UPDATE = 5000;  //  induction update delay loop
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
#define DISP_DEF_ADDRESS 0x3C  // OLED Display Adresse 3C oder 3D
#define OLED_RESET LED_BUILTIN // D4
bool useDisplay = false;
#define SDL D1
#define SDA D2
#define numberOfAddress 2
const int address[numberOfAddress] = {0x3C, 0x3D};

#include "icons.h" // Icons CraftbeerPi, WLAN und MQTT
// Display mit SH1106 Chip:
//
// Wichtig: Für Displays mit SH1106 wird folgende lib benötigt
// https://github.com/kferrari/Adafruit_SH1106

#include <Adafruit_SH1106.h>
Adafruit_SH1106 display(OLED_RESET);

// Display mit SSD1306 Chip
//
// #include <SPI.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #define SCREEN_WIDTH 128       // OLED display width, in pixels
// #define SCREEN_HEIGHT 64       // OLED display height, in pixels
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_OK 3
#define ALARM_ERROR 4
#define ALARM_ERROR2 5
const int PIN_BUZZER = D8; // Buzzer
bool startBuzzer = false;  // Aktiviere Buzzer

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
