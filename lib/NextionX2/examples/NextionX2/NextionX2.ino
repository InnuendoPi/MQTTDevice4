// Example for Arduino UNO

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "NextionX2.h"

SoftwareSerial softSerial(2, 3);

NextionComPort nextion;
NextionComponent version(nextion, 0, 1);
NextionComponent number(nextion, 0, 6);
NextionComponent text(nextion, 0, 7);
NextionComponent momentaryButton(nextion, 0, 2);
NextionComponent toggleButton(nextion, 0, 4);
NextionComponent slider(nextion, 0, 8);
NextionComponent checkbox(nextion, 0, 9);
NextionComponent xfloat(nextion, 0, 5);

uint32_t time;
#define TIMER 2000

void ledOn() {
	digitalWrite(LED_BUILTIN, HIGH);
	nextion.rectangleFilled(250, 150, 50, 50, RED);
	}

void ledOff() {
	digitalWrite(LED_BUILTIN, LOW);
	nextion.rectangleFilled(250, 150, 50, 50, BLACK);
	}

void ledToggle() {
	if(digitalRead(LED_BUILTIN)) ledOff();
	else ledOn();
	}

void setup() {
	nextion.begin(softSerial);
	//nextion.debug(Serial); // uncomment for debug
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	version.attribute("txt", "v.1.0.0");
	number.value(5);
	text.text("hello");
	nextion.text(50, 280, 200, 50, 1, WHITE, BLUE, CENTER, MIDDLE, SOLID, "Hello Nextion");
	nextion.picture(320, 100, 0);
	nextion.pictureCropX(320, 160, 50, 50, 0, 0, 0);
	momentaryButton.touch(ledOn);
	momentaryButton.release(ledOff);
	toggleButton.touch(ledToggle);
	time = millis();
	}

void loop() {
	nextion.update();
	if (millis() > time + TIMER) {
		char string[64];
		strcpy(string, text.text());
		int32_t valueNumber = number.value();
		int32_t valueSlider = slider.value();
		Serial.print("Text Field String: ");
		Serial.println(string);
		Serial.print("Number Field Value: ");
		Serial.println(valueNumber);
		Serial.print("Slider Value: ");
		Serial.println(valueSlider);
		Serial.println();
		time = millis();
		}
	}
