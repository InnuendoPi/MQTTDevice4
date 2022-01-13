# NextionX2
A new, alternative and universal library to interact with Nextion HMI displays from Arduino and compatible MCUs. The library is mainly based on Thierry's [NextionX](https://github.com/ITEAD-Thierry/NextionX) library and the [EasyNextionLibrary](https://github.com/Seithan/EasyNextionLibrary) from Seithan (return methods).

## General information
To be most universal, this library allows (in opposite to the official library) the use of multiple Nextion HMI displays connected to the same MCU under the condition to have enough hardware or software emulated serial ports (UARTs). 

On an Arduino MEGA, you could for example use the Serial1, Serial2 and Serial3 ports to connect up to 3 Nextion HMIs, while keeping the default Serial port free for debugging in the Serial Monitor of the Arduino IDE. On an Arduino UNO, you might use either the Nextion on Serial and no debugging or the Nextion on a SoftwareSerial port and use Serial for debugging.

The library is written without the use of dynamic memory allocation functions like String to avoid defragmentation of the heap.


## Example

In examples you will find a NextionX2.hmi file. This file file was created for an 3.5 Nextion Enhanced display but can simply modified with the Nextion editor.

![NextionX2 .hmi file](https://github.com/sstaub/NextionX2/blob/main/images/NextionX2.png?raw=true)
NextionX2 HMI demo file

The Nextionx2.ino shows the advantages of the library. The example use the SoftwareSerial libray, so the example can run on an Arduino UNO.

```cpp
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
  if (digitalRead(LED_BUILTIN)) ledOff();
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

```

# Documentation

## Object Constructor *NextionComPort*
```cpp
NextionComPort
```

Creates a display object, this can used for multiple displays

**Example**

```cpp
NextionComPort nextion;
```

## Object Constructor *NextionComponent*

```cpp
NextionComponent(NexComm_t &nexComm, uint8_t pageId, uint8_t objectId)
```
- **&nexComm** pointer to NextionComPort you want to use for
- **pageId** the page ID number for the component
- **objectId** the object ID number for the component

Creates a component object

**Example**

```cpp
NextionComponent text(nextion, 0, 7);
```

## Methods for *NextionComPort*

### begin()
```cpp
void begin(nextionSeriaType &nextionSerial, uint16_t baud = 9600)
```
- **&nextionSerial** pointer to Serial object you want to use
- **baud** the baud rate, standard is 9600


Method to initialize the communication<br>
This must done in the Arduino ```setup()``` function

**Example**

```cpp
void setup() {
  nextion.begin(softSerial); // for use with Softserial
  nextion.begin(Serial1); // for use with Serial1 e.g. Arduino MEGA
  }
```

### debug()
```cpp
void begin(nextionSeriaType &debugSerial, uint16_t baud = 9600)
```
- **&debugSerial** pointer to Serial object you want to use
- **baud** the baud rate, standard is 9600


Method to initialize the communication for simple debugging<br>
This must done in the Arduino ```setup()``` function

**Example**

```cpp
void setup() {
  nextion.begin(softSerial);
  nextion.debug(Serial);
  }
```

### update()
```cpp
void update()
```

This must done in the Arduino ```loop()``` function

**Example**

```cpp
void loop() {
  nextion.update();
  }
```

### command()
```cpp
void command(const char *cmd)
```
- **cmd** command string

Send a raw command to the display

**Example**

```cpp
nextion.command("cir 50,50,20,WHITE");
```

## Methods for *NextionComponent*

### touch()
```cpp
void touch(void (*onTouch)())
```
- ***onTouch** callback function

Add a callback function for the touch event

**Example**

```cpp
momentaryButton.touch(ledOn);
```

### release()
```cpp
void release(void (*onRelease)())
```
- ***onTouch** callback function

Add a callback function for the release event

**Example**

```cpp
momentaryButton.release(ledOff);
```

### attribute()
```cpp
void attribute(const char *attr, int32_t number)
void attribute(const char *attr, const char *text)
```
- **attr** attribute as a string
- **number** argument value
- **text** argument text

Set an attribute with a value or text

**Example**

```cpp
number.attribute("val", 5);
text.attribute("txt", "v.1.0.0");
```

### value()
```cpp
void value(int32_t number)
```
- **number** argument value

Set a value

**Example**

```cpp
number.value(5);
```

### text()
```cpp
void text(const char* txt)
```
- **text** argument text

Set a text

**Example**

```cpp
text.text("hello");
```

## Return Methods for *NextionComponent*

### attributeValue()
```cpp
int32_t attributeValue(const char *attr)
```
- **attr** attribute as a string

Returns the value of a component attribute, 0xFFFFFFFF if there are problems

**Example**

```cpp
int32_t valueNumber = number.attributeValue("val");
```

### attributeText()
```cpp
const char* attributeText(const char *attr)
```
- **attr** attribute as a string

Returns the text of a component attribute, "Error" if there are problems

**Example**

```cpp
char string[32];
strcpy(string, text.attributeValue("txt");
```

### value()
```cpp
int32_t value()
```
- **attr** attribute as a string

Returns the value ("val") of a component, 0xFFFFFFFF if there are problems

**Example**

```cpp
int32_t valueNumber = number.value();
```

### text()
```cpp
const char* attributeText(const char *attr)
```
- **attr** attribute as a string

Returns the text ("txt") of a component, "Error" if there are problems

**Example**

```cpp
char string[32];
strcpy(string, text.text();
```

## Graphic Methods for *NextionComPort*

### Graphic Enumarations for text objects

```cpp
enum fill_t { // background fill modes
	CROP,
	SOLID,
	IMAGE,
	NOFILL
};

enum alignhor_t { // horizontal alignment
	LEFT,
	CENTER,
	RIGHT
};

enum alignver_t { // vertical alignment
	TOP,
	MIDDLE,
	BOTTOM
}
```


### Colors

There are some colors predefined

```cpp
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
```

There is also helper function to convert RGB 8bit values to the 16bit 565 format used by Nextion.

```cpp
uint16_t color565(uint8_t red, uint8_t green, uint8_t blue)
```

### cls()
```cpp
void cls(uint16_t color)
```

Clears the complete screen with a given color

**Example**

```cpp
nextion.cls(BLACK);
```

### line()
```cpp
void line(uint16_t x1, uint16_t y1, int16_t x2, uint16_t y2, uint16_t color)
```
- **x1** start point x1
- **y1** start point y1
- **x2** end point x2
- **y1** end point y2
- **color** color in 565 16bit format

Draw a line

**Example**

```cpp
nextion.line(50, 50, 100, 100, RED);
```

### circle()
```cpp
void circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
```
- **x** center point x
- **y** center point y
- **radius** circle radius
- **color** color in 565 16bit format

Draw a circle

**Example**

```cpp
nextion.circle(200, 200, 50, BLUE);
```

### circleFilled()
```cpp
void circleFilled(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
```
- **x** center point x
- **y** center point y
- **radius** circle radius
- **color** color in 565 16bit format

Draw a filled circle

**Example**

```cpp
nextion.circleFilled(200, 200, 50, BLUE);
```

### rectangle()
```cpp
void rectangle(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color)
```
- **x** start point x
- **y** start point y
- **width** rectangle width
- **height** rectangle height
- **color** color in 565 16bit format

Draw a rectangle

**Example**

```cpp
nextion.rectangle(50, 50, 150, 50, YELLOW);
```

### rectangleFilled()
```cpp
void rectangleFilled(uint16_t x, uint16_t y, int16_t width, uint16_t height, uint16_t color)
```
- **x** start point x
- **y** start point y
- **width** rectangle width
- **height** rectangle height
- **color** color in 565 16bit format

Draw a filled rectangle

**Example**

```cpp
nextion.rectangleFilled(50, 50, 150, 50, YELLOW);
```

### text()
```cpp
void text(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t font, uint16_t colorfg, uint16_t colorbg, alignhor_t alignx, alignver_t aligny, fill_t fillbg, const char* text)
```
- **x** start point x
- **y** start point y
- **width** text area width
- **height** text area height
- **font** resource font number
- **colorfg** text foreground color in 565 16bit format
- **colorbg** text background color in 565 16bit format
- **alignx** text horizontal alignment LEFT / CENTER / RIGHT
- **aligny** text horizontal alignment TOP / MIDDLE / BOTTOM
- **fillbg** background fill mode CROP / SOLID / IMAGE / NONE
- **text** text string

Draw a filled rectangle

**Example**

```cpp
nextion.text(50, 280, 200, 50, 1, WHITE, BLUE, CENTER, MIDDLE, SOLID, "Hello Nextion");
```

### picture()
```cpp
void picture(uint16_t x, uint16_t y, uint8_t id)
```
- **x** upper left x coordinate
- **y** upper left y coordinate
- **id** resource picture id

Draw a picture

**Example**

```cpp
nextion.picture(320, 100, 0);
```

### pictureCrop()
```cpp
void pictureCrop(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t id)
```
- **x** upper left x coordinate
- **y** upper left y coordinate
- **width** crop width
- **height** crop height
- **id** resource picture id

Draw a croped picture, should only used with fullscreen picture!

**Example**

```cpp
nextion.pictureCrop(100, 100, 50, 50, 0);
```

### pictureCropX()
```cpp
void pictureCropX(uint16_t destx, uint16_t desty, uint16_t width, uint16_t height, uint16_t srcx, uint16_t srcy, uint8_t id)
```
- **destx** upper left x destination coordinate
- **desty** upper left y destination coordinate
- **width** crop width
- **height** crop height
- **srcx** upper left x source coordinate
- **srcy** upper left y source coordinate
- **id** resource picture id

Draw an extended croped picture

**Example**

```cpp
nextion.pictureCropX(320, 160, 50, 50, 0, 0, 0);
```