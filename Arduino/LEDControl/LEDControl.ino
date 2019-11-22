/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

#include "Renderer.h"

#define MAX_LEDS      (55)          // Maximum number of LEDs per shelf; used to allocate memory
#define NUM_RGB       (55)           // Number of actual LEDs we have connected (or that we want active)
#define NUM_BYTES     (NUM_RGB*3)   // Number of LEDs (3 per each WS281X)
#define NUM_BITS      (8)           // Constant value: bits per byte

const unsigned int STARTUP_DELAY = 200UL;     // Wait time before enabling the relays
const uint8_t RELAY1_OUTPUT_ENABLE_PIN = 0; 
const uint8_t RELAY2_OUTPUT_ENABLE_PIN = 1; 

// Just for convenience when configuring digital outputs
const uint8_t digitalOutPins[] = 
{
    0, 1,           // Relays
    2, 3, 4, 5,     // Left shelves
    8, 9, 10, 11,   // Right shelves
    6, 7            // Center shelves
};
const int NUM_DIGITAL_OUT_PINS = sizeof(digitalOutPins) / sizeof(uint8_t);

// Generate the rendering functions. This is because I'm bad as assembly and don't want to figure out the asm constraints.
// So we'll generate a full rendering function for each pin. No biggie.
RENDER_FUNC(LEFT_1,     PORTD, PORTD2);
RENDER_FUNC(LEFT_2,     PORTD, PORTD3);
RENDER_FUNC(LEFT_3,     PORTD, PORTD4);
RENDER_FUNC(LEFT_4,     PORTD, PORTD5);
RENDER_FUNC(CENT_LEFT,  PORTD, PORTD6);
RENDER_FUNC(CENT_RIGHT, PORTD, PORTD7);
RENDER_FUNC(RIGHT_1,    PORTB, PORTB0);
RENDER_FUNC(RIGHT_2,    PORTB, PORTB1);
RENDER_FUNC(RIGHT_3,    PORTB, PORTB2);
RENDER_FUNC(RIGHT_4,    PORTB, PORTB3);

uint8_t colorData[NUM_SHELVES][MAX_LEDS];

Shelf shelves[] = 
{   
    //Shelf  #LEDs Rendering func name        Prog  Context                  Data pointer
    { LEFT_1,   NUM_RGB, RENDER_FUNC_NAME(LEFT_1),  NULL, {0, 0, 0, 0, 0, false},  colorData[LEFT_1]  },
    { LEFT_2,   NUM_RGB, RENDER_FUNC_NAME(LEFT_2),  NULL, {0, 0, 0, 0, 1, false},  colorData[LEFT_2]  },
    { LEFT_3,   NUM_RGB, RENDER_FUNC_NAME(LEFT_3),  NULL, {0, 0, 0, 0, 1, false},  colorData[LEFT_3]  },
    { LEFT_4,   NUM_RGB, RENDER_FUNC_NAME(LEFT_4),  NULL, {0, 0, 0, 0, 0, false},  colorData[LEFT_4]  },
    { RIGHT_1,  NUM_RGB, RENDER_FUNC_NAME(RIGHT_1), NULL, {0, 0, 0, 0, 0, false},  colorData[RIGHT_1] },
    { RIGHT_2,  NUM_RGB, RENDER_FUNC_NAME(RIGHT_2), NULL, {0, 0, 0, 0, 1, false},  colorData[RIGHT_2] },
    { RIGHT_3,  NUM_RGB, RENDER_FUNC_NAME(RIGHT_3), NULL, {0, 0, 0, 0, 0, false},  colorData[RIGHT_3] },
    { RIGHT_4,  NUM_RGB, RENDER_FUNC_NAME(RIGHT_4), NULL, {0, 0, 0, 0, 1, false},  colorData[RIGHT_4] },
    { CENT_LEFT, NUM_RGB, RENDER_FUNC_NAME(CENT_LEFT), NULL, {0, 0, 0, 0, 0, false},  colorData[CENT_LEFT] },
    { CENT_RIGHT, NUM_RGB, RENDER_FUNC_NAME(CENT_RIGHT), NULL, {0, 0, 0, 0, 1, false},  colorData[CENT_RIGHT] },
};

enum mode
{
  FULL_WHITE,
  BLINK,
  MOVER,
  XMAS,
  NUM_FUNCS
};

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(9600);
  randomSeed(analogRead(0));
   
  for (int i = 0; i < NUM_DIGITAL_OUT_PINS; i++) {
    pinMode(digitalOutPins[i], OUTPUT);
    digitalWrite(digitalOutPins[i], LOW);
  }
  
  delay(STARTUP_DELAY);
  digitalWrite(RELAY1_OUTPUT_ENABLE_PIN, HIGH);   // turn the relay on
  digitalWrite(RELAY2_OUTPUT_ENABLE_PIN, HIGH);   // turn the relay on

  shelves[LEFT_1].program = loopFadeInOut;
  shelves[LEFT_2].program = loopFadeInOut;
  shelves[LEFT_3].program = loopFadeInOut;
  shelves[LEFT_4].program = loopFadeInOut;
  shelves[CENT_LEFT].program = loopFadeInOut;
  shelves[RIGHT_1].program = loopFadeInOut;
  shelves[RIGHT_2].program = loopFadeInOut;
  shelves[RIGHT_3].program = loopFadeInOut;
  shelves[RIGHT_4].program = loopFadeInOut;
  shelves[CENT_RIGHT].program = loopFadeInOut;
}

void loopFullWhite(Shelf &shelf)
{
  for(unsigned int i=0; i<shelf.numLeds; i++)
  {
      setColorRGB(shelf.data, i, 255, 255, 255);
  }
  shelf.renderer(shelf);
}

void loopMover(Shelf &shelf)
{
  unsigned int i;
  
  for(i = 0; i < shelf.numLeds; i++)
  {
    if (i == shelf.context.mover)
      setColorRGB(shelf.data, shelf.context.mover, 32, 32, 32);
    else
      setColorRGB(shelf.data, i, 0, 0, 0);
  }
  if (shelf.context.mover == (NUM_RGB - 1))
  {
    shelf.context.dir = -1;
  }
  else if (shelf.context.mover == 0)
  {
    shelf.context.dir = 1;
  }
    
  shelf.context.mover = shelf.context.mover + shelf.context.dir;
  shelf.renderer(shelf);
}

void loopXmas(Shelf &shelf)
{
  unsigned int i;

  for(i = 0; i < shelf.numLeds; i++)
  {
    if (shelf.context.color == 1)
      setColorRGB(shelf.data, i, 255, 0, 0);
    else
      setColorRGB(shelf.data, i, 0, 255, 0);
  }
  if ((shelf.context.j++ % 100) == 0) {
    shelf.context.color = 1 - shelf.context.color;
    shelf.renderer(shelf);
  }
}

void loopFadeInOut(Shelf &shelf)
{
  unsigned int i;

  for(i = 0; i < shelf.numLeds; i++)
  {
    if (shelf.context.color == 1)
    {
        if (i > shelf.numLeds/2)
            setColorRGB(shelf.data, i, shelf.context.j, 0, 0);
        else
            setColorRGB(shelf.data, i, 0, shelf.context.j, 0);
    }
    else
    {
        if (i > shelf.numLeds/2)
            setColorRGB(shelf.data, i, 0, shelf.context.j, 0);
        else
            setColorRGB(shelf.data, i, shelf.context.j, 0, 0);
            
    }
  }
  shelf.context.j += shelf.context.dir;
  if (shelf.context.j == 0) {
    shelf.context.color = 1 - shelf.context.color;
    shelf.context.dir = 1;
  }
  else if (shelf.context.j == 127)
  {
    shelf.context.dir = -1;
  }
  shelf.renderer(shelf);
}


void setColorRGB(uint8_t *data, uint16_t idx, uint8_t r, uint8_t g, uint8_t b) 
{
  if(idx < NUM_RGB) 
  {
    uint8_t *p = &data[idx*3]; 
    *p++ = g;  
    *p++ = r;
    *p = b;
  }
}

// the loop function runs over and over again forever
void loop() {
    for (int i = 0; i < (int)NUM_SHELVES; i++) {
        shelves[i].program(shelves[i]);
    }
  //delay(5);
}
