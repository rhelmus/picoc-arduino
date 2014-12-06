/* picoc variable storage. This provides ways of defining and accessing
 * variables */
 
#include "interpreter.h"

/* maximum size of a value to temporarily copy while we create a variable */
#define MAX_TMP_COPY_BUF 256

/* Stolen from http://websvn.hylands.org/filedetails.php?repname=Projects&path=%2Fcommon%2FCrc8.c&sc=1
 * Copyright (c) 2006 Dave Hylands     <dhylands@gmail.com> */
uint8_t Crc8(unsigned char Crc, unsigned char Data)
{
    uint8_t i;
    uint8_t ret;

    ret = Crc ^ Data;

    for (i=0; i<8; ++i)
    {
        if ((ret & 0x80) != 0)
        {
            ret <<= 1;
            ret ^= 0x07;
        }
        else
            ret <<= 1;
    }

    return ret;
}

uint8_t StrHash(const char *Str)
{
    uint8_t ret = 0;
    const char *p = Str;
    while (p && *p)
    {
        ret = Crc8(ret, *p);
        ++p;
    }
    return ret;
}

/* initialise the variable system */
void VariableInit(Picoc *pc)
{
    TableInitTable(&(pc->GlobalTable), &(pc->GlobalHashTable)[0], GLOBAL_TABLE_SIZE, TRUE);
    TableInitTable(&pc->StringLiteralTable, &pc->StringLiteralHashTable[0], STRING_LITERAL_TABLE_SIZE, TRUE);
    pc->TopStackFrame = NULL;
}

/* deallocate the contents of a variable */
void VariableFree(Picoc *pc, TValuePtr Val)
{
    if (Val->Flags & (FlagValOnHeap | FlagAnyValOnHeap))
    {
        /* free function bodies */
        if (Val->Typ == &pc->FunctionType && Val->Val->FuncDef.Intrinsic == NULL && Val->Val->FuncDef.Body != NULL)
        {
            if (Val->Val->FuncDef.Body->Pos)
                deallocMem(Val->Val->FuncDef.Body->Pos);
            HeapFreeMem(pc, (void *)Val->Val->FuncDef.Body);
        }

        /* free macro bodies */
        if (Val->Typ == &pc->MacroType)
            deallocMem(Val->Val->MacroDef.Body.Pos);

        /* free the AnyValue */
        if (Val->Flags & FlagAnyValOnHeap)
            deallocMem(Val->Val);
    }

    /* free the value */
    if (Val->Flags & FlagValOnHeap)
        deallocMem(Val);
}

/* deallocate the global table and the string literal table */
void VariableTableCleanup(Picoc *pc, struct Table *HashTable)
{
    struct TableEntry *Entry;
    struct TableEntry *NextEntry;
    int Count;
    
    for (Count = 0; Count < HashTable->Size; Count++)
    {
        for (Entry = HashTable->HashTable[Count]; Entry != NULL; Entry = NextEntry)
        {
            NextEntry = Entry->Next;
            VariableFree(pc, Entry->p.v.Val);
                
            /* free the hash table entry */
            HeapFreeMem(pc, Entry);
        }
    }
}

void VariableCleanup(Picoc *pc)
{
    VariableTableCleanup(pc, &pc->GlobalTable);
    VariableTableCleanup(pc, &pc->StringLiteralTable);
}

int varmemused = 0;

/* allocate some memory, either on the heap or the stack and check if we've run out */
void *VariableAlloc(Picoc *pc, struct ParseState *Parser, int Size, int OnHeap)
{
    void *NewValue;

    if (OnHeap)
        varmemused += Size;

    if (OnHeap)
        NewValue = HeapAllocMem(pc, Size);
    else
        NewValue = HeapAllocStack(pc, Size);
    
    if (NewValue == NULL)
        ProgramFail(Parser, "out of memory");
    
#ifdef DEBUG_HEAP
    if (!OnHeap)
        printf("pushing %d at 0x%lx\n", Size, (unsigned long)NewValue);
#endif

    debugline("VariableAlloc: %d\n", Size);

    return NewValue;
}

/* allocate a value either on the heap or the stack using space dependent on what type we want */
TValuePtr VariableAllocValueAndData(Picoc *pc, struct ParseState *Parser, int DataSize, int IsLValue, TValuePtr LValueFrom, int OnHeap)
{
    TValuePtr NewValue = allocMemVariable<Value>(Parser, !OnHeap, MEM_ALIGN(sizeof(struct Value)) + DataSize);
    NewValue->Val = (TAnyValuePtr)((TAnyValueCharPointer)(NewValue) + MEM_ALIGN(sizeof(struct Value)));
    NewValue->Flags = 0;
    if (OnHeap)
        NewValue->Flags |= FlagValOnHeap;
    else
        NewValue->Flags |= FlagOnStack;
    if (IsLValue)
        NewValue->Flags |= FlagIsLValue;
    NewValue->LValueFrom = LValueFrom;
    if (Parser) 
        NewValue->ScopeID = Parser->ScopeID;

    debugline("sizes: %d/%d/%d\n", sizeof(struct Value), MEM_ALIGN(sizeof(struct Value)), sizeof(NewValue->Flags));

    if (Parser)
        debugline("VariableAllocValueAndData: %d+%d: %s:%d:%d\n", MEM_ALIGN(sizeof(struct Value)), DataSize, Parser->FileName, Parser->Line, Parser->CharacterPos);

    return NewValue;
}

/* allocate a value given its type */
TValuePtr VariableAllocValueFromType(Picoc *pc, struct ParseState *Parser, struct ValueType *Typ, int IsLValue, TValuePtr LValueFrom, int OnHeap)
{
    int Size = TypeSize(Typ, Typ->ArraySize, FALSE);
    TValuePtr NewValue = VariableAllocValueAndData(pc, Parser, Size, IsLValue, LValueFrom, OnHeap);
    assert(Size >= 0 || Typ == &pc->VoidType);
    NewValue->Typ = Typ;

#ifdef WRAP_ANYVALUE
    if (Typ->Base == TypeArray)
    {
        NewValue->Val->ArrayMem = (TAnyValueCharPointer)(&NewValue->Val->ArrayMem) + MEM_ALIGN(sizeof(TAnyValueCharPointer));
        printf("size: %d\n", Size);
    }
#endif

    return NewValue;
}

/* allocate a value either on the heap or the stack and copy its value. handles overlapping data */
TValuePtr VariableAllocValueAndCopy(Picoc *pc, struct ParseState *Parser, TValuePtr FromValue, int OnHeap)
{
    struct ValueType *DType = FromValue->Typ;
    TValuePtr NewValue;
    char TmpBuf[MAX_TMP_COPY_BUF];
    int CopySize = TypeSizeValue(FromValue, TRUE);

    assert(CopySize <= MAX_TMP_COPY_BUF);
    memcpy((void *)&TmpBuf[0], FromValue->Val, CopySize);
    NewValue = VariableAllocValueAndData(pc, Parser, CopySize, (FromValue->Flags & FlagIsLValue), FromValue->LValueFrom, OnHeap);
    NewValue->Typ = DType;
    memcpy(NewValue->Val, (void *)&TmpBuf[0], CopySize);
    
    return NewValue;
}

/* allocate a value either on the heap or the stack from an existing AnyValue and type */
TValuePtr VariableAllocValueFromExistingData(struct ParseState *Parser, struct ValueType *Typ, TAnyValuePtr FromValue, int IsLValue, TValuePtr LValueFrom)
{
    TValuePtr NewValue = allocMemVariable<struct Value>(Parser, TRUE, sizeof(struct Value));
    NewValue->Typ = Typ;
    NewValue->Val = FromValue;
    NewValue->Flags = 0;
    if (IsLValue)
        NewValue->Flags |= FlagIsLValue;
    NewValue->LValueFrom = LValueFrom;

#ifdef WRAP_ANYVALUE
    if (Typ->Base == TypeArray)
        NewValue->Val->ArrayMem = (TAnyValueCharPointer)(&NewValue->Val->ArrayMem) + MEM_ALIGN(sizeof(TAnyValueCharPointer));
#endif

    return NewValue;
}

/* allocate a value either on the heap or the stack from an existing Value, sharing the value */
TValuePtr VariableAllocValueShared(struct ParseState *Parser, TValuePtr FromValue)
{
    return VariableAllocValueFromExistingData(Parser, FromValue->Typ, FromValue->Val, (FromValue->Flags & FlagIsLValue), (FromValue->Flags & FlagIsLValue) ? FromValue : NILL);
}

/* reallocate a variable so its data has a new size */
void VariableRealloc(struct ParseState *Parser, TValuePtr FromValue, int NewSize)
{
    if (FromValue->Flags & FlagAnyValOnHeap)
        deallocMem(FromValue->Val);

#ifdef WRAP_ANYVALUE
    struct ValueType *Typ = FromValue->Typ;
#endif

    FromValue->Val = allocMemVariable<AnyValue>(Parser, false, NewSize);
#ifdef WRAP_ANYVALUE
    if (Typ->Base == TypeArray)
        FromValue->Val->ArrayMem = (TAnyValueCharPointer)(&FromValue->Val->ArrayMem) + MEM_ALIGN(sizeof(TAnyValueCharPointer));
#endif
    FromValue->Flags |= FlagAnyValOnHeap;
}

int VariableScopeBegin(struct ParseState * Parser, int16_t* OldScopeID)
{
    struct TableEntry *Entry;
    struct TableEntry *NextEntry;
    Picoc * pc = Parser->pc;
    int Count;
    #ifdef VAR_SCOPE_DEBUG
    int FirstPrint = 0;
    #endif
    
    struct Table * HashTable = (pc->TopStackFrame == NULL) ? &(pc->GlobalTable) : &(pc->TopStackFrame)->LocalTable;

    if (Parser->ScopeID == -1) return -1;

    /* XXX dumb hash, let's hope for no collisions... */
    *OldScopeID = Parser->ScopeID;
    /*Parser->ScopeID = (int)(intptr_t)(Parser->SourceText) * ((int)(intptr_t)(Parser->Pos) / sizeof(char*));*/
    /* or maybe a more human-readable hash for debugging? */
    /*Parser->ScopeID = Parser->Line * 0x10000 + Parser->CharacterPos;*/
    Parser->ScopeID = (Parser->SourceText) ? StrHash(Parser->SourceText) : 0 + Parser->Line * 0x100 + Parser->CharacterPos;
    
    for (Count = 0; Count < HashTable->Size; Count++)
    {
        for (Entry = HashTable->HashTable[Count]; Entry != NULL; Entry = NextEntry)
        {
            NextEntry = Entry->Next;
            if (Entry->p.v.Val->ScopeID == Parser->ScopeID && Entry->p.v.Val->Flags & FlagOutOfScope)
            {
                Entry->p.v.Val->Flags &= ~FlagOutOfScope;
                setPtrFromNum(Entry->p.v.Key, (getNumPtr(Entry->p.v.Key) & ~1));
                #ifdef VAR_SCOPE_DEBUG
                if (!FirstPrint) { PRINT_SOURCE_POS; }
                FirstPrint = 1;
                printf(">>> back into scope: %s %x %d\n", Entry->p.v.Key, Entry->p.v.Val->ScopeID, Entry->p.v.Val->Val->Integer);
                #endif
            }
        }
    }

    return Parser->ScopeID;
}

void VariableScopeEnd(struct ParseState * Parser, int ScopeID, int16_t PrevScopeID)
{
    struct TableEntry *Entry;
    struct TableEntry *NextEntry;
    Picoc * pc = Parser->pc;
    int Count;
    #ifdef VAR_SCOPE_DEBUG
    int FirstPrint = 0;
    #endif

    struct Table * HashTable = (pc->TopStackFrame == NULL) ? &(pc->GlobalTable) : &(pc->TopStackFrame)->LocalTable;

    if (ScopeID == -1) return;

    for (Count = 0; Count < HashTable->Size; Count++)
    {
        for (Entry = HashTable->HashTable[Count]; Entry != NULL; Entry = NextEntry)
        {
            NextEntry = Entry->Next;
            if (Entry->p.v.Val->ScopeID == ScopeID && !(Entry->p.v.Val->Flags & FlagOutOfScope))
            {
                #ifdef VAR_SCOPE_DEBUG
                if (!FirstPrint) { PRINT_SOURCE_POS; }
                FirstPrint = 1;
                printf(">>> out of scope: %s %x %d\n", Entry->p.v.Key, Entry->p.v.Val->ScopeID, Entry->p.v.Val->Val->Integer);
                #endif
                Entry->p.v.Val->Flags |= FlagOutOfScope;
                setPtrFromNum(Entry->p.v.Key, (getNumPtr(Entry->p.v.Key) | 1)); /* alter the key so it won't be found by normal searches */
            }
        }
    }

    Parser->ScopeID = PrevScopeID;
}

int VariableDefinedAndOutOfScope(Picoc * pc, TConstRegStringPtr Ident)
{
    struct TableEntry *Entry;
    int Count;

    struct Table * HashTable = (pc->TopStackFrame == NULL) ? &(pc->GlobalTable) : &(pc->TopStackFrame)->LocalTable;
    for (Count = 0; Count < HashTable->Size; Count++)
    {
        for (Entry = HashTable->HashTable[Count]; Entry != NULL; Entry = Entry->Next)
        {
            if (Entry->p.v.Val->Flags & FlagOutOfScope && (getNumPtr(Entry->p.v.Key) & ~1) == getNumPtr(Ident))
                return TRUE;
        }
    }
    return FALSE;
}

/* define a variable. Ident must be registered */
TValuePtr VariableDefine(Picoc *pc, struct ParseState *Parser, TRegStringPtr Ident, TValuePtr InitValue, struct ValueType *Typ, int MakeWritable)
{
    TValuePtr AssignValue;
    struct Table * currentTable = (pc->TopStackFrame == NULL) ? &(pc->GlobalTable) : &(pc->TopStackFrame)->LocalTable;
    
    int16_t ScopeID = Parser ? Parser->ScopeID : -1;
#if 1//def VAR_SCOPE_DEBUG
    if (Parser) fprintf(stderr, "def %s %x (%s:%d:%d)\n", ptrUnwrap(Ident), ScopeID, ptrUnwrap(Parser->FileName), Parser->Line, Parser->CharacterPos);
#endif

    if (InitValue != NULL)
        AssignValue = VariableAllocValueAndCopy(pc, Parser, InitValue, pc->TopStackFrame == NULL);
    else
        AssignValue = VariableAllocValueFromType(pc, Parser, Typ, MakeWritable, NILL, pc->TopStackFrame == NULL);

    if (MakeWritable)
        AssignValue->Flags |= FlagIsLValue;
    else
        AssignValue->Flags &= ~FlagIsLValue;
    AssignValue->ScopeID = ScopeID;
    AssignValue->Flags &= ~FlagOutOfScope;

    if (!TableSet(pc, currentTable, Ident, AssignValue, Parser ? (Parser->FileName) : NILL, Parser ? Parser->Line : 0, Parser ? Parser->CharacterPos : 0))
        ProgramFail(Parser, "'%s' is already defined", Ident);
    
    return AssignValue;
}

/* define a variable. Ident must be registered. If it's a redefinition from the same declaration don't throw an error */
TValuePtr VariableDefineButIgnoreIdentical(struct ParseState *Parser, TRegStringPtr Ident, struct ValueType *Typ, int IsStatic, int *FirstVisit)
{
    Picoc *pc = Parser->pc;
    TValuePtr ExistingValue;
    const char *DeclFileName;
    int DeclLine;
    int DeclColumn;
    
    /* is the type a forward declaration? */
    if (TypeIsForwardDeclared(Parser, Typ))
        ProgramFail(Parser, "type '%t' isn't defined", Typ);

    if (IsStatic)
    {
        char MangledName[LINEBUFFER_MAX];
        char *MNPos = &MangledName[0];
        char *MNEnd = &MangledName[LINEBUFFER_MAX-1];
        TConstRegStringPtr RegisteredMangledName;
        
        /* make the mangled static name (avoiding using sprintf() to minimise library impact) */
        memset((void *)&MangledName, '\0', sizeof(MangledName));
        *MNPos++ = '/';
        strncpy(MNPos, Parser->FileName, MNEnd - MNPos);
        MNPos += strlen(MNPos);
        
        if (pc->TopStackFrame != NULL)
        {
            /* we're inside a function */
            if (MNEnd - MNPos > 0) *MNPos++ = '/';
            strncpy(MNPos, pc->TopStackFrame->FuncName, MNEnd - MNPos);
            MNPos += strlen(MNPos);
        }
            
        if (MNEnd - MNPos > 0) *MNPos++ = '/';
        strncpy(MNPos, Ident, MNEnd - MNPos);
        RegisteredMangledName = TableStrRegister(pc, MangledName);
        
        /* is this static already defined? */
        if (!TableGet(&pc->GlobalTable, RegisteredMangledName, &ExistingValue, &DeclFileName, &DeclLine, &DeclColumn))
        {
            /* define the mangled-named static variable store in the global scope */
            ExistingValue = VariableAllocValueFromType(Parser->pc, Parser, Typ, TRUE, NILL, TRUE);
            TableSet(pc, &pc->GlobalTable, RegisteredMangledName, ExistingValue, Parser->FileName, Parser->Line, Parser->CharacterPos);
            *FirstVisit = TRUE;
        }

        /* static variable exists in the global scope - now make a mirroring variable in our own scope with the short name */
        VariableDefinePlatformVar(Parser->pc, Parser, Ident, ExistingValue->Typ, ExistingValue->Val, TRUE);
        return ExistingValue;
    }
    else
    {
        if (Parser->Line != 0 && TableGet((pc->TopStackFrame == NULL) ? &pc->GlobalTable : &pc->TopStackFrame->LocalTable, Ident, &ExistingValue, &DeclFileName, &DeclLine, &DeclColumn))
        {
#ifndef DISABLE_TABLEENTRY_DECL
            if (DeclFileName == Parser->FileName && DeclLine == Parser->Line && DeclColumn == Parser->CharacterPos)
                return ExistingValue;
            debugline("-------- Mismatch!! --------\n");
#else
            return ExistingValue;
#endif
        }

        return VariableDefine(Parser->pc, Parser, Ident, NILL, Typ, TRUE);
#if 0
        if (Parser->Line != 0 && TableGet((pc->TopStackFrame == NULL) ? &pc->GlobalTable : &pc->TopStackFrame->LocalTable, Ident, &ExistingValue, &DeclFileName, &DeclLine, &DeclColumn)
                && DeclFileName == Parser->FileName && DeclLine == Parser->Line && DeclColumn == Parser->CharacterPos)
            return ExistingValue;
        else
            return VariableDefine(Parser->pc, Parser, Ident, NULL, Typ, TRUE);
#endif
    }
}

/* check if a variable with a given name is defined. Ident must be registered */
int VariableDefined(Picoc *pc, TConstRegStringPtr Ident)
{
    TValuePtr FoundValue;
    
    if (pc->TopStackFrame == NULL || !TableGet(&pc->TopStackFrame->LocalTable, Ident, &FoundValue, NULL, NULL, NULL))
    {
        if (!TableGet(&pc->GlobalTable, Ident, &FoundValue, NULL, NULL, NULL))
            return FALSE;
    }

    return TRUE;
}

/* get the value of a variable. must be defined. Ident must be registered */
void VariableGet(Picoc *pc, struct ParseState *Parser, TConstRegStringPtr Ident, TValuePtrPtr LVal)
{
    if (pc->TopStackFrame == NULL || !TableGet(&pc->TopStackFrame->LocalTable, Ident, LVal, NULL, NULL, NULL))
    {
        if (!TableGet(&pc->GlobalTable, Ident, LVal, NULL, NULL, NULL))
        {
            if (VariableDefinedAndOutOfScope(pc, Ident))
                ProgramFail(Parser, "'%s' is out of scope", Ident);
            else
                ProgramFail(Parser, "'%s' is undefined", Ident);
        }
    }
}

/* define a global variable shared with a platform global. Ident will be registered */
void VariableDefinePlatformVar(Picoc *pc, struct ParseState *Parser, const char *Ident, struct ValueType *Typ, TAnyValuePtr FromValue, int IsWritable)
{
    return VariableDefinePlatformVar(pc, Parser, ptrWrap(Ident), Typ, FromValue, IsWritable);
}

void VariableDefinePlatformVar(Picoc *pc, struct ParseState *Parser, TConstRegStringPtr Ident, struct ValueType *Typ, TAnyValuePtr FromValue, int IsWritable)
{
    TValuePtr SomeValue = VariableAllocValueAndData(pc, NULL, 0, IsWritable, NILL, TRUE);
    SomeValue->Typ = Typ;
    SomeValue->Val = FromValue;
    
    if (!TableSet(pc, (pc->TopStackFrame == NULL) ? &pc->GlobalTable : &pc->TopStackFrame->LocalTable, TableStrRegister(pc, Ident), SomeValue, Parser ? Parser->FileName : NILL, Parser ? Parser->Line : 0, Parser ? Parser->CharacterPos : 0))
        ProgramFail(Parser, "'%s' is already defined", Ident);
}

/* free and/or pop the top value off the stack. Var must be the top value on the stack! */
void VariableStackPop(struct ParseState *Parser, TValuePtr Var)
{
    int Success;
    
#ifdef DEBUG_HEAP
    if (Var->ValOnStack)
        printf("popping %ld at 0x%lx\n", (unsigned long)(sizeof(struct Value) + TypeSizeValue(Var, FALSE)), (unsigned long)Var);
#endif
        
    if (Var->Flags & FlagValOnHeap)
    { 
        if (Var->Val != NULL)
            deallocMem(Var->Val);
            
        Success = popStack(Var, sizeof(struct Value));                       /* free from heap */
    }
    else if (Var->Flags & FlagOnStack)
        Success = popStack(Var, sizeof(struct Value) + TypeSizeValue(Var, FALSE));  /* free from stack */
    else
        Success = popStack(Var, sizeof(struct Value));                       /* value isn't our problem */

    if (!Success)
        ProgramFail(Parser, "stack underrun");
}

/* add a stack frame when doing a function call */
void VariableStackFrameAdd(struct ParseState *Parser, TConstRegStringPtr FuncName, int NumParams)
{
    struct StackFrame *NewFrame;
    
    HeapPushStackFrame(Parser->pc);
    NewFrame = (struct StackFrame *)HeapAllocStack(Parser->pc, sizeof(struct StackFrame) + sizeof(TValuePtr) * NumParams);
    if (NewFrame == NULL)
        ProgramFail(Parser, "out of memory");
        
    ParserCopy(&NewFrame->ReturnParser, Parser);
    NewFrame->FuncName = FuncName;
    NewFrame->Parameter = (NumParams > 0) ? /*((TValuePtrPtr)((char *)NewFrame + sizeof(struct StackFrame))) UNDONE*/ (TValuePtrPtr)ptrWrap((char *)NewFrame + sizeof(struct StackFrame)) : NILL;
    TableInitTable(&NewFrame->LocalTable, &NewFrame->LocalHashTable[0], LOCAL_TABLE_SIZE, FALSE);
    NewFrame->PreviousStackFrame = Parser->pc->TopStackFrame;
    Parser->pc->TopStackFrame = NewFrame;
}

/* remove a stack frame */
void VariableStackFramePop(struct ParseState *Parser)
{
    if (Parser->pc->TopStackFrame == NULL)
        ProgramFail(Parser, "stack is empty - can't go back");
        
    ParserCopy(Parser, &Parser->pc->TopStackFrame->ReturnParser);
    Parser->pc->TopStackFrame = Parser->pc->TopStackFrame->PreviousStackFrame;
    HeapPopStackFrame(Parser->pc);
}

/* get a string literal. assumes that Ident is already registered. NULL if not found */
TValuePtr VariableStringLiteralGet(Picoc *pc, TRegStringPtr Ident)
{
    TValuePtr LVal = NILL;

    if (TableGet(&pc->StringLiteralTable, Ident, &LVal, NULL, NULL, NULL))
        return LVal;
    else
        return NILL;
}

/* define a string literal. assumes that Ident is already registered */
void VariableStringLiteralDefine(Picoc *pc, TRegStringPtr Ident, TValuePtr Val)
{
    TableSet(pc, &pc->StringLiteralTable, Ident, Val, NILL, 0, 0);
}

/* check a pointer for validity and dereference it for use */
TAnyValueVoidPointer VariableDereferencePointer(struct ParseState *Parser, TValuePtr PointerValue, TValuePtrPtr DerefVal, int *DerefOffset, struct ValueType **DerefType, int *DerefIsLValue)
{
    if (DerefVal != NULL)
        *DerefVal = NILL;
        
    if (DerefType != NULL)
        *DerefType = PointerValue->Typ->FromType;
        
    if (DerefOffset != NULL)
        *DerefOffset = 0;
        
    if (DerefIsLValue != NULL)
        *DerefIsLValue = TRUE;

    return PointerValue->Val->Pointer;
}

