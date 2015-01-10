/* picoc main header file - this has all the main data structures and 
 * function prototypes. If you're just calling picoc you should look at the
 * external interface instead, in picoc.h */
 
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "platform.h"
#include "vmem_utils.h"

/* handy definitions */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

#define MEM_ALIGN(x) (((x) + sizeof(ALIGN_TYPE) - 1) & ~(sizeof(ALIGN_TYPE)-1))

#define GETS_BUF_MAX 256

/* for debugging */
#define PRINT_SOURCE_POS ({ PrintSourceTextErrorLine(Parser->pc->CStdOut, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos); PlatformPrintf(Parser->pc->CStdOut, "\n"); })
#define PRINT_TYPE(typ) PlatformPrintf(Parser->pc->CStdOut, "%t\n", typ);

/* small processors use a simplified FILE * for stdio, otherwise use the system FILE * */
#ifdef BUILTIN_MINI_STDLIB
typedef struct OutputStream IOFILE;
#else
typedef FILE IOFILE;
#endif

/* coercion of numeric types to other numeric types */
#ifndef NO_FP
#define IS_FP(v) ((v)->Typ->Base == TypeFP)
#define FP_VAL(v) ((v)->Val->FP)
#else
#define IS_FP(v) 0
#define FP_VAL(v) 0
#endif

#define IS_POINTER_COERCIBLE(v, ap) ((ap) ? ((v)->Typ->Base == TypePointer) : 0)
#define POINTER_COERCE(v) ((int)(v)->Val->Pointer)

#define IS_INTEGER_NUMERIC_TYPE(t) ((t)->Base >= TypeInt && (t)->Base <= TypeUnsignedLong)
#define IS_INTEGER_NUMERIC(v) IS_INTEGER_NUMERIC_TYPE((v)->Typ)
#define IS_NUMERIC_COERCIBLE(v) (IS_INTEGER_NUMERIC(v) || IS_FP(v))
#define IS_NUMERIC_COERCIBLE_PLUS_POINTERS(v,ap) (IS_NUMERIC_COERCIBLE(v) || IS_POINTER_COERCIBLE(v,ap))


struct Table;
struct Picoc_Struct;

typedef struct Picoc_Struct Picoc;

/* lexical tokens */
enum LexToken
{
    /* 0x00 */ TokenNone, 
    /* 0x01 */ TokenComma,
    /* 0x02 */ TokenAssign, TokenAddAssign, TokenSubtractAssign, TokenMultiplyAssign, TokenDivideAssign, TokenModulusAssign,
    /* 0x08 */ TokenShiftLeftAssign, TokenShiftRightAssign, TokenArithmeticAndAssign, TokenArithmeticOrAssign, TokenArithmeticExorAssign,
    /* 0x0d */ TokenQuestionMark, TokenColon, 
    /* 0x0f */ TokenLogicalOr, 
    /* 0x10 */ TokenLogicalAnd, 
    /* 0x11 */ TokenArithmeticOr, 
    /* 0x12 */ TokenArithmeticExor, 
    /* 0x13 */ TokenAmpersand, 
    /* 0x14 */ TokenEqual, TokenNotEqual, 
    /* 0x16 */ TokenLessThan, TokenGreaterThan, TokenLessEqual, TokenGreaterEqual,
    /* 0x1a */ TokenShiftLeft, TokenShiftRight, 
    /* 0x1c */ TokenPlus, TokenMinus, 
    /* 0x1e */ TokenAsterisk, TokenSlash, TokenModulus,
    /* 0x21 */ TokenIncrement, TokenDecrement, TokenUnaryNot, TokenUnaryExor, TokenSizeof, TokenCast,
    /* 0x27 */ TokenLeftSquareBracket, TokenRightSquareBracket, TokenDot, TokenArrow, 
    /* 0x2b */ TokenOpenBracket, TokenCloseBracket,
    /* 0x2d */ TokenIdentifier, TokenIntegerConstant, TokenFPConstant, TokenStringConstant, TokenCharacterConstant,
    /* 0x32 */ TokenSemicolon, TokenEllipsis,
    /* 0x34 */ TokenLeftBrace, TokenRightBrace,
    /* 0x36 */ TokenIntType, TokenCharType, TokenFloatType, TokenDoubleType, TokenVoidType, TokenEnumType,
    /* 0x3c */ TokenLongType, TokenSignedType, TokenShortType, TokenStaticType, TokenAutoType, TokenRegisterType, TokenExternType, TokenStructType, TokenUnionType, TokenUnsignedType, TokenTypedef,
    /* 0x46 */ TokenContinue, TokenDo, TokenElse, TokenFor, TokenGoto, TokenIf, TokenWhile, TokenBreak, TokenSwitch, TokenCase, TokenDefault, TokenReturn,
    /* 0x52 */ TokenHashDefine, TokenHashInclude, TokenHashIf, TokenHashIfdef, TokenHashIfndef, TokenHashElse, TokenHashEndif,
    /* 0x59 */ TokenNew, TokenDelete,
    /* 0x5b */ TokenOpenMacroBracket,
    /* 0x5c */ TokenEOF, TokenEndOfLine, TokenEndOfFunction
};

/* some enums to replace boolean flags and save some bytes */
enum ValueTypeFlags
{
    FlagValTypeNone = 0,
    FlagOnHeap = (1<<0),             /* true if allocated on the heap */
    FlagStaticQualifier = (1<<1)     /* true if it's a static */
};

enum ValueFlags
{
    FlagValNone = 0,
    FlagValOnHeap = (1<<0),        /* this Value is on the heap */
    FlagOnStack = (1<<1),       /* the AnyValue is on the stack along with this Value */
    FlagAnyValOnHeap = (1<<2),  /* the AnyValue is separately allocated from the Value on the heap */
    FlagIsLValue = (1<<3),      /* is modifiable and is allocated somewhere we can usefully modify it */
    FlagOutOfScope = (1<<4)
};

/* used in dynamic memory allocation */
struct AllocNode
{
    unsigned int Size;
    struct AllocNode *NextFree;
};

/* whether we're running or skipping code */
enum RunMode
{
    RunModeRun,                 /* we're running code as we parse it */
    RunModeSkip,                /* skipping code, not running */
    RunModeReturn,              /* returning from a function */
    RunModeCaseSearch,          /* searching for a case label */
    RunModeBreak,               /* breaking out of a switch/while/do */
    RunModeContinue,            /* as above but repeat the loop */
    RunModeGoto                 /* searching for a goto label */
};

/* parser state - has all this detail so we can parse nested files */
struct ParseState
{
    Picoc *pc;                  /* the picoc instance this parser is a part of */
    TLexBufPtr Pos;             /* the character position in the source text */
    TRegStringPtr FileName;     /* what file we're executing (registered string) */
    void *LineFilePointer;      /* Pointer to file data for line by line parsing (Similar to Interactive mode) */
    short int Line;             /* line number we're executing */
    short int CharacterPos;     /* character/column in the line we're executing */
    enum RunMode Mode;          /* whether to skip or run code */
    int SearchLabel;            /* what case label we're searching for */
    TConstRegStringPtr SearchGotoLabel;/* what goto label we're searching for */
    const char *SourceText;     /* the entire source text */
    short int HashIfLevel;      /* how many "if"s we're nested down */
    short int HashIfEvaluateToLevel;    /* if we're not evaluating an if branch, what the last evaluated level was */
    char DebugMode;             /* debugging mode */
    int16_t ScopeID;            /* for keeping track of local variables (free them after they go out of scope) */
};

/* values */
enum BaseType
{
    TypeVoid,                   /* no type */
    TypeInt,                    /* integer */
    TypeShort,                  /* short integer */
    TypeChar,                   /* a single character (signed) */
    TypeLong,                   /* long integer */
    TypeUnsignedInt,            /* unsigned integer */
    TypeUnsignedShort,          /* unsigned short integer */
    TypeUnsignedChar,           /* unsigned 8-bit number */ /* must be before unsigned long */
    TypeUnsignedLong,           /* unsigned long integer */
#ifndef NO_FP
    TypeFP,                     /* floating point */
#endif
    TypeFunction,               /* a function */
    TypeMacro,                  /* a macro */
    TypePointer,                /* a pointer */
    TypeArray,                  /* an array of a sub-type */
    TypeStruct,                 /* aggregate type */
    TypeUnion,                  /* merged type */
    TypeEnum,                   /* enumerated integer type */
    TypeGotoLabel,              /* a label we can "goto" */
    Type_Type                   /* a type for storing types */
};

/* data type */
struct ValueType
{
    enum BaseType Base;             /* what kind of type this is */
    uint16_t ArraySize;             /* the size of an array type */
    uint16_t Sizeof;                /* the storage required */
    uint8_t AlignBytes;             /* the alignment boundary of this type */
    TConstRegStringPtr Identifier;  /* the name of a struct or union */
    TValueTypePtr FromType;     /* the type we're derived from (or NULL) */
    TValueTypePtr DerivedTypeList;  /* first in a list of types derived from this one */
    TValueTypePtr Next;         /* next item in the derived type list */
    TTablePtr Members;          /* members of a struct or union */
    uint8_t Flags;
};

/* function definition */
struct FuncDef
{
    TValueTypePtr ReturnType;   /* the return value type */
    int8_t NumParams;                  /* the number of parameters */
    int8_t VarArgs;                    /* has a variable number of arguments after the explicitly specified ones */
    TValueTypePtrPtr ParamType;   /* array of parameter types */
    TRegStringPtrPtr ParamName;     /* array of parameter names */
    void (*Intrinsic)(TParseStatePtr , TValuePtr, TValuePtrPtr, int); /* intrinsic call address or NULL */
    TParseStatePtr Body;        /* lexical tokens of the function body if not intrinsic (otherwise NULL) */
};

/* macro definition */
struct MacroDef
{
    int8_t NumParams;                  /* the number of parameters */
    TRegStringPtrPtr ParamName;        /* array of parameter names */
    struct ParseState Body;         /* lexical tokens of the function body if not intrinsic */
};

/* values */
union AnyValue
{
    char Character;
    short ShortInteger;
    int Integer;
    long LongInteger;
    unsigned short UnsignedShortInteger;
    unsigned int UnsignedInteger;
    unsigned long UnsignedLongInteger;
    unsigned char UnsignedCharacter;
    TRegStringPtr Identifier;
#ifdef WRAP_ANYVALUE
    TAnyValueCharPtr ArrayMem; /* placeholder for where the data starts, doesn't point to it */
#else
    char ArrayMem[2];               /* placeholder for where the data starts, doesn't point to it */
#endif
    TValueTypePtr Typ;
    struct FuncDef FuncDef;
    struct MacroDef MacroDef;
#ifndef NO_FP
    double FP;
#endif
    TAnyValueVoidPtr Pointer;                  /* unsafe native pointers */
};

struct Value
{
    TValueTypePtr Typ;          /* the type of this value */
    TAnyValuePtr Val;            /* pointer to the AnyValue which holds the actual content */
    TValuePtr LValueFrom;       /* if an LValue, this is a Value our LValue is contained within (or NULL) */
    uint8_t Flags;
    int16_t ScopeID;                    /* to know when it goes out of scope */
};

/* Used to disable usage of DeclFileName, DeclLine and DeclColumn from TableEntry (doesn't seem to be necessary) */
#define DISABLE_TABLEENTRY_DECL

/* hash table data structure */
struct TableEntry
{
    TTableEntryPtr Next;        /* next item in this hash chain */
#ifndef DISABLE_TABLEENTRY_DECL
    const char *DeclFileName;       /* where the variable was declared */
    unsigned short DeclLine;
    unsigned short DeclColumn;
#endif

    union TableEntryPayload
    {
        struct ValueEntry
        {
            TRegStringPtr Key;     /* points to the shared string table */
            TValuePtr Val;          /* the value we're storing */
        } v;                        /* used for tables of values */
        
#ifdef WRAP_REGSTRINGS
        TRegStringPtr Key;
#else
        char Key[1];                /* dummy size - used for the shared string table */
#endif
        
        struct BreakpointEntry      /* defines a breakpoint */
        {
            TConstRegStringPtr FileName;
            short int Line;
            short int CharacterPos;
        } b;
        
    } p;
};
    
struct Table
{
    short Size;
    short OnHeap;
    TTableEntryPtrPtr HashTable;
};

/* stack frame for function calls */
struct StackFrame
{
    struct ParseState ReturnParser;         /* how we got here */
    TConstRegStringPtr FuncName;            /* the name of the function we're in */
    TValuePtr ReturnValue;                  /* copy the return value here */
    TValuePtrPtr Parameter;                 /* array of parameter values */
    int8_t NumParams;                       /* the number of parameters */
    struct Table LocalTable;                /* the local variables and parameters */
    TTableEntryPtr LocalHashTable[LOCAL_TABLE_SIZE];
    TStackFramePtr PreviousStackFrame;      /* the next lower stack frame */
};

/* lexer state */
enum LexMode
{
    LexModeNormal,
    LexModeHashInclude,
    LexModeHashDefine,
    LexModeHashDefineSpace,
    LexModeHashDefineSpaceIdent
};

struct LexState
{
    TLexConstCharPtr Pos;
    TLexConstCharPtr End;
    TConstRegStringPtr FileName;
    int Line;
    int CharacterPos;
    const char *SourceText;
    enum LexMode Mode;
    int EmitExtraNewlines;
};

/* library function definition */
struct LibraryFunction
{
    void (*Func)(TParseStatePtr Parser, TValuePtr, TValuePtrPtr , int);
    const char *Prototype;
};

/* output stream-type specific state information */
union OutputStreamInfo
{
    struct StringOutputStream
    {
        TParseStatePtr Parser;
        TAnyValueCharPtr WritePos;
    } Str;
};

/* stream-specific method for writing characters to the console */
typedef void CharWriter(unsigned char, union OutputStreamInfo *);

/* used when writing output to a string - eg. sprintf() */
struct OutputStream
{
    CharWriter *Putch;
    union OutputStreamInfo i;
};

/* possible results of parsing a statement */
enum ParseResult { ParseResultEOF, ParseResultError, ParseResultOk };

/* a chunk of heap-allocated tokens we'll cleanup when we're done */
struct CleanupTokenNode
{
    TLexBufPtr Tokens;
    const char *SourceText;
    TCleanupNodePtr Next;
};

/* linked list of lexical tokens used in interactive mode */
struct TokenLine
{
    TTokenLinePtr Next;
    TLexBufPtr Tokens;
    int NumBytes;
};


/* a list of libraries we can include */
struct IncludeLibrary
{
    TRegStringPtr IncludeName;
    void (*SetupFunction)(Picoc *pc);
    struct LibraryFunction *FuncList;
    const char *SetupCSource;
    TIncludeLibraryPtr NextLib;
};

#define FREELIST_BUCKETS 8                          /* freelists for 4, 8, 12 ... 32 byte allocs */
#define SPLIT_MEM_THRESHOLD 16                      /* don't split memory which is close in size */
#define BREAKPOINT_TABLE_SIZE 21

/* the entire state of the picoc system */
struct Picoc_Struct
{
    /* parser global data */
    struct Table GlobalTable;
    TCleanupNodePtr CleanupTokenList;
    TTableEntryPtr GlobalHashTable[GLOBAL_TABLE_SIZE];
    
    /* lexer global data */
    TTokenLinePtr InteractiveHead;
    TTokenLinePtr InteractiveTail;
    TTokenLinePtr InteractiveCurrentLine;
    int LexUseStatementPrompt;
    union AnyValue LexAnyValue;
    struct Value LexValue;
    struct Table ReservedWordTable;
    TTableEntryPtr ReservedWordHashTable[RESERVED_WORD_TABLE_SIZE];

    /* the table of string literal values */
    struct Table StringLiteralTable;
    TTableEntryPtr StringLiteralHashTable[STRING_LITERAL_TABLE_SIZE];
    
    /* the stack */
    TStackFramePtr TopStackFrame;

    /* the value passed to exit() */
    int PicocExitValue;

    /* a list of libraries we can include */
    TIncludeLibraryPtr IncludeLibList;

    /* heap memory */
#ifdef USE_MALLOC_STACK
    unsigned char *HeapMemory;          /* stack memory since our heap is malloc()ed */
    void *HeapBottom;                   /* the bottom of the (downward-growing) heap */
    TStackVoidPtr StackFrame;           /* the current stack frame */
    TStackVoidPtr HeapStackTop;         /* the top of the stack */
    TStackVoidPtr StackBottom;          /* the bottom of the stack */
    TStackCharPtr StackStart;           /* the absolute start of the stack */
#else
# ifdef SURVEYOR_HOST
    unsigned char *HeapMemory;          /* all memory - stack and heap */
    void *HeapBottom;                   /* the bottom of the (downward-growing) heap */
    void *StackFrame;                   /* the current stack frame */
    void *HeapStackTop;                 /* the top of the stack */
    void *HeapMemStart;
# else
    unsigned char HeapMemory[HEAP_SIZE];  /* all memory - stack and heap */
    void *HeapBottom;                     /* the bottom of the (downward-growing) heap */
    TStackVoidPtr StackFrame;             /* the current stack frame */
    TStackVoidPtr HeapStackTop;           /* the top of the stack */
    TStackVoidPtr StackBottom;            /* the bottom of the stack */
    TStackVoidPtr StackStart;             /* the absolute start of the stack */
# endif
#endif

    struct AllocNode *FreeListBucket[FREELIST_BUCKETS];      /* we keep a pool of freelist buckets to reduce fragmentation */
    struct AllocNode *FreeListBig;                           /* free memory which doesn't fit in a bucket */

    /* types */    
    struct ValueType UberType;
    struct ValueType IntType;
    struct ValueType ShortType;
    struct ValueType CharType;
    struct ValueType LongType;
    struct ValueType UnsignedIntType;
    struct ValueType UnsignedShortType;
    struct ValueType UnsignedLongType;
    struct ValueType UnsignedCharType;
    #ifndef NO_FP
    struct ValueType FPType;
    #endif
    struct ValueType VoidType;
    struct ValueType TypeType;
    struct ValueType FunctionType;
    struct ValueType MacroType;
    struct ValueType EnumType;
    struct ValueType GotoLabelType;
    TValueTypePtr CharPtrType;
    TValueTypePtr CharPtrPtrType;
    TValueTypePtr CharArrayType;
    TValueTypePtr VoidPtrType;

    /* debugger */
    struct Table BreakpointTable;
    TTableEntryPtr BreakpointHashTable[BREAKPOINT_TABLE_SIZE];
    int BreakpointCount;
    int DebugManualBreak;
    
    /* C library */
    int BigEndian;
    int LittleEndian;

    IOFILE *CStdOut;
    IOFILE CStdOutBase;

    /* the picoc version string */
    TConstRegStringPtr VersionString;
    
    /* exit longjump buffer */
#if defined(UNIX_HOST) || defined(WIN32)
    jmp_buf PicocExitBuf;
#endif
#ifdef SURVEYOR_HOST
    int PicocExitBuf[41];
#endif
    
    /* string table */
    struct Table StringTable;
    TTableEntryPtr StringHashTable[STRING_TABLE_SIZE];
    TRegStringPtr StrEmpty;
};

/* table.c */
void TableInit(Picoc *pc);
TRegStringPtr TableStrRegister(Picoc *pc, const char *Str);
TRegStringPtr TableStrRegister(Picoc *pc, TConstRegStringPtr Str);
TRegStringPtr TableStrRegister2(Picoc *pc, const char *Str, int Len);
TRegStringPtr TableStrRegister2(Picoc *pc, TConstRegStringPtr Str, int Len);
void TableInitTable(TTablePtr Tbl, TTableEntryPtrPtr HashTable, int Size, int OnHeap);
int TableSet(Picoc *pc, TTablePtr Tbl, TConstRegStringPtr Key, TValuePtr Val, TConstRegStringPtr DeclFileName, int DeclLine, int DeclColumn);
int TableGet(TTablePtr Tbl, TConstRegStringPtr Key, TValuePtrPtr Val, const char **DeclFileName, int *DeclLine, int *DeclColumn);
TValuePtr TableDelete(Picoc *pc, TTablePtr Tbl, const TRegStringPtr Key);
TRegStringPtr TableSetIdentifier(Picoc *pc, TTablePtr Tbl, TConstRegStringPtr Ident, int IdentLen);
void TableStrFree(Picoc *pc);

/* lex.c */
void LexInit(Picoc *pc);
void LexCleanup(Picoc *pc);
TLexBufPtr LexAnalyse(Picoc *pc, TConstRegStringPtr FileName, const char *Source, int SourceLen, int *TokenLen);
void LexInitParser(TParseStatePtr Parser, Picoc *pc, const char *SourceText, TLexBufPtr TokenSource, TRegStringPtr FileName, void *FilePointer, int RunIt, int SetDebugMode);
enum LexToken LexGetToken(TParseStatePtr Parser, TValuePtrPtr Value, int IncPos);
enum LexToken LexRawPeekToken(TParseStatePtr Parser);
void LexToEndOfLine(TParseStatePtr Parser);
TLexBufPtr LexCopyTokens(TParseStatePtr Parser, const TLexBufPtr &StartParserPos, const TLexBufPtr &EndParserPos);
void LexInteractiveClear(Picoc *pc, TParseStatePtr Parser);
void LexInteractiveCompleted(Picoc *pc, TParseStatePtr Parser);
void LexInteractiveStatementPrompt(Picoc *pc);

/* parse.c */
/* the following are defined in picoc.h:
 * void PicocParse(const char *FileName, const char *Source, int SourceLen, int RunIt, int CleanupNow, int CleanupSource);
 * void PicocParseInteractive(); */
void PicocParseInteractiveNoStartPrompt(Picoc *pc, int EnableDebugger);
enum ParseResult ParseStatement(TParseStatePtr Parser, int CheckTrailingSemicolon);
TValuePtr ParseFunctionDefinition(TParseStatePtr Parser, TValueTypePtr ReturnType, TRegStringPtr Identifier);
void ParseCleanup(Picoc *pc);
void ParserCopyPos(TParseStatePtr To, TParseStatePtr From);
void ParserCopy(TParseStatePtr To, TParseStatePtr From);

/* expression.c */
int ExpressionParse(TParseStatePtr Parser, TValuePtrPtr Result);
long ExpressionParseInt(TParseStatePtr Parser);
void ExpressionAssign(TParseStatePtr Parser, TValuePtr DestValue, TValuePtr SourceValue, int Force, TConstRegStringPtr FuncName, int ParamNo, int AllowPointerCoercion);
long ExpressionCoerceInteger(TValuePtr Val);
unsigned long ExpressionCoerceUnsignedInteger(TValuePtr Val);
#ifndef NO_FP
double ExpressionCoerceFP(TValuePtr Val);
#endif

/* type.c */
void TypeInit(Picoc *pc);
void TypeCleanup(Picoc *pc);
int TypeSize(TValueTypePtr Typ, int ArraySize, int Compact);
int SizeOf(TValueTypePtr Typ, int ArraySize, int Compact);
int TypeSizeValue(TValuePtr Val, int Compact);
int TypeStackSizeValue(TValuePtr Val);
int TypeLastAccessibleOffset(Picoc *pc, TValuePtr Val);
int TypeParseFront(TParseStatePtr Parser, TValueTypePtrPtr Typ, int *IsStatic);
void TypeParseIdentPart(TParseStatePtr Parser, TValueTypePtr BasicTyp, TValueTypePtrPtr Typ, TRegStringPtrPtr Identifier);
void TypeParse(TParseStatePtr Parser, TValueTypePtrPtr Typ, TRegStringPtrPtr Identifier, int *IsStatic);
TValueTypePtr TypeGetMatching(Picoc *pc, TParseStatePtr Parser, TValueTypePtr ParentType, enum BaseType Base, int ArraySize, TConstRegStringPtr Identifier, int AllowDuplicates);
TValueTypePtr TypeCreateOpaqueStruct(Picoc *pc, TParseStatePtr Parser, TConstRegStringPtr StructName, int Size);
int TypeIsForwardDeclared(TParseStatePtr Parser, TValueTypePtr Typ);

/* heap.c */
void HeapInit(Picoc *pc, int StackSize);
void HeapCleanup(Picoc *pc);
TStackVoidPtr HeapAllocStack(Picoc *pc, int Size);
int HeapPopStack(Picoc *pc, TStackVoidPtr Addr, int Size);
void HeapUnpopStack(Picoc *pc, int Size);
void HeapPushStackFrame(Picoc *pc);
int HeapPopStackFrame(Picoc *pc);
void *HeapAllocMem(Picoc *pc, int Size);
void HeapFreeMem(Picoc *pc, void *Mem);

/* variable.c */
void VariableInit(Picoc *pc);
void VariableCleanup(Picoc *pc);
void VariableFree(Picoc *pc, TValuePtr Val);
void VariableTableCleanup(Picoc *pc, TTablePtr HashTable);
void *VariableAlloc(Picoc *pc, TParseStatePtr Parser, int Size, int OnHeap);
#ifdef USE_VIRTMEM
TVarAllocRet VariableAllocVirt(Picoc *pc, TParseStatePtr Parser, int Size, int OnHeap);
#endif
void VariableStackPop(TParseStatePtr Parser, TValuePtr Var);
TValuePtr VariableAllocValueAndData(Picoc *pc, TParseStatePtr Parser, int DataSize, int IsLValue, TValuePtr LValueFrom, int OnHeap);
TValuePtr VariableAllocValueAndCopy(Picoc *pc, TParseStatePtr Parser, TValuePtr FromValue, int OnHeap);
TValuePtr VariableAllocValueFromType(Picoc *pc, TParseStatePtr Parser, TValueTypePtr Typ, int IsLValue, TValuePtr LValueFrom, int OnHeap);
TValuePtr VariableAllocValueFromExistingData(TParseStatePtr Parser, TValueTypePtr Typ, TAnyValuePtr FromValue, int IsLValue, TValuePtr LValueFrom);
TValuePtr VariableAllocValueShared(TParseStatePtr Parser, TValuePtr FromValue);
TValuePtr VariableDefine(Picoc *pc, TParseStatePtr Parser, TRegStringPtr Ident, TValuePtr InitValue, TValueTypePtr Typ, int MakeWritable);
TValuePtr VariableDefineButIgnoreIdentical(TParseStatePtr Parser, TRegStringPtr Ident, TValueTypePtr Typ, int IsStatic, int *FirstVisit);
int VariableDefined(Picoc *pc, TConstRegStringPtr Ident);
int VariableDefinedAndOutOfScope(Picoc *pc, TConstRegStringPtr Ident);
void VariableRealloc(TParseStatePtr Parser, TValuePtr FromValue, int NewSize);
void VariableGet(Picoc *pc, TParseStatePtr Parser, TConstRegStringPtr Ident, TValuePtrPtr LVal);
void VariableDefinePlatformVar(Picoc *pc, TParseStatePtr Parser, const char *Ident, TValueTypePtr Typ, TAnyValuePtr FromValue, int IsWritable);
void VariableDefinePlatformVar(Picoc *pc, TParseStatePtr Parser, TConstRegStringPtr Ident, TValueTypePtr Typ, TAnyValuePtr FromValue, int IsWritable);
void VariableStackFrameAdd(TParseStatePtr Parser, TConstRegStringPtr FuncName, int NumParams);
void VariableStackFramePop(TParseStatePtr Parser);
TValuePtr VariableStringLiteralGet(Picoc *pc, TRegStringPtr Ident);
void VariableStringLiteralDefine(Picoc *pc, TRegStringPtr Ident, TValuePtr Val);
TAnyValueVoidPtr VariableDereferencePointer(TParseStatePtr Parser, TValuePtr PointerValue, TValuePtrPtr DerefVal, int *DerefOffset, TValueTypePtrPtr DerefType, int *DerefIsLValue);
int VariableScopeBegin(TParseStatePtr  Parser, int16_t *PrevScopeID);
void VariableScopeEnd(TParseStatePtr  Parser, int ScopeID, int16_t PrevScopeID);

/* clibrary.c */
void BasicIOInit(Picoc *pc);
void LibraryInit(Picoc *pc);
void LibraryAdd(Picoc *pc, TTablePtr GlobalTable, TConstRegStringPtr LibraryName, const struct LibraryFunction *FuncList);
#ifdef USE_VIRTMEM
inline void LibraryAdd(Picoc *pc, TTablePtr GlobalTable, const char *LibraryName, const struct LibraryFunction *FuncList)
{ return LibraryAdd(pc, GlobalTable, ptrWrap(LibraryName), FuncList); }
#endif
void CLibraryInit(Picoc *pc);
void PrintCh(char OutCh, IOFILE *Stream);
void PrintSimpleInt(long Num, IOFILE *Stream);
void PrintInt(long Num, int FieldWidth, int ZeroPad, int LeftJustify, IOFILE *Stream);
void PrintStr(const char *Str, IOFILE *Stream);
void PrintStr(TConstRegStringPtr Str, IOFILE *Stream);
void PrintFP(double Num, IOFILE *Stream);
void PrintType(TValueTypePtr Typ, IOFILE *Stream);
void LibPrintf(TParseStatePtr Parser, TValuePtr ReturnValue, TValuePtrPtr Param, int NumArgs);

#ifdef BUILTIN_MINI_STDLIB
const extern struct LibraryFunction CLibrary[];
#endif

/* platform.c */
/* the following are defined in picoc.h:
 * void PicocCallMain(int argc, char **argv);
 * int PicocPlatformSetExitPoint();
 * void PicocInitialise(int StackSize);
 * void PicocCleanup();
 * void PicocPlatformScanFile(const char *FileName);
 * extern int PicocExitValue; */
void ProgramFail(TParseStatePtr Parser, const char *Message, ...);
void ProgramFailNoParser(Picoc *pc, const char *Message, ...);
void AssignFail(TParseStatePtr Parser, const char *Format, TValueTypePtr Type1, TValueTypePtr Type2, int Num1, int Num2, TConstRegStringPtr FuncName, int ParamNo);
void LexFail(Picoc *pc, struct LexState *Lexer, const char *Message, ...);
void PlatformInit(Picoc *pc);
void PlatformCleanup(Picoc *pc);
char *PlatformGetLine(char *Buf, int MaxLen, const char *Prompt);
char *PlatformGetLineFromFile(char *Buf, int MaxLen, void *FilePointer);
int PlatformGetCharacter();
void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *);
void PlatformPrintf(IOFILE *Stream, const char *Format, ...);
void PlatformVPrintf(IOFILE *Stream, const char *Format, va_list Args);
void PlatformExit(Picoc *pc, int ExitVal);
TRegStringPtr PlatformMakeTempName(Picoc *pc, char *TempNameBuffer);
void PlatformCreatePtrArray(Picoc *pc, TParseStatePtr Parser, const char *Ident, TValueTypePtr Typ, void **Array, int Elements, int ElementSizeof, int IsWritable);
void PlatformLibraryInit(Picoc *pc);

/* include.c */
void IncludeInit(Picoc *pc);
void IncludeCleanup(Picoc *pc);
void IncludeRegister(Picoc *pc, const char *IncludeName, void (*SetupFunction)(Picoc *pc), struct LibraryFunction *FuncList, const char *SetupCSource);
void IncludeFile(Picoc *pc, TRegStringPtr Filename, int LineByLine);
/* the following is defined in picoc.h:
 * void PicocIncludeAllSystemHeaders(); */
 
/* debug.c */
void DebugInit(Picoc *pc);
void DebugCleanup(Picoc *pc);
void DebugCheckStatement(TParseStatePtr Parser);


/* stdio.c */
extern const char StdioDefs[];
extern struct LibraryFunction StdioFunctions[];
void StdioSetupFunc(Picoc *pc);

/* math.c */
extern struct LibraryFunction MathFunctions[];
void MathSetupFunc(Picoc *pc);

/* string.c */
extern struct LibraryFunction StringFunctions[];
void StringSetupFunc(Picoc *pc);

/* stdlib.c */
extern struct LibraryFunction StdlibFunctions[];
void StdlibSetupFunc(Picoc *pc);

/* time.c */
extern const char StdTimeDefs[];
extern struct LibraryFunction StdTimeFunctions[];
void StdTimeSetupFunc(Picoc *pc);

/* errno.c */
void StdErrnoSetupFunc(Picoc *pc);

/* ctype.c */
extern struct LibraryFunction StdCtypeFunctions[];

/* stdbool.c */
extern const char StdboolDefs[];
void StdboolSetupFunc(Picoc *pc);

/* unistd.c */
extern const char UnistdDefs[];
extern struct LibraryFunction UnistdFunctions[];
void UnistdSetupFunc(Picoc *pc);

#endif /* INTERPRETER_H */
