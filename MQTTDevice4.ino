//    Author:	Innuendo
//    Sketch für ESP8266
//    Kommunikation via MQTT mit CraftBeerPi v4
//
//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Unterstützung für Nextion Touchdisplay

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <ESP8266WiFi.h>       // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>  // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h> // DateiUpdate
#include <WiFiManager.h> // WiFiManager zur Einrichtung
#include <DNSServer.h>   // Benötigt für WiFiManager
#include <LittleFS.h>    // Dateisystem
#include <FS.h>          // Files
#include <ArduinoJson.h> // Lesen und schreiben von JSON Dateien
#include <ESP8266mDNS.h> // mDNS
#include <WiFiUdp.h>     // WiFi
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>        // Uhrzeit
#include <Ticker.h>
#include <PubSubClient.h>     // MQTT Kommunikation
#include <SoftwareSerial.h>   // Serieller Port für Display
#include "InnuTicker.h"       // Bibliothek für Hintergrund Aufgaben (Tasks)
#include "NextionX2.h"        // Display Nextion
#include "index_htm.h"
#include "edit_htm.h"
#include "MQTTDevice4.h"

// extern "C"
// {
// #include "user_interface.h"
// }

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

// Version
#define Version "4.56e"

// System Dateien
#define UPDATESYS "/updateSys.txt"
#define LOGUPDATESYS "/updateSys.log"
#define UPDATETOOLS "/updateTools.txt"
#define LOGUPDATETOOLS "/updateTools.log"
#define UPDATELOG "/webUpdateLog.txt"
#define DEVBRANCH "/dev.txt"
#define CERT "/ce.rts"
#define CONFIG "/config.txt"
#define MAXFRAGLEN 1024

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750
#define RESOLUTION_HIGH 12 // steps 9bit: 0.5°C 10bit: 0.25°C 11bit: 0.125°C 12bit: 0.0625°C
#define RESOLUTION 11
#define TEMP_OFFSET1 40
#define TEMP_OFFSET2 78

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
bool senRes = false;

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
//WLAN Events
WiFiEventHandler wifiConnectHandler, wifiDisconnectHandler;

PubSubClient pubsubClient(espClient);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate; // DateiUpdate

#define SSE_MAX_CHANNELS 8  // 8 SSE clients subscription erlaubt
struct SSESubscription {
  IPAddress clientIP;
  WiFiClient client;
  Ticker keepAliveTimer;
  bool check = false;
} subscription[SSE_MAX_CHANNELS];
uint8_t subscriptionCount = 0;
const unsigned int port = 80;

#define DEF_DELAY_IND 120000 // Standard Nachlaufzeit nach dem Ausschalten Induktionskochfeld

#define NUMBEROFPINS 9
bool pins_used[17]; // GPIO
static const int8_t pins[NUMBEROFPINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};

// Variablen
uint8_t numberOfSensors = 0; // Gesamtzahl der Sensoren
#define NUMBEROFSENSORSMAX 6       // Maximale Anzahl an Sensoren
unsigned char addressesFound[NUMBEROFSENSORSMAX][8];
#define DUTYCYLCE 5000      // Aktoren und HLT
uint8_t numberOfActors = 0; // Gesamtzahl der Aktoren
#define NUMBEROFACTORSMAX 10      // Maximale Anzahl an Aktoren
#define maxHostSign 17
#define maxUserSign 10
#define maxPassSign 10
char mqtthost[maxHostSign]; // MQTT Server
char mqttuser[maxUserSign];
char mqttpass[maxPassSign];
int mqttport;
char mqtt_clientid[maxHostSign]; // AP-Mode und Gerätename
uint8_t alertState = 0;        // WebUpdate Status
#define MAXVERSUCHE 3

// Zeitserver Einstellungen
#define NTP_OFFSET 60 * 60                // Offset Winterzeit in Sekunden
#define NTP_INTERVAL 60 * 60 * 1000       // Aktualisierung NTP in ms
#define NTP_ADDRESS "europe.pool.ntp.org" // NTP Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Event für Sensoren
#define EM_OK 0    // Normal mode
#define EM_CRCER 1 // Sensor CRC failed
#define EM_DEVER 2 // Sensor device error
#define EM_UNPL 3  // Sensor unplugged
#define EM_SENER 4 // Sensor all errors

// Event handling Status Variablen
bool StopOnMQTTError = false;     // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
uint8_t wlanStatus = 0;
bool mqtt_state = true;           // Status MQTT

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerAct;
InnuTicker TickerInd;
InnuTicker TickerMQTT;
InnuTicker TickerPUBSUB;
InnuTicker TickerDisp;

// Update Intervalle für Ticker Objekte
#define SEN_UPDATE 2000  //  sensors update
#define ACT_UPDATE 2000  //  actors update
#define IND_UPDATE 2000  //  induction update
#define DISP_UPDATE 1000 //  display update
#define tickerMQTT 10000 // für Ticker Objekt MQTT in ms
#define tickerPUSUB 50   // Ticker PubSubClient

// Systemstart
bool startMDNS = true; // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[maxHostSign] = "MQTTDevice";
bool shouldSaveConfig = false; // WiFiManager
bool devBranch = false;           // Check out development branch

unsigned long lastSenAct = 0; // Timestap actors on sensor error
unsigned long lastSenInd = 0; // Timestamp induction on sensor error

int sensorsStatus = 0;
int actorsStatus = 0;
int inductionStatus = 0;

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file

// Alarm codes
#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_INFO 3
#define ALARM_SUCCESS 4
#define ALARM_WARNING 5
#define ALARM_ERROR 6

const int PIN_BUZZER = D8; // Buzzer
bool startBuzzer = false;  // Aktiviere Buzzer
bool mqttBuzzer = false;   // MQTTBuzzer für CBPi4

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
