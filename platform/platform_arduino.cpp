#include <Arduino.h>
#include <SdFat.h>

#include "../picoc.h"
#include "../interpreter.h"

#if 0
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

/* get a line from a file */
char *PlatformGetLineFromFile(char *Buf, int MaxLen, void *FilePointer)
{
    if (ReadFileLineFunction)
        return ReadFileLineFunction(Buf, MaxLen, FilePointer);
    return NULL;
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
#endif

namespace {
SdFile sdFile;
}

void PlatformInit(Picoc *pc)
{
}

void PlatformCleanup(Picoc *pc)
{
    if (sdFile.isOpen())
        sdFile.close();
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

/* get a line from a file */
char *PlatformGetLineFromFile(char *Buf, int MaxLen, void *FilePointer)
{
    const int16_t read = sdFile.fgets(Buf, MaxLen);
    Serial.print("Source line: "); Serial.println((read == -1) ? "EOF" : Buf);
    return (read > 0) ? Buf : NULL;
}

/* read and scan a file for definitions */
void PicocPlatformScanFile(Picoc *pc, const char *FileName)
{
    if (!sdFile.open(FileName, O_READ))
        ProgramFailNoParser(pc, "opening source file for read failed");

    const uint32_t size = sdFile.fileSize();

    TLexCharPtr source = allocMem<char>(false, size + 1);
#ifdef USE_VIRTMEM
    const uint8_t bufsize = 64;
    char buf[bufsize];
    for (uint32_t i=0; i<size; i+=bufsize)
    {
        const uint32_t cpsize = ((i+bufsize) < size) ? bufsize : (size-i);
        sdFile.read(buf, cpsize);
        memcpy(&source[i], buf, cpsize);
    }
#else
    sdFile.read(source, size);
#endif

    source[size] = 0;
    sdFile.close();

    PicocParse(pc, FileName, source, size, TRUE, FALSE, TRUE, TRUE);
}

/* read and scan a file for definitions */
void PicocPlatformScanFileByLine(Picoc *pc, const char *FileName)
{
    if (!sdFile.open(FileName, O_READ))
        ProgramFailNoParser(pc, "opening source file for read failed");

    PicocParseLineByLine(pc, FileName, &sdFile, TRUE);
    sdFile.close();
}

/* exit the program */
void PlatformExit(Picoc *pc, int RetVal)
{
    //PicocExitValue = RetVal;
    while (true)
        ;
    // .. no exit (UNDONE?)
}

