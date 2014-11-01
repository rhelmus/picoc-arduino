#include "../interpreter.h"

void ArduinoSetupFunc()
{    
}

void Ctest (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) 
{
#if 0
    printf("test(%d)\n", Param[0]->Val->Integer);
#endif
    Param[0]->Val->Integer = 1234;
}

void Clineno (struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs) 
{
    ReturnValue->Val->Integer = Parser->Line;
}

/* list of all library functions and their prototypes */
struct LibraryFunction ArduinoFunctions[] =
{
    { Ctest,        "void test(int);" },
    { Clineno,      "int lineno();" },
    { NULL,         NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
    IncludeRegister(pc, "picoc_arduino.h", &ArduinoSetupFunc, &ArduinoFunctions[0], NULL);
}
