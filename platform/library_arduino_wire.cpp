#include <Arduino.h>
#include <Wire.h>

#include "../picoc.h"
#include "../interpreter.h"

namespace {

void WireBeginMaster(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    Wire.begin();
}

void WireBeginSlave(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    Wire.begin(Param[0]->Val->UnsignedCharacter);
}

void WireBeginTransmission(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    Wire.beginTransmission(Param[0]->Val->UnsignedCharacter);
}

void WireEndTransmission(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedCharacter = Wire.endTransmission(Param[0]->Val->UnsignedCharacter);
}

void WireWrite(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedLongInteger = Wire.write(Param[0]->Val->UnsignedCharacter);
}

void WireWriteBytes(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
#ifdef USE_VIRTMEM
    unsigned long n = Param[1]->Val->UnsignedLongInteger;
    TAnyValueCharPtr ptr = (TAnyValueCharPtr)Param[0]->Val->Pointer;
    ReturnValue->Val->UnsignedLongInteger = 0;

    while (n)
    {
        CVirtPtrLock<TAnyValueCharPtr> l = makeVirtPtrLock(ptr, n);
        const size_t written = Wire.write(*l, l.getLockSize());
        ReturnValue->Val->UnsignedLongInteger += written;

        if (written < l.getLockSize()) // abort if not everything could be written
            return;

        n -= written;
        ptr += written;
    }
#else
    ReturnValue->Val->UnsignedLongInteger = Wire.write((char *)Param[0]->Val->Pointer,
            Param[1]->Val->UnsignedLongInteger);
#endif
}

void WireRequestFrom(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->UnsignedCharacter =
            Wire.requestFrom(Param[0]->Val->UnsignedCharacter, Param[1]->Val->UnsignedCharacter,
            Param[2]->Val->UnsignedCharacter);
}

void WireAvailable(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = Wire.available();
}

void WireRead(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = Wire.read();
}

void WirePeek(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = Wire.peek();
}

void WireFlush(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    Wire.flush();
}

/* list of all library functions and their prototypes */
const struct LibraryFunction WireFunctions[] =
{
    { WireBeginMaster, "void wireBeginMaster(void);" },
    { WireBeginSlave, "void wireBeginSlave(uint8_t);" },
    { WireBeginTransmission, "void wireBeginTransmission(uint8_t);" },
    { WireEndTransmission, "uint8_t wireEndTransmission(uint8_t);" },
    { WireWrite, "unsigned long wireWrite(uint8_t);" },
    { WireWriteBytes, "unsigned long wireWriteBytes(uint8_t *, unsigned long);" },
    { WireRequestFrom, "uint8_t wireRequestFrom(uint8_t, uint8_t, uint8_t);" },
    { WireAvailable, "int wireAvailable(void);" },
    { WireRead, "int wireRead(void);" },
    { WirePeek, "int wirePeek(void);" },
    { WireFlush, "void wireFlush(void);" },
    { NULL, NULL }
};

}

void LibraryArduinoWireInit(Picoc *pc)
{
    LibraryAdd(pc, ptrWrap(&pc->GlobalTable), "arduino wire library", &WireFunctions[0]);
}

