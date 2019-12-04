#include "Renderer.h"
#include "SoftwareSerial.h"

#define MAX_LEDS      (60)          // Maximum number of LEDs per shelf; used to allocate memory
#define NUM_RGB       (55)           // Number of actual LEDs we have connected (or that we want active)
#define NUM_BYTES     (NUM_RGB*3)   // Number of LEDs (3 per each WS281X)
#define NUM_BITS      (8)           // Constant value: bits per byte

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
RENDER_FUNC(LEFT_3,     PORTD, PORTD5); // Yes, I swapped L3 & 4 electrically. Whoops.
RENDER_FUNC(LEFT_4,     PORTD, PORTD4);
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
    { LEFT_1,   NUM_RGB, RENDER_FUNC_NAME(LEFT_1),  NULL, {0, 0, 0, 0, 1, false},  colorData[LEFT_1]  },
    { LEFT_2,   NUM_RGB, RENDER_FUNC_NAME(LEFT_2),  NULL, {0, 0, 0, 0, 0, false},  colorData[LEFT_2]  },
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

// On the serial cable, the 5V transmit line (A4 on the Arduino side) is at 5V;
// there is a 4.1K/2.2K divider under the shrink tube to put us in the 3.2V area.
// Arduino            RPi header    Notes
// A5 (Rx) -- Purp --  8 (Tx)       5V the whole way; the pi's 3.3V will trigger the Ard's Rx
// A4 (Tx) -- Blue -- 10 (Rx)       5V at Arduino Tx, divided to 3.2V at pi's side
SoftwareSerial softSer(A5, A4); // (Rx, Tx)

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(A5, INPUT);
  pinMode(A4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT);
  pinMode(1, OUTPUT);
  Serial.begin(9600);
  Serial.println("Setting up...");
    softSer.begin(9600);
  randomSeed(analogRead(0));
   
  for (int i = 0; i < NUM_DIGITAL_OUT_PINS; i++) {
    pinMode(digitalOutPins[i], OUTPUT);
    digitalWrite(digitalOutPins[i], LOW);
  }
  
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

  for (int i = 0; i < (int)NUM_SHELVES; i++) {
        shelves[i].context.needsUpdate = true;
    }

  Serial.println("Setup complete.");
}

void loopFullWhite(Shelf &shelf)
{
  if (shelf.context.needsUpdate)
  {
    for(unsigned int i=0; i<shelf.numLeds; i++)
    {
        shelf.SetPixelRGB(i, 64, 64, 64);
    }
    shelf.renderer(shelf);
    shelf.context.needsUpdate = false;
  }
}

void loopMover(Shelf &shelf)
{
  unsigned int i;
  
  for(i = 0; i < shelf.numLeds; i++)
  {
    if (i == shelf.context.mover)
      shelf.SetPixelRGB(shelf.context.mover, 32, 32, 32);
    else
      shelf.SetPixelRGB(i, 0, 0, 0);
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

void loopFadeInOut(Shelf &shelf)
{
  unsigned int i;

  for(i = 0; i < shelf.numLeds; i++)
  {
    if (shelf.context.color == 1)
    {
      shelf.SetPixelRGB(i, shelf.context.j, 0, 0);
    }
    else
    {
      shelf.SetPixelRGB(i, 0, shelf.context.j, 0);
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

// the loop function runs over and over again forever
void loop() {
  byte buf[32];
  int num;
  if (softSer.available() > 0)
  {
    if ((num = softSer.readBytes(buf, 32)) > 0)
    {
      Serial.println("SoftSerial received: ");
      Serial.write(buf, num);
      buf[0]++;
      softSer.write(buf, num);
    }
  }

    for (int i = 0; i < (int)NUM_SHELVES; i++) {
        shelves[i].program(shelves[i]);
    }
  //delay(5);
}
