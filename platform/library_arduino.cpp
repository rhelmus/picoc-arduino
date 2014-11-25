#include <Arduino.h>

extern "C" {
#include "../interpreter.h"
}

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
    VariableDefinePlatformVar(pc, NULL, "HIGH", &pc->CharType, (union AnyValue *)&HIGHValue, FALSE);
    VariableDefinePlatformVar(pc, NULL, "LOW", &pc->CharType, (union AnyValue *)&LOWValue, FALSE);
    VariableDefinePlatformVar(pc, NULL, "INPUT", &pc->CharType, (union AnyValue *)&INPUTValue, FALSE);
    VariableDefinePlatformVar(pc, NULL, "OUTPUT", &pc->CharType, (union AnyValue *)&OUTPUTValue, FALSE);
    VariableDefinePlatformVar(pc, NULL, "INPUT_PULLUP", &pc->CharType, (union AnyValue *)&INPUT_PULLUPValue, FALSE);
}

void CpinMode(struct ParseState *Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    pinMode(Param[0]->Val->Character, Param[1]->Val->Character);
}

void CdigitalWrite(struct ParseState *Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
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
