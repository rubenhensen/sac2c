/*
 *
 * $Log$
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

extern void *Malloc (int size);
extern char *StringCopy (char *source);
extern char *itoa (long number);
extern void SystemCall (char *format, ...);
extern int SystemCall2 (char *format, ...);
extern int SystemTest (char *format, ...);
extern char *TmpVar ();

#define SWAP(ptr1, ptr2)                                                                 \
    {                                                                                    \
        void *tmp;                                                                       \
        tmp = (void *)ptr1;                                                              \
        ptr1 = (void *)ptr2;                                                             \
        ptr2 = (void *)tmp;                                                              \
    }

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

#ifndef DBUG_OFF
#define CHECK_DBUG_START                                                                 \
    {                                                                                    \
        if ((!dbug_active) && (compiler_phase >= dbug_from)                              \
            && (compiler_phase <= dbug_to)) {                                            \
            DBUG_PUSH (dbug_str);                                                        \
            dbug_active = 1;                                                             \
        }                                                                                \
    }

#define CHECK_DBUG_STOP                                                                  \
    {                                                                                    \
        if ((dbug_active) && (compiler_phase <= dbug_to))                                \
            DBUG_POP ();                                                                 \
    }
#else /* DBUG_OFF */
#define CHECK_DBUG_START
#define CHECK_DBUG_STOP
#endif /* DBUG_OFF */

#endif /* _internal_lib_h */
