//    Erstellt:	2022
//    Author:	Innuendo

//    Sketch für ESP8266
//    Kommunikation via MQTT mit CraftBeerPi v4

//    Unterstützung für DS18B20 Sensoren
//    Unterstützung für GPIO Aktoren
//    Unterstützung für GGM IDS2 Induktionskochfeld
//    Unterstützung für Web Update
//    Visulaisierung über Grafana
//    Unterstützung für Nextion Touchdisplay

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <ESP8266WiFi.h>       // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>  // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h> // WiFiManager zur Einrichtung
#include <DNSServer.h>   // Benötigt für WiFiManager
#include "LittleFS.h"
#include <ArduinoJson.h>  // Lesen und schreiben von JSON Dateien 6.18
#include <ESP8266mDNS.h>  // mDNS
#include <WiFiUdp.h>      // WiFi
#include <EventManager.h> // Eventmanager
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include "InnuTicker.h"       // Bibliothek für Hintergrund Aufgaben (Tasks)
#include <PubSubClient.h>     // MQTT Kommunikation 2.8.0
#include <CertStoreBearSSL.h> // WebUpdate
#include <SoftwareSerial.h>
#include <NextionX2.h>
#include "index_htm.h"
#include "edit_htm.h"
#include "mash_htm.h"
#include <FS.h>
#include <PID_v2.h>
#include <sTune.h>
#include <PCF8574.h>

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
#define Version "4.31"

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750
#define RESOLUTION 12 // 12bit resolution == 750ms update rate
#define TEMP_OFFSET1 40
#define TEMP_OFFSET2 78

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// I2C Port expander
PCF8574 pcf8574(0x20);

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient pubsubClient(espClient);
ESP8266HTTPUpdateServer httpUpdate;
MDNSResponder mdns;

// // Induktion Signallaufzeiten
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
unsigned char PWR_STEPS[6] = {0, 20, 40, 60, 80, 100};                                                     // Prozentuale Abstufung zwischen den Stufen

bool useI2C = false;
// bool pins_used[17]; // GPIO
// const unsigned char numberOfPins = 9;
// const unsigned char pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
// const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

// bool ppins_used[8]; // GPIO
// const unsigned char pnumberOfPins = 8;
// const unsigned char ppins[pnumberOfPins] = {P0, P1, P2, P3, P4, P5, P6, P7};
// const String ppin_names[pnumberOfPins] = {"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7"};
#define D9 17
#define D10 18
#define D11 19
#define D12 20
#define D13 21
#define D14 22
#define D15 23
#define D16 24
#define ALLPINS 17
#define GPIOPINS 9
#define PCFPINS 8
bool pins_used[25]; // GPIO
unsigned char numberOfPins = ALLPINS;
const unsigned char pins[ALLPINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16};
const String pin_names[ALLPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9*", "D10*", "D11*", "D12*", "D13*", "D14*", "D15*", "D16*"};

// Variablen
unsigned char numberOfSensors = 0; // Gesamtzahl der Sensoren
#define numberOfSensorsMax 6       // Maximale Anzahl an Sensoren
unsigned char addressesFound[numberOfSensorsMax][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0; // Gesamtzahl der Aktoren
#define numberOfActorsMax 10       // Maximale Anzahl an Aktoren
#define maxHostSign 15
#define maxUserSign 10
#define maxPassSign 10
char mqtthost[maxHostSign];                // MQTT Server
char mqttuser[maxUserSign];
char mqttpass[maxPassSign];
int mqttport = 1883;
char mqtt_clientid[maxHostSign];           // AP-Mode und Gerätename
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
#define EM_WEB 21
#define EM_MQTT 22
#define EM_MDNS 24
#define EM_NTP 25
#define EM_MDNSET 26
#define EM_MQTTCON 27
#define EM_MQTTSUB 28
#define EM_SETNTP 29
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
bool StopOnMQTTError = false;     // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry; // Zeitstempel bei Fehler MQTT
unsigned long wlanconnectlasttry; // Zeitstempel bei Fehler WLAN
bool mqtt_state = true;           // Status MQTT
bool devBranch = false;           // Check out development branch
bool mqttoff = false;             // Disable MQTT
// bool prevStateMQTT = false;

// Event handling Zeitintervall für Reconnects WLAN und MQTT
#define tickerWLAN 30000 // für Ticker Objekt WLAN in ms
#define tickerMQTT 30000 // für Ticker Objekt MQTT in ms
#define tickerPUSUB 300  // delay für PubSubClient

// Event handling Standard Verzögerungen
unsigned long wait_on_error_mqtt = 120000;             // How long should device wait between tries to reconnect WLAN      - approx in ms
unsigned long wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
unsigned long wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Ticker Objekte
InnuTicker TickerSen;
InnuTicker TickerInd;
InnuTicker TickerHlt;
InnuTicker TickerMQTT;
InnuTicker TickerPUBSUB;
InnuTicker TickerWLAN;
InnuTicker TickerNTP;
InnuTicker TickerDisp;
InnuTicker TickerMash;
InnuTicker TickerPID;
InnuTicker TickerHltPID;

// Update Intervalle für Ticker Objekte
int SEN_UPDATE = 4000; //  sensors update
int IND_UPDATE = 2000; //  sensors update
#define HLT_UPDATE 2000 //  sensors update
#define DISP_UPDATE 1000

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
enum { MSG_OK, CUSTOM, NOT_FOUND, BAD_REQUEST, ERROR };
#define TEXT_PLAIN "text/plain"
#define FS_INIT_ERROR "FS INIT ERROR"
#define FILE_NOT_FOUND "FileNotFound"

#define maxKettles 4
#define maxKettleSign 15
#define maxIdSign 23
#define maxSensorSign 23    
#define maxStepSign 30      
#define maxRemainSign 10
#define maxNotifySign 52
#define maxTempSign 10

bool useDisplay = false;
int startPage = 0;      // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
int activePage = 0;     // die aktuell angezeigte Seite

const unsigned char numberOfPages = 3;
const String page_names[numberOfPages] = {"BrewPage", "KettlePage", "InductionPage"};
#define maxTopicSigns 42

// char cbpi4steps_topic[maxTopicSigns] = "cbpi/stepupdate/+";     // SmqhKAMS6Z6ExTj9wa7y68
// char cbpi4kettle_topic[maxTopicSigns] = "cbpi/kettleupdate/+";  // BGAEZHXmHUfT44SLNV2xbF
// char cbpi4sensor_topic[maxTopicSigns] = "cbpi/sensordata/";     // mKdeC6LjHZmz9Sa2mVf5SV
// char cbpi4actor_topic[maxTopicSigns] = "cbpi/actorupdate/+";
// char cbpi4notification_topic[maxTopicSigns] = "cbpi/notification";

#define cbpi4steps_topic "cbpi/stepupdate/+"     // SmqhKAMS6Z6ExTj9wa7y68
#define cbpi4kettle_topic "cbpi/kettleupdate/+"  // BGAEZHXmHUfT44SLNV2xbF
#define cbpi4sensor_topic "cbpi/sensordata/"     // mKdeC6LjHZmz9Sa2mVf5SV
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

#define maxSteps 12
struct Steps
{
    char id[maxIdSign];
    char name[maxStepSign];
    char timer[maxRemainSign];
};
struct Steps structSteps[maxSteps];
int stepsCounter = 0;

char currentStepName[maxStepSign];      //= "no active step";
char currentStepRemain[maxRemainSign];  //= "0:00";
char nextStepName[maxStepSign];
char nextStepRemain[maxRemainSign];
bool activeBrew = false;

char notify[maxNotifySign]; //= "Waiting for data - start brewing";
int sliderval = 0;
char uhrzeit[6] ="00:00";

// SoftwareSerial softSerial(D1, D2);
SoftwareSerial softSerial;
NextionComPort nextion;

// Steuerung Buttons
NextionComponent page0(nextion, 0, 0);
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

#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_INFO 3
#define ALARM_SUCCESS 4
#define ALARM_WARNING 5
#define ALARM_ERROR 6

const int PIN_BUZZER = D8; // Buzzer
bool startBuzzer = false;  // Aktiviere Buzzer
bool mqttBuzzer = false;   // MQTTBuzzer

// PID
#define PID_UPDATE 3000     // checkTemp and send newPower
#define RUN_PID 1000        // PID SetSampleTime

float ids2Kp = 2, ids2Ki = 5, ids2Kd = 1;
float ids2Input = 0, ids2Output = 0, ids2Setpoint = 0;

float hltKp = 2, hltKi = 5, hltKd = 1;
float hltInput = 0, hltOutput = 0, hltSetpoint = 78;

// PID_v2 ids2PID(Kp, Ki, Kd, PID::Direct);
//PID::P_On::Measurement
//PID_v2 ids2PID(&ids2Input, &ids2Output, &Setpoint, Kp, Ki, Kd, PID::Direction::Direct);
PID_v2 ids2PID(ids2Kp, ids2Ki, ids2Kd, PID::Direction::Direct);
PID_v2 hltPID(hltKp, hltKi, hltKd, PID::Direction::Direct);

// modes
bool pidMode = false;
bool ids2AutoTune = false;
bool hltAutoTune = false;
// button states
bool statePower = false;
bool statePause = false;
bool statePlay = false;
// bool stateForward = false;

// autoTune user settings
uint32_t settleTimeSec = 10;
uint32_t testTimeSec = 500;
const uint16_t samples = 500;
const float inputSpan = 100;
const float outputSpan = 100;
float outputStart = 0;
float outputStep = 100;
float tempLimit = 75;

sTune tuner = sTune(&ids2Input, &ids2Output, tuner.Mixed_PID, tuner.directIP, tuner.printOFF);
sTune hltTuner = sTune(&hltInput, &hltOutput, tuner.Mixed_PID, tuner.directIP, tuner.printOFF);

// tuner.printDEBUG
// tuner.printSUMMARY
// tuner.direct5T

// Maischeplan
#define MASH_UPDATE 5000    // handleMash
#define maxSchritte 10
int maxActMashSteps = 0;    // maxArray
#define sizeImportMax 3072
#define sizeRezeptMax 1024
double pidDelta = 0.3;
String planResponse;
int actMashStep = 0;        // active mash step
struct Maischeplan
{
    String name;
    int duration;
    int temp;
    bool autonext;
};
struct Maischeplan structPlan[maxSchritte];

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
