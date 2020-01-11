#include "Renderer.h"
#include "SoftwareSerial.h"

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

Shelf shelves[] = 
{   
    //Shelf       #LEDs    Rendering func name           Prog  Context
    { LEFT_1,     NUM_RGB, RENDER_FUNC_NAME(LEFT_1),     NULL, {0, 0, 0, 0, 1, false} },
    { LEFT_2,     NUM_RGB, RENDER_FUNC_NAME(LEFT_2),     NULL, {0, 0, 0, 0, 0, false} },
    { LEFT_3,     NUM_RGB, RENDER_FUNC_NAME(LEFT_3),     NULL, {0, 0, 0, 0, 1, false} },
    { LEFT_4,     NUM_RGB, RENDER_FUNC_NAME(LEFT_4),     NULL, {0, 0, 0, 0, 0, false} },
    { RIGHT_1,    NUM_RGB, RENDER_FUNC_NAME(RIGHT_1),    NULL, {0, 0, 0, 0, 0, false} },
    { RIGHT_2,    NUM_RGB, RENDER_FUNC_NAME(RIGHT_2),    NULL, {0, 0, 0, 0, 1, false} },
    { RIGHT_3,    NUM_RGB, RENDER_FUNC_NAME(RIGHT_3),    NULL, {0, 0, 0, 0, 0, false} },
    { RIGHT_4,    NUM_RGB, RENDER_FUNC_NAME(RIGHT_4),    NULL, {0, 0, 0, 0, 1, false} },
    { CENT_LEFT,  NUM_RGB, RENDER_FUNC_NAME(CENT_LEFT),  NULL, {0, 0, 0, 0, 0, false} },
    { CENT_RIGHT, NUM_RGB, RENDER_FUNC_NAME(CENT_RIGHT), NULL, {0, 0, 0, 0, 1, false} },
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
  // Configure the non-shelf-related pins
  pinMode(A5, INPUT);           // Soft serial rx
  pinMode(A4, OUTPUT);          // Soft serial tx
  pinMode(LED_BUILTIN, OUTPUT); // Built-in LED
  pinMode(0, INPUT);            // UART rx
  pinMode(1, OUTPUT);           // UART tx

  // Set up our serial ports
  Serial.begin(9600);
  softSer.begin(9600);

  // Seeding the RNG is unecessary in our case but doesn't hurt
  randomSeed(analogRead(0));

  // Set all shelf output pins to output, low
  for (int i = 0; i < NUM_DIGITAL_OUT_PINS; i++) {
    pinMode(digitalOutPins[i], OUTPUT);
    digitalWrite(digitalOutPins[i], LOW);
  }

  // Default to basic white
  for (int i = 0; i < (int)NUM_SHELVES; i++) {
      shelves[i].program = loopFullWhite;
      shelves[i].context.needsUpdate = true;
  }

  Serial.println("Setup complete.");
  delay(500);
}

void loopFullWhite(Shelf &shelf)
{
  if (shelf.context.needsUpdate)
  {
    for(unsigned int i=0; i<shelf.numLeds; i++)
    {
        shelf.SetPixelRGB(i, 250, 107, 50);
    }
    shelf.renderer(shelf);
    //shelf.context.needsUpdate = false;
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
    if (shelf.context.color == 1)
    {
      shelf.SetOneColor(shelf.context.j, 0, 0);
    }
    else
    {
      shelf.SetOneColor(0, shelf.context.j, 0);
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

void WholeShelfChase()
{
    static unsigned int chaseIndex = 0;
    const unsigned int chaseOrder[] = {
        LEFT_4, LEFT_3, LEFT_2, LEFT_1, CENT_LEFT, CENT_RIGHT, RIGHT_1, RIGHT_2, RIGHT_3, RIGHT_4,
        RIGHT_3, RIGHT_2, RIGHT_1, CENT_RIGHT, CENT_LEFT, LEFT_1, LEFT_2, LEFT_3 };
    const unsigned int chaseLen = sizeof(chaseOrder) / sizeof(unsigned int);

    for (int i = 0; i < NUM_SHELVES; i++)
    {
        shelves[i].SetOneColor(0, 255, 0);
        shelves[i].renderer(shelves[i]);
    }
    shelves[chaseOrder[chaseIndex]].SetOneColor(255, 0, 0);
    shelves[chaseOrder[chaseIndex]].renderer(shelves[chaseOrder[chaseIndex]]);
    
    if (++chaseIndex == chaseLen)
        chaseIndex = 0;
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
    }
  }
  
#if 1
    for (int i = 0; i < (int)NUM_SHELVES; i++) {
        shelves[i].program(shelves[i]);
    }
#else
    WholeShelfChase();
    delay(75);

#endif
}
 
