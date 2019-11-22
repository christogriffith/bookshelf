// TODO licensing is boring af. The render() code at the bottom is blatantly stolen from an instructables.com article. 
// That code is under a beerware license. Link to the article is https://www.instructables.com/id/Bitbanging-step-by-step-Arduino-control-of-WS2811-/
//
// Someday I'll stop being lazy and learn the gcc inline assembly biz enough to not need to generate a separate function for each pin.
// But maybe I won't. If it fits (in memory), I sits.

enum ShelfName
{
    LEFT_1,
    LEFT_2,
    LEFT_3,
    LEFT_4,
    RIGHT_1,
    RIGHT_2,
    RIGHT_3,
    RIGHT_4,
    CENT_LEFT,
    CENT_RIGHT,
    NUM_SHELVES
};
struct Shelf;
typedef void (*RenderFunc)(const Shelf &shelf);
typedef void (*Program)(Shelf &shelf);

struct RenderContext
{
    unsigned int i;
    unsigned int j;
    unsigned int dir;
    unsigned int mover;
    unsigned int color;
    bool needsUpdate;
};

struct Shelf
{
    ShelfName name;
    uint_fast16_t numLeds;
    RenderFunc renderer;
    Program program;
    RenderContext context;
    uint8_t *data;
};

#define RENDER_FUNC_NAME(SHELFNAME) Render##SHELFNAME

#define RENDER_FUNC(SHELFNAME, PORT, PORTPIN)   \
void Render##SHELFNAME(const Shelf &shelf)      \
{                                               \
  if(!shelf.data) return;                       \
  static uint8_t t_f;                           \
  while((micros() - t_f) < 50L);                \
                                                \
  cli();                                        \
  volatile uint8_t                              \
   *p    = shelf.data,                          \
    val  = *p++,                                \
    high = PORT | _BV(PORTPIN),                 \
    low  = PORT & ~_BV(PORTPIN),                \
    tmp  = low,                                 \
    nbits= NUM_BITS                             \
    ;                                           \
  volatile uint16_t                             \
    nbytes = NUM_BYTES;                         \
  asm volatile(                                 \
                                                \
   "nextbit_%=:\n\t"                               \
    "sbi  %0, %1\n\t"                           \
    "sbrc %4, 7\n\t"                            \
     "mov  %6, %3\n\t"                          \
    "dec  %5\n\t"                               \
    "nop\n\t"                                   \
    "st   %a2, %6\n\t"                          \
    "mov  %6, %7\n\t"                           \
    "breq nextbyte_%=\n\t"                         \
    "rol  %4\n\t"                               \
    "rjmp .+0\n\t"                              \
    "cbi   %0, %1\n\t"                          \
    "rjmp .+0\n\t"                              \
    "nop\n\t"                                   \
    "rjmp nextbit_%=\n\t"                          \
    "nextbyte_%=:\n\t"                              \
    "ldi  %5, 8\n\t"                            \
    "ld   %4, %a8+\n\t"                         \
    "cbi   %0, %1\n\t"                          \
    "rjmp .+0\n\t"                              \
    "nop\n\t"                                   \
    "dec %9\n\t"                                \
    "brne nextbit_%=\n\t"                          \
    ::                                          \
                                                \
    "I" (_SFR_IO_ADDR(PORT)),                   \
    "I" (PORTPIN),                              \
    "e" (&PORT),                                \
    "r" (high),                                 \
    "r" (val),                                  \
    "r" (nbits),                                \
    "r" (tmp),                                  \
    "r" (low),                                  \
    "e" (p),                                    \
    "w" (nbytes)                                \
  );                                            \
  sei();                                        \
  t_f = micros();                               \
}                           


#if 0 // For reference only
// This is the function referenced in the header comment. I've already molested it. But it's close enough to get the point across.
void render(const Shelf &shelf, volatile byte &renderPort, uint_fast8_t renderPin) 
{
  if(!rgb_arr) return;

  while((micros() - t_f) < 50L);  // wait for 50us (data latch)

  cli(); // Disable interrupts so that timing is as precise as possible
  volatile uint8_t  
   *p    = shelf.data,   // Copy the start address of our data array
    val  = *p++,      // Get the current byte value & point to next byte
    high = shelf.port |  _BV(shelf.portPin), // Bitmask for sending HIGH to pin
    low  = shelf.port & ~_BV(shelf.portPin), // Bitmask for sending LOW to pin
    tmp  = low,       // Swap variable to adjust duty cycle 
    nbits= NUM_BITS
    ;  // Bit counter for inner loop
  volatile uint16_t
    nbytes = NUM_BYTES; // Byte counter for outer loop
  asm volatile(
    // Instruction        CLK     Description                 Phase
   "nextbit:\n\t"         // -    label                       (T =  0) 
    "sbi  %0, %1\n\t"     // 2    signal HIGH                 (T =  2) 
    "sbrc %4, 7\n\t"      // 1-2  if MSB set                  (T =  ?)          
     "mov  %6, %3\n\t"    // 0-1   tmp'll set signal high     (T =  4) 
    "dec  %5\n\t"         // 1    decrease bitcount           (T =  5) 
    "nop\n\t"             // 1    nop (idle 1 clock cycle)    (T =  6)
    "st   %a2, %6\n\t"    // 2    set PORT to tmp             (T =  8)
    "mov  %6, %7\n\t"     // 1    reset tmp to low (default)  (T =  9)
    "breq nextbyte\n\t"   // 1-2  if bitcount ==0 -> nextbyte (T =  ?)                
    "rol  %4\n\t"         // 1    shift MSB leftwards         (T = 11)
    "rjmp .+0\n\t"        // 2    nop nop                     (T = 13)
    "cbi   %0, %1\n\t"    // 2    signal LOW                  (T = 15)
    "rjmp .+0\n\t"        // 2    nop nop                     (T = 17)
    "nop\n\t"             // 1    nop                         (T = 18)
    "rjmp nextbit\n\t"    // 2    bitcount !=0 -> nextbit     (T = 20)
   "nextbyte:\n\t"        // -    label                       -
    "ldi  %5, 8\n\t"      // 1    reset bitcount              (T = 11)
    "ld   %4, %a8+\n\t"   // 2    val = *p++                  (T = 13)
    "cbi   %0, %1\n\t"    // 2    signal LOW                  (T = 15)
    "rjmp .+0\n\t"        // 2    nop nop                     (T = 17)
    "nop\n\t"             // 1    nop                         (T = 18)
    "dec %9\n\t"          // 1    decrease bytecount          (T = 19)
    "brne nextbit\n\t"    // 2    if bytecount !=0 -> nextbit (T = 20)
    ::
    // Input operands         Operand Id (w/ constraint)
    "I" (_SFR_IO_ADDR(renderPort)), // %0
    "I" (renderPin),           // %1
    "e" (&renderPort),              // %a2
    "r" (high),               // %3
    "r" (val),                // %4
    "r" (nbits),              // %5
    "r" (tmp),                // %6
    "r" (low),                // %7
    "e" (p),                  // %a8
    "w" (nbytes)              // %9
  );
  sei();                          // Enable interrupts
  t_f = micros();                 // t_f will be used to measure the 50us 
                                  // latching period in the next call of the 
                                  // function.
}
#endif
