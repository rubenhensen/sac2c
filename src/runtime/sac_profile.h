/*
 *
 * $Log$
 * Revision 1.4  1998/08/07 18:08:45  sbs
 * changed PROFILE_BEGIN_WITH -> SAC_PF_BEGIN_WITH
 * and     PROFILE_END_WITH   -> SAC_PF_END_WITH
 *
 * Revision 1.3  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.2  1998/03/24 13:53:24  cg
 * The presentation of profiling information is now entirely implemented
 * using macros. This is necessary because neither the number of functions
 * nor the maximum number of applications of a single function should be
 * statically fixed. These numbers are inherently application dependent,
 * so access to the timer arrays cannot be specified as part of the
 * runtime library.
 *
 * Revision 1.1  1998/03/19 16:54:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_profile.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   Profiling operations may be selectively activated by the global switches
 *    PROFILE_WITH  for profiling with-loops
 *    PROFILE_FUN   for profiling function applications
 *    PROFILE_INL   for profiling even inlined functions
 *    PROFILE_LIB   for profiling even library functions
 *
 *   The global switch PROFILE indicates any profiling activations.
 *
 *****************************************************************************/

#ifndef SAC_PROFILE_H

#define SAC_PROFILE_H

/*
 *  General profiling macros and declarations
 */

#if SAC_DO_PROFILE

/*
 * Type definitions
 */

#include <sys/time.h>
/*
 * for struct timeval
 */

#include <sys/resource.h>
/*
 * for struct rusage
 */

typedef struct timeval SAC_PF_TIMER;

/*
 * External declarations of C library functions needed
 */

extern int getrusage (int who, struct rusage *rusage);

/*
 * External declarations of library functions defined in libsac
 */

extern void SAC_PF_PrintHeader (char *title);
extern void SAC_PF_PrintTime (char *title, char *space, SAC_PF_TIMER *time);
extern void SAC_PF_PrintTimePercentage (char *title, char *space, SAC_PF_TIMER *time1,
                                        SAC_PF_TIMER *time2);

/*
 * External declarations of global variables defined in libsac
 */

extern int SAC_PF_act_funno;
extern int SAC_PF_act_funapno;
extern int SAC_PF_with_level;
extern struct rusage SAC_PF_start_timer;
extern struct rusage SAC_PF_stop_timer;

/*
 *  Macro definitions
 */

#define SAC_PF_DEFINE()                                                                  \
    SAC_PF_TIMER SAC_PF_fw_fun_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                  \
    SAC_PF_TIMER SAC_PF_fw_with_genarray_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];        \
    SAC_PF_TIMER SAC_PF_fw_with_modarray_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];        \
    SAC_PF_TIMER SAC_PF_fw_with_fold_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];            \
    SAC_PF_TIMER SAC_PF_fun_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                     \
    SAC_PF_TIMER SAC_PF_with_genarray_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];           \
    SAC_PF_TIMER SAC_PF_with_modarray_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];           \
    SAC_PF_TIMER SAC_PF_with_fold_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];               \
                                                                                         \
    SAC_PF_TIMER *SAC_PF_act_timer = &SAC_PF_fun_timer[0][0];                            \
                                                                                         \
    char *SAC_PF_fun_name[SAC_SET_MAXFUN] = SAC_SET_FUN_NAMES;                           \
    int SAC_PF_maxfunap[SAC_SET_MAXFUN] = SAC_SET_FUN_APPS;                              \
    int SAC_PF_funapline[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP] = SAC_SET_FUN_AP_LINES;

#define SAC_PF_SETUP()                                                                   \
    {                                                                                    \
        int i, j;                                                                        \
                                                                                         \
        SAC_PF_INIT_CLOCK ();                                                            \
        for (i = 0; i < SAC_SET_MAXFUN; i++) {                                           \
            for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                   \
                SAC_PF_INIT_TIMER (SAC_PF_fun_timer[i][j]);                              \
                SAC_PF_INIT_TIMER (SAC_PF_with_modarray_timer[i][j]);                    \
                SAC_PF_INIT_TIMER (SAC_PF_with_genarray_timer[i][j]);                    \
                SAC_PF_INIT_TIMER (SAC_PF_with_fold_timer[i][j]);                        \
                SAC_PF_INIT_TIMER (SAC_PF_fw_fun_timer[i][j]);                           \
                SAC_PF_INIT_TIMER (SAC_PF_fw_with_modarray_timer[i][j]);                 \
                SAC_PF_INIT_TIMER (SAC_PF_fw_with_genarray_timer[i][j]);                 \
                SAC_PF_INIT_TIMER (SAC_PF_fw_with_fold_timer[i][j]);                     \
            }                                                                            \
        }                                                                                \
        SAC_PF_START_CLOCK ();                                                           \
    }

#define SAC_PF_PRINT()                                                                   \
    {                                                                                    \
        SAC_PF_TIMER *tmp;                                                               \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER (*SAC_PF_act_timer);                                         \
        SAC_PF_PRINT_OVERALL (tmp, SAC_PF_DISPLAY_WITH, SAC_SET_MAXFUN, SAC_PF_maxfunap, \
                              SAC_PF_fun_timer, SAC_PF_with_modarray_timer,              \
                              SAC_PF_with_genarray_timer, SAC_PF_with_fold_timer,        \
                              SAC_PF_fw_fun_timer, SAC_PF_fw_with_modarray_timer,        \
                              SAC_PF_fw_with_genarray_timer, SAC_PF_fw_with_fold_timer); \
                                                                                         \
        SAC_PF_PRINT_FUNS (tmp, SAC_PF_DISPLAY_FUN, SAC_PF_DISPLAY_WITH, SAC_SET_MAXFUN, \
                           SAC_PF_maxfunap, SAC_PF_fun_timer,                            \
                           SAC_PF_with_modarray_timer, SAC_PF_with_genarray_timer,       \
                           SAC_PF_with_fold_timer, SAC_PF_fw_fun_timer,                  \
                           SAC_PF_fw_with_modarray_timer, SAC_PF_fw_with_genarray_timer, \
                           SAC_PF_fw_with_fold_timer, SAC_PF_fun_name);                  \
    }

#define SAC_PF_PRINT_OVERALL(tmp, display, maxfun, maxfunap, fun_timer,                  \
                             with_modarray_timer, with_genarray_timer, with_fold_timer,  \
                             fw_fun_timer, fw_with_modarray_timer,                       \
                             fw_with_genarray_timer, fw_with_fold_timer)                 \
    {                                                                                    \
        int i, j;                                                                        \
        SAC_PF_TIMER with_total, non_with_total;                                         \
        SAC_PF_TIMER total;                                                              \
                                                                                         \
        SAC_PF_INIT_TIMER (with_total);                                                  \
        SAC_PF_INIT_TIMER (non_with_total);                                              \
        SAC_PF_INIT_TIMER (total);                                                       \
                                                                                         \
        for (i = 0; i < maxfun; i++) {                                                   \
            for (j = 0; j < maxfunap[i]; j++) {                                          \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_modarray_timer[i][j]);   \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_genarray_timer[i][j]);   \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_fold_timer[i][j]);       \
                SAC_PF_ADD_TIMERS (with_total, with_total, fw_fun_timer[i][j]);          \
                SAC_PF_ADD_TIMERS (with_total, with_total,                               \
                                   fw_with_modarray_timer[i][j]);                        \
                SAC_PF_ADD_TIMERS (with_total, with_total,                               \
                                   fw_with_genarray_timer[i][j]);                        \
                SAC_PF_ADD_TIMERS (with_total, with_total, fw_with_fold_timer[i][j]);    \
                SAC_PF_ADD_TIMERS (non_with_total, non_with_total, fun_timer[i][j]);     \
            }                                                                            \
        }                                                                                \
        SAC_PF_ADD_TIMERS (total, with_total, non_with_total);                           \
                                                                                         \
        SAC_PF_PrintHeader ("Overall Profile");                                          \
        SAC_PF_PrintTime ("time used", SAC_PF_TIMER_SPACE, &total);                      \
                                                                                         \
        if (display) {                                                                   \
            SAC_PF_PrintTimePercentage ("   with-loop", "", &with_total, &total);        \
            SAC_PF_PrintTimePercentage ("   non-with-loop", "", &non_with_total,         \
                                        &total);                                         \
        }                                                                                \
                                                                                         \
        tmp = &total;                                                                    \
    }

#define SAC_PF_PRINT_FUNS(total_time, display_fun, display_with, maxfun, maxfunap,       \
                          fun_timer, with_modarray_timer, with_genarray_timer,           \
                          with_fold_timer, fw_fun_timer, fw_with_modarray_timer,         \
                          fw_with_genarray_timer, fw_with_fold_timer, fun_name)          \
    {                                                                                    \
        int i, j;                                                                        \
        SAC_PF_TIMER with_total, parent_with_total, non_with_total, total;               \
                                                                                         \
        SAC_PF_PrintHeader ("Function Profiles");                                        \
                                                                                         \
        for (i = 0; i < (display_fun ? maxfun : 1); i++) {                               \
            SAC_PF_INIT_TIMER (with_total);                                              \
            SAC_PF_INIT_TIMER (parent_with_total);                                       \
            SAC_PF_INIT_TIMER (non_with_total);                                          \
            SAC_PF_INIT_TIMER (total);                                                   \
                                                                                         \
            for (j = 0; j < maxfunap[i]; j++) {                                          \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_modarray_timer[i][j]);   \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_genarray_timer[i][j]);   \
                SAC_PF_ADD_TIMERS (with_total, with_total, with_fold_timer[i][j]);       \
                SAC_PF_ADD_TIMERS (with_total, with_total,                               \
                                   fw_with_modarray_timer[i][j]);                        \
                SAC_PF_ADD_TIMERS (with_total, with_total,                               \
                                   fw_with_genarray_timer[i][j]);                        \
                SAC_PF_ADD_TIMERS (with_total, with_total, fw_with_fold_timer[i][j]);    \
                SAC_PF_ADD_TIMERS (parent_with_total, parent_with_total,                 \
                                   fw_fun_timer[i][j]);                                  \
                SAC_PF_ADD_TIMERS (non_with_total, non_with_total, fun_timer[i][j]);     \
            }                                                                            \
                                                                                         \
            SAC_PF_ADD_TIMERS (total, with_total, non_with_total);                       \
            SAC_PF_ADD_TIMERS (total, total, parent_with_total);                         \
                                                                                         \
            if ((total.tv_sec != 0) || (total.tv_usec != 0)) {                           \
                SAC_PF_PrintHeader (fun_name[i]);                                        \
                SAC_PF_PrintTimePercentage ("time used", SAC_PF_TIMER_SPACE, &total,     \
                                            total_time);                                 \
                                                                                         \
                if (display_with) {                                                      \
                    SAC_PF_PrintTimePercentage ("   with-loop(own)", "", &with_total,    \
                                                &total);                                 \
                                                                                         \
                    SAC_PF_PrintTimePercentage ("   with-loop(parent)", "",              \
                                                &parent_with_total, &total);             \
                                                                                         \
                    SAC_PF_PrintTimePercentage ("   non-with-loop", "", &non_with_total, \
                                                &total);                                 \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define SAC_PF_INIT_CLOCK()

#define SAC_PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0

#define SAC_PF_START_CLOCK() getrusage (RUSAGE_SELF, &SAC_PF_start_timer)

#define SAC_PF_STOP_CLOCK() getrusage (RUSAGE_SELF, &SAC_PF_stop_timer)

#define SAC_PF_ADD_TO_TIMER(timer)                                                       \
    {                                                                                    \
        if (((timer).tv_usec += SAC_PF_stop_timer.ru_utime.tv_usec                       \
                                - SAC_PF_start_timer.ru_utime.tv_usec)                   \
            < 0) {                                                                       \
            (timer).tv_usec += 1000000;                                                  \
            (timer).tv_sec += SAC_PF_stop_timer.ru_utime.tv_sec                          \
                              - SAC_PF_start_timer.ru_utime.tv_sec - 1;                  \
        } else {                                                                         \
            (timer).tv_sec                                                               \
              += SAC_PF_stop_timer.ru_utime.tv_sec - SAC_PF_start_timer.ru_utime.tv_sec; \
        }                                                                                \
    }

#define SAC_PF_ADD_TIMERS(timer, timer1, timer2)                                         \
    {                                                                                    \
        timer##.tv_sec = timer1##.tv_sec + timer2##.tv_sec;                              \
        timer##.tv_usec = timer1##.tv_usec + timer2##.tv_usec;                           \
    }

#define SAC_PF_TIMER_SPACE "              "

#else /* SAC_DO_PROFILE */

#define SAC_PF_DEFINE()
#define SAC_PF_SETUP()
#define SAC_PF_PRINT()

#endif /* SAC_DO_PROFILE */

/*
 *  Macros for profiling with-loops
 */

#if (SAC_DO_PROFILE_WITH && SAC_DO_PROFILE)

#define SAC_PF_BEGIN_WITH(str)                                                           \
    {                                                                                    \
        SAC_PF_TIMER *SAC_PF_mem_act;                                                    \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_mem_act = SAC_PF_act_timer;                                               \
        SAC_PF_ADD_TO_TIMER (*SAC_PF_act_timer);                                         \
        SAC_PF_act_timer                                                                 \
          = (SAC_PF_with_level == 0                                                      \
               ? &SAC_PF_with_##str##_timer[SAC_PF_act_funno][SAC_PF_act_funapno]        \
               : &SAC_PF_fw_with_##str##_timer[SAC_PF_act_funno][SAC_PF_act_funapno]);   \
        SAC_PF_with_level++;                                                             \
        SAC_PF_START_CLOCK ();

#define SAC_PF_END_WITH(str)                                                             \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_with_level--;                                                                 \
    SAC_PF_ADD_TO_TIMER (*SAC_PF_act_timer);                                             \
    SAC_PF_act_timer = SAC_PF_mem_act;                                                   \
    SAC_PF_START_CLOCK ();                                                               \
    }

#define SAC_PF_DISPLAY_WITH 1

#else /* SAC_DO_PROFILE_WITH */

#define SAC_PF_BEGIN_WITH(str)
#define SAC_PF_END_WITH(str)
#define SAC_PF_DISPLAY_WITH 0

#endif /* SAC_DO_PROFILE_WITH */

/*
 *  Macros for profiling function applications
 */

#if (SAC_DO_PROFILE_FUN && SAC_DO_PROFILE)

#define PROFILE_BEGIN_UDF(funno, funapno)                                                \
    {                                                                                    \
        SAC_PF_TIMER *SAC_PF_mem_act;                                                    \
        int SAC_PF_mem_funno;                                                            \
        int SAC_PF_mem_funapno;                                                          \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_mem_act = SAC_PF_act_timer;                                               \
        SAC_PF_mem_funno = SAC_PF_act_funno;                                             \
        SAC_PF_mem_funapno = SAC_PF_act_funapno;                                         \
        SAC_PF_ADD_TO_TIMER (*SAC_PF_act_timer);                                         \
        SAC_PF_act_funno = funno;                                                        \
        SAC_PF_act_funapno = funapno;                                                    \
        SAC_PF_act_timer                                                                 \
          = (SAC_PF_with_level == 0 ? &SAC_PF_fun_timer[funno][funapno]                  \
                                    : &SAC_PF_fw_fun_timer[funno][funapno]);             \
        SAC_PF_START_CLOCK ();

#define PROFILE_END_UDF(funno, funapno)                                                  \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_ADD_TO_TIMER (*SAC_PF_act_timer);                                             \
    SAC_PF_act_timer = SAC_PF_mem_act;                                                   \
    SAC_PF_act_funno = SAC_PF_mem_funno;                                                 \
    SAC_PF_act_funapno = SAC_PF_mem_funapno;                                             \
    SAC_PF_START_CLOCK ();                                                               \
    }

#define SAC_PF_DISPLAY_FUN 1

#else /* SAC_DO_PROFILE_FUN */

#define PROFILE_BEGIN_UDF(funno, funapno)
#define PROFILE_END_UDF(funno, funapno)
#define SAC_PF_DISPLAY_FUN 0

#endif /* SAC_DO_PROFILE_FUN */

/*
 *  Macros for profiling even inlined function applications
 */

#if (SAC_DO_PROFILE_INL && SAC_DO_PROFILE)

#define PROFILE_INLINE(x) x

#else /* SAC_DO_PROFILE_INL */

#define PROFILE_INLINE(x)

#endif /* SAC_DO_PROFILE_INL */

/*
 *  Macros for profiling even library function applications
 */

#if (SAC_DO_PROFILE_LIB && SAC_DO_PROFILE)

#define PROFILE_LIBRARY(x) x

#else /* SAC_DO_PROFILE_LIB */

#define PROFILE_LIBRARY(x)

#endif /* SAC_DO_PROFILE_LIB */

#endif /* SAC_PROFILE_H */
