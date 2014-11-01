#include <Arduino.h>

extern "C" {
#include "../picoc.h"
#include "../interpreter.h"
}

namespace {

TReadFileFunc readFileFunc;

}

void PicocSetReadFile(TReadFileFunc f)
{
    readFileFunc = f;
}

char *PicocGetCharBuffer(Picoc *pc, int size)
{
    return (char *)HeapAllocMem(pc, size);
}

void PicocFreeCharBuffer(Picoc *pc, char *buf)
{
    HeapFreeMem(pc, (void *)buf);
}

void PlatformInit(Picoc *pc)
{
}

void PlatformCleanup()
{
}

/* get a line of interactive input */
char *PlatformGetLine(char *Buf, int MaxLen, const char *Prompt)
{
    if (Prompt != NULL)
        Serial.print(Prompt);
    
    while (true)
    {
        if (Serial.readBytesUntil('\n', Buf, MaxLen))
            return Buf;
    }
    
    return 0;
}

/* get a character of interactive input */
int PlatformGetCharacter()
{
    return Serial.read();
}

/* write a character to the console */
void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    Serial.write(OutCh);
}

/* read a file into memory */
char *PlatformReadFile(Picoc *pc, const char *FileName)
{
    if (readFileFunc)
        return readFileFunc(FileName);

    return 0;
}

/* read and scan a file for definitions */
void PicocPlatformScanFile(Picoc *pc, const char *FileName)
{
    char *SourceStr = PlatformReadFile(pc, FileName);

    PicocParse(pc, FileName, SourceStr, strlen(SourceStr), TRUE, FALSE, TRUE, TRUE);
}

/* exit the program */
void PlatformExit(Picoc *pc, int RetVal)
{
    //PicocExitValue = RetVal;
    // .. no exit (UNDONE?)
}

