#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include "fun-attrs.h"

#ifndef DBUG_PREFIX
#warning DBUG_PREFIX is undefined. Using the current file name.
#define DBUG_PREFIX __FILE__
#endif

/* Use getter/setter functions for custom exit function used
   by DBUG_ASSERT and DBUG_UNREACHABLE macros.  */
extern void set_debug_exit_function (void (*) (int));

/* Get the pointer on the custom exit function.  */
extern void (*get_debug_exit_function (void)) (int);

/* Wrap the custom exit function.  Used by DBUG_ASSERT macro.  */
static inline void
exit_func (int status)
{
    void (*func) (int) = get_debug_exit_function ();
    func (status);
}

/*! This macro handles the case when a code hits unreachable
    state.
    \param f    ``FILE *'' object where fprinf will pipe its
                output.  */
#define __sac_unreachable(f, ...)                                                        \
    do {                                                                                 \
        fprintf (f, "Internal compiler error\n");                                        \
        fprintf (f, "Program reached impossible state at %s:%i -- ", __FILE__,           \
                 __LINE__);                                                              \
        fprintf (f, __VA_ARGS__);                                                        \
        fprintf (f, "\n");                                                               \
        fprintf (f, "Please file a bug at: "                                             \
                    "https://gitlab.sac-home.org/sac-group/sac2c/-/issues\n");           \
        exit_func (EXIT_FAILURE);                                                        \
        /* Use abort to ensure non-return nature of the macro.  */                       \
        abort ();                                                                        \
    } while (0)

/*! This macro handles the case when a code fails to pass
    an assertion.
    \param f    ``FILE *'' object where fprinf will pipe its
                output.
    \param cond Expression to be asserted.  */
#define __sac_assert(f, cond, strcond, ...)                                              \
    ((void)(!(cond) ? (fprintf (f, "Internal compiler error\n"),                         \
                       fprintf (f,                                                       \
                                "Assertion \"%s\" failed at "                            \
                                "%s:%i -- ",                                             \
                                strcond, __FILE__, __LINE__),                            \
                       fprintf (f, __VA_ARGS__), fprintf (f, "\n"),                      \
                       fprintf (f, "Please file a bug at: "                              \
                                   "https://gitlab.sac-home.org/sac-group/sac2c/-/issues\n"), \
                       exit_func (EXIT_FAILURE), 0)                                      \
                    : 0))

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
                         int *);                            /* User function return */
extern void _db_pargs_ (int, char *);                       /* Remember args for line */
extern void _db_doprnt_ (char *, ...) PRINTF_FORMAT (1, 2); /* Print debug output */
extern void _db_doprnt_assert_1_ (char *, int, char *);     /* Print debug output */
extern void _db_doprnt_assert_2_ (char *, ...)
  PRINTF_FORMAT (1, 2);          /* Print debug output */
extern void _db_setjmp_ (void);  /* Save debugger environment */
extern void _db_longjmp_ (void); /* Restore debugger environment */

#endif

#ifdef DBUG_OFF
#define DBUG_ENTER()
#define DBUG_RETURN(x) return x
#define DBUG_PRINT(...) (void)0
#define DBUG_PRINT_TAG(tag, ...) (void)0
#define DBUG_EXECUTE(expr) (void)0
#define DBUG_EXECUTE_TAG(tag, expr) (void)0
#define DBUG_ASSERT(cond, ...) __sac_assert (stderr, cond, #cond, __VA_ARGS__)
#define DBUG_UNREACHABLE(...) __sac_unreachable (stderr, __VA_ARGS__)
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

#define DBUG_ASSERT(cond, ...) __sac_assert (_db_fp_, cond, #cond, __VA_ARGS__)

#define DBUG_UNREACHABLE(...) __sac_unreachable (_db_fp_, __VA_ARGS__)

#define DBUG_POP() _db_pop_ ()

#define DBUG_PUSH(var) _db_push_ (var)
#endif

#endif
