// Display Nextion
bool useDisplay = false;
int startPage = 0;  // Startseite: BrewPage = 0 Kettlepage = 1 InductionPage = 2
int activePage = 0; // die aktuell angezeigte Seite
int tempPage = -1; // die aktuell angezeigte Seite

#define NUMBEROFPAGES 3

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
