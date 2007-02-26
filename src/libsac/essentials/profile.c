/*
 *
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   profile.c
 *
 * prefix: SAC_
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

#define __PF_TIMER(timer) timer.tv_sec + timer.tv_usec / 1000000.0

#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    (timer1.tv_sec + timer1.tv_usec / 1000000.0) * 100                                   \
      / (timer2.tv_sec + timer2.tv_usec / 1000000.0)

/*
 * Global variables
 */

int SAC_PF_act_funno = 0;
int SAC_PF_act_funapno = 0;
int SAC_PF_with_level = 0;
struct rusage SAC_PF_start_timer;
struct rusage SAC_PF_stop_timer;

/******************************************************************************
 *
 * function:
 *   void SAC_PF_PrintHeader( char * title)
 *
 * description:
 *
 *   This function prints some header lines for presenting profiling
 *   information.
 *
 *
 ******************************************************************************/

void
SAC_PF_PrintHeader (char *title)
{
    fprintf (stderr, "\n****************************************"
                     "****************************************\n");
    fprintf (stderr, "*** %-72s ***\n", title);
    fprintf (stderr, "****************************************"
                     "****************************************\n");
}

/******************************************************************************
 *
 * function:
 *   void SAC_PF_PrintSubHeader( char * title, int lineno)
 *
 * description:
 *
 *   This function prints some header lines for presenting profiling
 *   information.
 *
 *
 ******************************************************************************/

void
SAC_PF_PrintSubHeader (char *title, int lineno)
{
    fprintf (stderr, "call to %s in line #%d:\n", title, lineno);
}

/******************************************************************************
 *
 * function:
 *   void SAC_PF_PrintTime( char * title, char * space, __PF_TIMER * time)
 *
 * description:
 *
 *   Function for priniting timing information in a formatted manner.
 *
 *
 *
 ******************************************************************************/

void
SAC_PF_PrintTime (char *title, char *space, __PF_TIMER *time)
{
    fprintf (stderr,
             "%-20s: %s "__PF_TIMER_FORMAT
             " sec\n",
             title, space, __PF_TIMER ((*time)));
}

/******************************************************************************
 *
 * function:
 *   void SAC_PF_PrintTimePercentage( char * title, char * space,
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
SAC_PF_PrintTimePercentage (char *title, char *space, __PF_TIMER *time1,
                            __PF_TIMER *time2)
{
    fprintf (stderr,
             "%-20s:%s  "__PF_TIMER_FORMAT
             " sec  %8.2f %%\n",
             title, space, __PF_TIMER ((*time1)),
             __PF_TIMER_PERCENTAGE ((*time1), (*time2)));
}
