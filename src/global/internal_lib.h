/*
 *
 * $Log$
 * Revision 3.17  2001/07/13 13:23:41  cg
 * DBUG tag MEM_LEAK_CHECK renamed to MEM_LEAK
 *
 * Revision 3.16  2001/06/20 11:34:19  ben
 * StrTok implemented
 *
 * Revision 3.15  2001/05/18 11:51:01  dkr
 * macro FREE_VECT added
 *
 * Revision 3.14  2001/05/18 10:38:58  cg
 * Old memory management macros MALLOC and FREE removed.
 *
 * Revision 3.13  2001/05/17 13:29:29  cg
 * Moved de-allocation function Free() from free.c to internal_lib.c
 *
 * Revision 3.12  2001/05/17 08:36:02  sbs
 * PHASE_DONE_EPILOG added.
 *
 * Revision 3.11  2001/04/24 13:19:49  dkr
 * FALSE, TRUE moved to types.h
 *
 * Revision 3.10  2001/04/24 09:59:30  dkr
 * macro STR_OR_UNKNOWN added
 *
 * Revision 3.9  2001/04/24 09:34:42  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 * STR_OR_NULL moved from my_debug.h to internal_lib.h
 *
 * Revision 3.8  2001/04/10 09:36:46  dkr
 * macro F_PTR added
 *
 * Revision 3.7  2001/03/28 14:53:26  dkr
 * CHECK_NULL moved from tree_compound.h to internal_lib.h
 *
 * Revision 3.6  2001/03/28 09:12:29  dkr
 * bug in MALLOC_INIT_VECT fixed
 *
 * Revision 3.5  2001/03/27 21:54:24  dkr
 * comment added
 *
 * Revision 3.4  2001/03/27 21:39:09  dkr
 * macros ..._VECT added
 *
 * Revision 3.3  2001/03/27 11:29:43  dkr
 * OptCmp() removed
 *
 * Revision 3.2  2000/12/15 10:43:20  dkr
 * macros for bit fields added
 *
 * Revision 3.1  2000/11/20 17:59:31  sacbase
 * new release made
 *
 * Revision 2.15  2000/04/27 13:50:19  cg
 * Bug fixed in DBUG_OFF versions of CHECK_DBUG_START and CHECK_DBUG_STOP.
 *
 * Revision 2.14  2000/03/17 20:43:26  dkr
 * fixed a bug in CHECK_DBUG_STOP
 *
 * Revision 2.13  2000/03/17 11:12:51  dkr
 * definition of macro PHASE_EPILOG modified:
 * ABORT_ON_ERROR embedded, lac<->fun rearranged
 *
 * Revision 2.12  2000/03/16 16:38:10  dkr
 * fixed a bug in PHASE_EPILOG
 *
 * Revision 2.11  2000/03/16 14:30:53  dkr
 * macros PHASE_PROLOG and PHASE_EPILOG added
 *
 * Revision 2.10  2000/02/11 16:26:43  dkr
 * function StringConcat added
 *
 * Revision 2.9  2000/01/25 13:39:26  dkr
 * all the constvec stuff moved to tree_compound.h
 *
 * Revision 2.8  1999/07/20 11:48:29  cg
 * Definition (!) of global variable malloc_align_step removed;
 * malloc_align_step is now defined in globals.c.
 *
 * Revision 2.7  1999/07/20 07:53:42  cg
 * Definition (!) of global variable malloc_align_step removed;
 * malloc_align_step is now defined in globals.c.
 *
 * Revision 2.6  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various
 * compilation stages.
 *
 * Revision 2.5  1999/05/12 08:41:11  sbs
 * CopyIntVector and friends eliminated ; instead,
 * CopyConstVec, AllocConstVec, and ModConstVec have been added.
 *
 * Revision 2.4  1999/04/19 09:56:42  jhs
 * TRUE and FALSE added
 *
 * Revision 2.3  1999/03/15 13:53:25  bs
 * CopyIntArray renamed into CopyIntVector, CopyFloatVector and
 * CopyDoubleVector added.
 *
 * Revision 2.2  1999/02/24 20:23:04  bs
 * New function added: CopyIntArray
 *
 * Revision 2.1  1999/02/23 12:39:24  sacbase
 * new release made
 *
 * [...]
 *
 */

#ifndef _internal_lib_h
#define _internal_lib_h

#include "types.h"

/*********************************
 * function prototypes
 *********************************/

extern void *Malloc (int size);
extern void *Free (void *address);

extern char *StringCopy (char *source);
extern char *StringConcat (char *first, char *second);
extern char *StrTok (char *first, char *sep);

extern int lcm (int x, int y);
extern char *itoa (long number);

extern void SystemCall (char *format, ...);
extern int SystemCall2 (char *format, ...);
extern int SystemTest (char *format, ...);

extern char *TmpVar ();
extern char *TmpVarName (char *postfix);

#ifdef SHOW_MALLOC
extern void ComputeMallocAlignStep (void);
#endif

#ifndef DBUG_OFF
extern void DbugMemoryLeakCheck ();
#endif

/*********************************
 * macro definitions
 *********************************/

/* format string for pointers */
#ifdef SAC_FOR_SOLARIS_SPARC
#define F_PTR "0x%p"
#else
#define F_PTR "%p"
#endif

/* handling of strings */
#define STR_OR_NULL(str, null_str) (((str) != NULL) ? (str) : (null_str))
#define STR_OR_EMPTY(str) STR_OR_NULL (str, "")
#define STR_OR_UNKNOWN(str) STR_OR_NULL (str, "?")

/* swapping two pointers */
#define SWAP(ptr1, ptr2)                                                                 \
    {                                                                                    \
        void *tmp;                                                                       \
        tmp = (void *)ptr1;                                                              \
        ptr1 = (void *)ptr2;                                                             \
        ptr2 = (void *)tmp;                                                              \
    }

/* min, max */
#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

/*
 * macros for handling bit fields
 */

/* without assignment */
#define SET_BIT(bf, bit) (bf | bit)
#define UNSET_BIT(bf, bit) (bf & ~bit)

/* with assignment (is a l-value) */
#define L_SET_BIT(bf, bit) bf = SET_BIT (bf, bit)
#define L_UNSET_BIT(bf, bit) bf = UNSET_BIT (bf, bit)

#define TEST_BIT(bf, bit) ((bf & bit) != 0)

/*
 * macros for handling vectors
 */

#define FREE_VECT(vect)                                                                  \
    if (vect != NULL) {                                                                  \
        Free (vect);                                                                     \
    }

#define MALLOC_VECT(vect, dims, type)                                                    \
    if (vect == NULL) {                                                                  \
        (vect) = (type *)Malloc ((dims) * sizeof (type));                                \
    }

/* caution: 'val' should occur in the macro implementation only once! */
#define MALLOC_INIT_VECT(vect, dims, type, val)                                          \
    MALLOC_VECT (vect, dims, type);                                                      \
    INIT_VECT (vect, dims, type, val)

/* caution: 'val' should occur in the macro implementation only once! */
#define INIT_VECT(vect, dims, type, val)                                                 \
    {                                                                                    \
        int d;                                                                           \
        for (d = 0; d < (dims); d++) {                                                   \
            (vect)[d] = val;                                                             \
        }                                                                                \
    }

#define DUP_VECT(new_vect, old_vect, dims, type)                                         \
    {                                                                                    \
        int d;                                                                           \
        if ((old_vect) != NULL) {                                                        \
            MALLOC_VECT (new_vect, dims, type);                                          \
            for (d = 0; d < (dims); d++) {                                               \
                (new_vect)[d] = (old_vect)[d];                                           \
            }                                                                            \
        }                                                                                \
    }

#define PRINT_VECT(handle, vect, dims, format)                                           \
    {                                                                                    \
        int d;                                                                           \
        if ((vect) != NULL) {                                                            \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < (dims); d++) {                                               \
                fprintf (handle, format, (vect)[d]);                                     \
                fprintf (handle, " ");                                                   \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

/*
 * macros defining the prolog and epilog code for each phase
 */

#define PHASE_PROLOG                                                                     \
    CHECK_DBUG_START;                                                                    \
    /* empty */

#define PHASE_DONE_EPILOG                                                                \
    ABORT_ON_ERROR;                                                                      \
    DBUG_EXECUTE ("MEM_LEAK", DbugMemoryLeakCheck ();)

#define PHASE_EPILOG                                                                     \
    if (do_fun2lac[compiler_phase]) {                                                    \
        syntax_tree = Fun2Lac (syntax_tree);                                             \
        ABORT_ON_ERROR;                                                                  \
    }                                                                                    \
    if (do_lac2fun[compiler_phase + 1]) {                                                \
        syntax_tree = Lac2Fun (syntax_tree);                                             \
        ABORT_ON_ERROR;                                                                  \
    }                                                                                    \
    CHECK_DBUG_STOP;

/*
 * macros for steering the DBUG tool
 */

#ifndef DBUG_OFF
#define CHECK_DBUG_START                                                                 \
    {                                                                                    \
        if ((my_dbug) && (!my_dbug_active) && (compiler_phase >= my_dbug_from)           \
            && (compiler_phase <= my_dbug_to)) {                                         \
            DBUG_PUSH (my_dbug_str);                                                     \
            my_dbug_active = 1;                                                          \
        }                                                                                \
    }
#define CHECK_DBUG_STOP                                                                  \
    {                                                                                    \
        if ((my_dbug) && (my_dbug_active) && (compiler_phase >= my_dbug_to)) {           \
            DBUG_POP ();                                                                 \
            my_dbug_active = 0;                                                          \
        }                                                                                \
    }
#else /* DBUG_OFF */
#define CHECK_DBUG_START
#define CHECK_DBUG_STOP
#endif /* DBUG_OFF */

#endif /* _internal_lib_h */
