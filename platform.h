/* all platform-specific includes and defines go in this file */
#ifndef PLATFORM_H
#define PLATFORM_H

#define USE_VIRTMEM
#define USE_VIRTSTACK
#define TRACE_MEMUSAGE

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
typedef TVirtPtr<struct CleanupTokenNode>::type TCleanupNodePtr;
typedef TVirtPtr<unsigned char>::type TLexBufPtr;
typedef CVirtPtrBase TLexVoidPtr;
typedef TVirtPtr<char>::type TLexCharPtr;
typedef TVirtPtr<unsigned char>::type TLexUnsignedCharPtr;
typedef TVirtPtr<const char>::type TLexConstCharPtr;
typedef TVirtPtr<TLexConstCharPtr>::type TLexConstCharPtrPtr;
typedef TVirtPtr<struct Value>::type TValuePtr;
typedef TVirtPtr<TValuePtr>::type TValuePtrPtr;
typedef TVirtPtr<union AnyValue>::type TAnyValuePtr;
typedef TVirtPtr<char>::type TRegStringPtr;
typedef TVirtPtr<const char>::type TConstRegStringPtr;
typedef TVirtPtr<TRegStringPtr>::type TRegStringPtrPtr;
typedef TVirtPtr<struct ReservedWord>::type TReservedWordPtr;
typedef TVirtPtr<char>::type TValueCharPtr;
typedef CVirtPtrBase TAnyValueVoidPtr;
typedef TVirtPtr<int>::type TAnyValueIntPtr;
typedef TVirtPtr<char>::type TAnyValueCharPtr;
typedef TVirtPtr<unsigned char>::type TAnyValueUCharPtr;
typedef TVirtPtr<char>::type TStdioCharPtr;
typedef TVirtPtr<const char>::type TStdioConstCharPtr;
typedef TVirtPtr<struct ValueType>::type TValueTypePtr;
typedef TVirtPtr<TValueTypePtr>::type TValueTypePtrPtr;
typedef TVirtPtr<struct TableEntry>::type TTableEntryPtr;
typedef TVirtPtr<TTableEntryPtr>::type TTableEntryPtrPtr;
typedef TVirtPtr<struct Table>::type TTablePtr;
typedef TVirtPtr<char>::type TTableCharPtr;
typedef TVirtPtr<struct IncludeLibrary>::type TIncludeLibraryPtr;
typedef TVirtPtr<struct ParseState>::type TParseStatePtr;
typedef TVirtPtr<struct ExpressionStack>::type TExpressionStackPtr;
typedef TVirtPtr<TExpressionStackPtr>::type TExpressionStackPtrPtr;
typedef TVirtPtr<struct TokenLine>::type TTokenLinePtr;
typedef TVirtPtr<uint8_t>::type TVarAllocRet;
typedef TVirtPtr<struct StackFrame>::type TStackFramePtr;

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
typedef CVirtPtrBase TStackVoidPtr;
typedef TVirtPtr<TStackVoidPtr>::type TStackVoidPtrPtr;
typedef TVirtPtr<char>::type TStackCharPtr;
typedef TVirtPtr<const char>::type TStackConstCharPtr;
typedef TVirtPtr<TStackConstCharPtr>::type TStackConstCharPtrPtr;
typedef TVirtPtr<unsigned char>::type TStackUnsignedCharPtr;
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
# define HEAP_SIZE (1024*1024)
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
# define HEAP_SIZE (32*1024)               /* space for the heap and the stack */
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
