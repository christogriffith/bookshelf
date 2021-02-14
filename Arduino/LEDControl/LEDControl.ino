#include <ArduinoJson.h>

#include "Renderer.h"
#include "SoftwareSerial.h"

#undef PRINT_DBG_HEARTBEAT

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

void DrainSerialBuffer();

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
  Serial.begin(115200);
  softSer.begin(57600);

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

      shelves[i].program(shelves[i]);
  }

  Serial.println("Setup complete.");
  softSer.println("Software serial port up.");
  DrainSerialBuffer();
  delay(500);
}

void PrintDebugHeartbeat()
{
#ifdef PRINT_DBG_HEARTBEAT
  static unsigned int lastTime = millis();

  if (millis() > (lastTime + 1000)) {
    Serial.println(".");
    lastTime = millis();
  }
#endif
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
    shelf.context.needsUpdate = true;
  }
}

uint8_t chksum(uint8_t *buf, size_t len)
{
  uint8_t chk = 0;
  uint8_t *idx = buf;
  while (idx < (buf + len)) {
    chk += *(idx++);
  }
  return chk;
}

void DrainSerialBuffer()
{
  int avail;
  uint8_t buf[16];
  while ((avail = softSer.available()) > 0) {
    softSer.readBytes(buf, 16);
  }
}

void ReadAndParseMsg()
{
  byte buf[256];
  int num;
  int bytesRead = 0;
  uint8_t universe = 0;
  uint8_t cmd = 0;
  uint16_t msgLen = 0;
  uint32_t startMs = 0;
  
  // Get message length
  if (softSer.available() >= 4) {
    num = softSer.readBytes(buf, 1);
    if (num != 1) {
      Serial.println("Error reading universe.");
    } else {
      universe = buf[0];
      Serial.print("U: "); Serial.println(universe);
    }

    num = softSer.readBytes(buf, 1);
    if (num != 1) {
      Serial.println("Error reading command.");
    } else {
      cmd = buf[0];
      Serial.print("C: "); Serial.println(cmd);
    }

    num = softSer.readBytes(buf, 2);
    if (num != 2) {
      Serial.println("Error reading length.");
    } else {
      msgLen = (buf[0] << 8) | buf[1];
      Serial.print("L: "); Serial.println(msgLen);
    }
    
    startMs = millis();
    while (bytesRead < msgLen) {
      if ((num = softSer.readBytes(&buf[bytesRead], softSer.available())) > 0) {
        bytesRead += num;
      }
      if ((millis() - startMs) > 500) {
        Serial.println("Soft serial timeout");
        DrainSerialBuffer();
        return;
      }
    }
    if (bytesRead == msgLen) {
      Serial.print("Read ");
      Serial.print(bytesRead);
      Serial.println(" bytes");

      memcpy(pixelData, buf, msgLen);
      shelves[universe].renderer(shelves[universe]);
      shelves[universe].context.needsUpdate = true;
      
    } else {
      Serial.print("Bad data packet");
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  PrintDebugHeartbeat();
  
  ReadAndParseMsg();
}
 
