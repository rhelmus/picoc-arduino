/* picoc main program - this varies depending on your operating system and
 * how you're using picoc */
 
/* include only picoc.h here - should be able to use it with only the external interfaces, no internals from interpreter.h */
#include "picoc.h"

/* platform-dependent code for running programs is in this file */

#if defined(UNIX_HOST) || defined(WIN32)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int ParamCount = 1;
    int DontRunMain = FALSE;
    int LineByLine = FALSE;
    int StackSize = getenv("STACKSIZE") ? atoi(getenv("STACKSIZE")) : HEAP_SIZE;
    Picoc pc;
    
    if (argc < 2)
    {
        printf("Format: picoc <csource1.c>... [- <arg1>...]    : run a program (calls main() to start it)\n"
               "        picoc -s <csource1.c>... [- <arg1>...] : script mode - runs the program without calling main()\n"
               "        picoc -i                               : interactive mode\n");
        exit(1);
    }
    
    PicocInitialise(&pc, StackSize);
    
    for (; ParamCount<=argc; ++ParamCount)
    {
        if (argv[ParamCount][0] != '-')
            break;
        switch(argv[ParamCount][1])
        {
        case 's':
        case 'm':
            DontRunMain = TRUE;
            PicocIncludeAllSystemHeaders(&pc);
            break;
        case 'l':
            LineByLine = TRUE;
            /*PicocIncludeAllSystemHeaders(&pc);*/ // UNDONE
            break;
        case 'i':
            PicocIncludeAllSystemHeaders(&pc);
            PicocParseInteractive(&pc);
            goto cleanup;
        }
    }

#if 0
    if (strcmp(argv[ParamCount], "-s") == 0 || strcmp(argv[ParamCount], "-m") == 0)
    {
        DontRunMain = TRUE;
        PicocIncludeAllSystemHeaders(&pc);
        ParamCount++;
    }
    else if (argc > ParamCount && strcmp(argv[ParamCount], "-l") == 0)
    {
        LineByLine = TRUE;
        /*PicocIncludeAllSystemHeaders(&pc);*/
        ParamCount++;
    }


    if (argc > ParamCount && strcmp(argv[ParamCount], "-i") == 0)
    {
        PicocIncludeAllSystemHeaders(&pc);
        PicocParseInteractive(&pc);
    }
    else
#endif
    {
        if (PicocPlatformSetExitPoint(&pc))
        {
            PicocCleanup(&pc);
            return pc.PicocExitValue;
        }
        
        for (; ParamCount < argc && strcmp(argv[ParamCount], "-") != 0; ParamCount++)
        {
            if (LineByLine)
                PicocPlatformScanFileByLine(&pc, argv[ParamCount]);
            else
                PicocPlatformScanFile(&pc, argv[ParamCount]);
        }

        if (!DontRunMain)
            PicocCallMain(&pc, argc - ParamCount, &argv[ParamCount]);
    }
    
cleanup:
    PicocCleanup(&pc);
    return pc.PicocExitValue;
}
#else
# ifdef SURVEYOR_HOST
#  define HEAP_SIZE C_HEAPSIZE
#  include <setjmp.h>
#  include "../srv.h"
#  include "../print.h"
#  include "../string.h"

int picoc(char *SourceStr)
{   
    char *pos;

    PicocInitialise(HEAP_SIZE);

    if (SourceStr)
    {
        for (pos = SourceStr; *pos != 0; pos++)
        {
            if (*pos == 0x1a)
            {
                *pos = 0x20;
            }
        }
    }

    PicocExitBuf[40] = 0;
    PicocPlatformSetExitPoint();
    if (PicocExitBuf[40]) {
        printf("Leaving PicoC\n\r");
        PicocCleanup();
        return PicocExitValue;
    }

    if (SourceStr)   
        PicocParse("nofile", SourceStr, strlen(SourceStr), TRUE, TRUE, FALSE);

    PicocParseInteractive();
    PicocCleanup();
    
    return PicocExitValue;
}
# endif
#endif
