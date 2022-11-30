//    Author:	Innuendo
//    Sketch für ESP8266
//    Kommunikation via MQTT mit CraftBeerPi v4
//    Brauen ohne CBPi4
//
//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Unterstützung für Nextion Touchdisplay
//    Unterstützung für PCF8574 I2C IO Modul

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <ESP8266WiFi.h>       // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>  // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h> // WiFiManager zur Einrichtung
#include <DNSServer.h>   // Benötigt für WiFiManager
#include "LittleFS.h"    // Dateisystem
#include <ArduinoJson.h> // Lesen und schreiben von JSON Dateien
#include <ESP8266mDNS.h> // mDNS
#include <WiFiUdp.h>     // WiFi
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>        // Uhrzeit
#include "InnuTicker.h"       // Bibliothek für Hintergrund Aufgaben (Tasks)
#include <PubSubClient.h>     // MQTT Kommunikation
#include <CertStoreBearSSL.h> // WebUpdate
#include <SoftwareSerial.h>   // Serieller Port für Display
#include "MQTTDevice.h"
#include "NextionX2.h"        // Display Nextion
#include "index_htm.h"
#include "edit_htm.h"
#include "mash_htm.h"
#include <FS.h>       // Files
#include "InnuAPID.h"
#include <PCF8574.h> // I2C IO Modul PCF8574

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
#define Version "4.37d"

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750
#define RESOLUTION 11  // 11 bits : 1/8 = 0.125 °C steps
#define TEMP_OFFSET1 40
#define TEMP_OFFSET2 78

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// I2C Port expander
PCF8574 pcf020(0x20);
#define PIN_SDA D5
#define PIN_SCL D6
bool statePCF = false;

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
unsigned char PWR_STEPS[6] = {0, 20, 40, 60, 80, 100};                                                    // Prozentuale Abstufung zwischen den Stufen

bool useI2C = false;
#define P0 17
#define P1 18
#define P2 19
#define P3 20
#define P4 21
#define P5 22
#define P6 23
#define P7 24
#define ALLPINS 17
#define GPIOPINS 9
#define PCFPINS 8
bool pins_used[25]; // GPIO
unsigned char numberOfPins = ALLPINS;
const unsigned char pins[ALLPINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, P0, P1, P2, P3, P4, P5, P6, P7};
const String pin_names[ALLPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7"};

// Variablen
unsigned char numberOfSensors = 0; // Gesamtzahl der Sensoren
#define numberOfSensorsMax 4       // Maximale Anzahl an Sensoren
unsigned char addressesFound[numberOfSensorsMax][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0; // Gesamtzahl der Aktoren
#define numberOfActorsMax 10      // Maximale Anzahl an Aktoren
#define maxHostSign 15
#define maxUserSign 10
#define maxPassSign 10
char mqtthost[maxHostSign]; // MQTT Server
char mqttuser[maxUserSign];
char mqttpass[maxPassSign];
int mqttport;
char mqtt_clientid[maxHostSign]; // AP-Mode und Gerätename
bool alertState = false;         // WebUpdate Status
// Toast messages
#define TOAST_INFO 0
#define TOAST_SUCCESS 1
#define TOAST_WARNING 2
#define TOAST_ERROR 3
String toastMessage = "";
byte toastHide = TOAST_INFO; 
unsigned long toastLast = 0;
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
unsigned long wlanconnectlasttry; // Zeitstempel bei Fehler WLAN
bool mqtt_state = true;           // Status MQTT
bool devBranch = false;           // Check out development branch
bool mqttoff = false;             // Disable MQTT

// Event handling Zeitintervall für Reconnects WLAN und MQTT
#define tickerWLAN 30000 // für Ticker Objekt WLAN in ms
#define tickerMQTT 30000 // für Ticker Objekt MQTT in ms
#define tickerPUSUB 1000 // Ticker PubSubClient

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerAct;
InnuTicker TickerInd;
InnuTicker TickerHlt;
InnuTicker TickerMQTT;
InnuTicker TickerPUBSUB;
InnuTicker TickerWLAN;
InnuTicker TickerNTP;
InnuTicker TickerDisp;
InnuTicker TickerMash;
InnuTicker TickerPID;

// Update Intervalle für Ticker Objekte
#define SEN_UPDATE 2500  //  sensors update
#define ACT_UPDATE 2000  //  actors update
#define IND_UPDATE 2500  //  induction update
#define HLT_UPDATE 2500  //  hlt update
#define DISP_UPDATE 1000 //  display update
#define MASH_UPDATE 5000 //  mash update
#define PID_UPDATE 2500  //  calc PID update

// Systemstart
bool startMDNS = true; // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[maxHostSign] = "MQTTDevice";
bool shouldSaveConfig = false; // WiFiManager

unsigned long lastSenAct = 0; // Timestap actors on sensor error
unsigned long lastSenInd = 0; // Timestamp induction on sensor error

int sensorsStatus = 0;
int actorsStatus = 0;
int inductionStatus = 0;
int hltStatus = 0;

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file
// bool m_fsOK = true;
enum
{
    MSG_OK,
    CUSTOM,
    NOT_FOUND,
    BAD_REQUEST,
    ERROR
};
#define TEXT_PLAIN "text/plain"
#define FS_INIT_ERROR "FS INIT ERROR"
#define FILE_NOT_FOUND "FileNotFound"

// Display Nextion
bool useDisplay = false;
int startPage = 0;  // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
int activePage = 0; // die aktuell angezeigte Seite

const unsigned char numberOfPages = 3;
const String page_names[numberOfPages] = {"BrewPage", "KettlePage", "InductionPage"};

SoftwareSerial softSerial; // Objekt SoftSerial ohne GPIO
NextionComPort nextion;    // Objekt Display Kommunikation

// Steuerung Buttons
  NextionComponent p0ForButton(nextion, 0, 19);
  NextionComponent p0BackButton(nextion, 0, 21);
  NextionComponent p1ForButton(nextion, 1, 9);
  NextionComponent p1BackButton(nextion, 1, 7);
  NextionComponent p2ForButton(nextion, 2, 10);
  NextionComponent p2BackButton(nextion, 2, 2);

  // BrewPage
  NextionComponent uhrzeit_text(nextion, 0, 10);
  NextionComponent currentStepName_text(nextion, 0, 6);
  NextionComponent currentStepRemain_text(nextion, 0, 5);
  NextionComponent nextStepName_text(nextion, 0, 7);
  NextionComponent nextStepRemain_text(nextion, 0, 8);
  NextionComponent kettleName1_text(nextion, 0, 1);
  NextionComponent kettleIst1_text(nextion, 0, 11);
  NextionComponent kettleSoll1_text(nextion, 0, 15);
  NextionComponent kettleName2_text(nextion, 0, 2);
  NextionComponent kettleIst2_text(nextion, 0, 12);
  NextionComponent kettleSoll2_text(nextion, 0, 16);
  NextionComponent kettleName3_text(nextion, 0, 3);
  NextionComponent kettleIst3_text(nextion, 0, 13);
  NextionComponent kettleSoll3_text(nextion, 0, 17);
  NextionComponent kettleName4_text(nextion, 0, 4);
  NextionComponent kettleIst4_text(nextion, 0, 14);
  NextionComponent kettleSoll4_text(nextion, 0, 18);
  NextionComponent progress(nextion, 0, 9);
  NextionComponent mqttDevice(nextion, 0, 20);
  NextionComponent notification(nextion, 0, 22);
  // KettlePage
  NextionComponent p1uhrzeit_text(nextion, 1, 3);
  NextionComponent p1current_text(nextion, 1, 4);
  NextionComponent p1remain_text(nextion, 1, 5);
  NextionComponent p1temp_text(nextion, 1, 1);
  NextionComponent p1target_text(nextion, 1, 2);
  NextionComponent p1progress(nextion, 1, 6);
  NextionComponent p1mqttDevice(nextion, 1, 8);
  NextionComponent p1notification(nextion, 1, 10);
  // InductionPage
  NextionComponent powerButton(nextion, 2, 3);
  NextionComponent p2uhrzeit_text(nextion, 2, 7);
  NextionComponent p2slider(nextion, 2, 1);
  NextionComponent p2temp_text(nextion, 2, 5);
  NextionComponent p2gauge(nextion, 2, 4);

// CraftbeerPi4 definitions

#define maxKettles 4
#define maxKettleSign 15
#define maxIdSign 23
#define maxSensorSign 23
#define maxStepSign 30
#define maxRemainSign 10
#define maxNotifySign 52
#define maxTempSign 10

#define maxTopicSigns 42
#define cbpi4steps_topic "cbpi/stepupdate/+"    // SmqhKAMS6Z6ExTj9wa7y68
#define cbpi4kettle_topic "cbpi/kettleupdate/+" // BGAEZHXmHUfT44SLNV2xbF
#define cbpi4sensor_topic "cbpi/sensordata/"    // mKdeC6LjHZmz9Sa2mVf5SV
#define cbpi4actor_topic "cbpi/actorupdate/+"
#define cbpi4notification_topic "cbpi/notification"

bool current_step = false;
struct Kettles
{
    char id[maxIdSign];
    char name[maxKettleSign];
    char current_temp[maxTempSign];
    char target_temp[maxTempSign];
    char sensor[maxSensorSign];
};
struct Kettles structKettles[maxKettles];

#define maxSteps 15
struct Steps
{
    char id[maxIdSign];
    char name[maxStepSign];
    char timer[maxRemainSign];
};
struct Steps structSteps[maxSteps];
int stepsCounter = 0;

char currentStepName[maxStepSign];     //= "no active step";
char currentStepRemain[maxRemainSign]; //= "0:00";
char nextStepName[maxStepSign];
char nextStepRemain[maxRemainSign];
bool activeBrew = false;

char notify[maxNotifySign]; //= "Waiting for data - start brewing";
int sliderval = 0;
char uhrzeit[6] = "00:00";

// PID modes
bool pidMode = false;
bool ids2AutoTune = false;
bool hltAutoTune = false;
// mash button states
bool statePower = false;
bool statePause = false;
bool statePlay = false;

const String rules_names[numberOfRules] = { "INDIVIDUAL_PID", "BREWING_20L", "BREWING_50L", "HLT_PID", "ZIEGLER_NICHOLS_PID", "ZIEGLER_NICHOLS_PI", "INTEGRAL_PID", "SOME_OVERSHOOT_PID", "NO_OVERSHOOT_PID", "TYREUS_LUYBEN_PID", "TYREUS_LUYBEN_PI", "CIANCONE_MARLIN_PID", "CIANCONE_MARLIN_PI" };
// const String rules_names[numberOfRules] = { "INDIVIDUAL_PID", "BREWING_20L", "BREWING_50L", "HLT_PID" };    
// APID controller
double ids2Input, ids2Output, ids2Setpoint;
double hltInput, hltOutput, hltSetpoint;

INNU_APID ids2PID(&ids2Input, &ids2Output, &ids2Setpoint, 0, 0, 0);
INNU_APID hltPID(&hltInput, &hltOutput, &hltSetpoint, 0, 0, 0);

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
bool chartVis = true;
bool toastVis = true;

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
void PCF_Reset()
{
    //   Serial.println("*** SYSINFO: PCF8574 I2C reset");
    pinMode(D5, OUTPUT); // remove output low
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    delay(10);
    pinMode(D5, INPUT); // and make SDA high i.e. send I2C STOP control.
}
