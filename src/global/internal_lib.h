/*
 *
 * $Log$
 * Revision 2.6  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various compilation stages.
 *
 * Revision 2.5  1999/05/12 08:41:11  sbs
 * CopyIntVector and friends eliminated ; instead,
 * CopyConstVec, AllocConstVec, and ModConstVec have been added.
 * /.
 *
 * Revision 2.4  1999/04/19 09:56:42  jhs
 * TRUE and FALSE added
 *
 * Revision 2.3  1999/03/15 13:53:25  bs
 * CopyIntArray renamed into CopyIntVector, CopyFloatVector and CopyDoubleVector added.
 *
 * Revision 2.2  1999/02/24 20:23:04  bs
 * New function added: CopyIntArray
 *
 * Revision 2.1  1999/02/23 12:39:24  sacbase
 * new release made
 *
 * Revision 1.16  1998/06/19 12:51:53  srs
 * compute_malloc_align_step() => ComputeMallocAlignStep()
 *
 * Revision 1.15  1998/03/17 11:54:43  dkr
 * added fun lcm()
 *
 * Revision 1.14  1998/03/13 13:13:47  dkr
 * removed a bug in macros CHECK_DBUG_START, CHECK_DBUG_STOP
 *
 * Revision 1.13  1998/03/02 13:57:21  cg
 * added function OptCmp() to compare two strings regardless of lower
 * or upper case letters (used for scanning optimization command line options.
 *
 * Revision 1.12  1998/02/24 16:10:43  srs
 * new function TmpVarName()
 *
 * Revision 1.11  1997/12/06 17:16:01  srs
 * new global var malloc_align_step
 * new function compute_malloc_align_step()
 *
 * Revision 1.10  1997/10/28 12:30:18  srs
 * inserted macro MALLOC
 *
 * Revision 1.9  1997/08/07 15:24:33  dkr
 * added DBUG_OFF at CHECK_DBUG_START, CHECK_DBUG_STOP
 *
 * Revision 1.8  1997/08/07 11:12:33  dkr
 * added macros CHECK_DBUG_START, CHECK_DBUG_STOP for new compiler option -_DBUG (main.c)
 *
 * Revision 1.7  1996/09/11 06:13:14  cg
 * Function SystemCall2 added that executes a system call and returns the
 * exit code rather than terminating with an error message upon failure.
 *
 * Revision 1.6  1996/01/16  16:44:42  cg
 * added function TmpVar for generation of variable names
 *
 * Revision 1.5  1996/01/05  12:26:54  cg
 * added functions SystemTest and SystemCall
 *
 * Revision 1.4  1995/07/24  09:01:48  asi
 * added function itoa
 *
 * Revision 1.3  1995/07/19  18:41:23  asi
 * added macros MIN and MAX
 *
 * Revision 1.2  1995/07/14  17:04:31  asi
 * added macro SWAP, it swaps two pointers
 *
 * Revision 1.1  1995/03/28  12:01:50  hw
 * Initial revision
 *
 *
 */

#ifndef _internal_lib_h

#define _internal_lib_h

#include "types.h"

extern void *Malloc (int size);
extern char *StringCopy (char *source);

extern void *CopyConstVec (simpletype vectype, int veclen, void *const_vec);
extern void *AllocConstVec (simpletype vectype, int veclen);
extern void *ModConstVec (simpletype vectype, void *const_vec, int idx, node *const_node);
extern node *AnnotateIdWithConstVec (node *expr, node *id);

extern int lcm (int x, int y);
extern char *itoa (long number);
extern void SystemCall (char *format, ...);
extern int SystemCall2 (char *format, ...);
extern int SystemTest (char *format, ...);
extern char *TmpVar ();
extern char *TmpVarName (char *postfix);
extern int OptCmp (char *first, char *second);

#ifdef SHOW_MALLOC
extern void ComputeMallocAlignStep (void);
int malloc_align_step;
#endif

#define SWAP(ptr1, ptr2)                                                                 \
    {                                                                                    \
        void *tmp;                                                                       \
        tmp = (void *)ptr1;                                                              \
        ptr1 = (void *)ptr2;                                                             \
        ptr2 = (void *)tmp;                                                              \
    }

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

#define TRUE 1
#define FALSE 0

#define MALLOC(size) Malloc (size)

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
        if ((my_dbug) && (my_dbug_active) && (compiler_phase <= my_dbug_to)) {           \
            DBUG_POP ();                                                                 \
            my_dbug_active = 0;                                                          \
        }                                                                                \
    }
#else /* DBUG_OFF */
#define CHECK_DBUG_START
#define CHECK_DBUG_STOP
#endif /* DBUG_OFF */

#endif /* _internal_lib_h */
