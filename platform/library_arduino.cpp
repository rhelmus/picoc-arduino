#include <Arduino.h>

#include "../interpreter.h"

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
#define min(a,b) ((a)<(b)?(a):(b))\
#define max(a,b) ((a)>(b)?(a):(b))\
#define abs(x) ((x)>0?(x):-(x))\
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))\
";

}

void ArduinoSetupFunc(Picoc *pc)
{
    VariableDefinePlatformVar(pc, NILL, "HIGH", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&HIGHValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LOW", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&LOWValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "OUTPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&OUTPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT_PULLUP", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUT_PULLUPValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LED_BUILTIN", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&LED_BUILTINValue), FALSE);
}

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
    ReturnValue->Integer = digitalRead(Param[0]->Val->UnsignedCharacter);
}

void CAnalogReference(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    analogReference(Param[0]->Val->UnsignedCharacter);
}

void CAnalogRead(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Integer = analogRead(Param[0]->Val->UnsignedCharacter);
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
    tone(Param[0]->Val->UnsignedChar, Param[1]->Val->UnsignedInteger, Param[2]->Val->UnsignedLongInteger);
}

void CNoTone(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    noTone(Param[0]->Val->UnsignedChar);
}

void CShiftOut(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    shiftOut(Param[0]->Val->UnsignedChar, Param[1]->Val->UnsignedChar, Param[2]->Val->UnsignedChar,
             Param[3]->Val->UnsignedChar);
}

void CShiftIn(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->UnsignedChar = shiftIn(Param[0]->Val->UnsignedChar, Param[1]->Val->UnsignedChar,
            Param[2]->Val->UnsignedChar);
}

void CPulseIn(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->UnsignedLong = pulseIn(Param[0]->Val->UnsignedChar, Param[1]->Val->UnsignedChar,
            Param[2]->Val->UnsignedLong);
}

void CMillis(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->UnsignedLong = millis();
}

void CMicros(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->UnsignedLong = micros();
}

void CDelay(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    delay(Param[0]->Val->UnsignedLong);
}

void CDelayMicroseconds(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    delayMicroseconds(Param[0]->Val->UnsignedInt);
}

void CDelayMicroseconds(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    delayMicroseconds(Param[0]->Val->UnsignedInt);
}

void CMap(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->LongInteger = map(Param[0]->Val->LongInteger, Param[1]->Val->LongInteger,
            Param[2]->Val->LongInteger, Param[3]->Val->LongInteger, Param[4]->Val->LongInteger);
}

void CRandom(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->LongInteger = random(Param[0]->Val->LongInteger, Param[1]->Val->LongInteger);
}

void CRandomSeed(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    randomSeed(Param[0]->Val->UnsignedInteger);
}

/* list of all library functions and their prototypes */
struct LibraryFunction ArduinoFunctions[] =
{
    { CpinMode,             "void pinMode(uint8_t, uint8_t);" },
    { CdigitalWrite,        "void digitalWrite(uint8_t, uint8_t);" },
    { CdigitalRead,         "int digitalRead(uint8_t);" },
    { NULL,         NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
#ifndef NO_FILE_SUPPORT // UNDONE
    IncludeRegister(pc, "Arduino.h", &ArduinoSetupFunc, &ArduinoFunctions[0], ArduinoDefs);
#endif
}
