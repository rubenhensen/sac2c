/*
 *
 * $Log$
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

#include <sys/time.h>
#include <sys/resource.h>

#include "sac_message.h"
#include "sac_profile.h"

#include "globals.h"

/*
 *  Internal type definitions
 */

typedef struct timeval __PF_TIMER;

/*
 *  Global variable definitions
 */

struct rusage _SAC_start_timer;
struct rusage _SAC_stop_timer;

/*
 *  Internal macro definitions
 */

#define __PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0

#define __PF_ADD_TIMERS(timer, timer1, timer2)                                           \
    timer##.tv_sec = timer1##.tv_sec + timer2##.tv_sec;                                  \
    timer##.tv_usec = timer1##.tv_usec + timer2##.tv_usec

#define __PF_TIMER_FORMAT "%8.2f"

#define __PF_TIMER_SPACE "              "

#define __PF_TIMER(timer) timer##.tv_sec + timer##.tv_usec / 1000000.0

#define __PF_TIMER_PERCENTAGE(timer1, timer2)                                            \
    (timer1##.tv_sec + timer1##.tv_usec / 1000000.0) * 100                               \
      / (timer2##.tv_sec + timer2##.tv_usec / 1000000.0)

/******************************************************************************
 *
 * function:
 *   void PrintTime( char * title, char * space, __PF_TIMER * time)
 *
 * description:
 *
 *   Function for priniting timing information in a formatted manner.
 *
 *
 *
 ******************************************************************************/

static void
PrintTime (char *title, char *space, __PF_TIMER *time)
{
    _SAC_Print ("%-20s: %s "__PF_TIMER_FORMAT
                " sec\n",
                title, space, __PF_TIMER ((*time)));
}

/******************************************************************************
 *
 * function:
 *   void PrintTimePercentage( char * title, char * space,
 *                             __PF_TIMER * time1, __PF_TIMER * time2)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

static void
PrintTimePercentage (char *title, char *space, __PF_TIMER *time1, __PF_TIMER *time2)
{
    _SAC_Print ("%-20s:%s  "__PF_TIMER_FORMAT
                " sec  %8.2f %%\n",
                title, space, __PF_TIMER ((*time1)),
                __PF_TIMER_PERCENTAGE ((*time1), (*time2)));
}

/******************************************************************************
 *
 * function:
 *   __PF_TIMER * _SAC_PrintProfileOverall(...)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

__PF_TIMER *
_SAC_PrintProfileOverall (int display_flag, int __PF_maxfun, int *__PF_maxfunap,
                          __PF_TIMER __PF_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_fw_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_fw_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_fw_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                          __PF_TIMER __PF_fw_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP])
{
    int i, j;
    __PF_TIMER with_total, non_with_total;
    static __PF_TIMER total;

    __PF_INIT_TIMER (with_total);
    __PF_INIT_TIMER (non_with_total);
    __PF_INIT_TIMER (total);
    for (i = 0; i < __PF_maxfun; i++) {
        for (j = 0; j < __PF_maxfunap[i]; j++) {
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_fun_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (non_with_total, non_with_total, __PF_fun_timer[i][j]);
        }
    }
    __PF_ADD_TIMERS (total, with_total, non_with_total);

    _SAC_PrintHeader ("Overall Profile");
    PrintTime ("time used", __PF_TIMER_SPACE, &total);
    if (display_flag != 0) {
        PrintTimePercentage ("   with-loop", "", &with_total, &total);
        PrintTimePercentage ("   non-with-loop", "", &non_with_total, &total);
    }
    return (&total);
}

/******************************************************************************
 *
 * function:
 *   void _SAC_PrintProfileFuns(...)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
_SAC_PrintProfileFuns (__PF_TIMER *total_time, int display_fun, int display_with,
                       int __PF_maxfun, int *__PF_maxfunap,
                       __PF_TIMER __PF_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_fw_fun_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_fw_with_modarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_fw_with_genarray_timer[PF_MAXFUN][PF_MAXFUNAP],
                       __PF_TIMER __PF_fw_with_fold_timer[PF_MAXFUN][PF_MAXFUNAP],
                       char *__PF_fun_name[PF_MAXFUN])
{
    int i, j;
    __PF_TIMER with_total, parent_with_total, non_with_total, total;

    _SAC_PrintHeader ("Function Profiles");
    if (display_fun == 0)
        __PF_maxfun = 1;
    for (i = 0; i < __PF_maxfun; i++) {
        __PF_INIT_TIMER (with_total);
        __PF_INIT_TIMER (parent_with_total);
        __PF_INIT_TIMER (non_with_total);
        __PF_INIT_TIMER (total);
        for (j = 0; j < __PF_maxfunap[i]; j++) {
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_modarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_genarray_timer[i][j]);
            __PF_ADD_TIMERS (with_total, with_total, __PF_fw_with_fold_timer[i][j]);
            __PF_ADD_TIMERS (parent_with_total, parent_with_total,
                             __PF_fw_fun_timer[i][j]);
            __PF_ADD_TIMERS (non_with_total, non_with_total, __PF_fun_timer[i][j]);
        }
        __PF_ADD_TIMERS (total, with_total, non_with_total);
        __PF_ADD_TIMERS (total, total, parent_with_total);
        if ((total.tv_sec != 0) || (total.tv_usec != 0)) {
            _SAC_PrintHeader (__PF_fun_name[i]);
            PrintTimePercentage ("time used", __PF_TIMER_SPACE, &total, total_time);
            if (display_with != 0) {
                PrintTimePercentage ("   with-loop(own)", "", &with_total, &total);
                PrintTimePercentage ("   with-loop(parent)", "", &parent_with_total,
                                     &total);
                PrintTimePercentage ("   non-with-loop", "", &non_with_total, &total);
            }
        }
    }
}
