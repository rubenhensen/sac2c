#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>

#ifndef DBUG_PREFIX
#warning DBUG_PREFIX is undefined. Using the cuurent file name.
#define DBUG_PREFIX __FILE__
#endif

#ifndef DBUG_OFF
extern int _db_on_;               /* TRUE if debug currently enabled */
extern int _db_dummy_;            /* dummy for fooling macro preprocessor */
extern FILE *_db_fp_;             /* Current debug output stream */
extern char *_db_process_;        /* Name of current process */
extern int _db_keyword_ (char *); /* Accept/reject keyword */
extern void _db_push_ (char *);   /* Push state, set up new state */
extern void _db_pop_ (void);      /* Pop previous debug state */
extern void _db_enter_ (const char *, char *, int, const char **, const char **,
                        int *); /* New user function entered */
extern void _db_return_ (int, const char **, const char **,
                         int *);                        /* User function return */
extern void _db_pargs_ (int, char *);                   /* Remember args for line */
extern void _db_doprnt_ (char *, ...);                  /* Print debug output */
extern void _db_doprnt_assert_1_ (char *, int, char *); /* Print debug output */
extern void _db_doprnt_assert_2_ (char *, ...);         /* Print debug output */
extern void _db_setjmp_ (void);                         /* Save debugger environment */
extern void _db_longjmp_ (void);                        /* Restore debugger environment */
extern void (*exit_func) (int); /* Custom exit function. Default is
                                   system `exit'  */
#endif

#ifdef DBUG_OFF
#define DBUG_ENTER()
#define DBUG_RETURN(x) return x
#define DBUG_PRINT(...) (void)0
#define DBUG_PRINT_TAG(tag, ...) (void)0
#define DBUG_EXECUTE(expr) (void)0
#define DBUG_EXECUTE_TAG(tag, expr) (void)0
#define DBUG_ASSERT(cond, ...) (void)0
#define DBUG_POP()
#define DBUG_PUSH(var)
#else
/* Using C99 feature __func__ which is automatically
   expanded to the name of current function.
   NOTE if __func__ is declared outside the function then it is
   expanded to empty string and gives a warning.  */
#define DBUG_ENTER()                                                                     \
    const char *_db_func_, *_db_file_;                                                   \
    int _db_level_;                                                                      \
    _db_enter_ (__func__, __FILE__, __LINE__, &_db_func_, &_db_file_, &_db_level_)

/* With this constructon we avoid having two versions of return.
   Now we can use DBUG_RETURN (val); and DBUG_RETURN ();  */
#define DBUG_RETURN(x)                                                                   \
    do {                                                                                 \
        _db_return_ (__LINE__, &_db_func_, &_db_file_, &_db_level_);                     \
        return x;                                                                        \
    } while (0)

/* Expression-like macro, could be used inside other expressions.
   For example: DBUG_RETURN (DBUG_PRINT ("Returning zero"), 0).  */
#define DBUG_PRINT(...)                                                                  \
    ((void)(_db_on_ ? (_db_pargs_ (__LINE__, DBUG_PREFIX), _db_doprnt_ (__VA_ARGS__), 0) \
                    : 0))

#define DBUG_PRINT_TAG(tag, ...)                                                         \
    ((void)(_db_on_ ? (_db_pargs_ (__LINE__, tag), _db_doprnt_ (__VA_ARGS__), 0) : 0))

#define DBUG_EXECUTE(...)                                                                \
    do {                                                                                 \
        if ((_db_on_ && _db_keyword_ (DBUG_PREFIX))) {                                   \
            __VA_ARGS__;                                                                 \
        }                                                                                \
    } while (0)

#define DBUG_EXECUTE_TAG(tag, ...)                                                       \
    do {                                                                                 \
        if ((_db_on_ && _db_keyword_ (tag))) {                                           \
            __VA_ARGS__;                                                                 \
        }                                                                                \
    } while (0)

#define DBUG_ASSERT(cond, ...)                                                           \
    ((void)(!(cond) ? (fprintf (_db_fp_,                                                 \
                                "%s:%i Assertion \"%s"                                   \
                                "\" failed!\n",                                          \
                                __FILE__, __LINE__, #cond),                              \
                       fprintf (_db_fp_, __VA_ARGS__), fprintf (_db_fp_, "\n"),          \
                       exit_func (EXIT_FAILURE), 0)                                      \
                    : 0))

#define DBUG_POP() _db_pop_ ()

#define DBUG_PUSH(var) _db_push_ (var)
#endif

#endif
