#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Picoc_Struct;
typedef struct Picoc_Struct Picoc;
struct ParseState;

void *HeapAllocMem(Picoc *pc, int Size);
void HeapFreeMem(Picoc *pc, void *Mem);
void *HeapAllocStack(Picoc *pc, int Size);
int HeapPopStack(Picoc *pc, void *Addr, int Size);
void *VariableAlloc(Picoc *pc, struct ParseState *Parser, int Size, int OnHeap);

template <typename T> class CPtrWrapper;
template <typename T, bool VA> class CAllocProxy;

class CPtrWrapperBase
{
    struct SNull { };

    static Picoc *pc;

    // Safe bool idiom from boost::function
    struct SDummy { void nonNull(void) { } };
    typedef void (SDummy::*TSafeBool)(void);

    template<typename> friend class CPtrWrapper;
    template<typename, bool> friend class CAllocProxy;
    friend void deallocMem(void *ptr);
    friend inline int popStack(void *ptr, int size);
    template <typename T> friend inline int popStack(CPtrWrapper<T> &p, int);
    friend CPtrWrapperBase memcpy(CPtrWrapperBase &dest, const CPtrWrapperBase &src, size_t n);
    friend CPtrWrapperBase memcpy(CPtrWrapperBase &dest, const void *src, size_t n);
    friend void *memcpy(void *dest, const CPtrWrapperBase &src, size_t n);
    friend inline char *strncpy(CPtrWrapper<char> dest, const char *src, size_t n);
    friend inline char *strncpy(CPtrWrapper<char> dest, const CPtrWrapper<char> &src, size_t n);
    friend inline char *strncpy(char *dest, const CPtrWrapper<char> &src, size_t n);
    friend inline int strncmp(const CPtrWrapper<char> &s1, const char *s2, size_t n);
    friend inline int strncmp(const CPtrWrapper<char> &s1, const CPtrWrapper<char> &s2, size_t n);
    friend inline int strncmp(const char *s1, const CPtrWrapper<char> &s2, size_t n);
    friend inline int strcmp(const CPtrWrapper<char> &s1, const char *s2);
    friend inline int strcmp(const CPtrWrapper<char> &s1, const CPtrWrapper<char> &s2);
    friend inline int strcmp(const char *s1, const CPtrWrapper<char> &s2);
    friend inline size_t strlen(const CPtrWrapper<char> &s);
    template <typename T1, typename T2> friend inline CPtrWrapper<T1> pointerCast(const T2 &p);
    friend intptr_t getNumPtr(const CPtrWrapperBase &pwb);
    friend void setPtrFromNum(CPtrWrapperBase &pwb, intptr_t ip);
    template <typename M> friend inline CPtrWrapperBase getMembrPtr(void *, const CPtrWrapper<M> &m);
    template <typename M> friend inline CPtrWrapperBase getMembrPtr(const CPtrWrapperBase &c, const M *m);
    template <typename M> friend inline CPtrWrapperBase getMembrPtr(const CPtrWrapperBase &c, const CPtrWrapper<M> &m);

protected:
    void *ptr;

public:
    static void setPicoc(Picoc *p) { pc = p; }
    static CPtrWrapperBase wrap(void *p) { CPtrWrapperBase ret; ret.ptr = p; return ret; }
    static CPtrWrapperBase wrap(const void *p) { CPtrWrapperBase ret; ret.ptr = const_cast<void *>(p); return ret; }
    static void *getPtr(const CPtrWrapperBase &p) { return p.ptr; } // UNDONE

    // HACK: this allows constructing CPtrWrapper objects from CPtrWrapperBase variables, similar to
    // initializing non void pointers with a void pointer
    // Note that we could have used a copy constructor in CPtrWrapper instead, but this would make the latter
    // class non-POD
    template <typename T> operator CPtrWrapper<T>(void) const { CPtrWrapper<T> ret; ret.ptr = ptr; return ret; }

    // allow checking with NULL
    inline bool operator==(const SNull *) const { return ptr == 0; }
    friend inline bool operator==(const SNull *, const CPtrWrapperBase &pw) { return pw.ptr == 0; }
    inline bool operator!=(const SNull *) const { return ptr != 0; }
    inline operator TSafeBool (void) const { return ptr == 0 ? 0 : &SDummy::nonNull; }

    inline bool operator==(const CPtrWrapperBase &pb) const { return ptr == pb.ptr; }
    inline bool operator!=(const CPtrWrapperBase &pb) const { return ptr != pb.ptr; }
    inline bool operator<(const CPtrWrapperBase &pb) const { return ptr < pb.ptr; }
    inline bool operator<=(const CPtrWrapperBase &pb) const { return ptr <= pb.ptr; }
    inline bool operator>=(const CPtrWrapperBase &pb) const { return ptr >= pb.ptr; }
    inline bool operator>(const CPtrWrapperBase &pb) const { return ptr > pb.ptr; }

    template <typename T> friend inline T *unwrap(const CPtrWrapper<T> &p) { return static_cast<T *>(p.ptr); } // UNDONE
};

template <typename T> class CPtrWrapper : public CPtrWrapperBase
{
    typedef size_t TArraySize;

public:
    class CValueWrapper
    {
        T *ptr;

        CValueWrapper(T *p) : ptr(p) { }
        CValueWrapper(const CValueWrapper &);

        template <typename> friend class CPtrWrapper;

    public:
//        operator T(void) const { return *ptr; }
        template <typename T2> operator T2(void) const { return static_cast<T2>(*ptr); }
        CValueWrapper &operator=(const T &v) { *ptr = v; return *this; }
        CPtrWrapper<T> operator&(void) { CPtrWrapper<T> ret; ret.ptr = ptr; return ret; }

        // allow pointer to pointer access
        T operator->(void) { return *ptr; }
        const T operator->(void) const { return *ptr; }

        inline bool operator==(const T &v) const { return *ptr == v; }
        inline bool operator!=(const T &v) const { return *ptr != v; }
    };

    // C style malloc/free
    static CPtrWrapper<T> alloc(bool st, size_t size=sizeof(T)) { CPtrWrapper<T> ret; ret.ptr = (st) ? HeapAllocStack(pc, size) : HeapAllocMem(pc, size); return ret; }
    static CPtrWrapper<T> allocVariable(ParseState *ps, bool st, size_t size=sizeof(T)) { CPtrWrapper<T> ret; ret.ptr = VariableAlloc(pc, ps, size, !st); return ret; }
    static void free(CPtrWrapper<T> &p) { HeapFreeMem(pc, p.ptr); }

    // C++ style new/delete --> call constructors (by placement new) and destructors
    static CPtrWrapper<T> construct(size_t size=sizeof(T)) { CPtrWrapper<T> ret = alloc(false, size); new (ret.ptr) T; return ret; }
    static void destruct(CPtrWrapper<T> &p) { static_cast<T *>(p.ptr)->~T(); free(p); }

    // C++ style new []/delete [] --> calls constructors and destructors
    // the size of the array (necessary for destruction) is stored at the beginning of the block.
    // A pointer offset from this is returned.
    static CPtrWrapper<T> constructArray(TArraySize elements)
    {
        CPtrWrapper<T> ret = alloc(false, sizeof(T) * elements + sizeof(TArraySize));
        *(static_cast<TArraySize *>(ret.ptr)) = elements;
        ret.ptr = static_cast<TArraySize *>(ret.ptr) + 1;
        for (TArraySize s=0; s<elements; ++s)
            new (static_cast<T *>(ret.ptr) + s) T;
        return ret;
    }
    static void destructArray(CPtrWrapper<T> &p)
    {
        TArraySize *size = (static_cast<TArraySize *>(p.ptr) - 1);
        for (TArraySize s=0; s<*size; ++s)
            (static_cast<T *>(p.ptr) + s)->~T();
        HeapFreeMem(pc, size); // *size points at beginning of actual block
    }

//    CPtrWrapper<T>& operator=(const T *p) { ptr = p; return *this; }
    CValueWrapper operator*(void) { return CValueWrapper(static_cast<T *>(ptr)); }
    T *operator->(void) { return static_cast<T *>(ptr); }
    const T *operator->(void) const { return static_cast<T *>(ptr); }
    // allow double wrapped pointer
    CPtrWrapper<CPtrWrapper<T> > operator&(void) { CPtrWrapper<CPtrWrapper<T> > ret; ret.ptr = this; return ret; }

    CPtrWrapper<T> &operator+=(int n) { ptr = static_cast<T *>(ptr) + n; return *this; }
    CPtrWrapper<T> &operator++(void) { ptr = (void *)(static_cast<T *>(ptr) + 1); return *this; }
    CPtrWrapper<T> operator++(int) { CPtrWrapper<T> ret; ret.ptr = ptr; ptr = (void *)(static_cast<T *>(ptr) + 1); return ret; }
    CPtrWrapper<T> &operator--(void) { ptr = static_cast<T *>(ptr) - 1; return *this; }
    CPtrWrapper<T> operator--(int) { CPtrWrapper<T> ret; ret.ptr = ptr; ptr = (void *)(static_cast<T *>(ptr) - 1); return ret; }
    CPtrWrapper<T> operator+(int n) const { CPtrWrapper<T> ret; ret.ptr = static_cast<T *>(ptr) + n; return ret; }
    CPtrWrapper<T> operator-(int n) const { CPtrWrapper<T> ret; ret.ptr = static_cast<T *>(ptr) - n; return ret; }
    int operator-(const CPtrWrapper<T> &other) const { return static_cast<T *>(ptr) - static_cast<T *>(other.ptr); }

    inline bool operator!=(const CPtrWrapper<T> &p) const { return ptr != p.ptr; }

    const CValueWrapper operator[](int i) const { return CValueWrapper(static_cast<T *>(ptr) + i); }
    CValueWrapper operator[](int i) { return CValueWrapper(static_cast<T *>(ptr) + i); }
};

template <typename T> inline CAllocProxy<T, false> allocMem(bool st, size_t size=sizeof(T));
template <typename T> inline CAllocProxy<T, true> allocMemVariable(ParseState *p, bool st, size_t size=sizeof(T));

// Proxy class for memory allocation: automatically determines whether we want to allocate for a
// raw pointer or CPtrWrapper
template <typename T, bool VA=false> class CAllocProxy
{
    size_t size;
    ParseState *ps;
    bool stack;

    CAllocProxy(size_t s, ParseState *p, bool st) : size(s), ps(p), stack(st) { }
    CAllocProxy(const CAllocProxy &);

    friend CAllocProxy<T, false> allocMem<>(bool st, size_t size);
    friend CAllocProxy<T, true> allocMemVariable<>(ParseState *p, bool st, size_t size);

public:
    inline operator T*(void) { return static_cast<T *>((VA) ? VariableAlloc(CPtrWrapperBase::pc, ps, size, !stack) : (stack) ? HeapAllocStack(CPtrWrapperBase::pc, size) : HeapAllocMem(CPtrWrapperBase::pc, size)); }
    inline operator CPtrWrapper<T>(void) { return (VA) ? CPtrWrapper<T>::allocVariable(ps, stack, size) : CPtrWrapper<T>::alloc(stack, size); }
};

template <typename T> inline CAllocProxy<T, false> allocMem(bool st, size_t size=sizeof(T))
{ return CAllocProxy<T, false>(size, NULL, st); }
template <typename T> inline CAllocProxy<T, true> allocMemVariable(ParseState *p, bool st, size_t size=sizeof(T))
{ return CAllocProxy<T, true>(size, p, st); }

inline void deallocMem(void *ptr) { HeapFreeMem(CPtrWrapperBase::pc, ptr); }
template <typename T> inline void deallocMem(CPtrWrapper<T> &p) { CPtrWrapper<T>::free(p); }
inline int popStack(void *ptr, int size) { return HeapPopStack(CPtrWrapperBase::pc, ptr, size); }
template <typename T> inline int popStack(CPtrWrapper<T> &p, int size) { return HeapPopStack(CPtrWrapperBase::pc, p.ptr, size); }

// Generic NULL type
class CNILL
{
    static CPtrWrapperBase nillPtrWrapperBase;

public:
//    template <typename T> inline operator T(void) const { return T(0); }
    template <typename T> inline operator T*(void) const { return NULL; }
    template <typename T> inline operator CPtrWrapper<T>(void) const { return nillPtrWrapperBase; }
    inline operator CPtrWrapperBase(void) const { return nillPtrWrapperBase; }

} extern const NILL;

template <typename T> struct SVoidPtr
{
    typedef void* type;
};

template<typename T> struct SVoidPtr<CPtrWrapper<T> >
{
    typedef CPtrWrapperBase type;
};

template <> struct SVoidPtr<CPtrWrapperBase>
{
    typedef CPtrWrapperBase type;
};

inline CPtrWrapperBase memcpy(CPtrWrapperBase &dest, const CPtrWrapperBase &src, size_t n) { memcpy(dest.ptr, src.ptr, n); return dest; }
inline CPtrWrapperBase memcpy(CPtrWrapperBase &dest, const void *src, size_t n) { memcpy(dest.ptr, src, n); return dest; }
inline void *memcpy(void *dest, const CPtrWrapperBase &src, size_t n) { return memcpy(dest, src.ptr, n); }
inline char *strncpy(CPtrWrapper<char> dest, const char *src, size_t n) { return strncpy((char *)dest.ptr, src, n); }
inline char *strncpy(CPtrWrapper<char> dest, const CPtrWrapper<char> &src, size_t n) { return strncpy((char *)dest.ptr, (const char *)src.ptr, n); }
inline char *strncpy(char *dest, const CPtrWrapper<char> &src, size_t n) { return strncpy(dest, (const char *)src.ptr, n); }
inline int strncmp(const CPtrWrapper<char> &s1, const char *s2, size_t n) { return strncmp((const char *)s1.ptr, s2, n); }
inline int strncmp(const CPtrWrapper<char> &s1, const CPtrWrapper<char> &s2, size_t n) { return strncmp((const char *)s1.ptr, (const char *)s2.ptr, n); }
inline int strncmp(const char *s1, const CPtrWrapper<char> &s2, size_t n) { return strcmp(s1, (const char *)s2.ptr); }
inline int strcmp(const CPtrWrapper<char> &s1, const char *s2) { return strcmp((const char *)s1.ptr, s2); }
inline int strcmp(const CPtrWrapper<char> &s1, const CPtrWrapper<char> &s2) { return strcmp((const char *)s1.ptr, (const char *)s2.ptr); }
inline int strcmp(const char *s1, const CPtrWrapper<char> &s2) { return strcmp(s1, (const char *)s2.ptr); }
inline size_t strlen(const CPtrWrapper<char> &s) { return strlen((const char *)s.ptr); }

template <typename T1, typename T2> inline T1 *pointerCast(const T2 *p) { return static_cast<T2 *>(p); }
template <typename T1, typename T2> inline CPtrWrapper<T1> pointerCast(const T2 &p)
{ CPtrWrapper<T1> ret; ret.ptr = p.ptr; return ret; }

inline intptr_t getNumPtr(const CPtrWrapperBase &pwb) { return reinterpret_cast<intptr_t>(pwb.ptr); }
inline intptr_t getNumPtr(const void *p) { return reinterpret_cast<intptr_t>(p); }
inline void setPtrFromNum(CPtrWrapperBase &pwb, intptr_t ip) { pwb.ptr = reinterpret_cast<void *>(ip); }

inline intptr_t getMembrPtrDiff(const void *c, const void *m) { return (const char *)m - (const char *)c; }
template <typename M> inline M *getMembrPtr(void *, M *m) { return m; }
template <typename M> inline CPtrWrapperBase getMembrPtr(void *, const CPtrWrapper<M> &m) { return m; }
template <typename M> inline CPtrWrapperBase getMembrPtr(const CPtrWrapperBase &c, const M *m)
{ CPtrWrapperBase ret; ret.ptr = ((void *)((char *)(c.ptr) + getMembrPtrDiff(c.ptr, m))); return ret; }
template <typename M> inline CPtrWrapperBase getMembrPtr(const CPtrWrapperBase &c, const CPtrWrapper<M> &m)
{ CPtrWrapperBase ret; ret.ptr = ((void *)((char *)(c.ptr) + getMembrPtrDiff(c.ptr, m.ptr))); return ret; }


#if 0
class CPtrWrapperBase
{
    static Picoc *pc;
protected:
    void *ptr;

public:
    static void setPicoc(Picoc *p) { pc = p; }
    void alloc(int size) { ptr = HeapAllocMem(pc, size); }
    void free(void) { HeapFreeMem(pc, ptr); }
    operator const void*(void) { return ptr; }
    //CPtrWrapperBase &operator=(CPtrWrapperBase &other) { ptr = other.ptr; return *this; }

    void memcpy(const void *other, size_t s) { ::memcpy(ptr, other, s); }

    template<typename> friend class CPtrWrapper;
    template<typename> friend class CPtrWrapperLite;
};

class CPtrWrapperBaseNonPOD : public CPtrWrapperBase
{
protected:
    CPtrWrapperBaseNonPOD(void *p) { ptr = p; }
    CPtrWrapperBaseNonPOD(void) { ptr = 0; }
    CPtrWrapperBaseNonPOD(int size) { alloc(size); }
};

template <typename T> class CPtrWrapperLite;

template <typename T> class CPtrWrapper : public CPtrWrapperBaseNonPOD
{
public:
    explicit CPtrWrapper(T *p) : CPtrWrapperBase(p) { }
    CPtrWrapper(void) { }
    CPtrWrapper(int size) : CPtrWrapperBaseNonPOD(size) { }
    CPtrWrapper(CPtrWrapperLite<T> &pwl) : CPtrWrapperBaseNonPOD(pwl.ptr) { }

    const T &operator *(void) const { return *static_cast<T *>(ptr); }
    T *operator->(void) { return static_cast<T *>(ptr); }
    CPtrWrapper<T> &operator=(const CPtrWrapperBase &pwb) { ptr = pwb.ptr; return *this; }
//    CPtrWrapper<T> &operator=(const CPtrWrapperLite<T> &pwl) { ptr = pwl.ptr; return *this; }
    T &operator [](int i) { return *(static_cast<T *>(ptr) + i); }
};

// Lite version which can act as POD, useful for unions
template <typename T> class CPtrWrapperLite : public CPtrWrapperBase
{
public:
    const T &operator *(void) const { return *static_cast<T *>(ptr); }
    T *operator->(void) { return static_cast<T *>(ptr); }
    CPtrWrapperLite<T> &operator=(const CPtrWrapperBase &pwb) { ptr = pwb.ptr; return *this; }
    CPtrWrapperLite<T> operator+=(int n) { ptr = static_cast<T *>(ptr) + n; return *this; }
    CPtrWrapperLite<T> operator+(int n) { CPtrWrapperLite<T> ret; ret.ptr = static_cast<T *>(ptr) + n; return ret; }
    CPtrWrapperLite<T> operator-(int n) { CPtrWrapperLite<T> ret; ret.ptr = static_cast<T *>(ptr) - n; return ret; }
    CPtrWrapperLite<T> operator-(const CPtrWrapperLite<T> &other) { CPtrWrapperLite<T> ret; ret.ptr = static_cast<T *>(ptr) - static_cast<T *>(other.ptr); return ret; }
    const T &operator [](const int i) const { return *(ptr + i); }
    T &operator [](int i) { return *(ptr + i); }
};

#if 0
template <typename T> class CPtrWrapperLite
{
    void *ptr;

public:
    const T &operator *(void) const { return *static_cast<T *>(ptr); }
    T *operator->(void) { return static_cast<T *>(ptr); }
    CPtrWrapperLite<T> &operator=(const CPtrWrapperBase &pwb) { ptr = pwb.ptr; return *this; }
    operator const void*(void) { return ptr; }
    CPtrWrapperLite<T> &operator+=(int n) { ptr += n; return *this; }
    CPtrWrapperLite<T> &operator+(int n) { CPtrWrapperLite<T> ret; ret.ptr = ptr + n; return ret; }
    CPtrWrapperLite<T> &operator-(int n) { CPtrWrapperLite<T> ret; ret.ptr = ptr - n; return ret; }
    CPtrWrapperLite<T> &operator-(const CPtrWrapperLite<T> &other) { CPtrWrapperLite<T> ret; ret.ptr = ptr - other.ptr; return ret; }
    const T &operator [](const int i) const { return *(ptr + i); }
    T &operator [](int i) { return *(ptr + i); }
};
#endif

template <typename T1, typename T2> int pointerWrapperDiff(const T1 &p1, const T2 &p2) { return (int)(const void *)(p1 - p2); }

// decltype is nice
#if !defined __cplusplus || __cplusplus <= 199711L
#define decltype typeof
#endif

#endif


#endif // UTIL_HPP
