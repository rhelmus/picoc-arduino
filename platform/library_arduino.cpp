#include <Arduino.h>

#include "../interpreter.h"

namespace {

uint8_t HIGHValue = HIGH;
uint8_t LOWValue = LOW;
uint8_t INPUTValue = INPUT;
uint8_t OUTPUTValue = OUTPUT;
uint8_t INPUT_PULLUPValue = INPUT_PULLUP;

const char ArduinoDefs[] = "\
typedef unsigned char uint8_t; \
typedef signed char int8_t;\
typedef unsigned short uint16_t;\
typedef signed short int16_t;\
typedef unsigned long uin32_t;\
typedef signed long int32_t;\
";

}

void ArduinoSetupFunc(Picoc *pc)
{
    VariableDefinePlatformVar(pc, NILL, "HIGH", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&HIGHValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LOW", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&LOWValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "OUTPUT", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&OUTPUTValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "INPUT_PULLUP", ptrWrap(&pc->CharType), (TAnyValuePtr)ptrWrap(&INPUT_PULLUPValue), FALSE);
}

void CpinMode(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    pinMode(Param[0]->Val->Character, Param[1]->Val->Character);
}

void CdigitalWrite(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    digitalWrite(Param[0]->Val->Character, Param[1]->Val->Character);
}

/* list of all library functions and their prototypes */
struct LibraryFunction ArduinoFunctions[] =
{
    { CpinMode,             "void pinMode(uint8_t, uint8_t);" },
    { CdigitalWrite,        "void digitalWrite(uint8_t, uint8_t);" },
    { NULL,         NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
    IncludeRegister(pc, "Arduino.h", &ArduinoSetupFunc, &ArduinoFunctions[0], ArduinoDefs);
}
