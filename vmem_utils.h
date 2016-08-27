#ifndef VMEM_UTILS_H
#define VMEM_UTILS_H

#include <inttypes.h>
#include <stdlib.h>

struct Picoc_Struct;
typedef struct Picoc_Struct Picoc;

#ifdef USE_VIRTMEM
#include "virtmem.h"

using namespace virtmem;

#ifdef ARDUINO_HOST
#if 0 // UNDONE
#include "alloc/sd_alloc.h"

typedef SDVAlloc TVirtAlloc;
#elif 0
#include "alloc/spiram_alloc.h"

typedef SPIRAMVAlloc TVirtAlloc;
#elif 1
#include "alloc/serial_alloc.h"

typedef SerialVAlloc TVirtAlloc;
#else
#include "alloc/static_alloc.h"

typedef StaticVAllocP<1024*40> TVirtAlloc;
#endif

#else
#if 1
#include "alloc/static_alloc.h"
typedef StaticVAlloc TVirtAlloc;
#else
#include "alloc/stdio_alloc.h"
typedef StdioVAlloc TVirtAlloc;
#endif

#endif

typedef TVirtAlloc::TVPtr<struct ParseState>::type TParseStatePtr;
typedef TVirtAlloc::TVPtr<uint8_t>::type TVarAllocRet;
TVarAllocRet VariableAllocVirt(Picoc *pc, TParseStatePtr Parser, int Size, int OnHeap);
#else
typedef struct ParseState *TParseStatePtr;
#ifndef NILL
#define NILL NULL
#endif
#endif

#ifdef USE_VIRTSTACK
typedef BaseVPtr TStackVoidPtr;
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
    inline operator VPtr<T, TVirtAlloc>(void)
    {
        if (varalloc)
            return static_cast<VPtr<T, TVirtAlloc> >(VariableAllocVirt(globalPicoc, ps, size, !stack));
        if (stack)
        {
#ifdef USE_VIRTSTACK
            return (VPtr<T, TVirtAlloc>)HeapAllocStack(globalPicoc, size);
#else
            return VPtr<T, TVirtAlloc>::wrap((T *)HeapAllocStack(globalPicoc, size));
#endif
        }
        return TVirtAlloc::getInstance()->alloc<T>(size);
    }
#endif
    ~CAllocProxy(void) { }
};

template <typename T> inline CAllocProxy<T> allocMem(bool st, size_t size)
{ return CAllocProxy<T>(size, NILL, st, false); }
template <typename T> inline CAllocProxy<T> allocMemVariable(TParseStatePtr p, bool st, size_t size)
{ return CAllocProxy<T>(size, p, st, true); }

#ifdef USE_VIRTMEM
inline BaseVPtr::PtrNum getNumPtr(const BaseVPtr &pwb) { return pwb.getRawNum(); }
inline void setPtrFromNum(BaseVPtr &pwb, BaseVPtr::PtrNum ip) { pwb.setRawNum(ip); }
#endif
inline intptr_t getNumPtr(const void *p) { return reinterpret_cast<intptr_t>(p); }
template <typename T> inline void setPtrFromNum(T *&p, intptr_t ip) { p = reinterpret_cast<T *>(ip); }

inline void deallocMem(void *ptr) { HeapFreeMem(globalPicoc, ptr); }
#ifdef USE_VIRTMEM
template <typename T> inline void deallocMem(VPtr<T, TVirtAlloc> p) { TVirtAlloc::getInstance()->free(p); }
#endif
inline int popStack(TStackVoidPtr ptr, int size) { return HeapPopStack(globalPicoc, ptr, size); }

#if !defined(USE_VIRTSTACK) && defined(USE_VIRTMEM)
template <typename T> inline int popStack(VPtr<T, TVirtAlloc> &p, int size) { return HeapPopStack(globalPicoc, p.unwrap(), size); }
#endif

#ifdef USE_VIRTMEM
template <typename T> VPtr<T, TVirtAlloc> ptrWrap(T *p) { return VPtr<T, TVirtAlloc>::wrap(p); }
template <typename T> T *ptrUnwrap(VPtr<T, TVirtAlloc> p) { return p.unwrap(); }
inline void *ptrUnwrap(BaseVPtr p) { return p.unwrap(); }
#else
#define ptrWrap /* empty */
#define ptrUnwrap /* empty */
template <typename C, typename M> M *getMembrPtr(C *c, M C::*m) { return &(c->*m); }
template <typename C, typename M, typename NC, typename NM> NM *getMembrPtr(C *c, M C::*m, NM NC::*nm) { return &(c->*m.*nm); }
#endif

#endif // VMEM_UTILS_H
