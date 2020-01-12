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

uint8_t chksum(uint8_t *buf, size_t len)
{
  uint8_t chk = 0;
  uint8_t *idx = buf;
  while (idx < (buf + len)) {
    chk += *(idx++);
  }
  return chk;
}

void ParseCmd(StaticJsonDocument<256> &msg)
{
  Serial.print("cmd:");
  const char *cs=msg["cmd"];
  Serial.println(cs);

  if (strcmp(cs, "ranges") == 0) {
    int shelf = msg["shelf"];

    JsonArray leds = msg["leds"].as<JsonArray>();

    for (JsonVariant l : leds) {
      int start = l["s"].as<int>();
      int end = l["e"].as<int>();
      int r = l["c"]["r"].as<int>();
      int g = l["c"]["g"].as<int>();
      int b = l["c"]["b"].as<int>();
      shelves[shelf].SetRangeOneColor(r, g, b, start, end, false);
      
      char dbg[128];
      snprintf(dbg, 128, "sh: %u, st: %u, end: %u, col(%u,%u,%u)",
        shelf, start, end, r, g, b);
      Serial.println(dbg);
      delay(100);
    }
    Serial.println("Rendering");
    shelves[shelf].renderer(shelves[shelf]);
    Serial.println("Complete");
  } else if (strcmp(cs, "entire") == 0) {
    int r = msg["c"]["r"].as<int>();
    int g = msg["c"]["g"].as<int>();
    int b = msg["c"]["b"].as<int>();
    char dbg[128];
    snprintf(dbg, 128, "col(%u,%u,%u)", r, g, b);
    Serial.println(dbg);
    delay(100);
    for (int i = 0;i < NUM_SHELVES; i++) {
      shelves[i].SetOneColor(r, g, b);
      shelves[i].renderer(shelves[i]);
    }
  }
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
  uint8_t msgLen = 0;
  uint32_t startMs = 0;
  StaticJsonDocument<256> msg;
  // Get message length
  if (softSer.available() > 0) {
    num = softSer.readBytes(buf, 1);
    if (num != 1) {
      Serial.println("Error reading message length.");
    } else {
      msgLen = buf[0];
      Serial.print("L: "); Serial.println(msgLen);
    }
    startMs = millis();
    while (bytesRead < msgLen) {
      if ((num = softSer.readBytes(&buf[bytesRead], softSer.available())) > 0) {
        bytesRead += num;
      }
      if ((millis() - startMs) > 1000) {
        Serial.println("Soft serial timeout");
        DrainSerialBuffer();
        softSer.println("NAK");
        return;
      }
    }
    if (bytesRead > 0) {
      Serial.print("Read ");
      Serial.print(bytesRead);
      Serial.println(" bytes.");
  
      uint8_t readChk = buf[bytesRead - 1];
      uint8_t calcChk = chksum(buf, bytesRead - 1);
      if (readChk != calcChk) {
        Serial.print("Checksum failed, read: ");
        Serial.print(readChk);
        Serial.print(", calc:");
        Serial.println(calcChk);
        softSer.println("NAK");
      } else {
        Serial.println("Checksum OK");
        softSer.println("ACK");
        DeserializationError error = deserializeMsgPack(msg, buf);
        // Test if parsing succeeded.
        if (error) {
          Serial.print("deserializeMsgPack() failed: ");
          Serial.println(error.c_str());
        } else {
          ParseCmd(msg);
        }
      }
    }
  }
}

// the loop function runs over and over again forever
void loop()
{
  PrintDebugHeartbeat();
  
  ReadAndParseMsg();
  
  // This loop really screws up SoftwareSerial. Interrupts?
#if 0
    for (int i = 0; i < (int)NUM_SHELVES; i++) {
        shelves[i].program(shelves[i]);
    }
//#else
    WholeShelfChase();
    delay(75);

#endif
}
 
