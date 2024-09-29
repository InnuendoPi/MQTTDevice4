//    Author:	Innuendo
//
//    Kommunikation via MQTT mit CraftBeerPi v4
//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für PT100/PT1000 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Unterstützung für Nextion Touchdisplay

#if defined(ESP8266)
#pragma message "/// ESP8266 ///"
#elif defined(ESP32)
#pragma message "/// ESP32 ///"
#endif

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <Adafruit_MAX31865.h>
#ifdef ESP32
#include <WiFi.h>             // Generelle WiFi Funktionalität
#include <WebServer.h>        // Unterstützung Webserver
#include <HTTPUpdateServer.h> // DateiUpdate
#include <ESPmDNS.h>          // mDNS
#include <HTTPClient.h>
#include <HTTPUpdate.h>
// #include <WiFiClientSecure.h>
#elif ESP8266
#include <ESP8266WiFi.h>      // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>        // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h> // DateiUpdate
#include <ESP8266mDNS.h>      // mDNS
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#endif
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>          // WiFi
#include <WiFiManager.h> // WiFiManager zur Einrichtung
#include <DNSServer.h>   // Benötigt für WiFiManager
#include <LittleFS.h>    // Dateisystem
#include <FS.h>          // Files
#include <ArduinoJson.h> // Lesen und schreiben von JSON Dateien
#include <Ticker.h>
#include <time.h>
#include <PubSubClient.h>   // MQTT Kommunikation
#include <SoftwareSerial.h> // Serieller Port für Display
#include "InnuTicker.h"     // Bibliothek für Hintergrund Aufgaben (Tasks)
#include "InnuNextion.h"
#include "index_htm.h"
#include "edit_htm.h"
#include "MQTTDevice.h"
#include "InnuLog.h"

// Sensoren
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// IDS2 Interrupt
volatile uint8_t newError = 0; // Fehlercode IDS als Integer
uint8_t oldError = 0;
// portMUX_TYPE errCode = portMUX_INITIALIZER_UNLOCKED;

// WiFi und MQTT
#ifdef ESP32
WebServer server(80);
HTTPUpdateServer httpUpdateServer; // DateiUpdate
#elif ESP8266
WiFiEventHandler wifiConnectHandler, wifiDisconnectHandler;
ESP8266WebServer server(80);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate; // DateiUpdate
#endif
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient pubsubClient(espClient);

#define SSE_MAX_CHANNELS 6 // 6 SSE clients subscription erlaubt
#define PORT 80
struct SSESubscription
{
  IPAddress clientIP;
  WiFiClient client;
  Ticker keepAliveTimer;
  bool check = false;
} subscription[SSE_MAX_CHANNELS];
uint8_t subscriptionCount = 0;


// Variablen
#ifdef ESP32
#define NUMBEROFSENSORSMAX 6 // Maximale Anzahl an Sensoren
#define NUMBEROFACTORSMAX 15 // Maximale Anzahl an Aktoren
#elif ESP8266
#define NUMBEROFSENSORSMAX 3 // Maximale Anzahl an Sensoren
#define NUMBEROFACTORSMAX 10 // Maximale Anzahl an Aktoren
#endif

int64_t DUTYCYLCE = 5000; // Aktoren und HLT
uint16_t SENCYLCE = 1; // Aktoren und HLT
bool senRes = false;
bool startSPI = false;
uint8_t numberOfActors = 0;  // Gesamtzahl der Aktoren
uint8_t numberOfSensors = 0; // Gesamtzahl der Sensoren
unsigned char addressesFound[NUMBEROFSENSORSMAX][8];
#define DEF_DELAY_IND 120000 // Standard Nachlaufzeit nach dem Ausschalten Induktionskochfeld

#define maxHostSign 31
#define maxUserSign 16
#define maxPassSign 16
char mqtthost[maxHostSign]; // MQTT Server
char mqttuser[maxUserSign];
char mqttpass[maxPassSign];
int mqttport;
char mqtt_clientid[maxHostSign]; // AP-Mode und Gerätename

// Zeitserver Einstellungen
#define maxNTPSign 31
#define NTP_ADDRESS "europe.pool.ntp.org" // NTP Server
#define NTP_ZONE "CET-1CEST,M3.5.0,M10.5.0/3"
char ntpServer[maxNTPSign] = NTP_ADDRESS;
char ntpZone[maxNTPSign] = NTP_ZONE;
struct tm timeinfo;
char zeit[9] = "00:00:00";
#define SCHALTJAHR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

// Event handling Status Variablen
bool StopOnMQTTError = false;     // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
bool mqtt_state = true;           // Status MQTT
uint8_t wlanStatus = 0;
uint8_t simErr = 0;

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerAct;
InnuTicker TickerInd;
InnuTicker TickerMQTT;
InnuTicker TickerDisp;
InnuTicker TickerPUBSUB;
InnuTicker TickerTime;

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Systemstart
bool startMDNS = true;          // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[maxHostSign] = "MQTTDevice";
bool shouldSaveConfig = false; // WiFiManager
bool alertState = false;       // WebUpdate Status
bool devBranch = false;        // Check out development branch

unsigned long lastSenAct = 0;   // Timestap actors on sensor error
unsigned long lastSenInd = 0;   // Timestamp induction on sensor error

int8_t sensorsStatus = 0;
int8_t actorsStatus = 0;
bool inductionStatus = false;
bool fermenterStatus = false;


bool mqttBuzzer = false;  // MQTTBuzzer für CBPi4

int8_t selLang = 0;       // Sprache

// Serial
#define DEF_SERIAL 115200
#define DEF_NEXTION 9600
SoftwareSerial softSerial;

// Display Next
#define NUMBEROFPAGES 3
bool useDisplay = false;
bool useFerm = false;
int startPage = 0;  // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
int activePage = 0; // die aktuell angezeigte Seite
int tempPage = -1;  // die aktuell angezeigte Seite
InnuNex nextion(softSerial); 

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
  Serial.println(WiFi.softAPIP());
  Serial.print("*** SYSINFO: Start configuration portal ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
