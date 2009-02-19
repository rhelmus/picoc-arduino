#include <stdio.h>

#include "picoc.h"

/* parameter passing area */
struct Value *Parameter[PARAMETER_MAX];
int ParameterUsed = 0;
struct Value *ReturnValue;

/* local prototypes */
int ParseArguments(struct ParseState *Parser, int RunIt);
int ParseStatementMaybeRun(struct ParseState *Parser, int Condition);


/* initialise the parser */
void ParseInit()
{
    VariableInit();
    IntrinsicInit(&GlobalTable);
    TypeInit();
}

/* do a function call */
void ParseFunctionCall(struct ParseState *Parser, struct Value **Result, int ResultOnHeap, const char *FuncName)
{
    struct Value *FuncValue;
    enum LexToken Token = LexGetToken(Parser, NULL, TRUE);    /* open bracket */
    
    if (Parser->Mode == RunModeRun) 
    { /* get the function definition */
        VariableGet(Parser, FuncName, &FuncValue);
        if (FuncValue->Typ->Base != TypeFunction)
            ProgramFail(Parser, "not a function - can't call");
    
        *Result = VariableAllocValueFromType(Parser, FuncValue->Val->FuncDef.ReturnType, FALSE, ResultOnHeap);
        if (FuncValue->Val->FuncDef.Intrinsic == NULL)
            VariableStackFrameAdd(Parser);
        else
            HeapPushStackFrame();
    }
        
    /* parse arguments */
    ParameterUsed = 0;
    do {
        if (ParseExpression(Parser, &Parameter[ParameterUsed], FALSE))
        {
            if (Parser->Mode == RunModeRun && ParameterUsed >= FuncValue->Val->FuncDef.NumParams)
                ProgramFail(Parser, "too many arguments");
                
            if (Parser->Mode == RunModeRun && FuncValue->Val->FuncDef.ParamType[ParameterUsed] != Parameter[ParameterUsed]->Typ)
                ProgramFail(Parser, "parameter %d to %s is the wrong type", ParameterUsed, FuncName);
                
            ParameterUsed++;
            Token = LexGetToken(Parser, NULL, TRUE);
            if (Token != TokenComma && Token != TokenCloseBracket)
                ProgramFail(Parser, "comma expected");
        }
        else
        {
            Token = LexGetToken(Parser, NULL, TRUE);
            if (!TokenCloseBracket)
                ProgramFail(Parser, "bad argument");
        }
    } while (Token != TokenCloseBracket);
    
    if (Parser->Mode == RunModeRun) 
    { /* run the function */
        int Count;
        
        if (FuncValue->Val->FuncDef.Intrinsic == NULL)
        { /* run a user-defined function */
            struct ParseState FuncParser = FuncValue->Val->FuncDef.Body;
            
            for (Count = 0; Count < ParameterUsed; Count++)
                VariableDefine(Parser, FuncValue->Val->FuncDef.ParamName[Count], Parameter[Count]);
            
            if (!ParseStatement(&FuncParser))
                ProgramFail(&FuncParser, "function body expected");
        
            if (FuncValue->Val->FuncDef.ReturnType != (*Result)->Typ)
                ProgramFail(&FuncParser, "bad type of return value");

            VariableStackFramePop(Parser);
        }
        else
        {
            FuncValue->Val->FuncDef.Intrinsic();
            HeapPopStackFrame();
        }
    }
}

/* parse a single value */
int ParseValue(struct ParseState *Parser, struct Value **Result, int ResultOnHeap)
{
    struct ParseState PreState = *Parser;
    struct Value *LexValue;
    int IntValue;
    enum LexToken Token = LexGetToken(Parser, &LexValue, TRUE);
    struct Value *LocalLValue = NULL;
    struct ValueType *VType;
    int Success = TRUE;
    
    switch (Token)
    {
        case TokenIntegerConstant: case TokenCharacterConstant: case TokenFPConstant: case TokenStringConstant:
            *Result = VariableAllocValueAndCopy(Parser, LexValue, ResultOnHeap);
            break;
        
        case TokenMinus:  case TokenUnaryExor: case TokenUnaryNot:
            IntValue = ParseIntExpression(Parser);
            if (Parser->Mode == RunModeRun)
            {
                *Result = VariableAllocValueFromType(Parser, &IntType, FALSE, ResultOnHeap);
                switch(Token)
                {
                    case TokenMinus: (*Result)->Val->Integer = -IntValue; break;
                    case TokenUnaryExor: (*Result)->Val->Integer = ~IntValue; break;
                    case TokenUnaryNot: (*Result)->Val->Integer = !IntValue; break;
                    default: break;
                }
            }
            break;

        case TokenOpenBracket:
            if (!ParseExpression(Parser, Result, ResultOnHeap))
                ProgramFail(Parser, "invalid expression");
            
            if (LexGetToken(Parser, NULL, TRUE) != TokenCloseBracket)
                ProgramFail(Parser, "')' expected");
            break;
                
        case TokenAsterisk:
            if (!ParseExpression(Parser, Result, ResultOnHeap))
                ProgramFail(Parser, "invalid expression");
            
            if ((*Result)->Typ->Base != TypePointer)
                ProgramFail(Parser, "can't dereference this non-pointer");
            
            LocalLValue = (*Result)->Val->Pointer.Segment;
            VariableStackPop(Parser, *Result);
            *Result = VariableAllocValueShared(Parser, LocalLValue, ResultOnHeap);
            break;

        case TokenAmpersand:
            if (!ParseValue(Parser, Result, ResultOnHeap) || !(*Result)->IsLValue)
                ProgramFail(Parser, "can't get the address of this");
            
            VType = (*Result)->Typ;
            VariableStackPop(Parser, *Result);
            *Result = VariableAllocValueFromType(Parser, TypeGetMatching(Parser, VType, TypePointer, 0, StrEmpty), FALSE, ResultOnHeap);
            (*Result)->Val->Pointer.Segment = LocalLValue;
            (*Result)->Val->Pointer.Data.Offset = 0;
            break;
            
        case TokenIdentifier:
            if (LexGetToken(Parser, NULL, FALSE) == TokenOpenBracket)
                ParseFunctionCall(Parser, Result, ResultOnHeap, LexValue->Val->String);
            else
            {
                if (Parser->Mode == RunModeRun)
                {
                    VariableGet(Parser, LexValue->Val->String, &LocalLValue);
                    if (LocalLValue->Typ->Base == TypeMacro)
                    {
                        struct ParseState MacroLexer = LocalLValue->Val->Parser;
                        
                        if (!ParseExpression(&MacroLexer, Result, ResultOnHeap))
                            ProgramFail(&MacroLexer, "expression expected");
                    }
                    else if (LocalLValue->Typ == TypeVoid)
                        ProgramFail(Parser, "a void value isn't much use here");
                    else
                    { /* it's a value variable */
                        *Result = VariableAllocValueShared(Parser, LocalLValue, ResultOnHeap);
                    }
                }
                
                /* see if there's a postfix operator */
                Token = LexGetToken(Parser, &LexValue, FALSE);
                if (Token == TokenIncrement || Token == TokenDecrement)
                { /* it's a postincrement or postdecrement */
                    LexGetToken(Parser, &LexValue, TRUE);
                    if (Parser->Mode == RunModeRun)
                    {
                        if (!(*Result)->IsLValue || (*Result)->Typ->Base != TypeInt) 
                            ProgramFail(Parser, "can't %s this", (Token == TokenIncrement) ? "increment" : "decrement");
                            
                        if (Token == TokenIncrement)
                            (*Result)->Val->Integer++;
                        else
                            (*Result)->Val->Integer--;
                    }
                }
                else if (Token == TokenLeftSquareBracket)
                { /* array access */
                    LexGetToken(Parser, &LexValue, TRUE);
                    IntValue = ParseIntExpression(Parser);
                    if (LexGetToken(Parser, &LexValue, TRUE) != TokenRightSquareBracket)
                        ProgramFail(Parser, "expected ']'");
                    
                    if (Parser->Mode == RunModeRun)
                    { /* look up the array element */
                        if ((*Result)->Typ->Base != TypeArray)
                            ProgramFail(Parser, "not an array");
                        
                        if (IntValue < 0 || IntValue >= (*Result)->Typ->ArraySize)
                            ProgramFail(Parser, "illegal array index");
                        
                        VariableStackPop(Parser, *Result);
                        *Result = VariableAllocValueFromExistingData(Parser, (*Result)->Typ->FromType, (union AnyValue *)((void *)(*Result)->Val + (*Result)->Typ->FromType->Sizeof * IntValue), TRUE, ResultOnHeap);
                    }
                }
            }
            break;
            
        default:
            *Parser = PreState;
            Success = FALSE;
            break;
    }
    
    return Success;
}

struct Value *ParsePushFP(struct ParseState *Parser, int ResultOnHeap, double NewFP)
{
    struct Value *Val = VariableAllocValueFromType(Parser, &FPType, FALSE, ResultOnHeap);
    Val->Val->FP = NewFP;
    return Val;
}

struct Value *ParsePushInt(struct ParseState *Parser, int ResultOnHeap, int NewInt)
{
    struct Value *Val = VariableAllocValueFromType(Parser, &IntType, FALSE, ResultOnHeap);
    Val->Val->Integer = NewInt;
    return Val;
}

/* parse an expression. operator precedence is not supported */
int ParseExpression(struct ParseState *Parser, struct Value **Result, int ResultOnHeap)
{
    struct Value *CurrentValue;
    struct Value *TotalValue;
    
    if (!ParseValue(Parser, &TotalValue, ResultOnHeap))
        return FALSE;
    
    while (TRUE)
    {
        enum LexToken Token = LexGetToken(Parser, NULL, FALSE);
        switch (Token)
        {
            case TokenPlus: case TokenMinus: case TokenAsterisk: case TokenSlash:
            case TokenEquality: case TokenLessThan: case TokenGreaterThan:
            case TokenLessEqual: case TokenGreaterEqual: case TokenLogicalAnd:
            case TokenLogicalOr: case TokenAmpersand: case TokenArithmeticOr: 
            case TokenArithmeticExor: 
                LexGetToken(Parser, NULL, TRUE);
                break;
            
            case TokenDot:
            {
                struct Value *Ident;
                
                LexGetToken(Parser, NULL, TRUE);
                if (LexGetToken(Parser, &Ident, TRUE) != TokenIdentifier)
                    ProgramFail(Parser, "need an structure or union member after '.'");

                if (Parser->Mode == RunModeRun)
                {                
                    void *TotalValueData = (void *)TotalValue->Val;

                    if (TotalValue->Typ->Base != TypeStruct && TotalValue->Typ->Base != TypeUnion)
                        ProgramFail(Parser, "can't use '.' on something that's not a struct or union");
                        
                    if (!TableGet(TotalValue->Typ->Members, Ident->Val->String, &CurrentValue))
                        ProgramFail(Parser, "structure doesn't have a member called '%s'", Ident->Val->String);
                    
                    VariableStackPop(Parser, TotalValue);
                    TotalValue = VariableAllocValueFromExistingData(Parser, CurrentValue->Typ, TotalValueData + CurrentValue->Val->Integer, TRUE, ResultOnHeap);
                }
                continue;
            }
            case TokenAssign: case TokenAddAssign: case TokenSubtractAssign:
                LexGetToken(Parser, NULL, TRUE);
                if (!ParseExpression(Parser, &CurrentValue, ResultOnHeap))
                    ProgramFail(Parser, "expression expected");
                
                if (Parser->Mode == RunModeRun)
                {
                    if (CurrentValue->Typ->Base != TypeInt || !TotalValue->IsLValue || TotalValue->Typ->Base != TypeInt)
                        ProgramFail(Parser, "can't assign");

                    switch (Token)
                    {
                        case TokenAddAssign: TotalValue->Val->Integer += CurrentValue->Val->Integer; break;
                        case TokenSubtractAssign: TotalValue->Val->Integer -= CurrentValue->Val->Integer; break;
                        default: TotalValue->Val->Integer = CurrentValue->Val->Integer; break;
                    }
                    VariableStackPop(Parser, CurrentValue);
                }
                // fallthrough
            
            default:
                if (Parser->Mode == RunModeRun)
                    *Result = TotalValue;
                return TRUE;
        }
        
        if (!ParseValue(Parser, &CurrentValue, ResultOnHeap))
            return FALSE;

        if (Parser->Mode == RunModeRun)
        {
            if (CurrentValue->Typ->Base == TypeFP || TotalValue->Typ->Base == TypeFP)
            { /* floating point expression */
                double FPTotal, FPCurrent, FPResult;

                if (CurrentValue->Typ->Base != TypeFP || TotalValue->Typ->Base != TypeFP)
                { /* convert both to floating point */
                    if (CurrentValue->Typ->Base == TypeInt)
                        FPCurrent = (double)CurrentValue->Val->Integer;
                    else if (CurrentValue->Typ->Base == TypeFP)
                        FPCurrent = CurrentValue->Val->FP;
                    else
                        ProgramFail(Parser, "bad type for operator");
                        
                    if (TotalValue->Typ->Base == TypeInt)
                        FPTotal = (double)TotalValue->Val->Integer;
                    else if (TotalValue->Typ->Base == TypeFP)
                        FPTotal = TotalValue->Val->FP;
                    else
                        ProgramFail(Parser, "bad type for operator");
                }

                VariableStackPop(Parser, CurrentValue);
                VariableStackPop(Parser, TotalValue);
                
                switch (Token)
                {
                    case TokenPlus:         FPResult = FPTotal + FPCurrent; break;
                    case TokenMinus:        FPResult = FPTotal - FPCurrent; break;
                    case TokenAsterisk:     FPResult = FPTotal * FPCurrent; break;
                    case TokenSlash:        FPResult = FPTotal / FPCurrent; break;
                    case TokenEquality:     FPResult = FPTotal == FPCurrent; break;
                    case TokenLessThan:     FPResult = FPTotal < FPCurrent; break;
                    case TokenGreaterThan:  FPResult = FPTotal > FPCurrent; break;
                    case TokenLessEqual:    FPResult = FPTotal <= FPCurrent; break;
                    case TokenGreaterEqual: FPResult = FPTotal >= FPCurrent; break;
                    case TokenLogicalAnd: case TokenLogicalOr: case TokenAmpersand: case TokenArithmeticOr: case TokenArithmeticExor: ProgramFail(Parser, "bad type for operator"); break;
                    default: break;
                }
                TotalValue = ParsePushFP(Parser, ResultOnHeap, FPResult);
            }
            else
            { /* integer expression */
                int IntX, IntY, IntResult;
                
                if (CurrentValue->Typ->Base != TypeInt || TotalValue->Typ->Base != TypeInt)
                    ProgramFail(Parser, "bad operand types");
            
                IntX = TotalValue->Val->Integer;
                IntY = CurrentValue->Val->Integer;
                VariableStackPop(Parser, CurrentValue);
                VariableStackPop(Parser, TotalValue);
                
                /* integer arithmetic */
                switch (Token)
                {
                    case TokenPlus:             IntResult = IntX + IntY; break;
                    case TokenMinus:            IntResult = IntX - IntY; break;
                    case TokenAsterisk:         IntResult = IntX * IntY; break;
                    case TokenSlash:            IntResult = IntX / IntY; break;
                    case TokenEquality:         IntResult = IntX == IntY; break;
                    case TokenLessThan:         IntResult = IntX < IntY; break;
                    case TokenGreaterThan:      IntResult = IntX > IntY; break;
                    case TokenLessEqual:        IntResult = IntX <= IntY; break;
                    case TokenGreaterEqual:     IntResult = IntX >= IntY; break;
                    case TokenLogicalAnd:       IntResult = IntX && IntY; break;
                    case TokenLogicalOr:        IntResult = IntX || IntY; break;
                    case TokenAmpersand:        IntResult = IntX & IntY; break;
                    case TokenArithmeticOr:     IntResult = IntX | IntY; break;
                    case TokenArithmeticExor:   IntResult = IntX ^ IntY; break;
                    default: break;
                }
                TotalValue = ParsePushInt(Parser, ResultOnHeap, IntResult);
            }
            
            *Result = TotalValue;
        }
    }
    
    return TRUE;
}

/* parse an expression. operator precedence is not supported */
int ParseIntExpression(struct ParseState *Parser)
{
    struct Value *Val;
    int Result = 0;
    
    if (!ParseExpression(Parser, &Val, FALSE))
        ProgramFail(Parser, "expression expected");
    
    if (Parser->Mode == RunModeRun)
    { 
        if (Val->Typ->Base != TypeInt)
            ProgramFail(Parser, "integer value expected");
    
        Result = Val->Val->Integer;
        VariableStackPop(Parser, Val);
    }
    
    return Result;
}

/* parse a function definition and store it for later */
struct Value *ParseFunctionDefinition(struct ParseState *Parser, struct ValueType *ReturnType, const char *Identifier, int IsPrototype)
{
    struct ValueType *ParamType;
    const char *ParamIdentifier;
    enum LexToken Token;
    struct Value *FuncValue;
    struct ParseState ParamParser;
    int ParamCount = 0;

    LexGetToken(Parser, NULL, TRUE);  /* open bracket */
    ParamParser = *Parser;
    Token = LexGetToken(Parser, NULL, TRUE);
    if (Token != TokenCloseBracket && Token != TokenEOF)
    { /* count the number of parameters */
        ParamCount++;
        while ((Token = LexGetToken(Parser, NULL, TRUE)) != TokenCloseBracket && Token != TokenEOF)
        { 
            if (Token == TokenComma)
                ParamCount++;
        } 
    }
    if (ParamCount > PARAMETER_MAX)
        ProgramFail(Parser, "too many parameters");
    
    FuncValue = VariableAllocValueAndData(Parser, sizeof(struct FuncDef) + sizeof(struct ValueType *) * ParamCount + sizeof(const char *) * ParamCount, FALSE, TRUE);
    FuncValue->Typ = &FunctionType;
    FuncValue->Val->FuncDef.ReturnType = ReturnType;
    FuncValue->Val->FuncDef.NumParams = ParamCount;
    FuncValue->Val->FuncDef.ParamType = (void *)FuncValue->Val + sizeof(struct FuncDef);
    FuncValue->Val->FuncDef.ParamName = (void *)FuncValue->Val->FuncDef.ParamType + sizeof(struct ValueType *) * ParamCount;
    FuncValue->Val->FuncDef.Body = *Parser;
   
    for (ParamCount = 0; ParamCount < FuncValue->Val->FuncDef.NumParams; ParamCount++)
    { /* harvest the parameters into the function definition */
        TypeParse(&ParamParser, &ParamType, &ParamIdentifier);
        FuncValue->Val->FuncDef.ParamType[ParamCount] = ParamType;
        FuncValue->Val->FuncDef.ParamName[ParamCount] = ParamIdentifier;
        
        Token = LexGetToken(&ParamParser, NULL, TRUE);
        if (Token != TokenComma && ParamCount != FuncValue->Val->FuncDef.NumParams-1)
            ProgramFail(&ParamParser, "comma expected");
    }
    
    if (!IsPrototype)
    {
        if (LexGetToken(Parser, NULL, FALSE) != TokenLeftBrace)
            ProgramFail(Parser, "bad function definition");
        
        if (!ParseStatementMaybeRun(Parser, FALSE))
            ProgramFail(Parser, "function definition expected");
    }
        
    if (!TableSet(&GlobalTable, Identifier, FuncValue))
        ProgramFail(Parser, "'%s' is already defined", Identifier);
    
    return FuncValue;
}

/* parse a #define macro definition and store it for later */
void ParseMacroDefinition(struct ParseState *Parser)
{
    struct Value *MacroName;
    struct Value *MacroValue = VariableAllocValueAndData(Parser, sizeof(struct ParseState), FALSE, TRUE);

    if (LexGetToken(Parser, &MacroName, TRUE) != TokenIdentifier)
        ProgramFail(Parser, "identifier expected");
    
    MacroValue->Val->Parser = *Parser;
    MacroValue->Typ = &MacroType;
    
    if (!TableSet(&GlobalTable, MacroName->Val->String, MacroValue))
        ProgramFail(Parser, "'%s' is already defined", &MacroName->Val->String);
}

/* copy where we're at in the parsing */
void ParserCopyPos(struct ParseState *To, struct ParseState *From)
{
    To->Pos = From->Pos;
    To->Line = From->Line;
}

/* parse a "for" statement */
void ParseFor(struct ParseState *Parser)
{
    int Condition;
    struct ParseState PreConditional;
    struct ParseState PreIncrement;
    struct ParseState PreStatement;
    struct ParseState After;

    if (LexGetToken(Parser, NULL, TRUE) != TokenOpenBracket)
        ProgramFail(Parser, "'(' expected");
                        
    if (!ParseStatement(Parser))
        ProgramFail(Parser, "statement expected");
    
    if (LexGetToken(Parser, NULL, TRUE) != TokenSemicolon)
        ProgramFail(Parser, "';' expected");
    
    ParserCopyPos(&PreConditional, Parser);
    Condition = ParseIntExpression(Parser);
    
    if (LexGetToken(Parser, NULL, TRUE) != TokenSemicolon)
        ProgramFail(Parser, "';' expected");
    
    ParserCopyPos(&PreIncrement, Parser);
    ParseStatementMaybeRun(Parser, FALSE);
    
    if (LexGetToken(Parser, NULL, TRUE) != TokenCloseBracket)
        ProgramFail(Parser, "')' expected");
    
    ParserCopyPos(&PreStatement, Parser);
    if (!ParseStatementMaybeRun(Parser, Condition))
        ProgramFail(Parser, "statement expected");
    
    if (Parser->Mode == RunModeContinue)
        Parser->Mode = RunModeRun;
        
    ParserCopyPos(&After, Parser);
        
    while (Condition && Parser->Mode == RunModeRun)
    {
        ParserCopyPos(Parser, &PreIncrement);
        ParseStatement(Parser);
                        
        ParserCopyPos(Parser, &PreConditional);
        Condition = ParseIntExpression(Parser);
        
        if (Condition)
        {
            ParserCopyPos(Parser, &PreStatement);
            ParseStatement(Parser);
            
            if (Parser->Mode == RunModeContinue)
                Parser->Mode = RunModeRun;                
        }
    }
    
    if (Parser->Mode == RunModeBreak)
        Parser->Mode = RunModeRun;
        
    ParserCopyPos(Parser, &After);
}

/* parse a statement, but only run it if Condition is TRUE */
int ParseStatementMaybeRun(struct ParseState *Parser, int Condition)
{
    if (Parser->Mode != RunModeSkip && !Condition)
    {
        enum RunMode OldMode = Parser->Mode;
        Parser->Mode = RunModeSkip;
        int Result = ParseStatement(Parser);
        Parser->Mode = OldMode;
        return Result;
    }
    else
        return ParseStatement(Parser);
}

/* parse a block of code and return what mode it returned in */
enum RunMode ParseBlock(struct ParseState *Parser, int AbsorbOpenBrace, int Condition)
{
    if (AbsorbOpenBrace && LexGetToken(Parser, NULL, TRUE) != TokenLeftBrace)
        ProgramFail(Parser, "'{' expected");
    
    if (Parser->Mode != RunModeSkip && !Condition)
    { /* condition failed - skip this block instead */
        enum RunMode OldMode = Parser->Mode;
        Parser->Mode = RunModeSkip;
        while (ParseStatement(Parser))
        {}
        Parser->Mode = OldMode;
    }
    else
    { /* just run it in its current mode */
        while (ParseStatement(Parser))
        {}
    }
    
    if (LexGetToken(Parser, NULL, TRUE) != TokenRightBrace)
        ProgramFail(Parser, "'}' expected");
        
    return Parser->Mode;
}

/* parse a statement */
int ParseStatement(struct ParseState *Parser)
{
    struct Value *CValue;
    int Condition;
    struct ParseState PreState = *Parser;
    const char *Identifier;
    struct ValueType *Typ;
    enum LexToken Token = LexGetToken(Parser, NULL, TRUE);
    
    switch (Token)
    {
        case TokenEOF:
            return FALSE;
            
        case TokenIdentifier: 
            *Parser = PreState;
            ParseExpression(Parser, &CValue, FALSE);
            if (Parser->Mode == RunModeRun) 
                VariableStackPop(Parser, CValue);
            break;
            
        case TokenLeftBrace:
            ParseBlock(Parser, FALSE, TRUE);
            break;
            
        case TokenIf:
            if (LexGetToken(Parser, NULL, TRUE) != TokenOpenBracket)
                ProgramFail(Parser, "'(' expected");
                
            Condition = ParseIntExpression(Parser);
            
            if (LexGetToken(Parser, NULL, TRUE) != TokenCloseBracket)
                ProgramFail(Parser, "')' expected");

            if (!ParseStatementMaybeRun(Parser, Condition))
                ProgramFail(Parser, "statement expected");
            
            if (LexGetToken(Parser, NULL, FALSE) == TokenElse)
            {
                LexGetToken(Parser, NULL, TRUE);
                if (!ParseStatementMaybeRun(Parser, !Condition))
                    ProgramFail(Parser, "statement expected");
            }
            break;
        
        case TokenWhile:
            {
                struct ParseState PreConditional;
                ParserCopyPos(&PreConditional, Parser);
                do
                {
                    ParserCopyPos(Parser, &PreConditional);
                    Condition = ParseIntExpression(Parser);
                    ParseBlock(Parser, TRUE, Condition);
                    if (Parser->Mode == RunModeContinue)
                        Parser->Mode = RunModeRun;
                    
                } while (Parser->Mode == RunModeRun && Condition);
                
                if (Parser->Mode == RunModeBreak)
                    Parser->Mode = RunModeRun;
            }
            break;
                
        case TokenDo:
            {
                struct ParseState PreStatement;
                ParserCopyPos(&PreStatement, Parser);
                do
                {
                    ParserCopyPos(Parser, &PreStatement);
                    ParseBlock(Parser, TRUE, TRUE);
                    if (Parser->Mode == RunModeContinue)
                        Parser->Mode = RunModeRun;

                    if (LexGetToken(Parser, NULL, TRUE) != TokenWhile)
                        ProgramFail(Parser, "'while' expected");
                    
                    Condition = ParseIntExpression(Parser);
                
                } while (Condition && Parser->Mode == RunModeRun);           
                
                if (Parser->Mode == RunModeBreak)
                    Parser->Mode = RunModeRun;
            }
            break;
                
        case TokenFor:
            ParseFor(Parser);
            break;

        case TokenSemicolon: break;

        case TokenIntType:
        case TokenCharType:
        case TokenFloatType:
        case TokenDoubleType:
        case TokenVoidType:
        case TokenStructType:
        case TokenUnionType:
            *Parser = PreState;
            TypeParse(Parser, &Typ, &Identifier);
            if (Token == TokenVoidType && Identifier != StrEmpty)
                ProgramFail(Parser, "can't define a void variable");
                
            if ((Token != TokenVoidType && Token != TokenStructType && Token != TokenUnionType) && Identifier == StrEmpty)
                ProgramFail(Parser, "identifier expected");
                
            if (Identifier != StrEmpty)
            {
                /* handle function definitions */
                if (LexGetToken(Parser, NULL, FALSE) == TokenOpenBracket)
                    ParseFunctionDefinition(Parser, Typ, Identifier, FALSE);
                else
                    VariableDefine(Parser, Identifier, VariableAllocValueFromType(Parser, Typ, TRUE, FALSE));
            }
            break;
        
        case TokenHashDefine:
            ParseMacroDefinition(Parser);
            break;
            
        case TokenHashInclude:
        {
            struct Value *LexerValue;
            if (LexGetToken(Parser, &LexerValue, TRUE) != TokenStringConstant)
                ProgramFail(Parser, "\"filename.h\" expected");
            
            ScanFile(LexerValue->Val->String);
            break;
        }

        case TokenSwitch:
            if (LexGetToken(Parser, NULL, TRUE) != TokenOpenBracket)
                ProgramFail(Parser, "'(' expected");
                
            Condition = ParseIntExpression(Parser);
            
            if (LexGetToken(Parser, NULL, TRUE) != TokenCloseBracket)
                ProgramFail(Parser, "')' expected");
            
            if (LexGetToken(Parser, NULL, FALSE) != TokenLeftBrace)
                ProgramFail(Parser, "'{' expected");
            
            { /* new block so we can store parser state */
                enum RunMode OldMode = Parser->Mode;
                int OldSearchLabel = Parser->SearchLabel;
                Parser->Mode = RunModeCaseSearch;
                Parser->SearchLabel = Condition;
                
                ParseBlock(Parser, TRUE, TRUE);
                
                Parser->Mode = OldMode;
                Parser->SearchLabel = OldSearchLabel;
            }
            break;

        case TokenCase:
            if (Parser->Mode == RunModeCaseSearch)
            {
                Parser->Mode = RunModeRun;
                Condition = ParseIntExpression(Parser);
                Parser->Mode = RunModeCaseSearch;
            }
            else
                Condition = ParseIntExpression(Parser);
                
            if (LexGetToken(Parser, NULL, TRUE) != TokenColon)
                ProgramFail(Parser, "':' expected");
            
            if (Parser->Mode == RunModeCaseSearch && Condition == Parser->SearchLabel)
                Parser->Mode = RunModeRun;
            break;
            
        case TokenDefault:
            if (LexGetToken(Parser, NULL, TRUE) != TokenColon)
                ProgramFail(Parser, "':' expected");
            
            if (Parser->Mode == RunModeCaseSearch)
                Parser->Mode = RunModeRun;
            break;

        case TokenBreak:
            if (Parser->Mode == RunModeRun)
                Parser->Mode = RunModeBreak;
            break;
            
        case TokenContinue:
            if (Parser->Mode == RunModeRun)
                Parser->Mode = RunModeContinue;
            break;
            
        case TokenReturn:
            if (Parser->Mode == RunModeRun)
            {
                // XXX - check return type
                // XXX - set return value
                Parser->Mode = RunModeContinue;
            }
            break;
            
        default:
            *Parser = PreState;
            return FALSE;
    }
    
    return TRUE;
}

/* quick scan a source file for definitions */
void Parse(const char *FileName, const char *Source, int SourceLen, int RunIt)
{
    struct ParseState Parser;
    
    void *Tokens = LexAnalyse(FileName, Source, SourceLen); // XXX - some better way of storing tokenised input?
    LexInitParser(&Parser, Tokens, FileName, 1, RunIt);

    while (ParseStatement(&Parser))
    {}
    
    if (LexGetToken(&Parser, NULL, FALSE) != TokenEOF)
        ProgramFail(&Parser, "parse error");
}
