// Version
#define Version "4.60"

// Definiere Pausen
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750
#define RESOLUTION_HIGH 12 // steps 9bit: 0.5°C 10bit: 0.25°C 11bit: 0.125°C 12bit: 0.0625°C
#define RESOLUTION 11
#define TEMP_OFFSET1 40
#define TEMP_OFFSET2 78

// Update Intervalle für Ticker Objekte
#define SEN_UPDATE 1000  //  sensors update
#define ACT_UPDATE 2000  //  actors update
#define IND_UPDATE 2000  //  induction update
#define DISP_UPDATE 1000 //  display update
// Event handling Zeitintervall für Reconnects WLAN und MQTT
#define tickerMQTT 10000 // für Ticker Objekt MQTT in ms
#define tickerPUSUB 50   // Ticker PubSubClient

// System Dateien
#define UPDATESYS "/updateSys.txt"
#define LOGUPDATESYS "/updateSys.log"
#define UPDATETOOLS "/updateTools.txt"
#define LOGUPDATETOOLS "/updateTools.log"
#define UPDATELOG "/webUpdateLog.txt"
#define DEVBRANCH "/dev.txt"
#define CERT "/ce.rts"
#define CONFIG "/config.txt"

#ifdef ESP32
#define Aus -100
#define D0 26
#define D1 22
#define D2 21
#define D3 17
#define D4 16
#define D5 18
#define D6 19
#define D7 23
#define D8 5
#define D9 27
#define D10 25
#define D11 32
#define D12 12
#define D13 4
#define D14 0
#define D15 2
#define D16 33
#define D17 14
#define D18 15
#define D19 13
// #define D20 10

bool pins_used[33]; // GPIO
#define NUMBEROFPINS 21
static const int8_t pins[NUMBEROFPINS] = {26, 22, 21, 17, 16, 18, 19, 23, 5, 27, 25, 32, 12, 4, 0, 2, 33, 14, 15, 13, -100};
static const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19", "-"};
#elif ESP8266
#define NUMBEROFPINS 10
bool pins_used[17]; // GPIO
static const int8_t pins[NUMBEROFPINS] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, -100};
const String pin_names[NUMBEROFPINS] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "-"};
#endif

#define ONE_WIRE_BUS D3
int8_t PIN_BUZZER = D8;

// Sensoren
#define RREF1000 4300.0
#define RNOMINAL1000 1000.0
#define RREF100 430.0
#define RNOMINAL100 100.0

// Event für Sensoren
#define EM_OK 0    // Normal mode
#define EM_CRCER 1 // Sensor CRC failed
#define EM_DEVER 2 // Sensor device error
#define EM_UNPL 3  // Sensor unplugged
#define EM_SENER 4 // Sensor all errors

#ifdef ESP32
// PT100x
#define SPI_MOSI D11
#define SPI_MISO D10
#define SPI_CLK D9
#define CS0 D13
#define CS1 D16
#define CS2 D17
#define CS3 D18
#define CS4 D19
#define CS5 D8
Adafruit_MAX31865 pt_0 = Adafruit_MAX31865(CS0, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_1 = Adafruit_MAX31865(CS1, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_2 = Adafruit_MAX31865(CS2, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_3 = Adafruit_MAX31865(CS3, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_4 = Adafruit_MAX31865(CS4, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_5 = Adafruit_MAX31865(CS5, SPI_MOSI, SPI_MISO, SPI_CLK);
bool activePT_0 = false, activePT_1 = false, activePT_2 = false, activePT_3 = false, activePT_4 = false, activePT_5 = false;
#elif ESP8266
// PT100x  - default SPI MOSI: D7 (13) MISO: D6 (12) CLK: D5 (14) SS: D8 (15)
#define SPI_MOSI D0
#define SPI_MISO D1
#define SPI_CLK D2
#define CS0 D4
#define CS1 D5
#define CS2 D6
Adafruit_MAX31865 pt_0 = Adafruit_MAX31865(CS0, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_1 = Adafruit_MAX31865(CS1, SPI_MOSI, SPI_MISO, SPI_CLK);
Adafruit_MAX31865 pt_2 = Adafruit_MAX31865(CS2, SPI_MOSI, SPI_MISO, SPI_CLK);
bool activePT_0 = false, activePT_1 = false, activePT_2 = false;
#endif


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
#define cbpi4fermenter_topic "cbpi/fermenterupdate/+"
// #define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/YdceEBous3wSRUQsTx9ZLq/+"
// #define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/+/+"
#define cbpi4fermentersteps_topic "cbpi/fermenterstepupdate/+"

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

// Alarm codes
#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_INFO 3
#define ALARM_SUCCESS 4
#define ALARM_WARNING 5
#define ALARM_ERROR 6
