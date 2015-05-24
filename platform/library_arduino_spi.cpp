#include <Arduino.h>
#include <SPI.h>

#include "../picoc.h"
#include "../interpreter.h"

namespace {

void SPIBegin(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    SPI.begin();
}

void SPIEnd(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    SPI.end();
}

void SPIBeginTransaction(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    SPI.beginTransaction(SPISettings(Param[0]->Val->UnsignedInteger,
                         Param[1]->Val->UnsignedCharacter,
                         Param[2]->Val->UnsignedCharacter));
}

void SPIEndTransaction(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    SPI.endTransaction();
}

void SPITransfer(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedCharacter = SPI.transfer(Param[0]->Val->UnsignedCharacter);
}

/* list of all library functions and their prototypes */
const struct LibraryFunction SPIFunctions[] =
{
    { SPIBegin, "void SPIBegin(void);" },
    { SPIEnd, "void SPIEnd(void);" },
    { SPIBeginTransaction, "void SPIBeginTransaction(uint32_t, uint8_t, uint8_t);" },
    { SPIEndTransaction, "void SPIEndTransaction(void);" },
    { SPITransfer, "uint8_t SPITransfer(uint8_t);" },
    { NULL, NULL }
};

}

void LibraryArduinoSPIInit(Picoc *pc)
{
    LibraryAdd(pc, ptrWrap(&pc->GlobalTable), "arduino SPI library", &SPIFunctions[0]);
}
