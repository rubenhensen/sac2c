#ifndef __DEBUG_H__
#define __DEBUG_H__

/* This is mechnism allowing dbug.h inclusion only from this file.
   If you include it directly it should prodice a warning that it
   is obsolete.  */
#define __ALLOW_FRED_FISH__ 0xbeef
#include "dbug.h"

#ifndef D_PREFIX
#warning D_PREFIX is undefined. Using the cuurent file name.
#define D_PREFIX __FILE__
#endif

/* Using C99 feature __func__ which is automatically
   expanded to the name of current function.
   NOTE if __func__ is declared outside the function then it is
   expanded to empty string and gives a warning.  */
#define denter() DBUG_ENTER (__func__)

/* With this constructon we avoid having two versions of return.
   Now we can use DRETURN (val); and DRETURN ();  */
#define dreturn(x)                                                                       \
    do {                                                                                 \
        DBUG_LEAVE;                                                                      \
        return x;                                                                        \
    } while (0)

/* Here the second parameter of the DBUG_PRINT is an argument list
   passed to some function.  */
#define dprint(...) DBUG_PRINT (D_PREFIX, (__VA_ARGS__))

/* Print something and leave the function.
   NOTE that the first parameter can be empty in case we are
   dealing with void functions.  */
#define dprint_return(retval, ...)                                                       \
    do {                                                                                 \
        dprint (__VA_ARGS__);                                                            \
        dreturn (retval);                                                                \
    } while (0)

/* Assert with parameters.  */
#define dassert(cond, ...) DBUG_ASSERTF ((cond), (__VA_ARGS__))

/* Thesea are mainly for the compatibility.  */
#define dexecute(expr) DBUG_EXECUTE (D_PREFIX, expr;)

#define dpop() DBUG_POP ()

#define dpush(var) DBUG_PUSH (var)

#endif
