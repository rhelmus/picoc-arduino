/* picoc mini standard C library - provides an optional tiny C standard library 
 * if BUILTIN_MINI_STDLIB is defined */ 
 
#include "picoc.h"
#include "interpreter.h"


/* endian-ness checking */
static const int __ENDIAN_CHECK__ = 1;
static int BigEndian;
static int LittleEndian;


/* global initialisation for libraries */
void LibraryInit(Picoc *pc)
{
    
    /* define the version number macro */
    pc->VersionString = TableStrRegister(pc, PICOC_VERSION);
    VariableDefinePlatformVar(pc, NILL, "PICOC_VERSION", pc->CharPtrType, (TAnyValuePtr)&pc->VersionString, FALSE);

    /* define endian-ness macros */
    BigEndian = ((*(char*)&__ENDIAN_CHECK__) == 0);
    LittleEndian = ((*(char*)&__ENDIAN_CHECK__) == 1);

    VariableDefinePlatformVar(pc, NILL, "BIG_ENDIAN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&BigEndian), FALSE);
    VariableDefinePlatformVar(pc, NILL, "LITTLE_ENDIAN", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&LittleEndian), FALSE);
}

/* add a library */
void LibraryAdd(Picoc *pc, TTablePtr GlobalTable, TConstRegStringPtr LibraryName, const struct LibraryFunction *FuncList)
{
    struct ParseState Parser;
    int Count;
    TRegStringPtr Identifier;
    TValueTypePtr ReturnType;
    TValuePtr NewValue;
    TLexBufPtr Tokens;
    TRegStringPtr IntrinsicName = TableStrRegister(pc, "c library"); /* UNDONE: Shouldn't this be LibraryName? */
    /*char *IntrinsicName = TableStrRegister(pc, LibraryName);*/
    
    /* read all the library definitions */
    for (Count = 0; FuncList[Count].Prototype != NULL; Count++)
    {
        Tokens = LexAnalyse(pc, IntrinsicName, FuncList[Count].Prototype, strlen((char *)FuncList[Count].Prototype), NULL);
        LexInitParser(ptrWrap(&Parser), pc, FuncList[Count].Prototype, Tokens, IntrinsicName, NULL, TRUE, FALSE);
        TypeParse(ptrWrap(&Parser), &ReturnType, &Identifier, NULL);
        NewValue = ParseFunctionDefinition(ptrWrap(&Parser), ReturnType, Identifier);
        NewValue->Val->FuncDef.Intrinsic = FuncList[Count].Func;
        NewValue->Val->FuncDef.Body = NILL;
        deallocMem(Tokens);
    }
}

/* print a type to a stream without using printf/sprintf */
void PrintType(TValueTypePtr Typ, IOFILE *Stream)
{
    switch (Typ->Base)
    {
        case TypeVoid:          PrintStr("void", Stream); break;
        case TypeInt:           PrintStr("int", Stream); break;
        case TypeShort:         PrintStr("short", Stream); break;
        case TypeChar:          PrintStr("char", Stream); break;
        case TypeLong:          PrintStr("long", Stream); break;
        case TypeUnsignedInt:   PrintStr("unsigned int", Stream); break;
        case TypeUnsignedShort: PrintStr("unsigned short", Stream); break;
        case TypeUnsignedLong:  PrintStr("unsigned long", Stream); break;
        case TypeUnsignedChar:  PrintStr("unsigned char", Stream); break;
#ifndef NO_FP
        case TypeFP:            PrintStr("double", Stream); break;
#endif
        case TypeFunction:      PrintStr("function", Stream); break;
        case TypeMacro:         PrintStr("macro", Stream); break;
        case TypePointer:       if (Typ->FromType) PrintType(Typ->FromType, Stream); PrintCh('*', Stream); break;
        case TypeArray:         PrintType(Typ->FromType, Stream); PrintCh('[', Stream); if (Typ->ArraySize != 0) PrintSimpleInt(Typ->ArraySize, Stream); PrintCh(']', Stream); break;
        case TypeStruct:        PrintStr("struct ", Stream); PrintStr( Typ->Identifier, Stream); break;
        case TypeUnion:         PrintStr("union ", Stream); PrintStr(Typ->Identifier, Stream); break;
        case TypeEnum:          PrintStr("enum ", Stream); PrintStr(Typ->Identifier, Stream); break;
        case TypeGotoLabel:     PrintStr("goto label ", Stream); break;
        case Type_Type:         PrintStr("type ", Stream); break;
    }
}


#ifdef BUILTIN_MINI_STDLIB

/* 
 * This is a simplified standard library for small embedded systems. It doesn't require
 * a system stdio library to operate.
 *
 * A more complete standard library for larger computers is in the library_XXX.c files.
 */
 
static int TRUEValue = 1;
static int ZeroValue = 0;

void BasicIOInit(Picoc *pc)
{
    pc->CStdOutBase.Putch = &PlatformPutc;
    pc->CStdOut = &pc->CStdOutBase;
}

/* initialise the C library */
void CLibraryInit(Picoc *pc)
{
    /* define some constants */
    VariableDefinePlatformVar(pc, NILL, "NULL", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ZeroValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "TRUE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&TRUEValue), FALSE);
    VariableDefinePlatformVar(pc, NILL, "FALSE", ptrWrap(&pc->IntType), (TAnyValuePtr)ptrWrap(&ZeroValue), FALSE);
}

/* stream for writing into strings */
void SPutc(unsigned char Ch, union OutputStreamInfo *Stream)
{
    struct OutputStreamInfo::StringOutputStream *Out = &Stream->Str;
    *Out->WritePos++ = Ch;
}

/* print a character to a stream without using printf/sprintf */
void PrintCh(char OutCh, struct OutputStream *Stream)
{
    (*Stream->Putch)(OutCh, &Stream->i);
}

/* print a string to a stream without using printf/sprintf */
void PrintStr(const char *Str, struct OutputStream *Stream)
{
    while (*Str != 0)
        PrintCh(*Str++, Stream);
}

/* print a string to a stream without using printf/sprintf */
void PrintStr(TConstRegStringPtr Str, struct OutputStream *Stream)
{
    while (*Str != 0)
        PrintCh(*Str++, Stream);
}

/* print a single character a given number of times */
void PrintRepeatedChar(char ShowChar, int Length, struct OutputStream *Stream)
{
    while (Length-- > 0)
        PrintCh(ShowChar, Stream);
}

/* print an unsigned integer to a stream without using printf/sprintf */
void PrintUnsigned(unsigned long Num, unsigned int Base, int FieldWidth, int ZeroPad, int LeftJustify, struct OutputStream *Stream)
{
    char Result[33];
    int ResPos = sizeof(Result);

    Result[--ResPos] = '\0';
    if (Num == 0)
        Result[--ResPos] = '0';
            
    while (Num > 0)
    {
        unsigned long NextNum = Num / Base;
        unsigned long Digit = Num - NextNum * Base;
        if (Digit < 10)
            Result[--ResPos] = '0' + Digit;
        else
            Result[--ResPos] = 'a' + Digit - 10;
        
        Num = NextNum;
    }
    
    if (FieldWidth > 0 && !LeftJustify)
        PrintRepeatedChar(ZeroPad ? '0' : ' ', FieldWidth - (sizeof(Result) - 1 - ResPos), Stream);
        
    PrintStr(&Result[ResPos], Stream);

    if (FieldWidth > 0 && LeftJustify)
        PrintRepeatedChar(' ', FieldWidth - (sizeof(Result) - 1 - ResPos), Stream);
}

/* print an integer to a stream without using printf/sprintf */
void PrintSimpleInt(long Num, struct OutputStream *Stream)
{
    PrintInt(Num, -1, FALSE, FALSE, Stream);
}

/* print an integer to a stream without using printf/sprintf */
void PrintInt(long Num, int FieldWidth, int ZeroPad, int LeftJustify, struct OutputStream *Stream)
{
    if (Num < 0)
    {
        PrintCh('-', Stream);
        Num = -Num;
        if (FieldWidth != 0)
            FieldWidth--;
    }
    
    PrintUnsigned((unsigned long)Num, 10, FieldWidth, ZeroPad, LeftJustify, Stream);
}

#ifndef NO_FP
/* print a double to a stream without using printf/sprintf */
void PrintFP(double Num, struct OutputStream *Stream)
{
    int Exponent = 0;
    int MaxDecimal;
    
    if (Num < 0)
    {
        PrintCh('-', Stream);
        Num = -Num;    
    }
    
    if (Num >= 1e7)
        Exponent = log10(Num);
    else if (Num <= 1e-7 && Num != 0.0)
        Exponent = log10(Num) - 0.999999999;
    
    Num /= pow(10.0, Exponent);    
    PrintInt((long)Num, 0, FALSE, FALSE, Stream);
    PrintCh('.', Stream);
    Num = (Num - (long)Num) * 10;
    if (fabs(Num) >= 1e-7)
    {
        for (MaxDecimal = 6; MaxDecimal > 0 && fabs(Num) >= 1e-7; Num = (Num - (long)(Num + 1e-7)) * 10, MaxDecimal--)
            PrintCh('0' + (long)(Num + 1e-7), Stream);
    }
    else
        PrintCh('0', Stream);
        
    if (Exponent != 0)
    {
        PrintCh('e', Stream);
        PrintInt(Exponent, 0, FALSE, FALSE, Stream);
    }
}
#endif

#ifndef NO_PRINTF
/* intrinsic functions made available to the language */
void GenericPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs, struct OutputStream *Stream)
{
    TStdioCharPtr FPos;
    TValuePtr NextArg = Param[0];
    TValueTypePtr FormatType;
    int ArgCount = 1;
    int LeftJustify = FALSE;
    int ZeroPad = FALSE;
    int FieldWidth = 0;
    TStdioCharPtr Format = (TStdioCharPtr)Param[0]->Val->Pointer;
    Picoc *pc = Parser->pc;
    
    for (FPos = Format; *FPos != '\0'; FPos++)
    {
        if (*FPos == '%')
        {
            FPos++;
            FieldWidth = 0;
            if (*FPos == '-')
            {
                /* a leading '-' means left justify */
                LeftJustify = TRUE;
                FPos++;
            }
            
            if (*FPos == '0')
            {
                /* a leading zero means zero pad a decimal number */
                ZeroPad = TRUE;
                FPos++;
            }
            
            /* get any field width in the format */
            while (isdigit((int)*FPos))
                FieldWidth = FieldWidth * 10 + (*FPos++ - '0');
            
            /* now check the format type */
            switch ((char)*FPos)
            {
                case 's': FormatType = pc->CharPtrType; break;
                case 'd': case 'u': case 'x': case 'b': case 'c': FormatType = ptrWrap(&pc->IntType); break;
#ifndef NO_FP
                case 'f': FormatType = ptrWrap(&pc->FPType); break;
#endif
                case '%': PrintCh('%', Stream); FormatType = NILL; break;
                case '\0': FPos--; FormatType = NILL; break;
                default:  PrintCh(*FPos, Stream); FormatType = NILL; break;
            }
            
            if (FormatType != NULL)
            { 
                /* we have to format something */
                if (ArgCount >= NumArgs)
                    PrintStr("XXX", Stream);   /* not enough parameters for format */
                else
                {
                    NextArg = (TValuePtr)((TValueCharPointer)(NextArg) + MEM_ALIGN(sizeof(struct Value) + TypeStackSizeValue(NextArg)));
                    if (NextArg->Typ != FormatType && 
                            !((FormatType == ptrWrap(&pc->IntType) || *FPos == 'f') && IS_NUMERIC_COERCIBLE(NextArg)) &&
                            !(FormatType == pc->CharPtrType && (NextArg->Typ->Base == TypePointer || 
                                                             (NextArg->Typ->Base == TypeArray && NextArg->Typ->FromType->Base == TypeChar) ) ) )
                        PrintStr("XXX", Stream);   /* bad type for format */
                    else
                    {
                        switch ((char)*FPos)
                        {
                            case 's':
                            {
                                TAnyValueCharPointer Str;
                                
                                if (NextArg->Typ->Base == TypePointer)
                                    Str = (TAnyValueCharPointer)NextArg->Val->Pointer;
                                else
                                    Str = &NextArg->Val->ArrayMem[0];
                                    
                                if (Str == NULL)
                                    PrintStr("NULL", Stream); 
                                else
                                    PrintStr(Str, Stream); 
                                break;
                            }
                            case 'd': PrintInt(ExpressionCoerceInteger(NextArg), FieldWidth, ZeroPad, LeftJustify, Stream); break;
                            case 'u': PrintUnsigned(ExpressionCoerceUnsignedInteger(NextArg), 10, FieldWidth, ZeroPad, LeftJustify, Stream); break;
                            case 'x': PrintUnsigned(ExpressionCoerceUnsignedInteger(NextArg), 16, FieldWidth, ZeroPad, LeftJustify, Stream); break;
                            case 'b': PrintUnsigned(ExpressionCoerceUnsignedInteger(NextArg), 2, FieldWidth, ZeroPad, LeftJustify, Stream); break;
                            case 'c': PrintCh(ExpressionCoerceUnsignedInteger(NextArg), Stream); break;
#ifndef NO_FP
                            case 'f': PrintFP(ExpressionCoerceFP(NextArg), Stream); break;
#endif
                        }
                    }
                }
                
                ArgCount++;
            }
        }
        else
            PrintCh(*FPos, Stream);
    }
}

/* printf(): print to console output */
void LibPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    struct OutputStream ConsoleStream;
    
    ConsoleStream.Putch = &PlatformPutc;
    GenericPrintf(Parser, ReturnValue, Param, NumArgs, &ConsoleStream);
}

/* sprintf(): print to a string */
void LibSPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    struct OutputStream StrStream;
    
    StrStream.Putch = &SPutc;
    StrStream.i.Str.Parser = Parser;
    StrStream.i.Str.WritePos = (TAnyValueCharPointer)Param[0]->Val->Pointer;

    GenericPrintf(Parser, ReturnValue, Param+1, NumArgs-1, &StrStream);
    PrintCh(0, &StrStream);
    ReturnValue->Val->Pointer = *Param;
}
#endif

/* get a line of input. protected from buffer overrun */
void LibGets(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    // UNDONE: this function doesn't seem very safe? (how do we now the user supplies enough space?)

#ifdef WRAP_ANYVALUE
    CPtrWrapLock l(Param[0]->Val->Pointer);

    if (PlatformGetLine((char *)&l, GETS_BUF_MAX, NULL) != NULL)
    {
        char *EOLPos = strchr((char *)&l, '\n');
        if (EOLPos != NULL)
            *EOLPos = '\0';

        ReturnValue->Val->Pointer = Param[0]->Val->Pointer;
    }
    else
        ReturnValue->Val->Pointer = NILL;
#else
    ReturnValue->Val->Pointer = PlatformGetLine((char *)Param[0]->Val->Pointer, GETS_BUF_MAX, NULL);
    if (ReturnValue->Val->Pointer != NULL)
    {
        char *EOLPos = strchr((char *)Param[0]->Val->Pointer, '\n');
        if (EOLPos != NULL)
            *EOLPos = '\0';
    }
#endif
}

void LibGetc(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Integer = PlatformGetCharacter();
}

void LibExit(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    PlatformExit(Parser->pc, Param[0]->Val->Integer);
}

#ifdef PICOC_LIBRARY
void LibSin(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = sin(Param[0]->Val->FP);
}

void LibCos(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = cos(Param[0]->Val->FP);
}

void LibTan(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = tan(Param[0]->Val->FP);
}

void LibAsin(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = asin(Param[0]->Val->FP);
}

void LibAcos(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = acos(Param[0]->Val->FP);
}

void LibAtan(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = atan(Param[0]->Val->FP);
}

void LibSinh(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = sinh(Param[0]->Val->FP);
}

void LibCosh(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = cosh(Param[0]->Val->FP);
}

void LibTanh(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = tanh(Param[0]->Val->FP);
}

void LibExp(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = exp(Param[0]->Val->FP);
}

void LibFabs(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = fabs(Param[0]->Val->FP);
}

void LibLog(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = log(Param[0]->Val->FP);
}

void LibLog10(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = log10(Param[0]->Val->FP);
}

void LibPow(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = pow(Param[0]->Val->FP, Param[1]->Val->FP);
}

void LibSqrt(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = sqrt(Param[0]->Val->FP);
}

void LibRound(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = floor(Param[0]->Val->FP + 0.5);   /* XXX - fix for soft float */
}

void LibCeil(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = ceil(Param[0]->Val->FP);
}

void LibFloor(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->FP = floor(Param[0]->Val->FP);
}
#endif

#ifndef NO_STRING_FUNCTIONS
void LibMalloc(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Pointer = ptrWrap(malloc(Param[0]->Val->Integer));
}

#ifndef NO_CALLOC
void LibCalloc(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Pointer = ptrWrap(calloc(Param[0]->Val->Integer, Param[1]->Val->Integer));
}
#endif

#ifndef NO_REALLOC
void LibRealloc(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    ReturnValue->Val->Pointer = ptrWrap(realloc(ptrUnwrap(Param[0]->Val->Pointer), Param[1]->Val->Integer));
}
#endif

void LibFree(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    free(ptrUnwrap(Param[0]->Val->Pointer));
}

void LibStrcpy(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer To = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    TAnyValueCharPointer From = (TAnyValueCharPointer)Param[1]->Val->Pointer;
    
    while (*From != '\0')
        *To++ = *From++;
    
    *To = '\0';
}

void LibStrncpy(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer To = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    TAnyValueCharPointer From = (TAnyValueCharPointer)Param[1]->Val->Pointer;
    int Len = Param[2]->Val->Integer;
    
    for (; *From != '\0' && Len > 0; Len--)
        *To++ = *From++;
    
    if (Len > 0)
        *To = '\0';
}

void LibStrcmp(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer Str1 = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    TAnyValueCharPointer Str2 = (TAnyValueCharPointer)Param[1]->Val->Pointer;
    int StrEnded;
    
    for (StrEnded = FALSE; !StrEnded; StrEnded = (*Str1 == '\0' || *Str2 == '\0'), Str1++, Str2++)
    {
         if (*Str1 < *Str2) { ReturnValue->Val->Integer = -1; return; } 
         else if (*Str1 > *Str2) { ReturnValue->Val->Integer = 1; return; }
    }
    
    ReturnValue->Val->Integer = 0;
}

void LibStrncmp(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer Str1 = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    TAnyValueCharPointer Str2 = (TAnyValueCharPointer)Param[1]->Val->Pointer;
    int Len = Param[2]->Val->Integer;
    int StrEnded;
    
    for (StrEnded = FALSE; !StrEnded && Len > 0; StrEnded = (*Str1 == '\0' || *Str2 == '\0'), Str1++, Str2++, Len--)
    {
         if (*Str1 < *Str2) { ReturnValue->Val->Integer = -1; return; } 
         else if (*Str1 > *Str2) { ReturnValue->Val->Integer = 1; return; }
    }
    
    ReturnValue->Val->Integer = 0;
}

void LibStrcat(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer To = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    TAnyValueCharPointer From = (TAnyValueCharPointer)Param[1]->Val->Pointer;
    
    while (*To != '\0')
        To++;
    
    while (*From != '\0')
        *To++ = *From++;
    
    *To = '\0';
}

void LibIndex(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer Pos = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    int SearchChar = Param[1]->Val->Integer;

    while (*Pos != '\0' && *Pos != SearchChar)
        Pos++;
    
    if (*Pos != SearchChar)
        ReturnValue->Val->Pointer = NILL;
    else
        ReturnValue->Val->Pointer = Pos;
}

void LibRindex(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer Pos = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    int SearchChar = Param[1]->Val->Integer;

    ReturnValue->Val->Pointer = NILL;
    for (; *Pos != '\0'; Pos++)
    {
        if (*Pos == SearchChar)
            ReturnValue->Val->Pointer = Pos;
    }
}

void LibStrlen(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueCharPointer Pos = (TAnyValueCharPointer)Param[0]->Val->Pointer;
    int Len;
    
    for (Len = 0; *Pos != '\0'; Pos++)
        Len++;
    
    ReturnValue->Val->Integer = Len;
}

void LibMemset(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    /* we can use the system memset() */
    memset(Param[0]->Val->Pointer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

void LibMemcpy(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    /* we can use the system memcpy() */
    memcpy(Param[0]->Val->Pointer, Param[1]->Val->Pointer, Param[2]->Val->Integer);
}

void LibMemcmp(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs)
{
    TAnyValueUCharPointer Mem1 = (TAnyValueUCharPointer)Param[0]->Val->Pointer;
    TAnyValueUCharPointer Mem2 = (TAnyValueUCharPointer)Param[1]->Val->Pointer;
    int Len = Param[2]->Val->Integer;
    
    for (; Len > 0; Mem1++, Mem2++, Len--)
    {
         if (*Mem1 < *Mem2) { ReturnValue->Val->Integer = -1; return; } 
         else if (*Mem1 > *Mem2) { ReturnValue->Val->Integer = 1; return; }
    }
    
    ReturnValue->Val->Integer = 0;
}
#endif

/* list of all library functions and their prototypes */
const struct LibraryFunction CLibrary[] =
{
#ifndef NO_PRINTF
    { LibPrintf,        "void printf(char *, ...);" },
    { LibSPrintf,       "char *sprintf(char *, char *, ...);" },
#endif
    { LibGets,          "char *gets(char *);" },
    { LibGetc,          "int getchar();" },
    { LibExit,          "void exit(int);" },
#ifdef PICOC_LIBRARY
    { LibSin,           "float sin(float);" },
    { LibCos,           "float cos(float);" },
    { LibTan,           "float tan(float);" },
    { LibAsin,          "float asin(float);" },
    { LibAcos,          "float acos(float);" },
    { LibAtan,          "float atan(float);" },
    { LibSinh,          "float sinh(float);" },
    { LibCosh,          "float cosh(float);" },
    { LibTanh,          "float tanh(float);" },
    { LibExp,           "float exp(float);" },
    { LibFabs,          "float fabs(float);" },
    { LibLog,           "float log(float);" },
    { LibLog10,         "float log10(float);" },
    { LibPow,           "float pow(float,float);" },
    { LibSqrt,          "float sqrt(float);" },
    { LibRound,         "float round(float);" },
    { LibCeil,          "float ceil(float);" },
    { LibFloor,         "float floor(float);" },
#endif
#ifndef NO_STRING_FUNCTIONS
    { LibMalloc,        "void *malloc(int);" },
#endif
#ifndef NO_CALLOC
    { LibCalloc,        "void *calloc(int,int);" },
#endif
#ifndef NO_REALLOC
    { LibRealloc,       "void *realloc(void *,int);" },
#endif
#ifndef NO_STRING_FUNCTIONS
    { LibFree,          "void free(void *);" },
    { LibStrcpy,        "void strcpy(char *,char *);" },
    { LibStrncpy,       "void strncpy(char *,char *,int);" },
    { LibStrcmp,        "int strcmp(char *,char *);" },
    { LibStrncmp,       "int strncmp(char *,char *,int);" },
    { LibStrcat,        "void strcat(char *,char *);" },
    { LibIndex,         "char *index(char *,int);" },
    { LibRindex,        "char *rindex(char *,int);" },
    { LibStrlen,        "int strlen(char *);" },
    { LibMemset,        "void memset(void *,int,int);" },
    { LibMemcpy,        "void memcpy(void *,void *,int);" },
    { LibMemcmp,        "int memcmp(void *,void *,int);" },
#endif
    { NULL,             NULL }
};

#endif /* BUILTIN_MINI_STDLIB */
