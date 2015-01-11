/* picoc heap memory allocation. This is a complete (but small) memory
 * allocator for embedded systems which have no memory allocator. Alternatively
 * you can define USE_MALLOC_HEAP to use your system's own malloc() allocator */
 
/* stack grows up from the bottom and heap grows down from the top of heap space */
#include "interpreter.h"

#define NVALGRIND

#ifndef NVALGRIND
# include <valgrind/memcheck.h>
#endif

//#define DEBUG_HEAP

#ifdef DEBUG_HEAP
void ShowBigList(Picoc *pc)
{
    struct AllocNode *LPos;
    
    printf("Heap: bottom=0x%lx 0x%lx-0x%lx, big freelist=", (long)pc->HeapBottom, (long)&(pc->HeapMemory)[0], (long)&(pc->HeapMemory)[HEAP_SIZE]);
    for (LPos = pc->FreeListBig; LPos != NULL; LPos = LPos->NextFree)
        printf("0x%lx:%d ", (long)LPos, LPos->Size);
    
    printf("\n");
}
#endif

/* initialise the stack and heap storage */
void HeapInit(Picoc *pc, int StackOrHeapSize)
{
    int Count;
    int AlignOffset = 0;
    
#ifdef USE_MALLOC_STACK
    pc->HeapMemory = (unsigned char *)malloc(StackOrHeapSize);
    pc->HeapBottom = NULL;                     /* the bottom of the (downward-growing) heap */
    pc->StackFrame = NILL;                     /* the current stack frame */
    pc->HeapStackTop = NILL;                          /* the top of the stack */
#else
# ifdef SURVEYOR_HOST
    pc->HeapMemory = (unsigned char *)C_HEAPSTART;      /* all memory - stack and heap */
    pc->HeapBottom = (void *)C_HEAPSTART + HEAP_SIZE;  /* the bottom of the (downward-growing) heap */
    pc->StackFrame = (void *)C_HEAPSTART;              /* the current stack frame */
    pc->HeapStackTop = (void *)C_HEAPSTART;                   /* the top of the stack */
    pc->HeapMemStart = (void *)C_HEAPSTART;
# else
//    pc->HeapBottom = &pc->HeapMemory[HEAP_SIZE];   /* the bottom of the (downward-growing) heap */
//    pc->StackFrame = &pc->HeapMemory[0];           /* the current stack frame */
//    pc->HeapStackTop = &pc->HeapMemory[0];                /* the top of the stack */
    pc->StackFrame = NILL;                     /* the current stack frame */
    pc->HeapStackTop = NILL;                          /* the top of the stack */
# endif
#endif

#ifndef USE_VIRTMEM
    pc->HeapBottom = &(pc->HeapMemory)[StackOrHeapSize-sizeof(ALIGN_TYPE)+AlignOffset];
#endif

#ifndef USE_VIRTSTACK
    pc->StackStart = (TStackCharPtr)pc->HeapMemory;

    while (((unsigned long)&pc->HeapMemory[AlignOffset] & (sizeof(ALIGN_TYPE)-1)) != 0)
        AlignOffset++;

    pc->StackFrame = &(pc->HeapMemory)[AlignOffset];
    pc->HeapStackTop = &(pc->HeapMemory)[AlignOffset];
    pc->StackBottom = pc->HeapBottom;
#else
    pc->StackStart = (TStackCharPtr)pc->StackStart.alloc(StackOrHeapSize);

    // UNDONE: need this?
    /*while (((unsigned long)&pc->HeapMemory[AlignOffset] & (sizeof(ALIGN_TYPE)-1)) != 0)
        AlignOffset++;*/

    pc->StackFrame = &(pc->StackStart)[AlignOffset];
    pc->HeapStackTop = &(pc->StackStart)[AlignOffset];
    // UNDONE: need this?
//    pc->StackBottom = &(pc->StackStart)[StackOrHeapSize-sizeof(ALIGN_TYPE)+AlignOffset];
    pc->StackBottom = &(pc->StackStart)[StackOrHeapSize-1];
#endif
    *(TStackVoidPtrPtr)(pc->StackFrame) = NILL;
#ifdef DEBUG_HEAP
    printf("stack top/frame/bottom/*frame: %u/%u/%u/%u\n", (unsigned)getNumPtr(pc->StackStart), (unsigned)getNumPtr(pc->StackFrame), (unsigned)getNumPtr(pc->StackBottom), (unsigned)getNumPtr((TStackVoidPtr)*(TStackVoidPtrPtr)(pc->StackFrame)));
#endif

    pc->FreeListBig = NULL;
    for (Count = 0; Count < FREELIST_BUCKETS; Count++)
        pc->FreeListBucket[Count] = NULL;

#ifndef NVALGRIND
    VALGRIND_CREATE_MEMPOOL(pc->HeapMemory, 0, 0);
    VALGRIND_MAKE_MEM_NOACCESS(pc->HeapMemory, StackOrHeapSize);
#endif
}

void HeapCleanup(Picoc *pc)
{
#ifndef NVALGRIND
    VALGRIND_DESTROY_MEMPOOL(pc->HeapStackTop);
#endif

#ifdef USE_MALLOC_STACK
    free(pc->HeapMemory);
#endif
}

/* allocate some space on the stack, in the current stack frame
 * clears memory. can return NULL if out of stack space */
TStackVoidPtr HeapAllocStack(Picoc *pc, int Size)
{
    TStackCharPtr NewMem = (TStackCharPtr)pc->HeapStackTop;
    TStackCharPtr NewTop = (TStackCharPtr)pc->HeapStackTop + MEM_ALIGN(Size);
#ifdef DEBUG_HEAP
    printf("HeapAllocStack(%ld) at 0x%lx\n", (unsigned long)MEM_ALIGN(Size), (unsigned long)pc->HeapStackTop.getRawNum());
#endif
    if (NewTop > (TStackCharPtr)pc->StackBottom)
        return NILL;

//    printf("Stack used: %ld\n", ((intptr_t)NewTop - (intptr_t)pc->HeapMemory));

    pc->HeapStackTop = NewTop;
    memset(NewMem, '\0', Size);

    return NewMem;
}

/* allocate some space on the stack, in the current stack frame */
void HeapUnpopStack(Picoc *pc, int Size)
{
#ifdef DEBUG_HEAP
    printf("HeapUnpopStack(%ld) at 0x%lx\n", (unsigned long)MEM_ALIGN(Size), (unsigned long)pc->HeapStackTop.getRawNum());
#endif

    pc->HeapStackTop = (TStackVoidPtr)((TStackCharPtr)pc->HeapStackTop + MEM_ALIGN(Size));
}

/* free some space at the top of the stack */
int HeapPopStack(Picoc *pc, TStackVoidPtr Addr, int Size)
{
    int ToLose = MEM_ALIGN(Size);
    if (ToLose > ((TStackCharPtr)pc->HeapStackTop - (TStackCharPtr)&(pc->StackStart)[0]))
        return FALSE;

#ifdef DEBUG_HEAP
    printf("HeapPopStack(0x%lx, %ld) back to 0x%lx\n", (unsigned long)getNumPtr(Addr), (unsigned long)MEM_ALIGN(Size), (unsigned long)getNumPtr(pc->HeapStackTop) - ToLose);
#endif
    pc->HeapStackTop = (TStackVoidPtr)((TStackCharPtr)pc->HeapStackTop - ToLose);
    assert(Addr == NULL || pc->HeapStackTop == Addr);

//    printf("Stack used: %ld\n", ((intptr_t)pc->HeapStackTop - (intptr_t)pc->HeapMemory));

    return TRUE;
}

/* push a new stack frame on to the stack */
void HeapPushStackFrame(Picoc *pc)
{
#ifdef DEBUG_HEAP
    printf("Adding stack frame at 0x%lx\n", (unsigned long)pc->HeapStackTop.getRawNum());
#endif
    *(TStackVoidPtrPtr)pc->HeapStackTop = pc->StackFrame;
    pc->StackFrame = pc->HeapStackTop;
#ifdef USE_VIRTSTACK
    pc->HeapStackTop = (TStackVoidPtr)((TStackCharPtr)pc->HeapStackTop + MEM_ALIGN(sizeof(TStackVoidPtr)));
#else
    pc->HeapStackTop = (TStackVoidPtr)((TStackCharPtr)pc->HeapStackTop + MEM_ALIGN(sizeof(ALIGN_TYPE)));
#endif
}

/* pop the current stack frame, freeing all memory in the frame. can return NULL */
int HeapPopStackFrame(Picoc *pc)
{
    if (*(TStackVoidPtrPtr)pc->StackFrame != NILL)
    {
        pc->HeapStackTop = pc->StackFrame;
        pc->StackFrame = *(TStackVoidPtrPtr)pc->StackFrame;
#ifdef DEBUG_HEAP
        printf("Popping stack frame back to 0x%lx\n", (unsigned long)pc->HeapStackTop.getRawNum());
#endif
        return TRUE;
    }
    else
        return FALSE;
}

#ifndef USE_VIRTMEM

int memused = 0;
/* allocate some dynamically allocated memory. memory is cleared. can return NULL if out of memory */
void *HeapAllocMem(Picoc *pc, int Size)
{
#ifdef USE_MALLOC_HEAP
    return calloc(Size, 1);
#else
    struct AllocNode *NewMem = NULL;
    struct AllocNode **FreeNode;
    unsigned int AllocSize = MEM_ALIGN(Size) + MEM_ALIGN(sizeof(NewMem->Size));
    int Bucket;
    void *ReturnMem;
    
    if (Size == 0)
        return NULL;
    
    assert(Size > 0);

    /* make sure we have enough space for an AllocNode */
    if (AllocSize < sizeof(struct AllocNode))
        AllocSize = sizeof(struct AllocNode);

    memused += AllocSize;

    Bucket = AllocSize >> 2;
    if (Bucket < FREELIST_BUCKETS && pc->FreeListBucket[Bucket] != NULL)
    { 
        /* try to allocate from a freelist bucket first */
#ifdef DEBUG_HEAP
        printf("allocating %d(%d) from bucket", Size, AllocSize);
#endif
        NewMem = pc->FreeListBucket[Bucket];
        assert((unsigned long)NewMem >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)NewMem - &(pc->HeapMemory)[0] < HEAP_SIZE);
        pc->FreeListBucket[Bucket] = *(struct AllocNode **)NewMem;
        assert(pc->FreeListBucket[Bucket] == NULL || ((unsigned long)pc->FreeListBucket[Bucket] >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)pc->FreeListBucket[Bucket] - &(pc->HeapMemory)[0] < HEAP_SIZE));
        NewMem->Size = AllocSize;
    }
    else if (pc->FreeListBig != NULL)
    { 
        /* grab the first item from the "big" freelist we can fit in */
        for (FreeNode = &pc->FreeListBig; *FreeNode != NULL && (*FreeNode)->Size < AllocSize; FreeNode = &(*FreeNode)->NextFree)
        {}
        
        if (*FreeNode != NULL)
        {
            assert((unsigned long)*FreeNode >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)*FreeNode - &(pc->HeapMemory)[0] < HEAP_SIZE);
            assert((*FreeNode)->Size < HEAP_SIZE && (*FreeNode)->Size > 0);
            if ((*FreeNode)->Size < AllocSize + SPLIT_MEM_THRESHOLD)
            { 
                /* close in size - reduce fragmentation by not splitting */
#ifdef DEBUG_HEAP
               printf("allocating %d(%d) from freelist, no split (%d)", Size, AllocSize, (*FreeNode)->Size);
#endif
                NewMem = *FreeNode;
                assert((unsigned long)NewMem >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)NewMem - &(pc->HeapMemory)[0] < HEAP_SIZE);
                *FreeNode = NewMem->NextFree;
            }
            else
            { 
                /* split this big memory chunk */
#ifdef DEBUG_HEAP
                printf("allocating %d(%d) from freelist, split chunk (%d)", Size, AllocSize, (*FreeNode)->Size);
#endif
                NewMem = (struct AllocNode *)((char *)*FreeNode + (*FreeNode)->Size - AllocSize);
                assert((unsigned long)NewMem >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)NewMem - &(pc->HeapMemory)[0] < HEAP_SIZE);
                (*FreeNode)->Size -= AllocSize;
                NewMem->Size = AllocSize;
            }
        }
    }
    
    if (NewMem == NULL)
    { 
        /* couldn't allocate from a freelist - try to increase the size of the heap area */
#ifdef DEBUG_HEAP
        printf("allocating %d(%d) at bottom of heap (0x%lx-0x%lx)", Size, AllocSize, (long)((char *)pc->HeapBottom - AllocSize), (long)HeapBottom);
#endif
#ifndef USE_VIRTSTACK
        if ((char *)pc->HeapBottom - AllocSize < (char *)pc->HeapStackTop)
            return NULL;
#endif
        
        pc->HeapBottom = (void *)((char *)pc->HeapBottom - AllocSize);
        NewMem = (struct AllocNode *)pc->HeapBottom;
        NewMem->Size = AllocSize;
    }
    
    ReturnMem = (void *)((char *)NewMem + MEM_ALIGN(sizeof(NewMem->Size)));
    memset(ReturnMem, '\0', AllocSize - MEM_ALIGN(sizeof(NewMem->Size)));
#ifdef DEBUG_HEAP
    printf(" = %lx\n", (unsigned long)ReturnMem);
#endif
    return ReturnMem;
#endif
}

/* free some dynamically allocated memory */
void HeapFreeMem(Picoc *pc, void *Mem)
{
#ifdef USE_MALLOC_HEAP
    free(Mem);
#else
    struct AllocNode *MemNode = (struct AllocNode *)((char *)Mem - MEM_ALIGN(sizeof(MemNode->Size)));
    int Bucket = MemNode->Size >> 2;
    
#ifdef DEBUG_HEAP
    printf("HeapFreeMem(0x%lx)\n", (unsigned long)Mem);
#endif
    assert((unsigned long)Mem >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)Mem - &(pc->HeapMemory)[0] < HEAP_SIZE);
    assert(MemNode->Size < HEAP_SIZE && MemNode->Size > 0);
    if (Mem == NULL)
        return;
    
    memused -= MemNode->Size;

    if ((void *)MemNode == pc->HeapBottom)
    { 
        /* pop it off the bottom of the heap, reducing the heap size */
#ifdef DEBUG_HEAP
        printf("freeing %d from bottom of heap\n", MemNode->Size);
#endif
        pc->HeapBottom = (void *)((char *)pc->HeapBottom + MemNode->Size);
#ifdef DEBUG_HEAP
        ShowBigList(pc);
#endif
    }
    else if (Bucket < FREELIST_BUCKETS)
    { 
        /* we can fit it in a bucket */
#ifdef DEBUG_HEAP
        printf("freeing %d to bucket\n", MemNode->Size);
#endif
        assert(pc->FreeListBucket[Bucket] == NULL || ((unsigned long)pc->FreeListBucket[Bucket] >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)pc->FreeListBucket[Bucket] - &pc->HeapMemory[0] < HEAP_SIZE));
        *(struct AllocNode **)MemNode = pc->FreeListBucket[Bucket];
        pc->FreeListBucket[Bucket] = (struct AllocNode *)MemNode;
    }
    else
    { 
        /* put it in the big memory freelist */
#ifdef DEBUG_HEAP
        printf("freeing %lx:%d to freelist\n", (unsigned long)Mem, MemNode->Size);
#endif
        assert(pc->FreeListBig == NULL || ((unsigned long)pc->FreeListBig >= (unsigned long)&(pc->HeapMemory)[0] && (unsigned char *)pc->FreeListBig - &(pc->HeapMemory)[0] < HEAP_SIZE));
        MemNode->NextFree = pc->FreeListBig;
        pc->FreeListBig = MemNode;
#ifdef DEBUG_HEAP
        ShowBigList(pc);
#endif
    }
#endif
}
#endif
