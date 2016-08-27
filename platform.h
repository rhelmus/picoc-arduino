/* all platform-specific includes and defines go in this file */
#ifndef PLATFORM_H
#define PLATFORM_H

#define USE_VIRTMEM
#define USE_VIRTSTACK
#define TRACE_MEMUSAGE
#define VIRTMEM_TRACE_STATS
#define VIRTMEM_WRAP_CPOINTERS

#ifdef ARDUINO
#define ARDUINO_HOST
#else
#define UNIX_HOST
#endif

#include "vmem_utils.h"

/* configurable options */
/* select your host type (or do it in the Makefile):
 * #define  UNIX_HOST
 * #define  FLYINGFOX_HOST
 * #define  SURVEYOR_HOST
 * #define  SRV1_UNIX_HOST
 * #define  UMON_HOST
 * #define  WIN32  (predefined on MSVC)
 */

#define LARGE_INT_POWER_OF_TEN 1000000000   /* the largest power of ten which fits in an int on this architecture */
#if defined(__hppa__) || defined(__sparc__)
#define ALIGN_TYPE double                   /* the default data type to use for alignment */
#else
#if 0 && defined(__x86_64__) || defined(_M_X64) // UNDONE
#define ALIGN_TYPE  __uint128_t
#else
#define ALIGN_TYPE void *                   /* the default data type to use for alignment */
#endif
#endif

#define GLOBAL_TABLE_SIZE 97                /* global variable table */
#define STRING_TABLE_SIZE 97                /* shared string table size */
#define STRING_LITERAL_TABLE_SIZE 97        /* string literal table size */
#define RESERVED_WORD_TABLE_SIZE 97         /* reserved word table size */
#define PARAMETER_MAX 16                    /* maximum number of parameters to a function */
#define LINEBUFFER_MAX 256                  /* maximum number of characters on a line */
#define LOCAL_TABLE_SIZE 11                 /* size of local variable table (can expand) */
#define STRUCT_TABLE_SIZE 11                /* size of struct/union member table (can expand) */

#define INTERACTIVE_PROMPT_START "starting picoc " PICOC_VERSION "\n"
#define INTERACTIVE_PROMPT_STATEMENT "picoc> "
#define INTERACTIVE_PROMPT_LINE "     > "

#ifdef USE_VIRTMEM
typedef TVirtAlloc::TVPtr<struct CleanupTokenNode>::type TCleanupNodePtr;
typedef TVirtAlloc::TVPtr<unsigned char>::type TLexBufPtr;
typedef BaseVPtr TLexVoidPtr;
typedef TVirtAlloc::TVPtr<char>::type TLexCharPtr;
typedef TVirtAlloc::TVPtr<unsigned char>::type TLexUnsignedCharPtr;
typedef TVirtAlloc::TVPtr<const char>::type TLexConstCharPtr;
typedef TVirtAlloc::TVPtr<TLexConstCharPtr>::type TLexConstCharPtrPtr;
typedef TVirtAlloc::TVPtr<struct Value>::type TValuePtr;
typedef TVirtAlloc::TVPtr<TValuePtr>::type TValuePtrPtr;
typedef TVirtAlloc::TVPtr<union AnyValue>::type TAnyValuePtr;
typedef TVirtAlloc::TVPtr<char>::type TRegStringPtr;
typedef TVirtAlloc::TVPtr<const char>::type TConstRegStringPtr;
typedef TVirtAlloc::TVPtr<TRegStringPtr>::type TRegStringPtrPtr;
typedef TVirtAlloc::TVPtr<struct ReservedWord>::type TReservedWordPtr;
typedef TVirtAlloc::TVPtr<char>::type TValueCharPtr;
typedef BaseVPtr TAnyValueVoidPtr;
typedef TVirtAlloc::TVPtr<int>::type TAnyValueIntPtr;
typedef TVirtAlloc::TVPtr<char>::type TAnyValueCharPtr;
typedef TVirtAlloc::TVPtr<unsigned char>::type TAnyValueUCharPtr;
typedef TVirtAlloc::TVPtr<char>::type TStdioCharPtr;
typedef TVirtAlloc::TVPtr<const char>::type TStdioConstCharPtr;
typedef TVirtAlloc::TVPtr<struct ValueType>::type TValueTypePtr;
typedef TVirtAlloc::TVPtr<TValueTypePtr>::type TValueTypePtrPtr;
typedef TVirtAlloc::TVPtr<struct TableEntry>::type TTableEntryPtr;
typedef TVirtAlloc::TVPtr<TTableEntryPtr>::type TTableEntryPtrPtr;
typedef TVirtAlloc::TVPtr<struct Table>::type TTablePtr;
typedef TVirtAlloc::TVPtr<char>::type TTableCharPtr;
typedef TVirtAlloc::TVPtr<struct IncludeLibrary>::type TIncludeLibraryPtr;
typedef TVirtAlloc::TVPtr<struct ParseState>::type TParseStatePtr;
typedef TVirtAlloc::TVPtr<struct ExpressionStack>::type TExpressionStackPtr;
typedef TVirtAlloc::TVPtr<TExpressionStackPtr>::type TExpressionStackPtrPtr;
typedef TVirtAlloc::TVPtr<struct TokenLine>::type TTokenLinePtr;
typedef TVirtAlloc::TVPtr<uint8_t>::type TVarAllocRet;
typedef TVirtAlloc::TVPtr<struct StackFrame>::type TStackFramePtr;

#define WRAP_REGSTRINGS
#define WRAP_ANYVALUE
#define MAX_INC_FILENAME 128
#else
typedef struct CleanupTokenNode *TCleanupNodePtr;
typedef unsigned char *TLexBufPtr;
typedef char *TLexCharPtr;
typedef const char *TLexConstCharPtr;
typedef const char **TLexConstCharPtrPtr;
typedef struct Value *TValuePtr;
typedef TValuePtr *TValuePtrPtr;
typedef union AnyValue *TAnyValuePtr;
typedef char *TRegStringPtr;
typedef const char *TConstRegStringPtr;
typedef TRegStringPtr *TRegStringPtrPtr;
typedef struct ReservedWord *TReservedWordPtr;
typedef char *TValueCharPtr;
typedef void *TAnyValueVoidPtr;
typedef int *TAnyValueIntPtr;
typedef char *TAnyValueCharPtr;
typedef unsigned char *TAnyValueUCharPtr;
typedef char *TStdioCharPtr;
typedef const char *TStdioConstCharPtr;
typedef struct ValueType *TValueTypePtr;
typedef TValueTypePtr *TValueTypePtrPtr;
typedef struct TableEntry *TTableEntryPtr;
typedef TTableEntryPtr *TTableEntryPtrPtr;
typedef struct Table *TTablePtr;
typedef char *TTableCharPtr;
typedef struct IncludeLibrary *TIncludeLibraryPtr;
typedef struct ParseState *TParseStatePtr;
typedef struct ExpressionStack *TExpressionStackPtr;
typedef TExpressionStackPtr *TExpressionStackPtrPtr;
typedef struct TokenLine *TTokenLinePtr;
typedef struct StackFrame *TStackFramePtr;
#endif

#if defined(USE_VIRTSTACK) && defined(USE_VIRTMEM)
typedef BaseVPtr TStackVoidPtr;
typedef TVirtAlloc::TVPtr<TStackVoidPtr>::type TStackVoidPtrPtr;
typedef TVirtAlloc::TVPtr<char>::type TStackCharPtr;
typedef TVirtAlloc::TVPtr<const char>::type TStackConstCharPtr;
typedef TVirtAlloc::TVPtr<TStackConstCharPtr>::type TStackConstCharPtrPtr;
typedef TVirtAlloc::TVPtr<unsigned char>::type TStackUnsignedCharPtr;
#else
typedef void *TStackVoidPtr;
typedef void **TStackVoidPtrPtr;
typedef char *TStackCharPtr;
typedef const char *TStackConstCharPtr;
typedef const char **TStackConstCharPtrPtr;
typedef unsigned char *TStackUnsignedCharPtr;
#endif


/* host platform includes */
#ifdef UNIX_HOST
//# define USE_MALLOC_STACK                   /* stack is allocated using malloc() */
//# define USE_MALLOC_HEAP                    /* heap is allocated using malloc() */
# define HEAP_SIZE (1024*32)
# define BUILTIN_MINI_STDLIB
# define debugline /*printf*/
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <assert.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <stdarg.h>
# include <setjmp.h>
# include <stdint.h>
# ifndef NO_FP
#  include <math.h>
#  define PICOC_MATH_LIBRARY
#  define USE_READLINE
#  undef BIG_ENDIAN
#  if defined(__powerpc__) || defined(__hppa__) || defined(__sparc__)
#   define BIG_ENDIAN
#  endif
# endif

extern jmp_buf ExitBuf;

#else
# ifdef WIN32
#  define USE_MALLOC_STACK                   /* stack is allocated using malloc() */
#  define USE_MALLOC_HEAP                    /* heap is allocated using malloc() */
#  include <stdio.h>
#  include <stdlib.h>
#  include <ctype.h>
#  include <string.h>
#  include <assert.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <stdarg.h>
#  include <setjmp.h>
#  include <math.h>
#  define PICOC_MATH_LIBRARY
#  undef BIG_ENDIAN

extern jmp_buf ExitBuf;

# else
#  ifdef FLYINGFOX_HOST
#   define HEAP_SIZE (16*1024)               /* space for the heap and the stack */
#   define NO_FILE_SUPPORT
#   include <stdlib.h>
#   include <ctype.h>
#   include <string.h>
#   include <sys/types.h>
#   include <stdarg.h>
#   include <setjmp.h>
#   include <math.h>
#   define assert(x)
#   define BUILTIN_MINI_STDLIB
#   undef BIG_ENDIAN

#  else
#   ifdef SURVEYOR_HOST
#    define HEAP_SIZE C_HEAPSIZE
#    define NO_FP
#    define NO_CTYPE
#    define NO_FILE_SUPPORT
#    define NO_MODULUS
#    include <cdefBF537.h>
#    include "../string.h"
#    include "../print.h"
#    include "../srv.h"
#    include "../setjmp.h"
#    include "../stdarg.h"
#    include "../colors.h"
#    include "../neural.h"
#    include "../gps.h"
#    include "../i2c.h"
#    include "../jpeg.h"
#    include "../malloc.h"
#    include "../xmodem.h"
#    define assert(x)
#    undef BIG_ENDIAN
#    define NO_CALLOC
#    define NO_REALLOC
#    define BROKEN_FLOAT_CASTS
#    define BUILTIN_MINI_STDLIB
#   else
#    ifdef UMON_HOST
#     define HEAP_SIZE (128*1024)               /* space for the heap and the stack */
#     define NO_FP
#     define BUILTIN_MINI_STDLIB
#     include <stdlib.h>
#     include <string.h>
#     include <ctype.h>
#     include <sys/types.h>
#     include <stdarg.h>
#     include <math.h>
#     include "monlib.h"
#     define assert(x)
#     define malloc mon_malloc
#     define calloc(a,b) mon_malloc(a*b)
#     define realloc mon_realloc
#     define free mon_free
#    endif
#   endif
#  endif

extern int ExitBuf[];

# endif
#endif

#ifdef ARDUINO_HOST
# define HEAP_SIZE (3*1024)               /* space for the heap and the stack */
//# define NO_FP
//# define NO_PRINTF
# define NO_FILE_SUPPORT
# define NO_DEBUGGER
# define NO_MALLOC
# define NO_CALLOC
# define NO_REALLOC
//# define NO_STRING_FUNCTIONS
# include <stdlib.h>
# include <ctype.h>
# include <stdint.h>
# include <string.h>
# include <stdarg.h>
# include <setjmp.h>
# include <math.h>
# define assert(x) /*do { if (!(x)) { Serial.print("assertion!: "); Serial.println(#x); } } while(false)*/
# define BUILTIN_MINI_STDLIB  /* UNDONE: use avr libc? */
# define debugline
# undef BIG_ENDIAN
# define ARDUINO_EXIT 2

#endif


#endif /* PLATFORM_H */
