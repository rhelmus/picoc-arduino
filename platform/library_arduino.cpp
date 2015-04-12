#include <Arduino.h>

#include "../picoc.h"
#include "../interpreter.h"

// Defined in serial library file
extern void LibraryArduinoSerialInit(Picoc *pc);

namespace {

uint8_t HIGHValue = HIGH;
uint8_t LOWValue = LOW;
uint8_t INPUTValue = INPUT;
uint8_t OUTPUTValue = OUTPUT;
uint8_t INPUT_PULLUPValue = INPUT_PULLUP;
uint8_t LED_BUILTINValue = LED_BUILTIN;

const char ArduinoDefs[] = "\
typedef unsigned char uint8_t; \
typedef signed char int8_t;\
typedef unsigned short uint16_t;\
typedef signed short int16_t;\
typedef unsigned long uin32_t;\
typedef signed long int32_t;\
\
#define min(a,b) ((a)<(b)?(a):(b))\n\
#define max(a,b) ((a)>(b)?(a):(b))\n\
#define abs(x) ((x)>0?(x):-(x))\n\
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))\n\
\
#define lowByte(w) ((uint8_t) ((w) & 0xff))\n\
#define highByte(w) ((uint8_t) ((w) >> 8))\n\
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)\n\
#define bitSet(value, bit) ((value) |= (1UL << (bit)))\n\
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))\n\
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))\n\
#define bit(b) (1UL << (b))\n\
";

void CpinMode(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    pinMode(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter);
}

void CdigitalWrite(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    digitalWrite(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter);
}

void CdigitalRead(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = digitalRead(Param[0]->Val->UnsignedCharacter);
}

void CAnalogReference(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    analogReference(Param[0]->Val->UnsignedCharacter);
}

void CAnalogRead(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = analogRead(Param[0]->Val->UnsignedCharacter);
}

void CAnalogWrite(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    analogWrite(Param[0]->Val->UnsignedCharacter, Param[1]->Val->Integer);
}

#if defined (__arm__) && defined (__SAM3X8E__)
void CAnalogReadResolution(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    analogReadResolution(Param[0]->Val->Integer);
}

void CAnalogWriteResolution(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    analogWriteResolution(Param[0]->Val->Integer);
}
#endif

void CTone(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    tone(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedInteger, Param[2]->Val->UnsignedLongInteger);
}

void CNoTone(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    noTone(Param[0]->Val->UnsignedCharacter);
}

void CShiftOut(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    shiftOut(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter, Param[2]->Val->UnsignedCharacter,
             Param[3]->Val->UnsignedCharacter);
}

void CShiftIn(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedCharacter = shiftIn(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter,
            Param[2]->Val->UnsignedCharacter);
}

void CPulseIn(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedLongInteger = pulseIn(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter,
            Param[2]->Val->UnsignedLongInteger);
}

void CMillis(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedLongInteger = millis();
}

void CMicros(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedLongInteger = micros();
}

void CDelay(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    delay(Param[0]->Val->UnsignedLongInteger);
}

void CDelayMicroseconds(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    delayMicroseconds(Param[0]->Val->UnsignedInteger);
}


void CMap(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->LongInteger = map(Param[0]->Val->LongInteger, Param[1]->Val->LongInteger,
            Param[2]->Val->LongInteger, Param[3]->Val->LongInteger, Param[4]->Val->LongInteger);
}

void CRandom(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->LongInteger = random(Param[0]->Val->LongInteger, Param[1]->Val->LongInteger);
}

void CRandomSeed(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    randomSeed(Param[0]->Val->UnsignedInteger);
}

/* list of all library functions and their prototypes */
const struct LibraryFunction ArduinoFunctions[] =
{
    { CpinMode, "void pinMode(uint8_t, uint8_t);" },
    { CdigitalWrite, "void digitalWrite(uint8_t, uint8_t);" },
    { CdigitalRead, "int digitalRead(uint8_t);" },
    { CAnalogReference, "void digitalReference(uint8_t);" },
    { CAnalogRead, "int analogRead(uint8_t);" },
    { CAnalogWrite, "void analogWrite(uint8_t, int);" },
#if defined (__arm__) && defined (__SAM3X8E__)
    { CAnalogReadResolution, "void analogReadResolution(int);" },
    { CAnalogWriteResolution, "void analogWriteResolution(int);" },
#endif
    { CTone, "void tone(uint8_t, unsigned int, unsigned long);" },
    { CNoTone, "void noTone(uint8_t);" },
    { CShiftOut, "void shiftOut(uint8_t, uint8_t, uint8_t, uint8_t);" },
    { CShiftIn, "uint8_t shiftIn(uint8_t, uint8_t, uint8_t);" },
    { CPulseIn, "unsigned long pulseIn(uint8_t, uint8_t, unsigned long);" },
    { CMillis, "unsigned long millis(void);" },
    { CMicros, "unsigned long micros(void);" },
    { CDelay, "void delay(unsigned long);" },
    { CDelayMicroseconds, "void delayMicroseconds(unsigned int);" },
    { CMap, "long map(long, long, long, long, long);" },
    { CRandom, "long random(long, long);" },
    { CRandomSeed, "void randomSeed(unsigned int);" },
    { NULL, NULL }
};

}


void PlatformLibraryInit(Picoc *pc)
{
    VariableDefinePlatformVar(pc, NILL, "HIGH", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&HIGHValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LOW", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&LOWValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "OUTPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&OUTPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT_PULLUP", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUT_PULLUPValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LED_BUILTIN", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&LED_BUILTINValue), FALSE);

    PicocParse(pc, "startup", ArduinoDefs, strlen(ArduinoDefs), TRUE, TRUE, FALSE, FALSE);

    // Do this AFTER parsing definitions as the functions depend on typedefs from those
    LibraryAdd(pc, ptrWrap(&pc->GlobalTable), "arduino library", &ArduinoFunctions[0]);

    LibraryArduinoSerialInit(pc);
}
