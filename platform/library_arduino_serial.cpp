#include <Arduino.h>

#include "../picoc.h"
#include "../interpreter.h"

// From clibrary.cpp
extern void GenericPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs, struct OutputStream *Stream);

namespace {

// Macros to handle multiple serial ports
#define execSerialFunc(port, func, ...) \
    switch (port) \
    { \
    case 0: Serial.func(__VA_ARGS__); break; \
    case 1: Serial1.func(__VA_ARGS__); break; \
    case 2: Serial2.func(__VA_ARGS__); break; \
    case 3: Serial3.func(__VA_ARGS__); break; \
    }

#define execSerialFuncAssign(port, func, ret, ...) \
    switch (port) \
    { \
    case 0: ret = Serial.func(__VA_ARGS__); break; \
    case 1: ret = Serial1.func(__VA_ARGS__); break; \
    case 2: ret = Serial2.func(__VA_ARGS__); break; \
    case 3: ret = Serial3.func(__VA_ARGS__); break; \
    }

// Printf helpers
void PutcSer1(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    Serial1.write(OutCh);
}

void PutcSer2(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    Serial2.write(OutCh);
}

void PutcSer3(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    Serial3.write(OutCh);
}


void SerBegin(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFunc(Param[0]->Val->UnsignedCharacter, begin, Param[1]->Val->UnsignedLongInteger);
}

void SerEnd(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFunc(Param[0]->Val->UnsignedCharacter, end);
}

void SerAvailable(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, available, ReturnValue->Val->Integer);
}

void SerFlush(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFunc(Param[0]->Val->UnsignedCharacter, flush);
}

void SerPeek(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, peek, ReturnValue->Val->Integer);
}

void SerWrite(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, write, ReturnValue->Val->Integer,
            Param[1]->Val->UnsignedCharacter);
}

void SerPrintInt(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, print, ReturnValue->Val->Integer,
            Param[1]->Val->Integer);
}

void SerPrintlnInt(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, println, ReturnValue->Val->Integer,
            Param[1]->Val->Integer);
}

void SerPrintStr(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, print, ReturnValue->Val->Integer,
            (const char *)Param[1]->Val->Pointer);
}

void SerPrintlnStr(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, println, ReturnValue->Val->Integer,
            (const char *)Param[1]->Val->Pointer);
}

#ifndef NO_PRINTF
void SerPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    struct OutputStream ConsoleStream;

    switch (Param[0]->Val->UnsignedCharacter)
    {
    case 0: ConsoleStream.Putch = &PlatformPutc; break;
    case 1: ConsoleStream.Putch = &PutcSer1; break;
    case 2: ConsoleStream.Putch = &PutcSer2; break;
    case 3: ConsoleStream.Putch = &PutcSer3; break;
    }

    GenericPrintf(Parser, ReturnValue, Param+1, NumArgs-1, &ConsoleStream);
}
#endif

void SerRead(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, read, ReturnValue->Val->Integer);
}

void SerReadBytes(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, readBytes,
            ReturnValue->Val->UnsignedLongInteger, (char *)Param[1]->Val->Pointer,
            Param[2]->Val->UnsignedLongInteger);
}

void SerReadBytesUntil(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFuncAssign(Param[0]->Val->UnsignedCharacter, readBytesUntil,
            ReturnValue->Val->UnsignedLongInteger, Param[1]->Val->UnsignedCharacter,
            (char *)Param[2]->Val->Pointer, Param[3]->Val->UnsignedLongInteger);
}

void SerSetTimeout(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    execSerialFunc(Param[0]->Val->UnsignedCharacter, setTimeout, Param[1]->Val->UnsignedLongInteger);
}

/* list of all library functions and their prototypes */
const struct LibraryFunction SerFunctions[] =
{
    { SerBegin, "void serBegin(uint8_t, unsigned long);" },
    { SerEnd, "void serEnd(uint8_t);" },
    { SerAvailable, "int serAvailable(uint8_t);" },
    { SerFlush, "void serFlush(uint8_t);" },
    { SerPeek, "int serPeek(uint8_t);" },
    { SerWrite, "unsigned long serWrite(uint8_t, uint8_t);" },
    { SerPrintInt, "unsigned long serPrintInt(uint8_t, int);" },
    { SerPrintlnInt, "unsigned long serPrintlnInt(uint8_t, int);" },
    { SerPrintStr, "unsigned long serPrintStr(uint8_t, char *);" },
    { SerPrintlnStr, "unsigned long serPrintlnStr(uint8_t, char *);" },
#ifndef NO_PRINTF
    { SerPrintf, "void serPrintf(uint8_t, char *, ...);" },
#endif
    { SerRead, "int serRead(uint8_t);" },
    { SerReadBytes, "unsigned long serReadBytes(uint8_t, char *, unsigned long);" },
    { SerReadBytesUntil, "unsigned long serReadBytesUntil(uint8_t, char, char *, unsigned long);" },
    { SerSetTimeout, "void serSetTimeout(uint8_t, unsigned long);" },
    { NULL, NULL }
};

}

void LibraryArduinoSerialInit(Picoc *pc)
{
    LibraryAdd(pc, ptrWrap(&pc->GlobalTable), "arduino serial library", &SerFunctions[0]);
}
