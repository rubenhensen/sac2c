/*
 *
 * $Log$
 * Revision 1.2  1998/03/24 13:51:45  cg
 * First working revision
 *
 * Revision 1.1  1998/03/19 16:36:14  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_profile.c
 *
 * prefix: _SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *   It contains function and global variable definitions needed for
 *   profiling.
 *
 *****************************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

/*
 *  Internal type definitions
 */

typedef struct timeval __PF_TIMER;

/*
 *  Internal macro definitions
 */

#define __PF_TIMER_FORMAT "%8.2f"

#define __PF_TIMER(timer) timer##.tv_sec + timer##.tv_usec / 1000000.0

#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    (timer1##.tv_sec + timer1##.tv_usec / 1000000.0) * 100                               \
      / (timer2##.tv_sec + timer2##.tv_usec / 1000000.0)

/******************************************************************************
 *
 * function:
 *   void _SAC_PrintHeader( char * title)
 *
 * description:
 *
 *   This function prints some header lines for presenting profiling
 *   information.
 *
 *
 ******************************************************************************/

void
_SAC_PrintHeader (char *title)
{
    fprintf (stderr, "****************************************"
                     "****************************************\n");
    fprintf (stderr, "*** %-72s ***\n", title);
    fprintf (stderr, "****************************************"
                     "****************************************\n");
}

/******************************************************************************
 *
 * function:
 *   void _SAC_PrintTime( char * title, char * space, __PF_TIMER * time)
 *
 * description:
 *
 *   Function for priniting timing information in a formatted manner.
 *
 *
 *
 ******************************************************************************/

void
_SAC_PrintTime (char *title, char *space, __PF_TIMER *time)
{
    fprintf (stderr,
             "%-20s: %s "__PF_TIMER_FORMAT
             " sec\n",
             title, space, __PF_TIMER ((*time)));
}

/******************************************************************************
 *
 * function:
 *   void _SAC_PrintTimePercentage( char * title, char * space,
 *                             __PF_TIMER * time1, __PF_TIMER * time2)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
_SAC_PrintTimePercentage (char *title, char *space, __PF_TIMER *time1, __PF_TIMER *time2)
{
    fprintf (stderr,
             "%-20s:%s  "__PF_TIMER_FORMAT
             " sec  %8.2f %%\n",
             title, space, __PF_TIMER ((*time1)),
             __PF_TIMER_PERCENTAGE ((*time1), (*time2)));
}
