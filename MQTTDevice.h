// Version
#define Version "4.65.4"

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
#define ACT_UPDATE 1000  //  actors update
#define IND_UPDATE 1000  //  induction update
#define DISP_UPDATE 1000 //  display update
#define TIME_UPDATE 30000
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

bool pins_used[34]; // GPIO
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

// KettlePage
#define uhrzeit_text "clock.txt"
#define currentStepName_text "currentStep.txt"
#define currentStepRemain_text "currentRemain.txt"
#define nextStepName_text "nextStep.txt"
#define nextStepRemain_text "nextTimer.txt"
#define kettleName1_text "Kettle1.txt"
#define kettleIst1_text "temp1.txt"
#define kettleSoll1_text "target1.txt"
#define kettleName2_text "Kettle2.txt"
#define kettleIst2_text "temp2.txt"
#define kettleSoll2_text "target2.txt"
#define kettleName3_text "Kettle3.txt"
#define kettleIst3_text "temp3.txt"
#define kettleSoll3_text "target3.txt"
#define kettleName4_text "Kettle4.txt"
#define kettleIst4_text "temp4.txt"
#define kettleSoll4_text "target4.txt"
#define progress "j0.txt"
#define mqttDevice "IP.txt"
#define notification "notification1.txt"
// BrewPage
#define p1uhrzeit_text "p1clock.txt"
#define p1current_text "p1current.txt"
#define p1remain_text "p1remain.txt"
#define p1temp_text "p1temp.txt"
#define p1target_text "p1target.txt"
#define p1progress "j0.txt"
#define p1mqttDevice "IP.txt"
#define p1notification "p1notification.txt"
// InductionPage
#define powerButton "buttonOnOff.val"
#define p2uhrzeit_text "p2clock.txt"
#define p2slider "sliderPower.val"
#define p2temp_text "textTemp.txt"
#define p2gauge "gauge.val"

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

// Select
#define OPTIONSTART "<option>"
#define OPTIONEND "</option>"
#define OPTIONDISABLED "</option><option disabled>──────────</option>"
#define TRENNLINIE "-----------------------"