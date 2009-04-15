#ifndef PICOC_H
#define PICOC_H

#include "platform.h"

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

#define MEM_ALIGN(x) (((x) + ARCH_ALIGN_WORDSIZE - 1) & ~(ARCH_ALIGN_WORDSIZE-1))

#define LOG10E 0.43429448190325182765
#define INTERACTIVE_FILE_NAME "input"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

struct Table;

/* lexical tokens */
enum LexToken
{
    TokenNone, 
    TokenComma,
    TokenAssign, TokenAddAssign, TokenSubtractAssign, TokenMultiplyAssign, TokenDivideAssign, TokenModulusAssign,
    TokenShiftLeftAssign, TokenShiftRightAssign, TokenArithmeticAndAssign, TokenArithmeticOrAssign, TokenArithmeticExorAssign,
    TokenQuestionMark, TokenColon, 
    TokenLogicalOr, 
    TokenLogicalAnd, 
    TokenArithmeticOr, 
    TokenArithmeticExor, 
    TokenAmpersand, 
    TokenEqual, TokenNotEqual, 
    TokenLessThan, TokenGreaterThan, TokenLessEqual, TokenGreaterEqual,
    TokenShiftLeft, TokenShiftRight, 
    TokenPlus, TokenMinus, 
    TokenAsterisk, TokenSlash, TokenModulus,
    TokenIncrement, TokenDecrement, TokenUnaryNot, TokenUnaryExor, TokenSizeof,
    TokenLeftSquareBracket, TokenRightSquareBracket, TokenDot, TokenArrow, 
    TokenOpenBracket, TokenCloseBracket,
    TokenIdentifier, TokenIntegerConstant, TokenFPConstant, TokenStringConstant, TokenCharacterConstant,
    TokenSemicolon, TokenEllipsis,
    TokenLeftBrace, TokenRightBrace,
    TokenIntType, TokenCharType, TokenFloatType, TokenDoubleType, TokenVoidType, TokenEnumType,
    TokenLongType, TokenSignedType, TokenShortType, TokenStructType, TokenUnionType, TokenUnsignedType, TokenTypedef,
    TokenContinue, TokenDo, TokenElse, TokenFor, TokenIf, TokenWhile, TokenBreak, TokenSwitch, TokenCase, TokenDefault, TokenReturn,
    TokenHashDefine, TokenHashInclude, TokenNew, TokenDelete,
    TokenEOF, TokenEndOfLine, TokenEndOfFunction
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
    RunModeContinue             /* as above but repeat the loop */
};

/* parser state - has all this detail so we can parse nested files */
struct ParseState
{
    const void *Pos;
    int Line;
    const char *FileName;
    enum RunMode Mode;          /* whether to skip or run code */
    int SearchLabel;            /* what case label we're searching for */
};

/* values */
enum BaseType
{
    TypeVoid,                   /* no type */
    TypeInt,                    /* integer */
#ifndef NO_FP
    TypeFP,                     /* floating point */
#endif
    TypeChar,                   /* a single character - acts like an integer except in machine memory access */
    TypeFunction,               /* a function */
    TypeMacro,                  /* a macro */
    TypePointer,                /* a pointer */
    TypeArray,                  /* an array of a sub-type */
    TypeStruct,                 /* aggregate type */
    TypeUnion,                  /* merged type */
    TypeEnum,                   /* enumated integer type */
    TypeType                    /* a type (eg. typedef) */
};

/* data type */
struct ValueType
{
    enum BaseType Base;             /* what kind of type this is */
    int ArraySize;                  /* the size of an array type */
    int Sizeof;                     /* the storage required */
    const char *Identifier;         /* the name of a struct or union */
    struct ValueType *FromType;     /* the type we're derived from (or NULL) */
    struct ValueType *DerivedTypeList;  /* first in a list of types derived from this one */
    struct ValueType *Next;         /* next item in the derived type list */
    struct Table *Members;          /* members of a struct or union */
    int OnHeap;                     /* true if allocated on the heap */
};

/* function definition */
struct FuncDef
{
    struct ValueType *ReturnType;   /* the return value type */
    int NumParams;                  /* the number of parameters */
    int VarArgs;                    /* has a variable number of arguments after the explicitly specified ones */
    struct ValueType **ParamType;   /* array of parameter types */
    char **ParamName;               /* array of parameter names */
    void (*Intrinsic)();            /* intrinsic call address or NULL */
    struct ParseState Body;         /* lexical tokens of the function body if not intrinsic */
};

/* values */
struct ArrayValue
{
    unsigned int Size;              /* the number of elements in the array */
    void *Data;                     /* pointer to the array data */
};

struct PointerValue
{
    struct Value *Segment;          /* array or basic value which this points to, NULL for machine memory access */
    unsigned int Offset;            /* index into an array */
};

union AnyValue
{
    unsigned char Character;
    short ShortInteger;
    int Integer;
#ifndef NO_FP
    double FP;
#endif
    char *Identifier;
    struct ArrayValue Array;
    struct PointerValue Pointer;
    struct ParseState Parser;
    struct ValueType *Typ;
    struct FuncDef FuncDef;
};

struct Value
{
    struct ValueType *Typ;          /* the type of this value */
    union AnyValue *Val;            /* pointer to the AnyValue which holds the actual content */
    struct Value *LValueFrom;       /* if an LValue, this is a Value our LValue is contained within (or NULL) */
    char ValOnHeap;                 /* the AnyValue is on the heap (but this Value is on the stack) */
    char ValOnStack;                /* the AnyValue is on the stack along with this Value */
    char IsLValue;                  /* is modifiable and is allocated somewhere we can usefully modify it */
};

/* hash table data structure */
struct TableEntry
{
    struct TableEntry *Next;        /* next item in this hash chain */
    union TableEntryPayload
    {
        struct ValueEntry
        {
            char *Key;              /* points to the shared string table */
            struct Value *Val;      /* the value we're storing */
        } v;                        /* used for tables of values */
        
        char Key[1];                /* dummy size - used for the shared string table */
    } p;
};
    
struct Table
{
    short Size;
    short OnHeap;
    struct TableEntry **HashTable;
};

/* stack frame for function calls */
struct StackFrame
{
    struct ParseState ReturnParser;         /* how we got here */
    struct Value *ReturnValue;              /* copy the return value here */
    struct Value **Parameter;               /* array of parameter values */
    int NumParams;                          /* the number of parameters */
    struct Table LocalTable;                /* the local variables and parameters */
    struct TableEntry *LocalHashTable[LOCAL_TABLE_SIZE];
    struct StackFrame *PreviousStackFrame;  /* the next lower stack frame */
};

/* lexer state */
struct LexState
{
    const char *Pos;
    const char *End;
    int Line;
    const char *FileName;
};

/* library function definition */
struct LibraryFunction
{
    void (*Func)(struct ParseState *Parser, struct Value *, struct Value **, int);
    const char *Prototype;
};

/* platform-specific method for writing characters to the console */
typedef void CharWriter(unsigned char);

/* globals */
extern void *HeapStackTop;
extern struct Table GlobalTable;
extern struct StackFrame *TopStackFrame;
extern struct ValueType UberType;
extern struct ValueType IntType;
extern struct ValueType CharType;
#ifndef NO_FP
extern struct ValueType FPType;
#endif
extern struct ValueType VoidType;
extern struct ValueType FunctionType;
extern struct ValueType MacroType;
extern struct ValueType *CharPtrType;
extern struct ValueType *CharArrayType;
extern char *StrEmpty;
extern struct LibraryFunction CLibrary[];
extern struct LibraryFunction PlatformLibrary[];

/* table.c */
void TableInit();
char *TableStrRegister(const char *Str);
char *TableStrRegister2(const char *Str, int Len);
void TableInitTable(struct Table *Tbl, struct TableEntry **HashTable, int Size, int OnHeap);
int TableSet(struct Table *Tbl, char *Key, struct Value *Val);
int TableGet(struct Table *Tbl, const char *Key, struct Value **Val);
struct Value *TableDelete(struct Table *Tbl, const char *Key);
char *TableSetIdentifier(struct Table *Tbl, const char *Ident, int IdentLen);
void TableStrFree();

/* lex.c */
void LexInit();
void LexCleanup();
void *LexAnalyse(const char *FileName, const char *Source, int SourceLen, int *TokenLen);
void LexInitParser(struct ParseState *Parser, void *TokenSource, const char *FileName, int Line, int RunIt);
enum LexToken LexGetToken(struct ParseState *Parser, struct Value **Value, int IncPos);
void LexToEndOfLine(struct ParseState *Parser);
void *LexCopyTokens(struct ParseState *StartParser, struct ParseState *EndParser);
void LexInteractiveClear(struct ParseState *Parser);
void LexInteractiveCompleted(struct ParseState *Parser);
void LexInteractiveStatementPrompt();

/* parse.c */
int ParseStatement(struct ParseState *Parser);
struct Value *ParseFunctionDefinition(struct ParseState *Parser, struct ValueType *ReturnType, char *Identifier, int IsProtoType);
void Parse(const char *FileName, const char *Source, int SourceLen, int RunIt);
void ParseInteractive();
void ParseCleanup();
void ParserCopyPos(struct ParseState *To, struct ParseState *From);

/* expression.c */
int ExpressionParse(struct ParseState *Parser, struct Value **Result);
int ExpressionParseInt(struct ParseState *Parser);

/* type.c */
void TypeInit();
void TypeCleanup();
int TypeSize(struct ValueType *Typ, int ArraySize);
int TypeSizeValue(struct Value *Val);
int TypeStackSizeValue(struct Value *Val);
int TypeParseFront(struct ParseState *Parser, struct ValueType **Typ);
void TypeParseIdentPart(struct ParseState *Parser, struct ValueType *BasicTyp, struct ValueType **Typ, char **Identifier);
void TypeParse(struct ParseState *Parser, struct ValueType **Typ, char **Identifier);
struct ValueType *TypeGetMatching(struct ParseState *Parser, struct ValueType *ParentType, enum BaseType Base, int ArraySize, const char *Identifier);

/* heap.c */
void HeapInit();
void *HeapAllocStack(int Size);
int HeapPopStack(void *Addr, int Size);
void HeapPushStackFrame();
int HeapPopStackFrame();
void *HeapAlloc(int Size);
void HeapFree(void *Mem);

/* variable.c */
void VariableInit();
void VariableCleanup();
void VariableFree(struct Value *Val);
void VariableTableCleanup(struct Table *HashTable);
void *VariableAlloc(struct ParseState *Parser, int Size, int OnHeap);
void VariableStackPop(struct ParseState *Parser, struct Value *Var);
struct Value *VariableAllocValueAndData(struct ParseState *Parser, int DataSize, int IsLValue, struct Value *LValueFrom, int OnHeap);
struct Value *VariableAllocValueAndCopy(struct ParseState *Parser, struct Value *FromValue, int OnHeap);
struct Value *VariableAllocValueFromType(struct ParseState *Parser, struct ValueType *Typ, int IsLValue, struct Value *LValueFrom);
struct Value *VariableAllocValueFromExistingData(struct ParseState *Parser, struct ValueType *Typ, union AnyValue *FromValue, int IsLValue, struct Value *LValueFrom);
struct Value *VariableAllocValueShared(struct ParseState *Parser, struct Value *FromValue);
void VariableDefine(struct ParseState *Parser, char *Ident, struct Value *InitValue);
int VariableDefined(const char *Ident);
void VariableGet(struct ParseState *Parser, const char *Ident, struct Value **LVal);
void VariableDefinePlatformVar(struct ParseState *Parser, char *Ident, struct ValueType *Typ, union AnyValue *FromValue, int IsWritable);
void VariableStackFrameAdd(struct ParseState *Parser, int NumParams);
void VariableStackFramePop(struct ParseState *Parser);
struct Value *VariableStringLiteralGet(char *Ident);
void VariableStringLiteralDefine(char *Ident, struct Value *Val);

/* library.c */
void LibraryInit(struct Table *GlobalTable, const char *LibraryName, struct LibraryFunction (*FuncList)[]);
void PrintInt(int Num, CharWriter *PutCh);
void PrintStr(const char *Str, CharWriter *PutCh);
void PrintFP(double Num, CharWriter *PutCh);

/* platform_support.c */
void ProgramFail(struct ParseState *Parser, const char *Message, ...);
void LexFail(struct LexState *Lexer, const char *Message, ...);
void PlatformCleanup();
void PlatformScanFile(const char *FileName);
char *PlatformGetLine(char *Buf, int MaxLen);
int PlatformGetCharacter();
void PlatformPutc(unsigned char OutCh);
void PlatformPrintf(const char *Format, ...);
void PlatformVPrintf(const char *Format, va_list Args);
void PlatformExit();
void PlatformLibraryInit();

#endif /* PICOC_H */
