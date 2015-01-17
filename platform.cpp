/* picoc's interface to the underlying platform. most platform-specific code
 * is in platform/platform_XX.c and platform/library_XX.c */
 
#include "picoc.h"
#include "interpreter.h"

Picoc *globalPicoc;

#ifdef USE_VIRTMEM
TVirtAlloc virtalloc;
#endif

/* initialise everything */
void PicocInitialise(Picoc *pc, int StackSize)
{
    memset(pc, '\0', sizeof(*pc));
    globalPicoc = pc; /* UNDONE: this effectively limits one Picoc struct per program, is this a problem? */
#ifdef USE_VIRTMEM
    virtalloc.start();
#endif
    PlatformInit(pc);
    BasicIOInit(pc);
    HeapInit(pc, StackSize);
    TableInit(pc);
    VariableInit(pc);
    LexInit(pc);
    TypeInit(pc);
#ifndef NO_HASH_INCLUDE
    IncludeInit(pc);
#endif
    LibraryInit(pc);
#ifdef BUILTIN_MINI_STDLIB
    LibraryAdd(pc, ptrWrap(&pc->GlobalTable), "c library", &CLibrary[0]);
    CLibraryInit(pc);
#endif
    PlatformLibraryInit(pc);
    DebugInit(pc);
}

/* free memory */
void PicocCleanup(Picoc *pc)
{
    DebugCleanup(pc);
#ifndef NO_HASH_INCLUDE
    IncludeCleanup(pc);
#endif
    ParseCleanup(pc);
    LexCleanup(pc);
    VariableCleanup(pc);
    TypeCleanup(pc);
    TableStrFree(pc);
    HeapCleanup(pc);
    PlatformCleanup(pc);
#ifdef USE_VIRTMEM
    virtalloc.stop();
#endif
}

/* platform-dependent code for running programs */
#if defined(UNIX_HOST) || defined(WIN32) || defined(ARDUINO_HOST)

#define CALL_MAIN_NO_ARGS_RETURN_VOID "main();"
#define CALL_MAIN_WITH_ARGS_RETURN_VOID "main(__argc,__argv);"
#define CALL_MAIN_NO_ARGS_RETURN_INT "__exit_value = main();"
#define CALL_MAIN_WITH_ARGS_RETURN_INT "__exit_value = main(__argc,__argv);"

void PicocCallMain(Picoc *pc, int argc, char **argv)
{
    /* check if the program wants arguments */
    TValuePtr FuncValue = NILL;

    if (!VariableDefined(pc, TableStrRegister(pc, "main")))
        ProgramFailNoParser(pc, "main() is not defined");
        
    VariableGet(pc, NILL, TableStrRegister(pc, "main"), &FuncValue);
    if (FuncValue->Typ->Base != TypeFunction)
        ProgramFailNoParser(pc, "main is not a function - can't call it");

    if (FuncValue->Val->FuncDef.NumParams != 0)
    {
        /* define the arguments */
        VariableDefinePlatformVar(pc, NILL, "__argc", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&argc), FALSE);
#ifdef WRAP_ANYVALUE
        PlatformCreatePtrArray(pc, NILL, "__argv", pc->CharPtrPtrType, (void **)argv, argc, sizeof(char *), FALSE);
#else
        VariableDefinePlatformVar(pc, NILL, "__argv", pc->CharPtrPtrType, (TAnyValuePtr)ptrWrap(&argv), FALSE);
#endif
    }

    if (FuncValue->Val->FuncDef.ReturnType == ptrWrap(&pc->VoidType))
    {
        if (FuncValue->Val->FuncDef.NumParams == 0)
            PicocParse(pc, "startup", CALL_MAIN_NO_ARGS_RETURN_VOID, strlen(CALL_MAIN_NO_ARGS_RETURN_VOID), TRUE, TRUE, FALSE, TRUE);
        else
            PicocParse(pc, "startup", CALL_MAIN_WITH_ARGS_RETURN_VOID, strlen(CALL_MAIN_WITH_ARGS_RETURN_VOID), TRUE, TRUE, FALSE, TRUE);
    }
    else
    {
        VariableDefinePlatformVar(pc, NILL, "__exit_value", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&pc->PicocExitValue), TRUE);
    
        if (FuncValue->Val->FuncDef.NumParams == 0)
            PicocParse(pc, "startup", CALL_MAIN_NO_ARGS_RETURN_INT, strlen(CALL_MAIN_NO_ARGS_RETURN_INT), TRUE, TRUE, FALSE, TRUE);
        else
            PicocParse(pc, "startup", CALL_MAIN_WITH_ARGS_RETURN_INT, strlen(CALL_MAIN_WITH_ARGS_RETURN_INT), TRUE, TRUE, FALSE, TRUE);
    }
}
#endif

void PrintSourceTextErrorLine(IOFILE *Stream, TConstRegStringPtr FileName, TLexConstCharPtr SourceText, int Line, int CharacterPos)
{
    int LineCount;
    TLexConstCharPtr LinePos;
    TLexConstCharPtr CPos;
    int CCount;
    
    if (SourceText != NULL)
    {
        /* find the source line */
        for (LinePos = SourceText, LineCount = 1; *LinePos != '\0' && LineCount < Line; LinePos++)
        {
            if (*LinePos == '\n')
                LineCount++;
        }
        
        /* display the line */
        for (CPos = LinePos; *CPos != '\n' && *CPos != '\0'; CPos++)
            PrintCh(*CPos, Stream);
        PrintCh('\n', Stream);
        
        /* display the error position */
        for (CPos = LinePos, CCount = 0; *CPos != '\n' && *CPos != '\0' && (CCount < CharacterPos || *CPos == ' '); CPos++, CCount++)
        {
            if (*CPos == '\t')
                PrintCh('\t', Stream);
            else
                PrintCh(' ', Stream);
        }
    }
    else
    {
        /* assume we're in interactive mode - try to make the arrow match up with the input text */
        for (CCount = 0; CCount < CharacterPos + (int)strlen(INTERACTIVE_PROMPT_STATEMENT); CCount++)
            PrintCh(' ', Stream);
    }

#ifdef WRAP_REGSTRINGS
    char buf[128];
    strncpy(buf, FileName, 127);
    buf[127] = 0;
    PlatformPrintf(Stream, "^\n%s:%d:%d ", buf, Line, CharacterPos);
#else
    PlatformPrintf(Stream, "^\n%s:%d:%d ", FileName, Line, CharacterPos);
#endif
    
}

/* exit with a message */
void ProgramFail(TParseStatePtr Parser, const char *Message, ...)
{
    va_list Args;

    PrintSourceTextErrorLine(Parser->pc->CStdOut, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos);
    va_start(Args, Message);
    PlatformVPrintf(Parser->pc->CStdOut, Message, Args);
    va_end(Args);
    PlatformPrintf(Parser->pc->CStdOut, "\n");
    PlatformExit(Parser->pc, 1);
}

/* exit with a message, when we're not parsing a program */
void ProgramFailNoParser(Picoc *pc, const char *Message, ...)
{
    va_list Args;

    va_start(Args, Message);
    PlatformVPrintf(pc->CStdOut, Message, Args);
    va_end(Args);
    PlatformPrintf(pc->CStdOut, "\n");
    PlatformExit(pc, 1);
}

/* like ProgramFail() but gives descriptive error messages for assignment */
void AssignFail(TParseStatePtr Parser, const char *Format, TValueTypePtr Type1, TValueTypePtr Type2, int Num1, int Num2, TConstRegStringPtr FuncName, int ParamNo)
{
    IOFILE *Stream = Parser->pc->CStdOut;
    
    PrintSourceTextErrorLine(Parser->pc->CStdOut, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos);
    PlatformPrintf(Stream, "can't %s ", (FuncName == NULL) ? "assign" : "set");
        
    if (Type1 != NULL)
        PlatformPrintf(Stream, Format, Type1, Type2);
    else
        PlatformPrintf(Stream, Format, Num1, Num2);
    
    if (FuncName != NULL)
        PlatformPrintf(Stream, " in argument %d of call to %s()", ParamNo, FuncName);
    
    PlatformPrintf(Stream, "\n");
    PlatformExit(Parser->pc, 1);
}

/* exit lexing with a message */
void LexFail(Picoc *pc, struct LexState *Lexer, const char *Message, ...)
{
    va_list Args;

    PrintSourceTextErrorLine(pc->CStdOut, Lexer->FileName, Lexer->SourceText, Lexer->Line, Lexer->CharacterPos);
    va_start(Args, Message);
    PlatformVPrintf(pc->CStdOut, Message, Args);
    va_end(Args);
    PlatformPrintf(pc->CStdOut, "\n");
    PlatformExit(pc, 1);
}

/* printf for compiler error reporting */
void PlatformPrintf(IOFILE *Stream, const char *Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    PlatformVPrintf(Stream, Format, Args);
    va_end(Args);
}

void PlatformVPrintf(IOFILE *Stream, const char *Format, va_list Args)
{
    const char *FPos;
    
    for (FPos = Format; *FPos != '\0'; FPos++)
    {
        if (*FPos == '%')
        {
            FPos++;
            switch (*FPos)
            {
            case 's': PrintStr(va_arg(Args, char *), Stream); break;
            case 'd': PrintSimpleInt(va_arg(Args, int), Stream); break;
            case 'c': PrintCh(va_arg(Args, int), Stream); break;
            case 't': PrintType(va_arg(Args, TValueTypePtr), Stream); break;
#ifndef NO_FP
            case 'f': PrintFP(va_arg(Args, double), Stream); break;
#endif
            case '%': PrintCh('%', Stream); break;
            case '\0': FPos--; break;
            }
        }
        else
            PrintCh(*FPos, Stream);
    }
}

/* make a new temporary name. takes a static buffer of char [7] as a parameter. should be initialised to "XX0000"
 * where XX can be any characters */
TRegStringPtr PlatformMakeTempName(Picoc *pc, char *TempNameBuffer)
{
    int CPos = 5;
    
    while (CPos > 1)
    {
        if (TempNameBuffer[CPos] < '9')
        {
            TempNameBuffer[CPos]++;
            return TableStrRegister(pc, TempNameBuffer);
        }
        else
        {
            TempNameBuffer[CPos] = '0';
            CPos--;
        }
    }

    return TableStrRegister(pc, TempNameBuffer);
}

#ifdef WRAP_ANYVALUE
// UNDONE: test this for other types than char **?
void PlatformCreatePtrArray(Picoc *pc, TParseStatePtr Parser, const char *Ident, TValueTypePtr Typ, void **Array, int Elements, int ElementSizeof, int IsWritable)
{
    int i;
    int ElementSize = TypeSize(Typ, Elements, FALSE);

    TValuePtr ArrayValue = VariableAllocValueAndData(pc, NILL, ElementSize * Elements + sizeof(TAnyValueVoidPtr), IsWritable, NILL, TRUE);
    ArrayValue->Typ = Typ;
    ArrayValue->Val->Pointer = (TAnyValueCharPtr)ArrayValue->Val + sizeof(TAnyValueVoidPtr);

    for (i=0; i<Elements; ++i)
    {
        TAnyValuePtr val = (TAnyValuePtr)((TAnyValueCharPtr)ArrayValue->Val->Pointer + i * ElementSize);
        val->Pointer = (TAnyValueVoidPtr)ptrWrap(*(char **)((char *)Array + i * ElementSizeof));
    }

    if (!TableSet(pc, (pc->TopStackFrame == NULL) ? ptrWrap(&pc->GlobalTable) : getMembrPtr(pc->TopStackFrame, &StackFrame::LocalTable), TableStrRegister(pc, Ident), ArrayValue, Parser ? Parser->FileName : NILL, Parser ? Parser->Line : 0, Parser ? Parser->CharacterPos : 0))
        ProgramFail(Parser, "'%s' is already defined", Ident);
}
#endif
