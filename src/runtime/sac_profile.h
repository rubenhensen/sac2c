/*
 *
 * $Log$
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

#include <sys/time.h>
/*
 * for struct timeval
 */

#include <sys/resource.h>
/*
 * for struct rusage
 */

extern int getrusage (int who, struct rusage *rusage);

typedef struct timeval __PF_TIMER;

extern struct rusage _SAC_start_timer; /* defined in profile.c */
extern struct rusage _SAC_stop_timer;  /* defined in profile.c */

extern __PF_TIMER *_SAC_PrintProfileOverall (
  int display_with, int __PF_maxfun, int *__PF_maxfunap, void *__PF_fun_timer,
  void *__PF_with_modarray_timer, void *__PF_with_genarray_timer,
  void *__PF_with_fold_timer, void *__PF_fw_fun_timer, void *__PF_fw_with_modarray_timer,
  void *__PF_fw_with_genarray_timer, void *__PF_fw_with_fold_timer);

extern void _SAC_PrintProfileFuns (__PF_TIMER *total_time, int display_fun,
                                   int display_with, int __PF_maxfun, int *__PF_maxfunap,
                                   void *__PF_fun_timer, void *__PF_with_modarray_timer,
                                   void *__PF_with_genarray_timer,
                                   void *__PF_with_fold_timer, void *__PF_fw_fun_timer,
                                   void *__PF_fw_with_modarray_timer,
                                   void *__PF_fw_with_genarray_timer,
                                   void *__PF_fw_with_fold_timer, void *__PF_fun_name);

#define PROFILE_SETUP(maxfun)                                                            \
    __PF_act_timer = &__PF_fun_timer[0][0];                                              \
    __PF_act_funno = 0;                                                                  \
    __PF_act_funapno = 0;                                                                \
    __PF_maxfun = maxfun;                                                                \
    __PF_INIT_CLOCK ();                                                                  \
    {                                                                                    \
        int i, j;                                                                        \
        for (i = 0; i < maxfun; i++) {                                                   \
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

#define PROFILE_PRINT()                                                                  \
    __PF_STOP_CLOCK ();                                                                  \
    __PF_ADD_TO_TIMER (*__PF_act_timer);                                                 \
    {                                                                                    \
        __PF_TIMER *tmp;                                                                 \
                                                                                         \
        tmp = _SAC_PrintProfileOverall (DISPLAY_WITH, __PF_maxfun, __PF_maxfunap,        \
                                        __PF_fun_timer, __PF_with_modarray_timer,        \
                                        __PF_with_genarray_timer, __PF_with_fold_timer,  \
                                        __PF_fw_fun_timer, __PF_fw_with_modarray_timer,  \
                                        __PF_fw_with_genarray_timer,                     \
                                        __PF_fw_with_fold_timer);                        \
        _SAC_PrintProfileFuns (tmp, DISPLAY_FUN, DISPLAY_WITH, __PF_maxfun,              \
                               __PF_maxfunap, __PF_fun_timer, __PF_with_modarray_timer,  \
                               __PF_with_genarray_timer, __PF_with_fold_timer,           \
                               __PF_fw_fun_timer, __PF_fw_with_modarray_timer,           \
                               __PF_fw_with_genarray_timer, __PF_fw_with_fold_timer,     \
                               __PF_fun_name);                                           \
    }

#define __PF_INIT_CLOCK()

#define __PF_INIT_TIMER(timer) timer##.tv_sec = timer##.tv_usec = 0

#define __PF_START_CLOCK() getrusage (RUSAGE_SELF, &_SAC_start_timer)

#define __PF_STOP_CLOCK() getrusage (RUSAGE_SELF, &_SAC_stop_timer)

#define __PF_ADD_TO_TIMER(timer)                                                         \
    if (((timer).tv_usec                                                                 \
         += _SAC_stop_timer.ru_utime.tv_usec - _SAC_start_timer.ru_utime.tv_usec)        \
        < 0) {                                                                           \
        (timer).tv_usec += 1000000;                                                      \
        (timer).tv_sec                                                                   \
          += _SAC_stop_timer.ru_utime.tv_sec - _SAC_start_timer.ru_utime.tv_sec - 1;     \
    } else {                                                                             \
        (timer).tv_sec                                                                   \
          += _SAC_stop_timer.ru_utime.tv_sec - _SAC_start_timer.ru_utime.tv_sec;         \
    }

#else /* PROFILE */

#define PROFILE_SETUP(maxfun)
#define PROFILE_PRINT()

#endif /* PROFILE */

/*
 *  Macros for profiling with-loops
 */

#if PROFILE_WITH

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

#define DISPLAY_WITH 1

#else /* PROFILE_WITH */

#define PROFILE_BEGIN_WITH(str)
#define PROFILE_END_WITH(str)
#define DISPLAY_WITH 0

#endif /* PROFILE_WITH */

/*
 *  Macros for profiling function applications
 */

#if PROFILE_FUN

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

#define DISPLAY_FUN 1

#else /* PROFILE_FUN */

#define PROFILE_BEGIN_UDF(funno, funapno)
#define PROFILE_END_UDF(funno, funapno)
#define DISPLAY_FUN 0

#endif /* PROFILE_FUN */

/*
 *  Macros for profiling even inlined function applications
 */

#if PROFILE_INL

#define PROFILE_INLINE(x) x

#else /* PROFILE_INL */

#define PROFILE_INLINE(x)

#endif /* PROFILE_INL */

/*
 *  Macros for profiling even library function applications
 */

#if PROFILE_LIB

#define PROFILE_LIBRARY(x) x

#else /* PROFILE_LIB */

#define PROFILE_LIBRARY(x)

#endif /* PROFILE_LIB */

#endif /* SAC_PROFILE_H */
