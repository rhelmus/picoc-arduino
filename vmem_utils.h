#ifndef VMEM_UTILS_H
#define VMEM_UTILS_H

#include <inttypes.h>

struct Picoc_Struct;
typedef struct Picoc_Struct Picoc;

#ifdef USE_VIRTMEM
#include "virtmem.h"

#ifdef ARDUINO_HOST
#include "sdfatlib_alloc.h"

typedef CSdfatlibVirtMemAlloc<> TVirtAlloc;
#define TVirtPtr TSdfatlibVirtPtr
#else
#include "stdio_alloc.h"

typedef CStdioVirtMemAlloc<> TVirtAlloc;
#define TVirtPtr TStdioVirtPtr
#endif

using namespace virtmem;

typedef TVirtPtr<struct ParseState>::type TParseStatePtr;
typedef TVirtPtr<uint8_t>::type TVarAllocRet;
TVarAllocRet VariableAllocVirt(Picoc *pc, TParseStatePtr Parser, int Size, int OnHeap);
#else
typedef struct ParseState *TParseStatePtr;
#endif

#ifdef USE_VIRTSTACK
typedef CVirtPtrBase TStackVoidPtr;
#else
typedef void *TStackVoidPtr;
#endif

void *HeapAllocMem(Picoc *pc, int Size);
void HeapFreeMem(Picoc *pc, void *Mem);
TStackVoidPtr HeapAllocStack(Picoc *pc, int Size);
int HeapPopStack(Picoc *pc, TStackVoidPtr Addr, int Size);
void *VariableAlloc(Picoc *pc, TParseStatePtr Parser, int Size, int OnHeap);

template <typename T> class CAllocProxy;

template <typename T> inline CAllocProxy<T> allocMem(bool st, size_t size=sizeof(T));
template <typename T> inline CAllocProxy<T> allocMemVariable(TParseStatePtr p, bool st, size_t size=sizeof(T));

extern Picoc *globalPicoc;

// Proxy class for memory allocation: automatically determines whether we want to allocate for a
// raw pointer or CPtrWrapper
template <typename T> class CAllocProxy
{
    size_t size;
    TParseStatePtr ps;
    bool stack, varalloc;

    CAllocProxy(size_t s, TParseStatePtr p, bool st, bool va) : size(s), ps(p), stack(st), varalloc(va) { }
    CAllocProxy(const CAllocProxy &);
    CAllocProxy &operator=(const CAllocProxy &);

    friend CAllocProxy<T> allocMem<>(bool st, size_t size);
    friend CAllocProxy<T> allocMemVariable<>(TParseStatePtr p, bool st, size_t size);

public:
#ifndef USE_VIRTMEM
    inline operator T*(void)
    {
        if (varalloc)
            return static_cast<T *>(VariableAlloc(globalPicoc, ps, size, !stack));
        if (stack)
            return static_cast<T *>(HeapAllocStack(globalPicoc, size));
        return static_cast<T *>(HeapAllocMem(globalPicoc, size));
    }
#else
    inline operator CVirtPtr<T, TVirtAlloc>(void)
    {
        if (varalloc)
            return static_cast<CVirtPtr<T, TVirtAlloc> >(VariableAllocVirt(globalPicoc, ps, size, !stack));
        if (stack)
        {
#ifdef USE_VIRTSTACK
            return (CVirtPtr<T, TVirtAlloc>)HeapAllocStack(globalPicoc, size);
#else
            return CVirtPtr<T, TVirtAlloc>::wrap((T *)HeapAllocStack(globalPicoc, size));
#endif
        }
        return CVirtPtr<T, TVirtAlloc>::alloc(size);
    }
#endif
    ~CAllocProxy(void) { }
};

template <typename T> inline CAllocProxy<T> allocMem(bool st, size_t size=sizeof(T))
{ return CAllocProxy<T>(size, NILL, st, false); }
template <typename T> inline CAllocProxy<T> allocMemVariable(TParseStatePtr p, bool st, size_t size=sizeof(T))
{ return CAllocProxy<T>(size, p, st, true); }

inline CVirtPtrBase::TPtrNum getNumPtr(const CVirtPtrBase &pwb) { return pwb.getRawNum(); }
inline intptr_t getNumPtr(const void *p) { return reinterpret_cast<intptr_t>(p); }
inline void setPtrFromNum(CVirtPtrBase &pwb, CVirtPtrBase::TPtrNum ip) { pwb.setRawNum(ip); }
inline void setPtrFromNum(void *&p, intptr_t ip) { p = reinterpret_cast<void *>(ip); }

inline void deallocMem(void *ptr) { HeapFreeMem(globalPicoc, ptr); }
template <typename T> inline void deallocMem(CVirtPtr<T, TVirtAlloc> &p) { p.free(p); } // UNDONE
inline int popStack(TStackVoidPtr ptr, int size) { return HeapPopStack(globalPicoc, ptr, size); }

#ifndef USE_VIRTSTACK
template <typename T> inline int popStack(CVirtPtr<T, TVirtAlloc> &p, int size) { return HeapPopStack(globalPicoc, p.unwrap(), size); }
#endif

#ifdef USE_VIRTMEM
template <typename T> CVirtPtr<T, TVirtAlloc> ptrWrap(T *p) { return CVirtPtr<T, TVirtAlloc>::wrap(p); }
template <typename T> T *ptrUnwrap(CVirtPtr<T, TVirtAlloc> p) { return p.unwrap(); }
inline void *ptrUnwrap(CVirtPtrBase p) { return p.unwrap(); }
#else
#define ptrWrap /* empty */
#define ptrUnwrap /* empty */
#endif

#endif // VMEM_UTILS_H
