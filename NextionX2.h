#ifndef NEXTION_X2_H
#define NEXTION_X2_H

#include "Arduino.h"

#define MAX_BUFFER_LENGTH       14
#define MAX_LIST_LENGTH         24
#define ATTRIBUTE_TEXT_LENGTH	  48
#define ATTRIBUTE_TEXT_LENGTH_X 64
#define ATTRIBUTE_NUM_LENGTH    32
#define ATTRIBUTE_NUM_LENGTH_X  48
#define RECEIVE_STRING_LENGTH   32

#define TIMEOUT 100

// color definitions
#define BLACK      0x0000
#define BLUE       0x001F
#define RED        0xF800
#define GREEN      0x07E0
#define CYAN       0x07FF
#define MAGENTA    0xF81F
#define YELLOW     0xFFE0
#define LIGHT_GREY 0xBDF7
#define GREY       0x8430
#define DARK_GREY  0x4208
#define WHITE      0xFFFF

/**
 * @brief converts rgb to 16bit color in 656 format
 * 
 * @param red 
 * @param green 
 * @param blue 
 * @return uint16_t 16bit color
 */
uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
	red >>= 3;
	green >>= 2;
	blue >>= 3;
	return (red << 11) | (green << 5) | blue;
	}

/**
 * @brief converts an long integer number to an array
 * 
 * @param number 
 * @return const char* 
 */
const char* i32toa(int32_t number) {
	static char numstring[16];
	sprintf(numstring, "%ld", number);
	return numstring;
	}

enum fill_t {
	CROP,
	SOLID,
	IMAGE,
	NOFILL
	};

enum alignhor_t {
	LEFT,
	CENTER,
	RIGHT
	};

enum alignver_t {
	TOP,
	MIDDLE,
	BOTTOM
	};

/**
 * @brief Component Id declaration
 * 
 */
typedef union ComponentIdType {
	uint16_t guid;
	struct {
		uint8_t page;
		uint8_t object;
		};
	} componentId_t;

/**
 * @brief Forward declaration (needed for the reference in NextionComponent)
 * 
 */
class NextionComPort;

/**
 * @brief NextionComponent declaration
 * 
 */
class NextionComponent {

	public:

		/**
		 * @brief Construct a new Nex Comp object
		 * 
		 * @tparam NexComm_t 
		 */
		template <class NexComm_t>
		NextionComponent(NexComm_t &nexComm, uint8_t pageId, uint8_t objectId);

		/**
		 * @brief Set the Attr object
		 * 
		 * @param number 
		 */
		void attribute(const char *attr, int32_t number);

		/**
		 * @brief Set the Attr object
		 * 
		 * @param text 
		 */
		void attribute(const char *attr, const char *text);

		/**
		 * @brief returns the guid
		 * 
		 * @return uint16_t 
		 */
		uint16_t guid();

		/**
		 * @brief add a touch callback
		 * 
		 * @param callback function
		 */
		void touch(void (*onTouch)());

		/**
		 * @brief add a release callback
		 * 
		 * @param callback function
		 */
		void release(void (*onRelease)());

		/**
		 * @brief get the value of an object attribute
		 * 
		 * @param attribute 
		 * @return int32_t object atrribute value
		 */
		int32_t attributeValue(const char *attr);

		/**
		 * @brief get the text string of an object attribute
		 * 
		 * @param attribute 
		 * @return const char* object attribute text
		 */
		const char* attributeText(const char *attr);

		/**
		 * @brief get the value of the object
		 * 
		 * @return int32_t value
		 */
		int32_t value();
		
		/**
		 * @brief set the value of the object
		 * 
		 * @param number object value, 0xFFFFFFFF if not defined
		 */
		void value(int32_t number);

		/**
		 * @brief get the text of the object
		 * 
		 * @return const char* text
		 */
		const char* text();

		/**
		 * @brief set the text of the object
		 * 
		 * @param txt object text
		 */
		void text(const char* txt);

		/**
		 * @brief event callback
		 * 
		 * @param event 
		 */
		void callback(uint8_t event);

	private:
		NextionComPort *nexComm;
		componentId_t myId;
		void (*onTouch)() = nullptr;
		void (*onRelease)() = nullptr;
	};

/**
 * @brief List Elements declaration
 * 
 */
typedef struct ListElement {
	uint16_t guid;
	NextionComponent *component;
	} listElement_t;

/**
 * @brief NextionComPort declaration
 * 
 */
class NextionComPort {

	public:

		int currentPageID;
		
		/**
		 * @brief Construct a new Nex Comm object
		 * 
		 */
		NextionComPort();

		/**
		 * @brief start a serial com port
		 * 
		 * @tparam nextionSeriaType 
		 * @param baud serial baudrate, default 9600
		 */
		template <class nextionSeriaType>
		void begin(nextionSeriaType &nextionSerial, uint16_t baud = 9600);

		/**
		 * @brief start a debug serial com port
		 * 
		 * @tparam debugSerialType 
		 * @param baud serial baudrate, default 9600
		 */
		template <class debugSerialType>
		void debug(debugSerialType &debugSerial, uint16_t baud = 9600);

		/**
		 * @brief send a command string
		 * 
		 * @param cmd command string
		 */
		void command(const char *cmd);

		/**
		 * @brief update the event loop
		 * 
		 */
		void update();

		/**
		 * @brief clear screen with a given color
		 * 
		 * @param color 
		 */
		void cls(uint16_t color);

		/**
		 * @brief draw a line
		 * 
		 * @param x1 
		 * @param y1 
		 * @param x2 
		 * @param y2 
		 * @param color 
		 */
		void line(uint16_t x1, uint16_t y1, int16_t x2, uint16_t y2, uint16_t color);

		/**
		 * @brief draw a circle
		 * 
		 * @param x 
		 * @param y 
		 * @param diameter 
		 * @param color 
		 */
		void circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color);

		/**
		 * @brief draw a filled circle
		 * 
		 * @param x 
		 * @param y 
		 * @param diameter 
		 * @param color 
		 */
		void circleFilled(uint16_t x, uint16_t y, uint16_t radius, uint16_t color);

		/**
		 * @brief draw a rectangle
		 * 
		 * @param x 
		 * @param y 
		 * @param width 
		 * @param height 
		 * @param color 
		 */
		void rectangle(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color);

		/**
		 * @brief draw a filled rectangle
		 * 
		 * @param x 
		 * @param y 
		 * @param width 
		 * @param height 
		 * @param color 
		 */
		void rectangleFilled(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color);

		/**
		 * @brief draw a text
		 * 
		 * @param x upper left x coordinate
		 * @param y upper left y coordinate
		 * @param width of text area
		 * @param height height of text area
		 * @param font resource font number
		 * @param colorfg foreground color
		 * @param colorbg background color
		 * @param alignx horizontal alignment LEFT, CENTER, RIGHT
		 * @param aligny vertical alignment TOP, CENTER, BOTTOM
		 * @param fillbg background fill mode CROP, SOLID, IMAGE, NOFILL
		 * @param text string content
		 */
		void text(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t colorfg, uint16_t colorbg, alignhor_t alignx, alignver_t aligny, fill_t fillbg, const char* text);

		/**
		 * @brief draw a picture
		 * 
		 * @param x upper left x coordinate
		 * @param y upper left y coordinate
		 * @param id resource picture id
		 */
		void picture(uint16_t x, uint16_t y, uint8_t id);

		/**
		 * @brief draw a croped picture
		 * 
		 * @param x upper left x coordinate
		 * @param y upper left y coordinate
		 * @param width new width of the picture
		 * @param height new height of the picture
		 * @param id resource picture id
		 */
		void pictureCrop(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t id);

		/**
		 * @brief draw a advanced croped picture
		 * 
		 * @param destx upper left x coordinate of desination
		 * @param desty upper left y coordinate of desination
		 * @param width new width of the picture
		 * @param height new height of the picture
		 * @param srcx upper left x coordinate of source
		 * @param srcy upper left y coordinate of source
		 * @param id resource picture id
		 */
		void pictureCropX(uint16_t destx, uint16_t desty, uint16_t width, uint16_t height, uint16_t srcx, uint16_t srcy, uint8_t id);

	protected:
		void addComponentList(NextionComponent *);
		uint8_t inputString[MAX_BUFFER_LENGTH] = {0};
		uint8_t length = 0;
		Stream *nextionSerial = nullptr;
		Stream *debugSerial = nullptr;

	private:
		void dbgLoop();
		uint8_t readNextionReturn();
		int32_t nextionValue();
		const char* nextionText();
		uint8_t indexByGuid(uint16_t guid);
		uint8_t inputPointer = 0;
		uint8_t counterFF = 0;
		uint8_t lastPointer = 0;
		listElement_t lastList[MAX_LIST_LENGTH];

		friend NextionComponent;
	};

template <class NexComm_t>
NextionComponent::NextionComponent(NexComm_t &nexComm, uint8_t pageId, uint8_t objectId) : nexComm(&nexComm) {
	myId.page = pageId;
	myId.object = objectId;
	}

void NextionComponent::attribute(const char *attr, int32_t number) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "p[");
	strcat(commandString, i32toa(myId.page));
	strcat(commandString, "].b[");
	strcat(commandString, i32toa(myId.object));
	strcat(commandString, "].");
	strcat(commandString, attr);
	strcat(commandString, "=");
	strcat(commandString, i32toa(number));
	nexComm->command(commandString);
	}

void NextionComponent::attribute(const char *attr, const char *text) {
	char commandString[ATTRIBUTE_TEXT_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "p[");
	strcat(commandString, i32toa(myId.page));
	strcat(commandString, "].b[");
	strcat(commandString, i32toa(myId.object));
	strcat(commandString, "].");
	strcat(commandString, attr);
	strcat(commandString, "=\"");
	strcat(commandString, text);
	strcat(commandString, "\"");
	nexComm->command(commandString);
	}

uint16_t NextionComponent::guid() {
	return myId.guid;
	}

void NextionComponent::touch(void (*onTouch)() = nullptr) {
	this->onTouch = onTouch;
	nexComm->addComponentList(this);
	}

void NextionComponent::release(void (*onRelease)() = nullptr) {
	this->onRelease = onRelease;
	nexComm->addComponentList(this);
	}

int32_t NextionComponent::attributeValue(const char *attr) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "get p[");
	strcat(commandString, i32toa(myId.page));
	strcat(commandString, "].b[");
	strcat(commandString, i32toa(myId.object));
	strcat(commandString, "].");
	strcat(commandString, attr);
	nexComm->command(commandString);
	return nexComm->nextionValue();
	}

const char* NextionComponent::attributeText(const char *attr) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "get p[");
	strcat(commandString, i32toa(myId.page));
	strcat(commandString, "].b[");
	strcat(commandString, i32toa(myId.object));
	strcat(commandString, "].");
	strcat(commandString, attr);
	nexComm->command(commandString);
	return nexComm->nextionText();
	}

int32_t NextionComponent::value() {
	return attributeValue("val");
	}

void NextionComponent::value (int32_t number) {
	attribute("val", number);
	}

const char* NextionComponent::text() {
	return attributeText("txt");
	}

void NextionComponent::text(const char* txt) {
	attribute("txt", txt);
	}

void NextionComponent::callback(uint8_t event) {
	switch (event) {
		case 0:
			if (onRelease != nullptr) onRelease();
			break;
		case 1:
			if (onTouch != nullptr) onTouch();
			break;
		}
	}


NextionComPort::NextionComPort() {}

template <class nextionSeriaType>
void NextionComPort::begin(nextionSeriaType &nextionSerial, uint16_t baud) {
	nextionSerial.begin(baud);
	delay(100);
	this->nextionSerial = &nextionSerial;
	command("");
	command("bkcmd=0");
	}

template <class debugSerialType>
void NextionComPort::debug(debugSerialType &debugSerial, uint16_t baud) {
	debugSerial.begin(baud);
	delay(100);
	this->debugSerial = &debugSerial;
	command("bkcmd=3");
	}

void NextionComPort::command(const char *cmd) {
	update();
	nextionSerial->print(cmd); 
	nextionSerial->print("\xFF\xFF\xFF");
	if ((debugSerial != nullptr) && (strlen(cmd) > 0)) {
		debugSerial->write("Command ");
		debugSerial->println(cmd);
		debugSerial->println();
		}
	}

void NextionComPort::update() {
	componentId_t component;
	length = readNextionReturn();
	if ((length > 0) && (debugSerial != nullptr)) dbgLoop();
	if (inputString[0] == 0x23) currentPageID = inputString[1];
	if ((length == 4) && (inputString[0] == 0x65)) {
		component.page = inputString[1];
		component.object = inputString[2];
		uint8_t listpos = indexByGuid(component.guid);
		if (listpos < MAX_LIST_LENGTH) lastList[listpos].component->callback(inputString[3]);
		}
	}

void NextionComPort::addComponentList(NextionComponent *component) {
	listElement_t myComponent;
	myComponent.component = component;
	myComponent.guid = component->guid();
	if (indexByGuid(myComponent.guid) == 0xFF) { lastList[lastPointer++] = myComponent;
		if (lastPointer >= MAX_LIST_LENGTH) lastPointer = 0;
		}
	}

void NextionComPort::dbgLoop() {
	if ((length == 1) && ((inputString[0] < 0x25) || (inputString[0] > 0x85))) {
		if (inputString[0] == 1) debugSerial->write("Success\n");
		else if (inputString[0] < 0x25) {
			debugSerial->write("Error ");
			debugSerial->println(inputString[0], HEX);
			debugSerial->println();
			}
		else if (inputString[0] > 0x85) {
			debugSerial->write("Status ");
			debugSerial->println(inputString[0], HEX);
			debugSerial->println();
			}
		}
	else if ((length == 4) && (inputString[0] == 0x65)) {
		if (inputString[3] == true) debugSerial->write("Touch");
		else debugSerial->write("Release");
		debugSerial->write(" page ");
		debugSerial->print(inputString[1], DEC);
		debugSerial->write(" object ");
		debugSerial->println(inputString[2], DEC);
		debugSerial->println();
		}
	}

uint8_t NextionComPort::readNextionReturn() {
	uint8_t messageLength = 0;
	while (nextionSerial->available() && counterFF < 3) {
		uint8_t inputByte = nextionSerial->read();
		inputString[inputPointer++] = inputByte;
		if (inputByte == 255) counterFF++;
		else {
			counterFF = 0;
			if (inputPointer > (MAX_BUFFER_LENGTH - 3)) inputPointer = MAX_BUFFER_LENGTH - 3;
			}
		}
	if (counterFF == 3) {
		messageLength = inputPointer - 3;
		counterFF = 0;
		inputPointer = 0;
		}
	return messageLength;
	}

int32_t NextionComPort::nextionValue() {
	bool endOfCommandFound;
	bool error = false;
	uint8_t tempChar = 0;
	uint8_t buffer[4];
	memset(buffer, 0, sizeof(buffer));
	int32_t value = 0;
	uint8_t inputByte = 0;
	uint32_t  timer = millis();  
	while (nextionSerial->available() < 8) {
		if ((millis() - timer) > TIMEOUT) {
			error = true;
	 		break;
			}
		}
	if (nextionSerial->available() > 7) {
		inputByte = nextionSerial->read();
		timer = millis();
		while (inputByte != 0x71) {
			if (nextionSerial->available()) inputByte = nextionSerial->read();
			if((millis() - timer) > TIMEOUT) {
				error = true;
				break;
				}   
			}
		if (inputByte == 0x71) {
			for (int i = 0; i < 4; i++) buffer[i] = nextionSerial->read();
			int endBytes = 0;
			timer = millis();
			while (endOfCommandFound == false) {
				tempChar = nextionSerial->read();
				if (tempChar == 0xFF) {
					endBytes++ ;
					if (endBytes == 3) endOfCommandFound = true;
					}
				else {
					error = true;
					break;
					}
				if ((millis() - timer) > TIMEOUT){
					error = true;
					break;
					}
				}
			}
		}
	if (endOfCommandFound == true) {
		value = buffer[3];
		value <<= 8;
		value |= buffer[2];
		value <<= 8;
		value |= buffer[1];
		value <<= 8;
		value |= buffer[0];
		}
	else error = true;
	if (error) value = 0xFFFFFFFF;
	return value;
	}

const char* NextionComPort::nextionText() {
	bool endOfCommandFound = false;
	bool error = false;
	uint8_t tempChar = 0;
	static char buffer[RECEIVE_STRING_LENGTH];
	memset(buffer, 0, sizeof(buffer));
	uint8_t inputByte;
	uint32_t  timer = millis();  
	while (nextionSerial->available() < 4) {
		if ((millis() - timer) > TIMEOUT){
			error = true;
	 		break;
			}
		}
	if (nextionSerial->available() > 3) {
		inputByte = nextionSerial->read();
		timer = millis();
		while (inputByte != 0x70) {
			if (nextionSerial->available()) inputByte = nextionSerial->read();
			if((millis() - timer) > TIMEOUT) {
				error = true;
				break;
				}   
			}
		if (inputByte == 0x70) {
			int endBytes = 0;
			timer = millis();
			while (endOfCommandFound == false) {
				if (nextionSerial->available()) {
					tempChar = nextionSerial->read();
					if (tempChar == 0xFF) {
						endBytes++ ;
						if (endBytes == 3) endOfCommandFound = true;
						}
					else {
						char tempString[2];
  					tempString[0] = tempChar;
  					tempString[1] = 0;
						strcat(buffer, tempString);
						}
					}
				if ((millis() - timer) > TIMEOUT){
					error = true;
					break;
					}
				}
			}
		}
	if (error) strcpy(buffer, "Error");
	return buffer;
	}

uint8_t NextionComPort::indexByGuid(uint16_t guid) {
	uint8_t returnValue = 0xFF;
	for (uint8_t i = 0; i < MAX_LIST_LENGTH; i++) {
		if (lastList[i].guid == guid) {
			returnValue = i;
			break;
			}
		}
		return returnValue;
	}

void NextionComPort::cls(uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "cls ");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::line(uint16_t x1, uint16_t y1, int16_t x2, uint16_t y2, uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "line ");
	strcat(commandString, i32toa(x1));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y1));
	strcat(commandString, ",");
	strcat(commandString, i32toa(x2));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y2));
	strcat(commandString, ",");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::rectangle(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "draw ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(x + width));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y + height));
	strcat(commandString, ",");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::rectangleFilled(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "fill ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(width));
	strcat(commandString, ",");
	strcat(commandString, i32toa(height));
	strcat(commandString, ",");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "cir ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(radius));
	strcat(commandString, ",");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::circleFilled(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "cirs ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(radius));
	strcat(commandString, ",");
	strcat(commandString, i32toa(color));
	command(commandString);
	}

void NextionComPort::text(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t colorfg, uint16_t colorbg, alignhor_t alignx, alignver_t aligny, fill_t fillbg, const char* text) {
	char commandString[ATTRIBUTE_TEXT_LENGTH_X];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "xstr ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(width));
	strcat(commandString, ",");
	strcat(commandString, i32toa(height));
	strcat(commandString, ",");
	strcat(commandString, i32toa(font));
	strcat(commandString, ",");
	strcat(commandString, i32toa(colorfg));
	strcat(commandString, ",");
	strcat(commandString, i32toa(colorbg));
	strcat(commandString, ",");
	strcat(commandString, i32toa(alignx));
	strcat(commandString, ",");
	strcat(commandString, i32toa(aligny));
	strcat(commandString, ",");
	strcat(commandString, i32toa(fillbg));
	strcat(commandString, ",");
	strcat(commandString, "\"");
	strcat(commandString, text);
	strcat(commandString, "\"");
	command(commandString);
	}

void NextionComPort::picture(uint16_t x, uint16_t y, uint8_t id) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "pic ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(id));
	command(commandString);
	}

void NextionComPort::pictureCrop(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t id) {
	char commandString[ATTRIBUTE_NUM_LENGTH];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "picq ");
	strcat(commandString, i32toa(x));
	strcat(commandString, ",");
	strcat(commandString, i32toa(y));
	strcat(commandString, ",");
	strcat(commandString, i32toa(width));
	strcat(commandString, ",");
	strcat(commandString, i32toa(height));
	strcat(commandString, ",");
	strcat(commandString, i32toa(id));
	command(commandString);
	}

void NextionComPort::pictureCropX(uint16_t destx, uint16_t desty, uint16_t width, uint16_t height, uint16_t srcx, uint16_t srcy, uint8_t id) {
	char commandString[ATTRIBUTE_NUM_LENGTH_X];
	memset(commandString, 0, sizeof(commandString));
	strcat(commandString, "xpic ");
	strcat(commandString, i32toa(destx));
	strcat(commandString, ",");
	strcat(commandString, i32toa(desty));
	strcat(commandString, ",");
	strcat(commandString, i32toa(width));
	strcat(commandString, ",");
	strcat(commandString, i32toa(height));
	strcat(commandString, ",");
	strcat(commandString, i32toa(srcx));
	strcat(commandString, ",");
	strcat(commandString, i32toa(srcy));
	strcat(commandString, ",");
	strcat(commandString, i32toa(id));
	command(commandString);
	}

#endif
