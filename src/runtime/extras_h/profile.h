/*****************************************************************************
 *
 * file:   sac_profile.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   Profiling operations may be selectively activated by the global switches
 *    PROFILE_WITH      for profiling with-loops
 *    PROFILE_FUN       for profiling function applications
 *    PROFILE_INL       for profiling even inlined functions
 *    PROFILE_LIB       for profiling even library functions
 *    PROFILE_DISTMEM   for profiling the distributed memory backend
 *
 *   The global switch PROFILE indicates any profiling activations.
 *
 *****************************************************************************/

#ifndef _SAC_PROFILE_H
#define _SAC_PROFILE_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#if SAC_MUTC_MACROS
#define SAC_PF_BEGIN_WITH(str)
#define SAC_PF_END_WITH(str)
#endif

#if !SAC_MUTC_MACROS

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

typedef enum {
    PF_ow_fun = 0,
    PF_ow_genarray = 1,
    PF_ow_modarray = 2,
    PF_ow_fold = 3,
    PF_iw_fun = 4,
    PF_iw_genarray = 5,
    PF_iw_modarray = 6,
    PF_iw_fold = 7
} SAC_PF_timer_type;

typedef struct timer_record {
    int funno;
    int funapno;
    SAC_PF_timer_type timer_type;
    struct timer_record *parent;
    int *cycle_tag;
} SAC_PF_TIMER_RECORD;

/*
 * External declarations of C library functions needed
 */

SAC_C_EXTERN int getrusage (int who, struct rusage *rusage);

/*
 * External declarations of library functions defined in libsac
 */

SAC_C_EXTERN void SAC_PF_PrintHeader (char *title);
SAC_C_EXTERN void SAC_PF_PrintSubHeader (char *title, int lineno);
SAC_C_EXTERN void SAC_PF_PrintTime (char *title, char *space, SAC_PF_TIMER *time);
SAC_C_EXTERN void SAC_PF_PrintCount (char *title, char *space, unsigned long count);
SAC_C_EXTERN void SAC_PF_PrintTimePercentage (char *title, char *space,
                                              SAC_PF_TIMER *time1, SAC_PF_TIMER *time2);

/*
 * External declarations of global variables defined in libsac
 */

SAC_C_EXTERN int SAC_PF_act_funno;
SAC_C_EXTERN int SAC_PF_act_funapno;
SAC_C_EXTERN int SAC_PF_with_level;
SAC_C_EXTERN struct rusage SAC_PF_start_timer;
SAC_C_EXTERN struct rusage SAC_PF_stop_timer;

/*
 *  Macro definitions
 */

#if SAC_DO_COMPILE_MODULE
#define SAC_PF_DEFINE()                                                                  \
    SAC_C_EXTERN SAC_PF_TIMER SAC_PF_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP][8];         \
    SAC_C_EXTERN int SAC_PF_cycle_tag[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                 \
                                                                                         \
    SAC_C_EXTERN int SAC_PF_act_cycle_tag;                                               \
    SAC_C_EXTERN SAC_PF_TIMER_RECORD *SAC_PF_act_record;                                 \
                                                                                         \
    SAC_C_EXTERN char *SAC_PF_fun_name[SAC_SET_MAXFUN];                                  \
    SAC_C_EXTERN int SAC_PF_maxfunap[SAC_SET_MAXFUN];                                    \
    SAC_C_EXTERN int SAC_PF_funapline[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                 \
    SAC_C_EXTERN int SAC_PF_parentfunno[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];
#else
#define SAC_PF_DEFINE()                                                                  \
    SAC_PF_TIMER SAC_PF_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP][8];                      \
    int SAC_PF_cycle_tag[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                              \
                                                                                         \
    int SAC_PF_act_cycle_tag;                                                            \
    SAC_PF_TIMER_RECORD SAC_PF_initial_record;                                           \
    SAC_PF_TIMER_RECORD *SAC_PF_act_record = &SAC_PF_initial_record;                     \
                                                                                         \
    char *SAC_PF_fun_name[SAC_SET_MAXFUN] = SAC_SET_FUN_NAMES;                           \
    int SAC_PF_maxfunap[SAC_SET_MAXFUN] = SAC_SET_FUN_APPS;                              \
    int SAC_PF_funapline[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP] = SAC_SET_FUN_AP_LINES;       \
    int SAC_PF_parentfunno[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP] = SAC_SET_FUN_PARENTS;
#endif

#define SAC_PF_SETUP()                                                                   \
    {                                                                                    \
        int i, j, k;                                                                     \
                                                                                         \
        SAC_PF_INIT_CLOCK ();                                                            \
        for (i = 0; i < SAC_SET_MAXFUN; i++) {                                           \
            for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                   \
                for (k = 0; k < 8; k++) {                                                \
                    SAC_PF_INIT_TIMER (SAC_PF_timer[i][j][k]);                           \
                }                                                                        \
                SAC_PF_cycle_tag[i][j] = 0;                                              \
            }                                                                            \
        }                                                                                \
        SAC_PF_act_record->funno = 0;                                                    \
        SAC_PF_act_record->funapno = 0;                                                  \
        SAC_PF_act_record->timer_type = PF_ow_fun;                                       \
        SAC_PF_act_record->parent = NULL;                                                \
        SAC_PF_act_record->cycle_tag = &SAC_PF_cycle_tag[0][0];                          \
        SAC_PF_act_cycle_tag = 0;                                                        \
        SAC_PF_START_CLOCK ();                                                           \
    }

#define SAC_PF_PRINT()                                                                   \
    {                                                                                    \
        SAC_PF_TIMER grand_total;                                                        \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_PRINT_OVERALL (grand_total);                                              \
        SAC_PF_PRINT_FUNS (grand_total);                                                 \
        SAC_PF_PRINT_DISTMEM ();                                                         \
    }

#define SAC_PF_PRINT_OVERALL(total)                                                      \
    {                                                                                    \
        int i, j, k;                                                                     \
        SAC_PF_TIMER with_total;                                                         \
                                                                                         \
        SAC_PF_INIT_TIMER (with_total);                                                  \
        SAC_PF_INIT_TIMER (total);                                                       \
                                                                                         \
        for (k = PF_ow_genarray; k <= PF_iw_fold; k++) {                                 \
            SAC_PF_ADD_TIMERS (with_total, with_total, SAC_PF_timer[0][0][k]);           \
        }                                                                                \
        SAC_PF_ADD_TIMERS (total, with_total, SAC_PF_timer[0][0][PF_ow_fun]);            \
                                                                                         \
        SAC_PF_PrintHeader ("Overall Profile");                                          \
        SAC_PF_PrintTime ("time used", SAC_PF_TIMER_SPACE, &total);                      \
                                                                                         \
        if (SAC_PF_DISPLAY_WITH) {                                                       \
            SAC_PF_PrintTimePercentage ("   with-loop", "", &with_total, &total);        \
            SAC_PF_PrintTimePercentage ("   non-with-loop", "",                          \
                                        &SAC_PF_timer[0][0][PF_ow_fun], &total);         \
        }                                                                                \
    }

#define SAC_PF_PRINT_FUNS(total_time)                                                    \
    {                                                                                    \
        int i, j, k, l;                                                                  \
        SAC_PF_TIMER with_total, parent_with_total, non_with_total, total;               \
                                                                                         \
        SAC_PF_PrintHeader ("Function Profiles");                                        \
                                                                                         \
        for (i = 0; i < (SAC_PF_DISPLAY_FUN ? SAC_SET_MAXFUN : 1); i++) {                \
            SAC_PF_INIT_TIMER (with_total);                                              \
            SAC_PF_INIT_TIMER (parent_with_total);                                       \
            SAC_PF_INIT_TIMER (non_with_total);                                          \
            SAC_PF_INIT_TIMER (total);                                                   \
                                                                                         \
            for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                   \
                for (k = PF_ow_genarray; k <= PF_ow_fold; k++) {                         \
                    SAC_PF_ADD_TIMERS (with_total, with_total, SAC_PF_timer[i][j][k]);   \
                }                                                                        \
                for (k = PF_iw_fun; k <= PF_iw_fold; k++) {                              \
                    SAC_PF_ADD_TIMERS (parent_with_total, parent_with_total,             \
                                       SAC_PF_timer[i][j][k]);                           \
                }                                                                        \
                SAC_PF_ADD_TIMERS (non_with_total, non_with_total,                       \
                                   SAC_PF_timer[i][j][PF_ow_fun]);                       \
            }                                                                            \
                                                                                         \
            SAC_PF_ADD_TIMERS (total, with_total, non_with_total);                       \
            SAC_PF_ADD_TIMERS (total, total, parent_with_total);                         \
                                                                                         \
            if ((total.tv_sec != 0) || (total.tv_usec != 0)) {                           \
                SAC_PF_PrintHeader (SAC_PF_fun_name[i]);                                 \
                SAC_PF_PrintTimePercentage ("time used", SAC_PF_TIMER_SPACE, &total,     \
                                            &total_time);                                \
                                                                                         \
                if (SAC_PF_DISPLAY_WITH) {                                               \
                    SAC_PF_PrintTimePercentage ("   with-loop(own)", "", &with_total,    \
                                                &total);                                 \
                                                                                         \
                    SAC_PF_PrintTimePercentage ("   with-loop(parent)", "",              \
                                                &parent_with_total, &total);             \
                                                                                         \
                    SAC_PF_PrintTimePercentage ("   non-with-loop", "", &non_with_total, \
                                                &total);                                 \
                }                                                                        \
                if (SAC_PF_DISPLAY_FUN) {                                                \
                    for (k = 0; k < SAC_SET_MAXFUN; k += (k == (i - 1) ? 2 : 1)) {       \
                        if (k == i) {                                                    \
                            k++;                                                         \
                        } /* required for i==0 only! */                                  \
                        for (j = 0; j < SAC_PF_maxfunap[k]; j++) {                       \
                            if (SAC_PF_parentfunno[k][j] == i) {                         \
                                SAC_PF_TIMER sub_total;                                  \
                                SAC_PF_INIT_TIMER (sub_total);                           \
                                SAC_PF_PrintSubHeader (SAC_PF_fun_name[k],               \
                                                       SAC_PF_funapline[k][j]);          \
                                for (l = 0; l < 8; l++) {                                \
                                    SAC_PF_ADD_TIMERS (sub_total, sub_total,             \
                                                       SAC_PF_timer[k][j][l]);           \
                                }                                                        \
                                SAC_PF_PrintTimePercentage (" relative time used",       \
                                                            SAC_PF_TIMER_SPACE,          \
                                                            &sub_total, &total);         \
                            }                                                            \
                        }                                                                \
                    }                                                                    \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define SAC_PF_INIT_CLOCK()

#define SAC_PF_INIT_TIMER(timer) timer.tv_sec = timer.tv_usec = 0

#define SAC_PF_START_CLOCK() getrusage (RUSAGE_SELF, &SAC_PF_start_timer)

#define SAC_PF_STOP_CLOCK() getrusage (RUSAGE_SELF, &SAC_PF_stop_timer)

#define SAC_PF_RECORD_TIMER(record)                                                      \
    (SAC_PF_timer[record->funno][record->funapno][record->timer_type])

#define SAC_PF_ADD_TO_TIMER(record)                                                      \
    {                                                                                    \
        if ((SAC_PF_RECORD_TIMER (record).tv_usec                                        \
             += SAC_PF_stop_timer.ru_utime.tv_usec                                       \
                - SAC_PF_start_timer.ru_utime.tv_usec)                                   \
            < 0) {                                                                       \
            SAC_PF_RECORD_TIMER (record).tv_usec += 1000000;                             \
            SAC_PF_RECORD_TIMER (record).tv_sec += SAC_PF_stop_timer.ru_utime.tv_sec     \
                                                   - SAC_PF_start_timer.ru_utime.tv_sec  \
                                                   - 1;                                  \
        } else {                                                                         \
            SAC_PF_RECORD_TIMER (record).tv_sec                                          \
              += SAC_PF_stop_timer.ru_utime.tv_sec - SAC_PF_start_timer.ru_utime.tv_sec; \
        }                                                                                \
    }

#define SAC_PF_ADD_TO_TIMER_CHAIN(record)                                                \
    {                                                                                    \
        SAC_PF_TIMER_RECORD *tmp;                                                        \
        tmp = record;                                                                    \
        SAC_PF_act_cycle_tag++;                                                          \
        do {                                                                             \
            if (*(tmp->cycle_tag) != SAC_PF_act_cycle_tag) {                             \
                *(tmp->cycle_tag) = SAC_PF_act_cycle_tag;                                \
                SAC_PF_ADD_TO_TIMER (tmp);                                               \
            }                                                                            \
            tmp = tmp->parent;                                                           \
        } while (tmp != NULL);                                                           \
    }

#define SAC_PF_ADD_TIMERS(timer, timer1, timer2)                                         \
    {                                                                                    \
        timer.tv_usec = timer1.tv_usec + timer2.tv_usec;                                 \
        if (timer.tv_usec >= 1000000) {                                                  \
            timer.tv_usec -= 1000000;                                                    \
            timer.tv_sec = timer1.tv_sec + timer2.tv_sec + 1;                            \
        } else {                                                                         \
            timer.tv_sec = timer1.tv_sec + timer2.tv_sec;                                \
        }                                                                                \
    }

#define SAC_PF_COPY_TIMER(totimer, fromtimer)                                            \
    {                                                                                    \
        totimer.tv_sec = fromtimer.tv_sec;                                               \
        totimer.tv_usec = fromtimer.tv_usec;                                             \
    }

#define SAC_PF_TIMER_SPACE "              "
#define SAC_PF_COUNT_SPACE "              "

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
        SAC_PF_TIMER_RECORD SAC_PF_new_record;                                           \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_new_record.parent = SAC_PF_act_record;                                    \
        SAC_PF_new_record.funno = SAC_PF_act_record->funno;                              \
        SAC_PF_new_record.funapno = SAC_PF_act_record->funapno;                          \
        SAC_PF_new_record.timer_type                                                     \
          = (SAC_PF_with_level == 0 ? PF_ow_##str : PF_iw_##str);                        \
        SAC_PF_new_record.cycle_tag = SAC_PF_act_record->cycle_tag;                      \
        SAC_PF_act_record = &SAC_PF_new_record;                                          \
        SAC_PF_with_level++;                                                             \
        SAC_PF_START_CLOCK ();

#define SAC_PF_END_WITH(str)                                                             \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_with_level--;                                                                 \
    SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                       \
    SAC_PF_act_record = SAC_PF_act_record->parent;                                       \
    SAC_PF_START_CLOCK ();                                                               \
    }

#define SAC_PF_DISPLAY_WITH 1

#else /* SAC_DO_PROFILE_WITH */

#define SAC_PF_BEGIN_WITH(str)
#define SAC_PF_END_WITH(str)
#define SAC_PF_DISPLAY_WITH 0

#endif /* SAC_DO_PROFILE_WITH */

/*
 *  Macros for profiling the distributed memory backend
 */

#if (SAC_DO_PROFILE_DISTMEM && SAC_DO_PROFILE)

#define SAC_PF_PRINT_DISTMEM()                                                           \
    {                                                                                    \
        SAC_PF_PrintHeader ("Distributed Memory Backend Profile");                       \
        SAC_PF_PrintCount ("Distributed arrays:", SAC_PF_COUNT_SPACE,                    \
                           SAC_DISTMEM_TR_num_arrays);                                   \
        SAC_PF_PrintCount ("Invalidated pages:", SAC_PF_COUNT_SPACE,                     \
                           SAC_DISTMEM_TR_num_inval_pages);                              \
        SAC_PF_PrintCount ("Seg faults/page fetches: ", SAC_PF_COUNT_SPACE,              \
                           SAC_DISTMEM_TR_num_segfaults);                                \
        SAC_PF_PrintCount ("Pointer calculations: ", SAC_PF_COUNT_SPACE,                 \
                           SAC_DISTMEM_TR_num_ptr_calcs);                                \
        SAC_PF_PrintCount ("Barriers: ", SAC_PF_COUNT_SPACE,                             \
                           SAC_DISTMEM_TR_num_barriers);                                 \
    }

#else /* SAC_DO_PROFILE_DISTMEM && SAC_DO_PROFILE */

#define SAC_PF_PRINT_DISTMEM()

#endif /* SAC_DO_PROFILE_DISTMEM && SAC_DO_PROFILE */

#ifdef DEBUG_PROFILING

#include <stdio.h>

#define DEBUG_PROFILE_PRINT(record)                                                      \
    {                                                                                    \
        SAC_PF_TIMER_RECORD *tmp;                                                        \
        tmp = SAC_PF_act_record;                                                         \
        while (tmp != NULL) {                                                            \
            fprintf (stderr, "funno: %d funapno: %d timer_type: %d\n", tmp->funno,       \
                     tmp->funapno, tmp->timer_type);                                     \
            tmp = tmp->parent;                                                           \
        }                                                                                \
    }

#define DEBUG_PROFILE_ENTER(record)                                                      \
    {                                                                                    \
        fprintf (stderr, "entering UDF:\n");                                             \
        DEBUG_PROFILE_PRINT (record);                                                    \
    }

#define DEBUG_PROFILE_LEAVE(record)                                                      \
    {                                                                                    \
        fprintf (stderr, "leaving UDF:\n");                                              \
        DEBUG_PROFILE_PRINT (record);                                                    \
    }

#else
#define DEBUG_PROFILE_ENTER(record)
#define DEBUG_PROFILE_LEAVE(record)

#endif

/*
 *  Macros for profiling function applications
 */

#if (SAC_DO_PROFILE_FUN && SAC_DO_PROFILE)

#define PROFILE_BEGIN_UDF(funno_in, funapno_in)                                          \
    {                                                                                    \
        SAC_PF_TIMER_RECORD SAC_PF_new_record;                                           \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_new_record.parent = SAC_PF_act_record;                                    \
        SAC_PF_new_record.funno = funno_in;                                              \
        SAC_PF_new_record.funapno = funapno_in;                                          \
        SAC_PF_new_record.timer_type = (SAC_PF_with_level == 0 ? PF_ow_fun : PF_iw_fun); \
        SAC_PF_new_record.cycle_tag = &SAC_PF_cycle_tag[funno_in][funapno_in];           \
        SAC_PF_act_record = &SAC_PF_new_record;                                          \
        DEBUG_PROFILE_ENTER (SAC_PF_act_record);                                         \
        SAC_PF_START_CLOCK ();

#define PROFILE_END_UDF(funno, funapno)                                                  \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                       \
    SAC_PF_act_record = SAC_PF_act_record->parent;                                       \
    DEBUG_PROFILE_LEAVE (SAC_PF_act_record);                                             \
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

#else
#define SAC_PF_DEFINE()
#endif /* mutc */

#endif /* _SAC_PROFILE_H */
