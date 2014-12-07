#include <Arduino.h>

#include "../picoc.h"
#include "../interpreter.h"

namespace {

ReadCompleteFileFunc ReadFileFunction;
OpenFileFunc OpenFileFunction;
CloseFileFunc CloseFileFunction;
ReadFileLineFunc ReadFileLineFunction;

}

void PicocSetReadCompleteFileFunc(ReadCompleteFileFunc f)
{
    ReadFileFunction = f;
}

void PicocSetOpenFileFunc(OpenFileFunc f)
{
    OpenFileFunction = f;
}

void PicocSetCloseFileFunc(CloseFileFunc f)
{
    CloseFileFunction = f;
}

void PicocSetReadFileLineFunc(ReadFileLineFunc f)
{
    ReadFileLineFunction = f;
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

/* get a line from a file */
char *PlatformGetLineFromFile(char *Buf, int MaxLen, void *FilePointer)
{
    if (ReadFileLineFunction)
        return ReadFileLineFunction(Buf, MaxLen, FilePointer);
    return NULL;
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

/* read and scan a file for definitions */
void PicocPlatformScanFile(Picoc *pc, const char *FileName)
{
    if (ReadFileFunction)
    {
        char *SourceStr = ReadFileFunction(FileName);
        if (SourceStr)
            PicocParse(pc, FileName, SourceStr, strlen(SourceStr), TRUE, FALSE, TRUE, TRUE); /* UNDONE: always clean? */
    }
}

/* read and scan a file for definitions */
void PicocPlatformScanFileByLine(Picoc *pc, const char *FileName)
{
    if (!OpenFileFunction)
        return;

    void *fp = OpenFileFunction(FileName);

    if (fp)
    {
        PicocParseLineByLine(pc, FileName, fp, TRUE);
        if (CloseFileFunction)
            CloseFileFunction(fp);
    }
}

/* exit the program */
void PlatformExit(Picoc *pc, int RetVal)
{
    //PicocExitValue = RetVal;
    // .. no exit (UNDONE?)
}

