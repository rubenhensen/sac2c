/*
 *
 * $Log$
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

#if PROFILE

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

typedef struct timeval __PF_TIMER;

/*
 * External declarations of C library functions needed
 */

extern int getrusage (int who, struct rusage *rusage);

/*
 * External declarations of library functions defined in libsac
 */

extern void _SAC_PrintHeader (char *title);
extern void _SAC_PrintTime (char *title, char *space, __PF_TIMER *time);
extern void _SAC_PrintTimePercentage (char *title, char *space, __PF_TIMER *time1,
                                      __PF_TIMER *time2);

/*
 *  Macro definitions
 */

#define __PROFILE_SETUP()                                                                \
    __PF_INIT_CLOCK ();                                                                  \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 0; i < __PROFILE_MAXFUN; i++) {                                         \
            for (j = 0; j < __PF_maxfunap[i]; j++) {                                     \
                __PF_INIT_TIMER (__PF_fun_timer[i][j]);                                  \
                __PF_INIT_TIMER (__PF_with_modarray_timer[i][j]);                        \
                __PF_INIT_TIMER (__PF_with_genarray_timer[i][j]);                        \
                __PF_INIT_TIMER (__PF_with_fold_timer[i][j]);                            \
                __PF_INIT_TIMER (__PF_fw_fun_timer[i][j]);                               \
                __PF_INIT_TIMER (__PF_fw_with_modarray_timer[i][j]);                     \
                __PF_INIT_TIMER (__PF_fw_with_genarray_timer[i][j]);                     \
                __PF_INIT_TIMER (__PF_fw_with_fold_timer[i][j]);                         \
            }                                                                            \
        }                                                                                \
    }                                                                                    \
    __PF_START_CLOCK ()

#define __PROFILE_PRINT()                                                                \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    {                                                                                    \
        __PF_TIMER *tmp;                                                                 \
                                                                                         \
        __PROFILE_PRINT_OVERALL (tmp, __PF_DISPLAY_WITH, __PROFILE_MAXFUN,               \
                                 __PF_maxfunap, __PF_fun_timer,                          \
                                 __PF_with_modarray_timer, __PF_with_genarray_timer,     \
                                 __PF_with_fold_timer, __PF_fw_fun_timer,                \
                                 __PF_fw_with_modarray_timer,                            \
                                 __PF_fw_with_genarray_timer, __PF_fw_with_fold_timer);  \
                                                                                         \
        __PROFILE_PRINT_FUNS (tmp, __PF_DISPLAY_FUN, __PF_DISPLAY_WITH,                  \
                              __PROFILE_MAXFUN, __PF_maxfunap, __PF_fun_timer,           \
                              __PF_with_modarray_timer, __PF_with_genarray_timer,        \
                              __PF_with_fold_timer, __PF_fw_fun_timer,                   \
                              __PF_fw_with_modarray_timer, __PF_fw_with_genarray_timer,  \
                              __PF_fw_with_fold_timer, __PF_fun_name);                   \
    }

#define __PROFILE_PRINT_OVERALL(tmp, display, maxfun, maxfunap, fun_timer,               \
                                with_modarray_timer, with_genarray_timer,                \
                                with_fold_timer, fw_fun_timer, fw_with_modarray_timer,   \
                                fw_with_genarray_timer, fw_with_fold_timer)              \
    {                                                                                    \
        int i, j;                                                                        \
        __PF_TIMER with_total, non_with_total;                                           \
        __PF_TIMER total;                                                                \
                                                                                         \
        __PF_INIT_TIMER (with_total);                                                    \
        __PF_INIT_TIMER (non_with_total);                                                \
        __PF_INIT_TIMER (total);                                                         \
                                                                                         \
        for (i = 0; i < maxfun; i++) {                                                   \
            for (j = 0; j < maxfunap[i]; j++) {                                          \
                __PF_ADD_TIMERS (with_total, with_total, with_modarray_timer[i][j]);     \
                __PF_ADD_TIMERS (with_total, with_total, with_genarray_timer[i][j]);     \
                __PF_ADD_TIMERS (with_total, with_total, with_fold_timer[i][j]);         \
                __PF_ADD_TIMERS (with_total, with_total, fw_fun_timer[i][j]);            \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_modarray_timer[i][j]);  \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_genarray_timer[i][j]);  \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_fold_timer[i][j]);      \
                __PF_ADD_TIMERS (non_with_total, non_with_total, fun_timer[i][j]);       \
            }                                                                            \
        }                                                                                \
        __PF_ADD_TIMERS (total, with_total, non_with_total);                             \
                                                                                         \
        _SAC_PrintHeader ("Overall Profile");                                            \
        _SAC_PrintTime ("time used", __PF_TIMER_SPACE, &total);                          \
        if (display) {                                                                   \
            _SAC_PrintTimePercentage ("   with-loop", "", &with_total, &total);          \
            _SAC_PrintTimePercentage ("   non-with-loop", "", &non_with_total, &total);  \
        }                                                                                \
                                                                                         \
        tmp = &total;                                                                    \
    }

#define __PROFILE_PRINT_FUNS(total_time, display_fun, display_with, maxfun, maxfunap,    \
                             fun_timer, with_modarray_timer, with_genarray_timer,        \
                             with_fold_timer, fw_fun_timer, fw_with_modarray_timer,      \
                             fw_with_genarray_timer, fw_with_fold_timer, fun_name)       \
    {                                                                                    \
        int i, j;                                                                        \
        __PF_TIMER with_total, parent_with_total, non_with_total, total;                 \
                                                                                         \
        _SAC_PrintHeader ("Function Profiles");                                          \
                                                                                         \
        for (i = 0; i < (display_fun ? maxfun : 1); i++) {                               \
            __PF_INIT_TIMER (with_total);                                                \
            __PF_INIT_TIMER (parent_with_total);                                         \
            __PF_INIT_TIMER (non_with_total);                                            \
            __PF_INIT_TIMER (total);                                                     \
                                                                                         \
            for (j = 0; j < maxfunap[i]; j++) {                                          \
                __PF_ADD_TIMERS (with_total, with_total, with_modarray_timer[i][j]);     \
                __PF_ADD_TIMERS (with_total, with_total, with_genarray_timer[i][j]);     \
                __PF_ADD_TIMERS (with_total, with_total, with_fold_timer[i][j]);         \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_modarray_timer[i][j]);  \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_genarray_timer[i][j]);  \
                __PF_ADD_TIMERS (with_total, with_total, fw_with_fold_timer[i][j]);      \
                __PF_ADD_TIMERS (parent_with_total, parent_with_total,                   \
                                 fw_fun_timer[i][j]);                                    \
                __PF_ADD_TIMERS (non_with_total, non_with_total, fun_timer[i][j]);       \
            }                                                                            \
            __PF_ADD_TIMERS (total, with_total, non_with_total);                         \
            __PF_ADD_TIMERS (total, total, parent_with_total);                           \
                                                                                         \
            if ((total.tv_sec != 0) || (total.tv_usec != 0)) {                           \
                _SAC_PrintHeader (fun_name[i]);                                          \
                _SAC_PrintTimePercentage ("time used", __PF_TIMER_SPACE, &total,         \
                                          total_time);                                   \
                                                                                         \
                if (display_with) {                                                      \
                    _SAC_PrintTimePercentage ("   with-loop(own)", "", &with_total,      \
                                              &total);                                   \
                                                                                         \
                    _SAC_PrintTimePercentage ("   with-loop(parent)", "",                \
                                              &parent_with_total, &total);               \
                                                                                         \
                    _SAC_PrintTimePercentage ("   non-with-loop", "", &non_with_total,   \
                                              &total);                                   \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define __PF_INIT_CLOCK()

#define __PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0

#define __PF_START_CLOCK() getrusage (RUSAGE_SELF, &__PF_start_timer)

#define __PF_STOP_CLOCK() getrusage (RUSAGE_SELF, &__PF_stop_timer)

#define __PF_ADD_TO_TIMER(timer)                                                         \
    if (((timer).tv_usec                                                                 \
         += __PF_stop_timer.ru_utime.tv_usec - __PF_start_timer.ru_utime.tv_usec)        \
        < 0) {                                                                           \
        (timer).tv_usec += 1000000;                                                      \
        (timer).tv_sec                                                                   \
          += __PF_stop_timer.ru_utime.tv_sec - __PF_start_timer.ru_utime.tv_sec - 1;     \
    } else {                                                                             \
        (timer).tv_sec                                                                   \
          += __PF_stop_timer.ru_utime.tv_sec - __PF_start_timer.ru_utime.tv_sec;         \
    }

#define __PF_ADD_TIMERS(timer, timer1, timer2)                                           \
    timer##.tv_sec = timer1##.tv_sec + timer2##.tv_sec;                                  \
    timer##.tv_usec = timer1##.tv_usec + timer2##.tv_usec

#define __PF_TIMER_SPACE "              "

#else /* PROFILE */

#define __PROFILE_SETUP()
#define __PROFILE_PRINT()

#endif /* PROFILE */

/*
 *  Macros for profiling with-loops
 */

#if (PROFILE_WITH && PROFILE)

#define PROFILE_BEGIN_WITH(str)                                                          \
    __PF_TIMER *__PF_mem_act;                                                            \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_mem_act = __PF_act_timer;                                                       \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer                                                                       \
      = (__PF_with_level == 0                                                            \
           ? &__PF_with_##str##_timer[__PF_act_funno][__PF_act_funapno]                  \
           : &__PF_fw_with_##str##_timer[__PF_act_funno][__PF_act_funapno]);             \
    __PF_with_level++;                                                                   \
    __PF_START_CLOCK ()

#define PROFILE_END_WITH(str)                                                            \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_with_level--;                                                                   \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer = __PF_mem_act;                                                       \
    __PF_START_CLOCK ()

#define __PF_DISPLAY_WITH 1

#else /* PROFILE_WITH */

#define PROFILE_BEGIN_WITH(str)
#define PROFILE_END_WITH(str)
#define __PF_DISPLAY_WITH 0

#endif /* PROFILE_WITH */

/*
 *  Macros for profiling function applications
 */

#if (PROFILE_FUN && PROFILE)

#define PROFILE_BEGIN_UDF(funno, funapno)                                                \
    {                                                                                    \
        __PF_TIMER *__PF_mem_act;                                                        \
        int __PF_mem_funno;                                                              \
        int __PF_mem_funapno;                                                            \
        __PF_STOP_CLOCK ();                                                              \
        __PF_mem_act = __PF_act_timer;                                                   \
        __PF_mem_funno = __PF_act_funno;                                                 \
        __PF_mem_funapno = __PF_act_funapno;                                             \
        __PF_ADD_TO_TIMER (*__PF_act_timer);                                             \
        __PF_act_funno = funno;                                                          \
        __PF_act_funapno = funapno;                                                      \
        __PF_act_timer = (__PF_with_level == 0 ? &__PF_fun_timer[funno][funapno]         \
                                               : &__PF_fw_fun_timer[funno][funapno]);    \
        __PF_START_CLOCK ()

#define PROFILE_END_UDF(funno, funapno)                                                  \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    __PF_act_timer = __PF_mem_act;                                                       \
    __PF_act_funno = __PF_mem_funno;                                                     \
    __PF_act_funapno = __PF_mem_funapno;                                                 \
    __PF_START_CLOCK ();                                                                 \
    }

#define __PF_DISPLAY_FUN 1

#else /* PROFILE_FUN */

#define PROFILE_BEGIN_UDF(funno, funapno)
#define PROFILE_END_UDF(funno, funapno)
#define __PF_DISPLAY_FUN 0

#endif /* PROFILE_FUN */

/*
 *  Macros for profiling even inlined function applications
 */

#if (PROFILE_INL && PROFILE)

#define PROFILE_INLINE(x) x

#else /* PROFILE_INL */

#define PROFILE_INLINE(x)

#endif /* PROFILE_INL */

/*
 *  Macros for profiling even library function applications
 */

#if (PROFILE_LIB && PROFILE)

#define PROFILE_LIBRARY(x) x

#else /* PROFILE_LIB */

#define PROFILE_LIBRARY(x)

#endif /* PROFILE_LIB */

#endif /* SAC_PROFILE_H */
