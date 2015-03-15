#include <Arduino.h>

#include "../picoc.h"

#ifndef NO_FILE_SUPPORT
#include <SdFat.h>

namespace {
SdFile sdFile;
}
#endif

namespace {

#ifdef USE_VIRTMEM
template <typename VA> struct CSerialInputTrait
{
    static int read(void) { return Serial.read(); }
    static bool available(void) { return Serial.available(); }
};

// Must use different functions with serial ram allocator
template <> struct CSerialInputTrait<class CSerRAMVirtMemAlloc>
{
    static int read(void) { return TVirtAlloc::getInstance()->input.read(); }
    static bool available(void) { return TVirtAlloc::getInstance()->input.available(); }
};

typedef CSerialInputTrait<TVirtAlloc> CSerialInput;

#else
struct CSerialInput
{
    static int read(void) { return Serial.read(); }
    static bool available(void) { return Serial.available(); }
};
#endif

}

/* mark where to end the program for platforms which require this */
jmp_buf PicocExitBuf;

void PlatformInit(Picoc *pc)
{
}

void PlatformCleanup(Picoc *pc)
{
#ifndef NO_FILE_SUPPORT
    if (sdFile.isOpen())
        sdFile.close();
#endif
}

/* get a line of interactive input */
char *PlatformGetLine(char *Buf, int MaxLen, const char *Prompt)
{
    if (Prompt != NULL)
        Serial.print(Prompt);

    while (true)
    {
        if (CSerialInput::available())
        {
            int index = 0;
            do
            {
                if (CSerialInput::available())
                    Buf[index++] = CSerialInput::read();
            }
            while (Buf[index-1] != '\n' && (index < (MaxLen-1)));

            Buf[index] = 0;
            if (Prompt != NULL)
                Serial.print('\n');
            return Buf;
        }
    }

    return 0;
}

/* get a character of interactive input */
int PlatformGetCharacter()
{
    return CSerialInput::read();
}

/* write a character to the console */
void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    Serial.write(OutCh);
}

#ifndef NO_FILE_SUPPORT
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
#endif

/* exit the program */
void PlatformExit(Picoc *pc, int RetVal)
{
    pc->PicocExitValue = RetVal;
    longjmp(pc->PicocExitBuf, 1);
}
