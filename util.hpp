#ifndef UTIL_HPP
#define UTIL_HPP

struct Picoc_Struct;
typedef struct Picoc_Struct Picoc;

void *HeapAllocMem(Picoc *pc, int Size);
void HeapFreeMem(Picoc *pc, void *Mem);

class CPtrWrapperBase
{
protected:
    void *ptr;

    CPtrWrapperBase(void *p) : ptr(p) { }
    CPtrWrapperBase(void) : ptr(0) { }

public:
    static CPtrWrapperBase alloc(Picoc *pc, int size) { return CPtrWrapperBase(HeapAllocMem(pc, size)); }
    static void free(Picoc *pc, CPtrWrapperBase &pwb) { HeapFreeMem(pc, pwb.ptr); }
    operator const void*(void) { return ptr; }
    CPtrWrapperBase &operator=(CPtrWrapperBase &other) { ptr = other.ptr; return *this; }

    template<typename> friend class CPtrWrapper;
};

#if 0
template <typename C> class CPtrWrapper
{
    C *ptr;

public:
    explicit CPtrWrapper(C *p) : ptr(p) { }
    CPtrWrapper(void) : ptr(0) { }

    /*static CPtrWrapper<C> &make(Picoc *pc, size_t size=sizeof(C)) { return CPtrWrapper(reinterpret_cast<C *>(HeapAllocMem(pc, size))); }*/
    void alloc(Picoc *pc, size_t size=sizeof(C)) { ptr = reinterpret_cast<C *>(HeapAllocMem(pc, size)); }
    void free(Picoc *pc) { HeapFreeMem(pc, ptr); }

    const C &operator *(void) const { return *ptr; }
    /*operator int(void) { return reinterpret_cast<intptr_t>(ptr); }*/
    operator const void*(void) { return reinterpret_cast<void *>(ptr); }
    C *operator->(void) { return ptr; }
    CPtrWrapper<C> &operator=(CPtrWrapper<C> &other) { ptr = other.ptr; return *this; }
};
#endif

template <typename C> class CPtrWrapper : public CPtrWrapperBase
{
public:
    explicit CPtrWrapper(C *p) : CPtrWrapperBase(p) { }
    CPtrWrapper(void) { }

    const C &operator *(void) const { return *static_cast<C *>(ptr); }
    C *operator->(void) { return static_cast<C *>(ptr); }
    CPtrWrapper<C> &operator=(const CPtrWrapperBase &pwb) { ptr = pwb.ptr; return *this; }
};


#endif // UTIL_HPP
