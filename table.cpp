/* picoc hash table module. This hash table code is used for both symbol tables
 * and the shared string table. */
 
#include "interpreter.h"

/* initialise the shared string system */
void TableInit(Picoc *pc)
{
    TableInitTable(&pc->StringTable, &pc->StringHashTable[0], STRING_TABLE_SIZE, TRUE);
    pc->StrEmpty = TableStrRegister(pc, "");
}

/* hash function for strings */
static unsigned int TableHash(TConstRegStringPtr Key, int Len)
{
    unsigned int Hash = Len;
    int Offset;
    int Count;
    
    for (Count = 0, Offset = 8; Count < Len; Count++, Offset+=7)
    {
        if (Offset > (int)sizeof(unsigned int) * 8 - 7)
            Offset -= (int)sizeof(unsigned int) * 8 - 6;
            
        Hash ^= (char)*Key++ << Offset;
    }
    
    return Hash;
}

/* initialise a table */
void TableInitTable(struct Table *Tbl, struct TableEntry **HashTable, int Size, int OnHeap)
{
    Tbl->Size = Size;
    Tbl->OnHeap = OnHeap;
    Tbl->HashTable = HashTable;
    memset((void *)HashTable, '\0', sizeof(struct TableEntry *) * Size);
}

/* check a hash table entry for a key */
static struct TableEntry *TableSearch(struct Table *Tbl, TConstRegStringPtr Key, int *AddAt)
{
    struct TableEntry *Entry;
    int HashValue = ((unsigned long)(CPtrWrapperBase::getPtr(Key))) % Tbl->Size;   /* shared strings have unique addresses so we don't need to hash them */
    
    for (Entry = Tbl->HashTable[HashValue]; Entry != NULL; Entry = Entry->Next)
    {
        if (Entry->p.v.Key == Key)
            return Entry;   /* found */
    }
    
    *AddAt = HashValue;    /* didn't find it in the chain */
    return NULL;
}

/* set an identifier to a value. returns FALSE if it already exists. 
 * Key must be a shared string from TableStrRegister() */
int TableSet(Picoc *pc, struct Table *Tbl, TRegStringPtr Key, TValuePtr Val, TConstRegStringPtr DeclFileName, int DeclLine, int DeclColumn)
{
    int AddAt;
    struct TableEntry *FoundEntry = TableSearch(Tbl, Key, &AddAt);
    
    if (FoundEntry == NULL)
    {   /* add it to the table */
        struct TableEntry *NewEntry = (struct TableEntry *)VariableAlloc(pc, NULL, sizeof(struct TableEntry), Tbl->OnHeap);
#ifndef DISABLE_TABLEENTRY_DECL
        NewEntry->DeclFileName = DeclFileName;
        NewEntry->DeclLine = DeclLine;
        NewEntry->DeclColumn = DeclColumn;
#endif
        NewEntry->p.v.Key = Key;
        NewEntry->p.v.Val = Val;
        NewEntry->Next = Tbl->HashTable[AddAt];
        Tbl->HashTable[AddAt] = NewEntry;

        return TRUE;
    }

    return FALSE;
}

/* find a value in a table. returns FALSE if not found. 
 * Key must be a shared string from TableStrRegister() */
int TableGet(struct Table *Tbl, TConstRegStringPtr Key, TValuePtrPtr Val, const char **DeclFileName, int *DeclLine, int *DeclColumn)
{
    int AddAt;
    struct TableEntry *FoundEntry = TableSearch(Tbl, Key, &AddAt);
    if (FoundEntry == NULL)
        return FALSE;
    
    *Val = FoundEntry->p.v.Val;
    
    if (DeclFileName != NULL)
    {
#ifndef DISABLE_TABLEENTRY_DECL
        *DeclFileName = FoundEntry->DeclFileName;
        *DeclLine = FoundEntry->DeclLine;
        *DeclColumn = FoundEntry->DeclColumn;
#endif
    }
    
    return TRUE;
}

/* remove an entry from the table */
TValuePtr TableDelete(Picoc *pc, struct Table *Tbl, const TRegStringPtr Key)
{
    struct TableEntry **EntryPtr;
    int HashValue = ((unsigned long)CPtrWrapperBase::getPtr(Key)) % Tbl->Size;   /* shared strings have unique addresses so we don't need to hash them */
    
    for (EntryPtr = &Tbl->HashTable[HashValue]; *EntryPtr != NULL; EntryPtr = &(*EntryPtr)->Next)
    {
        if ((*EntryPtr)->p.v.Key == Key)
        {
            struct TableEntry *DeleteEntry = *EntryPtr;
            TValuePtr Val = DeleteEntry->p.v.Val;
            *EntryPtr = DeleteEntry->Next;
            HeapFreeMem(pc, DeleteEntry);

            return Val;
        }
    }

    return NILL;
}

/* check a hash table entry for an identifier */
static struct TableEntry *TableSearchIdentifier(struct Table *Tbl, TConstRegStringPtr Key, int Len, int *AddAt)
{
    struct TableEntry *Entry;
    int HashValue = TableHash(Key, Len) % Tbl->Size;
    
    for (Entry = Tbl->HashTable[HashValue]; Entry != NULL; Entry = Entry->Next)
    {
        if (strncmp(getMembrPtr(Entry,  &Entry->p.Key[0]), Key, Len) == 0 && Entry->p.Key[Len] == '\0')
            return Entry;   /* found */
    }
    
    *AddAt = HashValue;    /* didn't find it in the chain */
    return NULL;
}

/* set an identifier and return the identifier. share if possible */
TRegStringPtr TableSetIdentifier(Picoc *pc, struct Table *Tbl, TConstRegStringPtr Ident, int IdentLen)
{
    int AddAt;
    struct TableEntry *FoundEntry = TableSearchIdentifier(Tbl, Ident, IdentLen, &AddAt);

    if (FoundEntry != NULL)
        return &FoundEntry->p.Key[0];
    else
    {
#ifndef WRAP_REGSTRINGS
        /* add it to the table - we economise by not allocating the whole structure here */
        struct TableEntry *NewEntry = (struct TableEntry *)HeapAllocMem(pc, sizeof(struct TableEntry) - sizeof(union TableEntry::TableEntryPayload) + IdentLen + 1);
#else
        /* we have a slight bit of extra memory overhead for shared strings here */
        struct TableEntry *NewEntry = (struct TableEntry *)HeapAllocMem(pc, sizeof(struct TableEntry) - sizeof(union TableEntry::TableEntryPayload) + sizeof(TRegStringPtr) + IdentLen + 1);
#endif

        if (NewEntry == NULL)
            ProgramFailNoParser(pc, "out of memory");

#ifdef WRAP_REGSTRINGS
        NewEntry->p.Key = (TRegStringPtr)getMembrPtr(NewEntry, &NewEntry->p.Key) + sizeof(TRegStringPtr); // point just past pointer
#endif

        strncpy((TRegStringPtr)&NewEntry->p.Key[0], Ident, IdentLen);
        NewEntry->p.Key[IdentLen] = '\0';
        NewEntry->Next = Tbl->HashTable[AddAt];
        Tbl->HashTable[AddAt] = NewEntry;
        return &NewEntry->p.Key[0];
    }
}

/* register a string in the shared string store */
TRegStringPtr TableStrRegister2(Picoc *pc, const char *Str, int Len)
{
    return TableSetIdentifier(pc, &pc->StringTable, CPtrWrapperBase::wrap(Str), Len);
}

TRegStringPtr TableStrRegister2(Picoc *pc, TConstRegStringPtr Str, int Len)
{
    return TableSetIdentifier(pc, &pc->StringTable, Str, Len);
}

TRegStringPtr TableStrRegister(Picoc *pc, const char *Str)
{
    return TableStrRegister2(pc, Str, strlen((char *)Str));
}

TRegStringPtr TableStrRegister(Picoc *pc, TConstRegStringPtr Str)
{
    return TableStrRegister2(pc, Str, strlen(Str));
}

/* free all the strings */
void TableStrFree(Picoc *pc)
{
    struct TableEntry *Entry;
    struct TableEntry *NextEntry;
    int Count;
    
    for (Count = 0; Count < pc->StringTable.Size; Count++)
    {
        for (Entry = pc->StringTable.HashTable[Count]; Entry != NULL; Entry = NextEntry)
        {
            NextEntry = Entry->Next;
            HeapFreeMem(pc, Entry);
        }
    }
}
