/* picoc data type module. This manages a tree of data types and has facilities
 * for parsing data types. */
 
#include "interpreter.h"

/* some basic types */
static int PointerAlignBytes;
static int IntAlignBytes;


/* add a new type to the set of types we know about */
TValueTypePtr TypeAdd(Picoc *pc, TParseStatePtr Parser, TValueTypePtr ParentType, enum BaseType Base, int ArraySize, TConstRegStringPtr Identifier, int Sizeof, int AlignBytes)
{
    TValueTypePtr NewType = allocMemVariable<struct ValueType>(Parser, FALSE);
    NewType->Base = Base;
    NewType->ArraySize = ArraySize;
    NewType->Sizeof = Sizeof;
    NewType->AlignBytes = AlignBytes;
    NewType->Identifier = Identifier;
    NewType->Members = NILL;
    NewType->FromType = ParentType;
    NewType->DerivedTypeList = NILL;
    NewType->Flags |= FlagOnHeap;
    NewType->Next = ParentType->DerivedTypeList;
    ParentType->DerivedTypeList = NewType;
    
    return NewType;
}

/* given a parent type, get a matching derived type and make one if necessary.
 * Identifier should be registered with the shared string table. */
TValueTypePtr TypeGetMatching(Picoc *pc, TParseStatePtr Parser, TValueTypePtr ParentType, enum BaseType Base, int ArraySize, TConstRegStringPtr Identifier, int AllowDuplicates)
{
    int Sizeof;
    int AlignBytes;
    TValueTypePtr ThisType = ParentType->DerivedTypeList;
    while (ThisType != NULL && (ThisType->Base != Base || ThisType->ArraySize != ArraySize || ThisType->Identifier != Identifier))
        ThisType = ThisType->Next;
    
    if (ThisType != NULL)
    {
        if (AllowDuplicates)
            return ThisType;
        else
            ProgramFail(Parser, "data type '%s' is already defined", Identifier);
    }
        
    switch (Base)
    {
        case TypePointer:   Sizeof = sizeof(void *); AlignBytes = PointerAlignBytes; break;
#ifdef WRAP_ANYVALUE
        case TypeArray:     Sizeof = sizeof(TAnyValueCharPointer) + ArraySize * ParentType->Sizeof; AlignBytes = ParentType->AlignBytes; break;
#else
        case TypeArray:     Sizeof = ArraySize * ParentType->Sizeof; AlignBytes = ParentType->AlignBytes; break;
#endif
        case TypeEnum:      Sizeof = sizeof(int); AlignBytes = IntAlignBytes; break;
        default:            Sizeof = 0; AlignBytes = 0; break;      /* structs and unions will get bigger when we add members to them */
    }

    return TypeAdd(pc, Parser, ParentType, Base, ArraySize, Identifier, Sizeof, AlignBytes);
}

/* stack space used by a value */
int TypeStackSizeValue(TValuePtr Val)
{
    if (Val != NULL && Val->Flags & FlagOnStack)
        return TypeSizeValue(Val, FALSE);
    else
        return 0;
}

/* memory used by a value */
int TypeSizeValue(TValuePtr Val, int Compact)
{
    if (IS_INTEGER_NUMERIC(Val) && !Compact)
        return sizeof(ALIGN_TYPE);     /* allow some extra room for type extension */
    else if (Val->Typ->Base != TypeArray)
        return Val->Typ->Sizeof;
    else
#ifdef WRAP_ANYVALUE
        return Val->Typ->FromType->Sizeof * Val->Typ->ArraySize + sizeof(TAnyValueCharPointer);
#else
        return Val->Typ->FromType->Sizeof * Val->Typ->ArraySize;
#endif
}

/* memory used by a variable given its type and array size */
int TypeSize(TValueTypePtr Typ, int ArraySize, int Compact)
{
    if (IS_INTEGER_NUMERIC_TYPE(Typ) && !Compact)
        return sizeof(ALIGN_TYPE);     /* allow some extra room for type extension */
    else if (Typ->Base != TypeArray)
        return Typ->Sizeof;
    else
#ifdef WRAP_ANYVALUE
        return Typ->FromType->Sizeof * ArraySize + sizeof(TAnyValueCharPointer);
#else
        return Typ->FromType->Sizeof * ArraySize;
#endif
}

/* memory used by a variable given its type and array size (without extra mem used for arrays) */
int SizeOf(TValueTypePtr Typ, int ArraySize, int Compact)
{
#ifdef WRAP_ANYVALUE
    if (Typ->Base == TypeArray)
        return TypeSize(Typ, ArraySize, Compact) - sizeof(TAnyValueCharPointer);
#endif
    return TypeSize(Typ, ArraySize, Compact);
}

/* add a base type */
void TypeAddBaseType(Picoc *pc, TValueTypePtr TypeNode, enum BaseType Base, int Sizeof, int AlignBytes)
{
    TypeNode->Base = Base;
    TypeNode->ArraySize = 0;
    TypeNode->Sizeof = Sizeof;
    TypeNode->AlignBytes = AlignBytes;
    TypeNode->Identifier = pc->StrEmpty;
    TypeNode->Members = NILL;
    TypeNode->FromType = NILL;
    TypeNode->DerivedTypeList = NILL;
    TypeNode->Flags &= ~FlagOnHeap;
    TypeNode->Next = pc->UberType.DerivedTypeList;
    pc->UberType.DerivedTypeList = TypeNode;
}

/* initialise the type system */
void TypeInit(Picoc *pc)
{
    struct IntAlign { char x; int y; } ia;
    struct ShortAlign { char x; short y; } sa;
    struct CharAlign { char x; char y; } ca;
    struct LongAlign { char x; long y; } la;
#ifndef NO_FP
    struct DoubleAlign { char x; double y; } da;
#endif
    struct PointerAlign { char x; void *y; } pa;
    
    IntAlignBytes = (char *)&ia.y - &ia.x;
    PointerAlignBytes = (char *)&pa.y - &pa.x;
    
    pc->UberType.DerivedTypeList = NILL;
    TypeAddBaseType(pc, ptrWrap(&pc->IntType), TypeInt, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, ptrWrap(&pc->ShortType), TypeShort, sizeof(short), (char *)&sa.y - &sa.x);
    TypeAddBaseType(pc, ptrWrap(&pc->CharType), TypeChar, sizeof(char), (char *)&ca.y - &ca.x);
    TypeAddBaseType(pc, ptrWrap(&pc->LongType), TypeLong, sizeof(long), (char *)&la.y - &la.x);
    TypeAddBaseType(pc, ptrWrap(&pc->UnsignedIntType), TypeUnsignedInt, sizeof(unsigned int), IntAlignBytes);
    TypeAddBaseType(pc, ptrWrap(&pc->UnsignedShortType), TypeUnsignedShort, sizeof(unsigned short), (char *)&sa.y - &sa.x);
    TypeAddBaseType(pc, ptrWrap(&pc->UnsignedLongType), TypeUnsignedLong, sizeof(unsigned long), (char *)&la.y - &la.x);
    TypeAddBaseType(pc, ptrWrap(&pc->UnsignedCharType), TypeUnsignedChar, sizeof(unsigned char), (char *)&ca.y - &ca.x);
    TypeAddBaseType(pc, ptrWrap(&pc->VoidType), TypeVoid, 0, 1);
    TypeAddBaseType(pc, ptrWrap(&pc->FunctionType), TypeFunction, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, ptrWrap(&pc->MacroType), TypeMacro, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, ptrWrap(&pc->GotoLabelType), TypeGotoLabel, 0, 1);
#ifndef NO_FP
    TypeAddBaseType(pc, ptrWrap(&pc->FPType), TypeFP, sizeof(double), (char *)&da.y - &da.x);
    TypeAddBaseType(pc, ptrWrap(&pc->TypeType), Type_Type, sizeof(double), (char *)&da.y - &da.x);  /* must be large enough to cast to a double */
#else
    TypeAddBaseType(pc, ptrWrap(&pc->TypeType), Type_Type, sizeof(TValueTypePtr ), PointerAlignBytes);
#endif
    pc->CharArrayType = TypeAdd(pc, NILL, ptrWrap(&pc->CharType), TypeArray, 0, pc->StrEmpty, sizeof(char), (char *)&ca.y - &ca.x);
    pc->CharPtrType = TypeAdd(pc, NILL, ptrWrap(&pc->CharType), TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
    pc->CharPtrPtrType = TypeAdd(pc, NILL, pc->CharPtrType, TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
    pc->VoidPtrType = TypeAdd(pc, NILL, ptrWrap(&pc->VoidType), TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
}

/* deallocate heap-allocated types */
void TypeCleanupNode(Picoc *pc, TValueTypePtr Typ)
{
    TValueTypePtr SubType;
    TValueTypePtr NextSubType;
    
    /* clean up and free all the sub-nodes */
    for (SubType = Typ->DerivedTypeList; SubType != NULL; SubType = NextSubType)
    {
        NextSubType = SubType->Next;
        TypeCleanupNode(pc, SubType);
        if (SubType->Flags & FlagOnHeap)
        {
            /* if it's a struct or union deallocate all the member values */
            if (SubType->Members != NULL)
            {
                VariableTableCleanup(pc, SubType->Members);
                deallocMem(SubType->Members);
            }

            /* free this node */
            deallocMem(SubType);
        }
    }
}

void TypeCleanup(Picoc *pc)
{
    TypeCleanupNode(pc, ptrWrap(&pc->UberType));
}

/* parse a struct or union declaration */
void TypeParseStruct(TParseStatePtr Parser, TValueTypePtrPtr Typ, int IsStruct)
{
    TValuePtr LexValue;
    TValueTypePtr MemberType;
    TRegStringPtr MemberIdentifier;
    TRegStringPtr StructIdentifier;
    TValuePtr MemberValue;
    enum LexToken Token;
    int AlignBoundary;
    Picoc *pc = Parser->pc;
    
    Token = LexGetToken(Parser, &LexValue, FALSE);
    if (Token == TokenIdentifier)
    {
        LexGetToken(Parser, &LexValue, TRUE);
        StructIdentifier = LexValue->Val->Identifier;
        Token = LexGetToken(Parser, NILL, FALSE);
    }
    else
    {
        static char TempNameBuf[7] = "^s0000";
        StructIdentifier = PlatformMakeTempName(pc, TempNameBuf);
    }

    *Typ = TypeGetMatching(pc, Parser, ptrWrap(&Parser->pc->UberType), IsStruct ? TypeStruct : TypeUnion, 0, StructIdentifier, TRUE);
    if (Token == TokenLeftBrace && (*Typ)->Members != NULL)
        ProgramFail(Parser, "data type '%t' is already defined", (TValueTypePtr)*Typ);

    Token = LexGetToken(Parser, NILL, FALSE);
    if (Token != TokenLeftBrace)
    { 
        /* use the already defined structure */
#if 0
        if ((*Typ)->Members == NULL)
            ProgramFail(Parser, "structure '%s' isn't defined", LexValue->Val->Identifier);
#endif            
        return;
    }
    
    if (pc->TopStackFrame != NULL)
        ProgramFail(Parser, "struct/union definitions can only be globals");
        
    LexGetToken(Parser, NILL, TRUE);
    (*Typ)->Members = allocMemVariable<struct Table>(Parser, false, sizeof(struct Table) + STRUCT_TABLE_SIZE * sizeof(struct TableEntry));
    (*Typ)->Members->HashTable = (TTableEntryPtrPtr)((TTableCharPtr)(*Typ)->Members + sizeof(struct Table));
    TableInitTable((*Typ)->Members, (TTableEntryPtrPtr)((TTableCharPtr)(*Typ)->Members + sizeof(struct Table)), STRUCT_TABLE_SIZE, TRUE);
    
    do {
        TypeParse(Parser, &MemberType, &MemberIdentifier, NULL);
        if (MemberType == NULL || MemberIdentifier == NULL)
            ProgramFail(Parser, "invalid type in struct");
        
        MemberValue = VariableAllocValueAndData(pc, Parser, sizeof(int), FALSE, NILL, TRUE);
        MemberValue->Typ = MemberType;
        if (IsStruct)
        { 
            /* allocate this member's location in the struct */
            AlignBoundary = MemberValue->Typ->AlignBytes;
            if (((*Typ)->Sizeof & (AlignBoundary-1)) != 0)
                (*Typ)->Sizeof += AlignBoundary - ((*Typ)->Sizeof & (AlignBoundary-1));
                
            MemberValue->Val->Integer = (*Typ)->Sizeof;
            (*Typ)->Sizeof += TypeSizeValue(MemberValue, TRUE);
        }
        else
        { 
            /* union members always start at 0, make sure it's big enough to hold the largest member */
            MemberValue->Val->Integer = 0;
            if (MemberValue->Typ->Sizeof > (*Typ)->Sizeof)
                (*Typ)->Sizeof = TypeSizeValue(MemberValue, TRUE);
        }

        /* make sure to align to the size of the largest member's alignment */
        if ((*Typ)->AlignBytes < MemberValue->Typ->AlignBytes)
            (*Typ)->AlignBytes = MemberValue->Typ->AlignBytes;
        
        /* define it */
        if (!TableSet(pc, (*Typ)->Members, MemberIdentifier, MemberValue, Parser->FileName, Parser->Line, Parser->CharacterPos))
            ProgramFail(Parser, "member '%s' already defined", &MemberIdentifier);
            
        if (LexGetToken(Parser, NILL, TRUE) != TokenSemicolon)
            ProgramFail(Parser, "semicolon expected");
                    
    } while (LexGetToken(Parser, NILL, FALSE) != TokenRightBrace);
    
    /* now align the structure to the size of its largest member's alignment */
    AlignBoundary = (*Typ)->AlignBytes;
    if (((*Typ)->Sizeof & (AlignBoundary-1)) != 0)
        (*Typ)->Sizeof += AlignBoundary - ((*Typ)->Sizeof & (AlignBoundary-1));
    
    LexGetToken(Parser, NILL, TRUE);
}

/* create a system struct which has no user-visible members */
TValueTypePtr TypeCreateOpaqueStruct(Picoc *pc, TParseStatePtr Parser, TConstRegStringPtr StructName, int Size)
{
    TValueTypePtr Typ = TypeGetMatching(pc, Parser, ptrWrap(&pc->UberType), TypeStruct, 0, StructName, FALSE);
    
    /* create the (empty) table */
    Typ->Members = allocMemVariable<struct Table>(Parser, false, sizeof(struct Table) + STRUCT_TABLE_SIZE * sizeof(struct TableEntry));
    Typ->Members->HashTable = (TTableEntryPtrPtr)((TTableCharPtr)Typ->Members + sizeof(struct Table));
    TableInitTable(Typ->Members, (TTableEntryPtrPtr)((TTableCharPtr)Typ->Members + sizeof(struct Table)), STRUCT_TABLE_SIZE, TRUE);
    Typ->Sizeof = Size;
    
    return Typ;
}

/* parse an enum declaration */
void TypeParseEnum(TParseStatePtr Parser, TValueTypePtrPtr Typ)
{
    TValuePtr LexValue;
    struct Value InitValue;
    enum LexToken Token;
    int EnumValue = 0;
    TRegStringPtr EnumIdentifier;
    Picoc *pc = Parser->pc;
    
    Token = LexGetToken(Parser, &LexValue, FALSE);
    if (Token == TokenIdentifier)
    {
        LexGetToken(Parser, &LexValue, TRUE);
        EnumIdentifier = LexValue->Val->Identifier;
        Token = LexGetToken(Parser, NILL, FALSE);
    }
    else
    {
        static char TempNameBuf[7] = "^e0000";
        EnumIdentifier = PlatformMakeTempName(pc, TempNameBuf);
    }

    TypeGetMatching(pc, Parser, ptrWrap(&pc->UberType), TypeEnum, 0, EnumIdentifier, Token != TokenLeftBrace);
    *Typ = ptrWrap(&pc->IntType);
    if (Token != TokenLeftBrace)
    { 
        /* use the already defined enum */
        if ((*Typ)->Members == NULL)
            ProgramFail(Parser, "enum '%s' isn't defined", EnumIdentifier);
            
        return;
    }
    
    if (pc->TopStackFrame != NULL)
        ProgramFail(Parser, "enum definitions can only be globals");
        
    LexGetToken(Parser, NILL, TRUE);
    (*Typ)->Members = ptrWrap(&pc->GlobalTable);
    memset((void *)&InitValue, '\0', sizeof(struct Value));
    InitValue.Typ = ptrWrap(&pc->IntType);
    InitValue.Val = (TAnyValuePtr)ptrWrap(&EnumValue);
    do {
        if (LexGetToken(Parser, &LexValue, TRUE) != TokenIdentifier)
            ProgramFail(Parser, "identifier expected");
        
        EnumIdentifier = LexValue->Val->Identifier;
        if (LexGetToken(Parser, NILL, FALSE) == TokenAssign)
        {
            LexGetToken(Parser, NILL, TRUE);
            EnumValue = ExpressionParseInt(Parser);
        }
        
        VariableDefine(pc, Parser, EnumIdentifier, ptrWrap(&InitValue), NILL, FALSE);
            
        Token = LexGetToken(Parser, NILL, TRUE);
        if (Token != TokenComma && Token != TokenRightBrace)
            ProgramFail(Parser, "comma expected");
        
        EnumValue++;
                    
    } while (Token == TokenComma);
}

/* parse a type - just the basic type */
int TypeParseFront(TParseStatePtr Parser, TValueTypePtrPtr Typ, int *IsStatic)
{
    struct ParseState Before;
    TValuePtr LexerValue;
    enum LexToken Token;
    int Unsigned = FALSE;
    TValuePtr VarValue;
    int StaticQualifier = FALSE;
    Picoc *pc = Parser->pc;
    *Typ = NILL;

    /* ignore leading type qualifiers */
    ParserCopy(ptrWrap(&Before), Parser);
    Token = LexGetToken(Parser, &LexerValue, TRUE);
    while (Token == TokenStaticType || Token == TokenAutoType || Token == TokenRegisterType || Token == TokenExternType)
    {
        if (Token == TokenStaticType)
            StaticQualifier = TRUE;
            
        Token = LexGetToken(Parser, &LexerValue, TRUE);
    }
    
    if (IsStatic != NULL)
        *IsStatic = StaticQualifier;
        
    /* handle signed/unsigned with no trailing type */
    if (Token == TokenSignedType || Token == TokenUnsignedType)
    {
        enum LexToken FollowToken = LexGetToken(Parser, &LexerValue, FALSE);
        Unsigned = (Token == TokenUnsignedType);
        
        if (FollowToken != TokenIntType && FollowToken != TokenLongType && FollowToken != TokenShortType && FollowToken != TokenCharType)
        {
            if (Token == TokenUnsignedType)
                *Typ = ptrWrap(&pc->UnsignedIntType);
            else
                *Typ = ptrWrap(&pc->IntType);
            
            return TRUE;
        }
        
        Token = LexGetToken(Parser, &LexerValue, TRUE);
    }
    
    switch (Token)
    {
        case TokenIntType: *Typ = Unsigned ? ptrWrap(&pc->UnsignedIntType) : ptrWrap(&pc->IntType); break;
        case TokenShortType: *Typ = Unsigned ? ptrWrap(&pc->UnsignedShortType) : ptrWrap(&pc->ShortType); break;
        case TokenCharType: *Typ = Unsigned ? ptrWrap(&pc->UnsignedCharType) : ptrWrap(&pc->CharType); break;
        case TokenLongType: *Typ = Unsigned ? ptrWrap(&pc->UnsignedLongType) : ptrWrap(&pc->LongType); break;
#ifndef NO_FP
        case TokenFloatType: case TokenDoubleType: *Typ = ptrWrap(&pc->FPType); break;
#endif
        case TokenVoidType: *Typ = ptrWrap(&pc->VoidType); break;
        
        case TokenStructType: case TokenUnionType: 
            if (*Typ != NILL)
                ProgramFail(Parser, "bad type declaration");
                
            TypeParseStruct(Parser, Typ, Token == TokenStructType);
            break;

        case TokenEnumType:
            if (*Typ != NILL)
                ProgramFail(Parser, "bad type declaration");
                
            TypeParseEnum(Parser, Typ);
            break;
        
        case TokenIdentifier:
            /* we already know it's a typedef-defined type because we got here */
            VariableGet(pc, Parser, LexerValue->Val->Identifier, &VarValue);
            *Typ = VarValue->Val->Typ;
            break;

        default: ParserCopy(Parser, ptrWrap(&Before)); return FALSE;
    }
    
    return TRUE;
}

/* parse a type - the part at the end after the identifier. eg. array specifications etc. */
TValueTypePtr TypeParseBack(TParseStatePtr Parser, TValueTypePtr FromType)
{
    enum LexToken Token;
    struct ParseState Before;

    ParserCopy(ptrWrap(&Before), Parser);
    Token = LexGetToken(Parser, NILL, TRUE);
    if (Token == TokenLeftSquareBracket)
    {
        /* add another array bound */
        if (LexGetToken(Parser, NILL, FALSE) == TokenRightSquareBracket)
        {
            /* an unsized array */
            LexGetToken(Parser, NILL, TRUE);
            return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, 0, Parser->pc->StrEmpty, TRUE);
        }
        else
        {
            /* get a numeric array size */
            enum RunMode OldMode = Parser->Mode;
            int ArraySize;
            Parser->Mode = RunModeRun;
            ArraySize = ExpressionParseInt(Parser);
            Parser->Mode = OldMode;
            
            if (LexGetToken(Parser, NILL, TRUE) != TokenRightSquareBracket)
                ProgramFail(Parser, "']' expected");
            
            return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, ArraySize, Parser->pc->StrEmpty, TRUE);
        }
    }
    else
    {
        /* the type specification has finished */
        ParserCopy(Parser, ptrWrap(&Before));
        return FromType;
    }
}

/* parse a type - the part which is repeated with each identifier in a declaration list */
void TypeParseIdentPart(TParseStatePtr Parser, TValueTypePtr BasicTyp, TValueTypePtrPtr Typ, TRegStringPtrPtr Identifier)
{
    struct ParseState Before;
    enum LexToken Token;
    TValuePtr LexValue;
    int Done = FALSE;
    *Typ = BasicTyp;
    *Identifier = Parser->pc->StrEmpty;
    
    while (!Done)
    {
        ParserCopy(ptrWrap(&Before), Parser);
        Token = LexGetToken(Parser, &LexValue, TRUE);
        switch (Token)
        {
            case TokenOpenBracket:
                if (*Typ != NILL)
                    ProgramFail(Parser, "bad type declaration");
                
                TypeParse(Parser, Typ, Identifier, NULL);
                if (LexGetToken(Parser, NILL, TRUE) != TokenCloseBracket)
                    ProgramFail(Parser, "')' expected");
                break;
                
            case TokenAsterisk:
                if (*Typ == NILL)
                    ProgramFail(Parser, "bad type declaration");

                *Typ = TypeGetMatching(Parser->pc, Parser, *Typ, TypePointer, 0, Parser->pc->StrEmpty, TRUE);
                break;
            
            case TokenIdentifier:
                if (*Typ == NILL || *Identifier != Parser->pc->StrEmpty)
                    ProgramFail(Parser, "bad type declaration");
                
                *Identifier = LexValue->Val->Identifier;
                Done = TRUE;
                break;
                
            default: ParserCopy(Parser, ptrWrap(&Before)); Done = TRUE; break;
        }
    }
    
    if (*Typ == NILL)
        ProgramFail(Parser, "bad type declaration");

    if (*Identifier != Parser->pc->StrEmpty)
    { 
        /* parse stuff after the identifier */
        *Typ = TypeParseBack(Parser, *Typ);
    }
}

/* parse a type - a complete declaration including identifier */
void TypeParse(TParseStatePtr Parser, TValueTypePtrPtr Typ, TRegStringPtrPtr Identifier, int *IsStatic)
{
    TValueTypePtr BasicType;
    
    TypeParseFront(Parser, &BasicType, IsStatic);
    TypeParseIdentPart(Parser, BasicType, Typ, Identifier);
}

/* check if a type has been fully defined - otherwise it's just a forward declaration */
int TypeIsForwardDeclared(TParseStatePtr Parser, TValueTypePtr Typ)
{
    if (Typ->Base == TypeArray)
        return TypeIsForwardDeclared(Parser, Typ->FromType);
    
    if ( (Typ->Base == TypeStruct || Typ->Base == TypeUnion) && Typ->Members == NULL)
        return TRUE;
        
    return FALSE;
}
