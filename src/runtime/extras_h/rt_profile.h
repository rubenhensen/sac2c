/*****************************************************************************
 *
 * file:   rt_profile.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   Profiling operations may be selectively activated by the global switches
 *    PROFILE_WITH      for profiling with-loops
 *    PROFILE_FUN       for profiling function applications
 *    PROFILE_INL       for profiling even inlined functions
 *    PROFILE_LIB       for profiling even library functions
 *    PROFILE_MEM       for profiling memory operations
 *    PROFILE_DISTMEM   for profiling the distributed memory backend
 *
 *   The global switch PROFILE indicates any profiling activations.
 *
 * We only have one timer. So whenever we measure a code fragment,
 * we have to make sure it is attributed to the right call *and* all its parents!
 * We do so by maintaining a chain of records which reflect the call graph. Whenever we
 * finish an inner function call, we add the time delta measured to all parents in the
 * call graph! However, we *must* not blindly do that (!) because we may have recursion
 * which ends up calling cyclicly the same code! That is why we simply tag the parents
 * when following the call graph with a unique new number which is devised upon each
 * function application we perform. If we find a parent with a tag identical to the
 * current one, we know that we have added the current time and we do not add the time
 * again.
 *
 * The distributed memory backend profiling is independent of the ordinary profiling.
 * The reason is that we are interested in the overall time spent in execution modes or
 * waiting at barriers rather than the time spent waiting at a barrier in a specific
 * function.
 *
 *****************************************************************************/

#ifndef _SAC_RT_PROFILE_H
#define _SAC_RT_PROFILE_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#if SAC_MUTC_MACROS

#define SAC_PF_BEGIN_WITH(str)
#define SAC_PF_END_WITH(str)
#define SAC_PF_DEFINE()

#else /* SAC_MUTC_MACROS */

/* For profiling communications */
#if SAC_DO_COMPILE_MODULE
SAC_C_EXTERN void SAC_PF_BeginComm (void);
SAC_C_EXTERN void SAC_PF_EndComm (void);
#endif

/*
 *  General profiling macros and declarations
 */
#if SAC_DO_PROFILE

/*
 * PROFILER Main macros
 */
#if SAC_DO_COMPILE_MODULE

#define SAC_PF_DEFINE()                                                                  \
   SAC_PF_DEFINE_EXT()                                                                   \
   SAC_PF_DEFINE_EXTMOD()

#else /* SAC_DO_COMPILE_MODULE */

#define SAC_PF_DEFINE()                                                                  \
   SAC_PF_DEFINE_EXT()                                                                   \
   SAC_PF_DEFINE_EXTLOC()                                                                \
   SAC_PF_DEFINE_LOC()

#endif /* SAC_DO_COMPILE_MODULE */

/***
 * external declarations shared between
 * modules and the main
 */
#define SAC_PF_DEFINE_EXT()                                                              \
    SAC_C_EXTERN int SAC_PF_act_cycle_tag;                                               \
    SAC_C_EXTERN unsigned SAC_PF_with_level;                                             \
    SAC_C_EXTERN SAC_PROFILE_RECORD *SAC_PF_act_record;                                  \
    SAC_C_EXTERN struct rusage SAC_PF_start_timer;                                       \
    SAC_C_EXTERN struct rusage SAC_PF_stop_timer;                                        \
                                                                                         \
    SAC_PF_DISTMEM_DEFINE_EXT()

/***
 * external declarations for modules
 * Here, we do not know the sizes...
 */
#define SAC_PF_DEFINE_EXTMOD()                                                           \
    SAC_C_EXTERN SAC_PF_TIMER ***SAC_PF_timer;                                           \
    SAC_C_EXTERN int **SAC_PF_cycle_tag;                                                 \
    SAC_C_EXTERN char **SAC_PF_fun_name;                                                 \
    SAC_C_EXTERN int *SAC_PF_maxfunap;                                                   \
    SAC_C_EXTERN size_t **SAC_PF_funapline;                                              \
    SAC_C_EXTERN size_t **SAC_PF_parentfunno;                                            \
                                                                                         \
    SAC_PF_MEM_DEFINE_EXTMOD()                                                           \
    SAC_PF_OPS_DEFINE_EXTMOD() 

/***
 * external declarations for the main
 * Here, we do need to match the known sizes
 */
#define SAC_PF_DEFINE_EXTLOC()                                                           \
    SAC_C_EXTERN SAC_PF_TIMER SAC_PF_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP]             \
                             [SAC_PF_NUM_RECORD_TYPES];                                  \
    SAC_C_EXTERN int SAC_PF_cycle_tag[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                 \
    SAC_C_EXTERN char *SAC_PF_fun_name[SAC_SET_MAXFUN];                                  \
    SAC_C_EXTERN int SAC_PF_maxfunap[SAC_SET_MAXFUN];                                    \
    SAC_C_EXTERN size_t SAC_PF_funapline[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];              \
    SAC_C_EXTERN size_t SAC_PF_parentfunno[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];            \
                                                                                         \
    SAC_PF_MEM_DEFINE_EXTLOC()                                                           \
    SAC_PF_OPS_DEFINE_EXTLOC()

/***
 * here the definitions for the main
 */
#define SAC_PF_DEFINE_LOC()                                                              \
    SAC_PF_TIMER SAC_PF_timer[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP]                          \
                             [SAC_PF_NUM_RECORD_TYPES];                                  \
    int SAC_PF_cycle_tag[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                              \
                                                                                         \
    int SAC_PF_act_cycle_tag;                                                            \
    unsigned SAC_PF_with_level = 0;                                                      \
    static SAC_PROFILE_RECORD SAC_PF_initial_record;                                     \
    SAC_PROFILE_RECORD *SAC_PF_act_record = &SAC_PF_initial_record;                      \
    struct rusage SAC_PF_start_timer;                                                    \
    struct rusage SAC_PF_stop_timer;                                                     \
                                                                                         \
    SAC_PF_MEM_DEFINE_LOC()                                                              \
    SAC_PF_OPS_DEFINE_LOC()                                                              \
    SAC_PF_DISTMEM_DEFINE_LOC()                                                          \
                                                                                         \
    char *SAC_PF_fun_name[SAC_SET_MAXFUN] = SAC_SET_FUN_NAMES;                           \
    int SAC_PF_maxfunap[SAC_SET_MAXFUN] = SAC_SET_FUN_APPS;                              \
    size_t SAC_PF_funapline[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP] = SAC_SET_FUN_AP_LINES;    \
    size_t SAC_PF_parentfunno[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP] = SAC_SET_FUN_PARENTS;   \
                                                                                         \
    static void SAC_PF_BeginComm (void)                                                  \
    {                                                                                    \
        SAC_PF_BEGIN_COMM ();                                                            \
    }                                                                                    \
                                                                                         \
    static void SAC_PF_EndComm (void)                                                    \
    {                                                                                    \
        SAC_PF_END_COMM ();                                                              \
    }


/*
 * Macros that do profiling for user-defined functions
 */
#define PROFILE_BEGIN_UDF(funno_in, funapno_in)                                          \
    {                                                                                    \
        SAC_PROFILE_RECORD SAC_PF_new_record;                                            \
        SAC_PF_BEGIN_FUNCTION_START ()                                                   \
        SAC_PF_new_record.parent = SAC_PF_act_record;                                    \
        SAC_PF_new_record.funno = funno_in;                                              \
        SAC_PF_new_record.funapno = funapno_in;                                          \
        SAC_PF_new_record.record_type = (SAC_PF_with_level == 0 ? PF_ow_fun : PF_iw_fun);\
        SAC_PF_new_record.cycle_tag = &SAC_PF_cycle_tag[funno_in][funapno_in];           \
        SAC_PF_act_record = &SAC_PF_new_record;                                          \
        SAC_PF_BEGIN_MEM ()                                                              \
        SAC_PF_BEGIN_FUNCTION_END ()

#define PROFILE_END_UDF(funno, funapno)                                                  \
        SAC_PF_END_FUNCTION_START ()                                                     \
        SAC_PF_act_record = SAC_PF_act_record->parent;                                   \
        SAC_PF_END_MEM ()                                                                \
        SAC_PF_END_FUNCTION_END ()                                                       \
    }

#define SAC_PF_SETUP()                                                                   \
    {                                                                                    \
        int i, j, k;                                                                     \
                                                                                         \
        SAC_PF_INIT_CLOCK ();                                                            \
        for (i = 0; i < SAC_SET_MAXFUN; i++) {                                           \
            for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                   \
                for (k = 0; k < SAC_PF_NUM_RECORD_TYPES; k++) {                          \
                    SAC_PF_INIT_TIMER (SAC_PF_timer[i][j][k]);                           \
                }                                                                        \
                SAC_PF_cycle_tag[i][j] = 0;                                              \
            }                                                                            \
        }                                                                                \
        SAC_PF_act_record->funno = 0;                                                    \
        SAC_PF_act_record->funapno = 0;                                                  \
        SAC_PF_act_record->record_type = PF_ow_fun;                                      \
        SAC_PF_act_record->parent = NULL;                                                \
        SAC_PF_act_record->cycle_tag = &SAC_PF_cycle_tag[0][0];                          \
        SAC_PF_act_cycle_tag = 0;                                                        \
                                                                                         \
        SAC_PF_MEM_SETUP ()                                                              \
        SAC_PF_DISTMEM_SETUP ()                                                          \
                                                                                         \
        SAC_PF_START_CLOCK ();                                                           \
    }

#define SAC_PF_PRINT()                                                                   \
    {                                                                                    \
        SAC_PF_TIMER grand_total;                                                        \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_PRINT_OVERALL (grand_total);                                              \
                                                                                         \
        if (SAC_DO_PROFILE_DISTMEM) {                                                    \
            if (SAC_DISTMEM_rank                                                         \
              == SAC_DISTMEM_RANK_UNDEFINED /* Distributed memory backend not used */    \
            || SAC_DISTMEM_trace_profile_rank                                            \
                 == SAC_DISTMEM_TRACE_PROFILE_RANK_ANY /* Print profiling for any node   \
                                                        */                               \
            || SAC_DISTMEM_trace_profile_rank                                            \
                 == (int)SAC_DISTMEM_rank) { /* Print profiling for this node */         \
                SAC_PF_PRINT_DISTMEM (grand_total);                                      \
            }                                                                            \
        }                                                                                \
                                                                                         \
        SAC_PF_MEM_PRINT_STAT();                                                         \
        SAC_PF_OPS_PRINT_STAT();                                                         \
        SAC_PF_PRINT_FUNS (grand_total);                                                 \
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
                                                                                         \
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
    (SAC_PF_timer[record->funno][record->funapno][record->record_type])

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
                                                                                         \
        SAC_PF_ADD_STIME_TO_TIMER (record)                                               \
    }

#define SAC_PF_ADD_TO_TIMER_CHAIN(record)                                                \
    {                                                                                    \
        SAC_PROFILE_RECORD *tmp;                                                         \
        tmp = record;                                                                    \
        SAC_PF_act_cycle_tag++;                                                          \
        do {                                                                             \
            if (*(tmp->cycle_tag) != SAC_PF_act_cycle_tag) {                             \
                *(tmp->cycle_tag) = SAC_PF_act_cycle_tag;                                \
                SAC_PF_ADD_TO_TIMER (tmp);                                               \
            }                                                                            \
            tmp = tmp->parent;                                                           \
        } while (tmp != NULL);                                                           \
        SAC_PF_ADD_TO_DISTMEM_TIMER_CHAIN ()                                             \
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

#define SAC_PF_SUBTRACT_TIMERS(timer, timer1, timer2)                                    \
    {                                                                                    \
        if (timer1.tv_usec < timer2.tv_usec) {                                           \
            int nsec = (timer2.tv_usec - timer1.tv_usec) / 1000000 + 1;                  \
            timer2.tv_usec -= 1000000 * nsec;                                            \
            timer2.tv_sec += nsec;                                                       \
        }                                                                                \
                                                                                         \
        if (timer1.tv_usec - timer2.tv_usec > 1000000) {                                 \
            int nsec = (timer1.tv_usec - timer2.tv_usec) / 1000000;                      \
            timer2.tv_usec += 1000000 * nsec;                                            \
            timer2.tv_sec -= nsec;                                                       \
        }                                                                                \
                                                                                         \
        /* tv_usec is certainly positive. */                                             \
        timer.tv_sec = timer1.tv_sec - timer2.tv_sec;                                    \
        timer.tv_usec = timer1.tv_usec - timer2.tv_usec;                                 \
    }

#define SAC_PF_COPY_TIMER(totimer, fromtimer)                                            \
    {                                                                                    \
        totimer.tv_sec = fromtimer.tv_sec;                                               \
        totimer.tv_usec = fromtimer.tv_usec;                                             \
    }

#define SAC_PF_TIMER_SPACE ""
#define SAC_PF_COUNT_SPACE ""

#else /* SAC_DO_PROFILE */

#if SAC_DO_COMPILE_MODULE

#define SAC_PF_DEFINE()

#else /* SAC_DO_COMPILE_MODULE */
/*
 * It is ugly to define the SAC_PF_BeginComm and SAC_PF_EndComm functions
 * here but there is no other easy solution.
 * The communication profiling is called from within libsacdistmem but when
 * the libsacdistmem is built we don't know whether profiling will be active.
 * Also, we don't want to build profiling and non-profiling versions of
 * libsacdistmem since there are already plenty of versions.
 */
#define SAC_PF_DEFINE()                                                                  \
    /* Dummy definitions for when profiling is disabled */                               \
    static void SAC_PF_BeginComm (void)                                                  \
    {                                                                                    \
    }                                                                                    \
    static void SAC_PF_EndComm (void)                                                    \
    {                                                                                    \
    }

#endif /* SAC_DO_COMPILE_MODULE */

#define PROFILE_BEGIN_UDF(funno_in, funapno_in)
#define PROFILE_END_UDF(funno, funapno)
#define SAC_PF_SETUP()
#define SAC_PF_PRINT()

#endif /* SAC_DO_PROFILE */

/*
 *  Macros for profiling with-loops
 */

#if (SAC_DO_PROFILE_WITH && SAC_DO_PROFILE)

#define SAC_PF_BEGIN_WITH(str)                                                           \
    {                                                                                    \
        SAC_PROFILE_RECORD SAC_PF_new_record;                                            \
                                                                                         \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_new_record.parent = SAC_PF_act_record;                                    \
        SAC_PF_new_record.funno = SAC_PF_act_record->funno;                              \
        SAC_PF_new_record.funapno = SAC_PF_act_record->funapno;                          \
        SAC_PF_new_record.record_type                                                    \
          = (SAC_PF_with_level == 0 ? PF_ow_##str : PF_iw_##str);                        \
        SAC_PF_new_record.cycle_tag = SAC_PF_act_record->cycle_tag;                      \
        SAC_PF_act_record = &SAC_PF_new_record;                                          \
        SAC_PF_with_level++;                                                             \
        SAC_PF_START_CLOCK ();

#define SAC_PF_END_WITH(str)                                                             \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_with_level--;                                                             \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                   \
        SAC_PF_act_record = SAC_PF_act_record->parent;                                   \
        SAC_PF_START_CLOCK ();                                                           \
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

#define SAC_PF_NUM_RECORD_TYPES 15

#define SAC_PF_DISTMEM_DEFINE_EXT()                                                      \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_initial_record;                       \
    SAC_C_EXTERN SAC_PROFILE_RECORD *SAC_PF_distmem_act_record;                          \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_rep_record;                           \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_dist_record;                          \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_side_effects_record;                  \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_rep_barrier_record;                   \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_dist_barrier_record;                  \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_side_effects_barrier_record;          \
    SAC_C_EXTERN SAC_PROFILE_RECORD SAC_PF_distmem_comm_record;

#define SAC_PF_DISTMEM_DEFINE_LOC()                                                      \
    SAC_PROFILE_RECORD SAC_PF_distmem_initial_record;                                    \
    SAC_PROFILE_RECORD *SAC_PF_distmem_act_record = &SAC_PF_distmem_initial_record;      \
    SAC_PROFILE_RECORD SAC_PF_distmem_rep_record;                                        \
    SAC_PROFILE_RECORD SAC_PF_distmem_dist_record;                                       \
    SAC_PROFILE_RECORD SAC_PF_distmem_side_effects_record;                               \
    SAC_PROFILE_RECORD SAC_PF_distmem_rep_barrier_record;                                \
    SAC_PROFILE_RECORD SAC_PF_distmem_dist_barrier_record;                               \
    SAC_PROFILE_RECORD SAC_PF_distmem_side_effects_barrier_record;                       \
    SAC_PROFILE_RECORD SAC_PF_distmem_comm_record;

#define SAC_PF_DISTMEM_SETUP()                                                           \
    SAC_PF_distmem_rep_record.record_type = PF_distmem_exec_rep;                         \
    SAC_PF_distmem_dist_record.record_type = PF_distmem_exec_dist;                       \
    SAC_PF_distmem_side_effects_record.record_type = PF_distmem_exec_side_effects;       \
    SAC_PF_distmem_rep_barrier_record.record_type = PF_distmem_rep_barrier;              \
    SAC_PF_distmem_dist_barrier_record.record_type = PF_distmem_dist_barrier;            \
    SAC_PF_distmem_side_effects_barrier_record.record_type                               \
      = PF_distmem_side_effects_barrier;                                                 \
    SAC_PF_distmem_comm_record.record_type = PF_distmem_comm;                            \
    SAC_PF_distmem_act_record->funno = 0;                                                \
    SAC_PF_distmem_act_record->funapno = 0;                                              \
    SAC_PF_distmem_act_record->record_type = PF_distmem_exec_rep;                        \
    SAC_PF_distmem_act_record->parent = NULL;                                            \
    SAC_PF_distmem_act_record->cycle_tag = NULL;

/* When profiling the distributed memory backend, we also
 * measure the system CPU time to get meaningful measurements for
 * the overhead caused by barriers. */
#define SAC_PF_ADD_STIME_TO_TIMER(record)                                                \
    if ((SAC_PF_RECORD_TIMER (record).tv_usec                                            \
         += SAC_PF_stop_timer.ru_stime.tv_usec - SAC_PF_start_timer.ru_stime.tv_usec)    \
        < 0) {                                                                           \
        SAC_PF_RECORD_TIMER (record).tv_usec += 1000000;                                 \
        SAC_PF_RECORD_TIMER (record).tv_sec                                              \
          += SAC_PF_stop_timer.ru_stime.tv_sec - SAC_PF_start_timer.ru_stime.tv_sec - 1; \
    } else {                                                                             \
        SAC_PF_RECORD_TIMER (record).tv_sec                                              \
          += SAC_PF_stop_timer.ru_stime.tv_sec - SAC_PF_start_timer.ru_stime.tv_sec;     \
    }

#define SAC_PF_PRINT_DISTMEM(total)                                                      \
    {                                                                                    \
        int i;                                                                           \
        SAC_PF_TIMER barrier_total;                                                      \
        SAC_PF_INIT_TIMER (barrier_total);                                               \
        for (i = PF_distmem_rep_barrier; i <= PF_distmem_side_effects_barrier; i++) {    \
            SAC_PF_ADD_TIMERS (barrier_total, barrier_total, SAC_PF_timer[0][0][i]);     \
        }                                                                                \
        SAC_PF_PrintHeader ("Distributed Memory Backend Profile");                       \
        SAC_PF_PrintCount ("Invalidated pages                    ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_inval_pages);                              \
        SAC_PF_PrintCount ("Seg faults/page fetches              ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_segfaults);                                \
        SAC_PF_PrintCount ("Pointer calculations                 ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_ptr_calcs);                                \
        SAC_PF_PrintCount ("Avoided ptr calcs (local writes)     ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_writes);           \
        SAC_PF_PrintCount ("Avoided ptr calcs (local reads)      ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_avoided_ptr_calcs_local_reads);            \
        SAC_PF_PrintCount ("Avoided ptr calcs (known local reads)", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_avoided_ptr_calcs_known_local_reads);      \
        SAC_PF_PrintCount ("Avoided ptr calcs (remote reads)     ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_avoided_ptr_calcs_remote_reads);           \
        SAC_PF_PrintCount ("Ptr cache updates (remote reads)     ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_ptr_cache_updates);                        \
        SAC_PF_PrintCount ("Barriers                             ", SAC_PF_COUNT_SPACE,  \
                           SAC_DISTMEM_TR_num_barriers);                                 \
                                                                                         \
        SAC_PF_PrintTimePercentage ("Waiting at barriers    ", SAC_PF_TIMER_SPACE,       \
                                    &barrier_total, &total);                             \
        SAC_PF_PrintTimePercentage ("Communications         ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_comm]), &total);     \
        SAC_PF_PrintTimePercentage ("Replicated execution   ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_exec_rep]), &total); \
        SAC_PF_PrintTimePercentage ("   Waiting at barriers ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_rep_barrier]),       \
                                    &(SAC_PF_timer[0][0][PF_distmem_exec_rep]));         \
        SAC_PF_PrintTimePercentage ("Distributed execution  ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_exec_dist]),         \
                                    &total);                                             \
        SAC_PF_PrintTimePercentage ("   Waiting at barriers ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_dist_barrier]),      \
                                    &(SAC_PF_timer[0][0][PF_distmem_exec_dist]));        \
        SAC_PF_PrintTimePercentage ("Side effects execution ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0][PF_distmem_exec_side_effects]), \
                                    &total);                                             \
        SAC_PF_PrintTimePercentage ("   Waiting at barriers ", SAC_PF_TIMER_SPACE,       \
                                    &(SAC_PF_timer[0][0]                                 \
                                                  [PF_distmem_side_effects_barrier]),    \
                                    &(SAC_PF_timer[0][0]                                 \
                                                  [PF_distmem_exec_side_effects]));      \
    }

/* For profiling barriers */
#define SAC_PF_BEGIN_BARRIER()                                                           \
    switch (SAC_DISTMEM_exec_mode) {                                                     \
    case SAC_DISTMEM_exec_mode_dist:                                                     \
        SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_dist_barrier_record)                        \
        break;                                                                           \
    case SAC_DISTMEM_exec_mode_side_effects_outer:                                       \
    case SAC_DISTMEM_exec_mode_side_effects:                                             \
        SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_side_effects_barrier_record)                \
        break;                                                                           \
    default: /*  SAC_DISTMEM_exec_mode_sync */                                           \
        SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_rep_barrier_record)                         \
        break;                                                                           \
    }

#define SAC_PF_END_BARRIER()                                                             \
    switch (SAC_DISTMEM_exec_mode) {                                                     \
    case SAC_DISTMEM_exec_mode_dist:                                                     \
        SAC_PF_END_DISTMEM (SAC_PF_distmem_dist_barrier_record)                          \
        break;                                                                           \
    case SAC_DISTMEM_exec_mode_side_effects_outer:                                       \
    case SAC_DISTMEM_exec_mode_side_effects:                                             \
        SAC_PF_END_DISTMEM (SAC_PF_distmem_side_effects_barrier_record)                  \
        break;                                                                           \
    default: /*  SAC_DISTMEM_exec_mode_sync */                                           \
        SAC_PF_END_DISTMEM (SAC_PF_distmem_rep_barrier_record)                           \
        break;                                                                           \
    }

/* For profiling communications */
#define SAC_PF_BEGIN_COMM() SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_comm_record)

#define SAC_PF_END_COMM() SAC_PF_END_DISTMEM (SAC_PF_distmem_comm_record)

/* For profiling execution modes */
#define SAC_PF_BEGIN_EXEC_SIDE_EFFECTS()                                                 \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_sync) {                           \
        SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_side_effects_record)                        \
    }

#define SAC_PF_BEGIN_EXEC_DIST() SAC_PF_BEGIN_DISTMEM (SAC_PF_distmem_dist_record)

/* Ends profiling of the current execution mode. */
#define SAC_PF_END_EXEC_MODE()                                                           \
    switch (SAC_DISTMEM_exec_mode) {                                                     \
    case SAC_DISTMEM_exec_mode_dist:                                                     \
        SAC_PF_END_DISTMEM (SAC_PF_distmem_dist_record)                                  \
        break;                                                                           \
    case SAC_DISTMEM_exec_mode_side_effects_outer:                                       \
        SAC_PF_END_DISTMEM (SAC_PF_distmem_side_effects_record)                          \
        break;                                                                           \
    default:                                                                             \
        /* Unreachable */                                                                \
        break;                                                                           \
    }

#define SAC_PF_ADD_TO_DISTMEM_TIMER_CHAIN()                                              \
    SAC_PF_ADD_TO_TIMER (SAC_PF_distmem_act_record);                                     \
    if (SAC_PF_distmem_act_record->record_type == PF_distmem_rep_barrier                 \
        || SAC_PF_distmem_act_record->record_type == PF_distmem_dist_barrier             \
        || SAC_PF_distmem_act_record->record_type == PF_distmem_side_effects_barrier     \
        || SAC_PF_distmem_act_record->record_type == PF_distmem_comm) {                  \
        SAC_PF_ADD_TO_TIMER (SAC_PF_distmem_act_record->parent);                         \
    }

/* Generic functionality for distributed memory profiling */
#define SAC_PF_BEGIN_DISTMEM(timer_record)                                               \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                       \
    timer_record.parent = SAC_PF_distmem_act_record;                                     \
    timer_record.funno = SAC_PF_distmem_act_record->funno;                               \
    timer_record.funapno = SAC_PF_distmem_act_record->funapno;                           \
    timer_record.cycle_tag = SAC_PF_distmem_act_record->cycle_tag;                       \
    SAC_PF_distmem_act_record = &timer_record;                                           \
    SAC_PF_START_CLOCK ();

#define SAC_PF_END_DISTMEM(timer_record)                                                 \
    SAC_PF_STOP_CLOCK ();                                                                \
    SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);                                       \
    SAC_PF_distmem_act_record = SAC_PF_distmem_act_record->parent;                       \
    SAC_PF_START_CLOCK ();

#else /* SAC_DO_PROFILE_DISTMEM && SAC_DO_PROFILE */

#define SAC_PF_NUM_RECORD_TYPES 8
#define SAC_PF_DISTMEM_DEFINE_EXT()
#define SAC_PF_DISTMEM_DEFINE_LOC()
#define SAC_PF_DISTMEM_SETUP()
#define SAC_PF_ADD_STIME_TO_TIMER(record)
#define SAC_PF_PRINT_DISTMEM(total)
#define SAC_PF_BEGIN_BARRIER()
#define SAC_PF_END_BARRIER()
#define SAC_PF_BEGIN_COMM()
#define SAC_PF_END_COMM()
#define SAC_PF_BEGIN_EXEC_SIDE_EFFECTS()
#define SAC_PF_BEGIN_EXEC_DIST()
#define SAC_PF_END_EXEC_MODE()
#define SAC_PF_ADD_TO_DISTMEM_TIMER_CHAIN()

#endif /* SAC_DO_PROFILE_DISTMEM && SAC_DO_PROFILE */

/*
 * Macros for DEBUG profiling
 */

#ifdef DEBUG_PROFILING

#include <stdio.h>

#define DEBUG_PROFILE_PRINT(record)                                                      \
    {                                                                                    \
        SAC_PROFILE_RECORD *tmp;                                                         \
        tmp = SAC_PF_act_record;                                                         \
        while (tmp != NULL) {                                                            \
            fprintf (stderr, "funno: %d funapno: %d record_type: %d\n", tmp->funno,      \
                     tmp->funapno, tmp->record_type);                                    \
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

#else /* DBUG_PROFILING */
#define DEBUG_PROFILE_ENTER(record)
#define DEBUG_PROFILE_LEAVE(record)

#endif /* DBUG_PROFILING */

/*
 *  Macros for profiling function applications
 */

#if (SAC_DO_PROFILE_FUN && SAC_DO_PROFILE)

#define SAC_PF_BEGIN_FUNCTION_START()                                                    \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);

#define SAC_PF_BEGIN_FUNCTION_END()                                                      \
        DEBUG_PROFILE_ENTER (SAC_PF_act_record);                                         \
        SAC_PF_START_CLOCK ();

#define SAC_PF_END_FUNCTION_START()                                                      \
        SAC_PF_STOP_CLOCK ();                                                            \
        SAC_PF_ADD_TO_TIMER_CHAIN (SAC_PF_act_record);

#define SAC_PF_END_FUNCTION_END()                                                        \
        DEBUG_PROFILE_LEAVE (SAC_PF_act_record);                                         \
        SAC_PF_START_CLOCK ();

#define SAC_PF_DISPLAY_FUN 1

#else /* SAC_DO_PROFILE_FUN */

#define SAC_PF_BEGIN_FUNCTION_START()
#define SAC_PF_BEGIN_FUNCTION_END()
#define SAC_PF_END_FUNCTION_START()
#define SAC_PF_END_FUNCTION_END()
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

/*
 * Macros for profiling memory opertions
 */

#if (SAC_DO_PROFILE_MEM && SAC_DO_PROFILE)

#define SAC_PF_DISPLAY_MEM 1

#define SAC_PF_MEM_DEFINE_EXTMOD()                                                       \
    SAC_C_EXTERN SAC_PF_MEMORY_RECORD **SAC_PF_memory;                                   \
    SAC_C_EXTERN SAC_PF_MEMORY_RECORD *SAC_PF_memory_record;

#define SAC_PF_MEM_DEFINE_EXTLOC()                                                       \
    SAC_C_EXTERN SAC_PF_MEMORY_RECORD SAC_PF_memory[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];   \
    SAC_C_EXTERN SAC_PF_MEMORY_RECORD *SAC_PF_memory_record;

#define SAC_PF_MEM_DEFINE_LOC()                                                          \
    SAC_PF_MEMORY_RECORD SAC_PF_memory[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                \
    SAC_PF_MEMORY_RECORD *SAC_PF_memory_record;

#define SAC_PF_MEM_INIT_RECORD(mem_record)                                               \
    mem_record.alloc_mem_count = mem_record.free_mem_count = 0;                          \
    mem_record.alloc_desc_count = mem_record.free_desc_count = 0;                        \
    mem_record.reuse_mem_count = 0;

#define SAC_PF_MEM_SETUP()                                                               \
    for (i = 0; i < SAC_SET_MAXFUN; i++) {                                               \
        for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                       \
            SAC_PF_MEM_INIT_RECORD (SAC_PF_memory[i][j]);                                \
        }                                                                                \
    }                                                                                    \
    SAC_PF_memory_record = &SAC_PF_memory[0][0];

#define SAC_PF_MEM_INC_ALLOC(size)                                                       \
    SAC_PF_MEM_AllocMemcnt (size);                                                       \
    SAC_PF_MEM_AddToMax (size);                                                          \
    SAC_PF_memory_record->alloc_mem_count += 1;

#define SAC_PF_MEM_INC_FREE(size)                                                        \
    SAC_PF_MEM_FreeMemcnt (size);                                                        \
    SAC_PF_memory_record->free_mem_count += 1;

#define SAC_PF_MEM_INC_ALLOC_DESC(size)                                                  \
    SAC_PF_MEM_AllocDescnt (size);                                                       \
    SAC_PF_memory_record->alloc_desc_count += 1;

#define SAC_PF_MEM_INC_FREE_DESC(size)                                                   \
    SAC_PF_MEM_FreeDescnt (size);                                                        \
    SAC_PF_memory_record->free_desc_count += 1;

#define SAC_PF_MEM_INC_REUSE()                                                           \
    SAC_PF_MEM_ReuseMemcnt ();                                                           \
    SAC_PF_memory_record->reuse_mem_count += 1;

#define SAC_PF_MEM_PRINT_STAT()                                                          \
    {                                                                                    \
        int i;                                                                           \
        SAC_PF_MEM_PrintStats ();                                                        \
        for (i = 0; i < SAC_SET_MAXFUN; i += 1) {                                        \
            SAC_PF_MEM_PrintFunStats (SAC_PF_fun_name[i], SAC_PF_maxfunap[i],            \
                                      SAC_PF_memory[i]);                                 \
        }                                                                                \
    }

#define SAC_PF_BEGIN_MEM()                                                               \
    SAC_PF_memory_record                                                                 \
      = &SAC_PF_memory[SAC_PF_act_record->funno][SAC_PF_act_record->funapno];

#define SAC_PF_END_MEM()                                                                 \
    SAC_PF_memory_record                                                                 \
      = &SAC_PF_memory[SAC_PF_act_record->funno][SAC_PF_act_record->funapno];

#else /* SAC_DO_PROFILE_MEM */

#define SAC_PF_DISPLAY_MEM 0

#define SAC_PF_MEM_DEFINE_EXTMOD()
#define SAC_PF_MEM_DEFINE_EXTLOC()
#define SAC_PF_MEM_DEFINE_LOC()
#define SAC_PF_MEM_INIT_RECORD(mem_record)
#define SAC_PF_MEM_SETUP()
#define SAC_PF_BEGIN_MEM()
#define SAC_PF_END_MEM()
#define SAC_PF_MEM_INC_ALLOC(size)
#define SAC_PF_MEM_INC_FREE(size)
#define SAC_PF_MEM_INC_ALLOC_DESC(size)
#define SAC_PF_MEM_INC_FREE_DESC(size)
#define SAC_PF_MEM_INC_REUSE()
#define SAC_PF_MEM_PRINT_STAT()

#endif /* SAC_DO_PROFILE_MEM */


#if (SAC_DO_PROFILE_OPS && SAC_DO_PROFILE)

#define SAC_PF_DISPLAY_OPS 1

#define SAC_PF_OPS_DEFINE_EXTMOD()                                                       \
    SAC_C_EXTERN SAC_PF_OPS_RECORD **SAC_PF_ops;                                         \
    SAC_C_EXTERN SAC_PF_OPS_RECORD *SAC_PF_ops_record;

#define SAC_PF_OPS_DEFINE_EXTLOC()                                                       \
    SAC_C_EXTERN SAC_PF_OPS_RECORD SAC_PF_ops[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];         \
    SAC_C_EXTERN SAC_PF_OPS_RECORD *SAC_PF_ops_record;

#define SAC_PF_OPS_DEFINE_LOC()                                                          \
    SAC_PF_OPS_RECORD SAC_PF_ops[SAC_SET_MAXFUN][SAC_SET_MAXFUNAP];                      \
    SAC_PF_OPS_RECORD *SAC_PF_ops_record;

#define SAC_PF_OPS_INIT_RECORD(ops_record)                                               \
    ops_record.ops_count[T_int] = 0;                                                     \
    ops_record.ops_count[T_double] = 0;                                                  \
    ops_record.ops_count[T_float] = 0;

#define SAC_PF_OPS_SETUP()                                                               \
    for (i = 0; i < SAC_SET_MAXFUN; i++) {                                               \
        for (j = 0; j < SAC_PF_maxfunap[i]; j++) {                                       \
            SAC_PF_OPS_INIT_RECORD (SAC_PF_ops[i][j]);                                   \
        }                                                                                \
    }                                                                                    \
    SAC_PF_ops_record = &SAC_PF_ops[0][0];

#define SAC_PF_OPS_PRINT_STAT()                                                          \
    {                                                                                    \
        size_t i;                                                                           \
        SAC_PF_OPS_PrintStats ();                                                        \
        for (i = 0; i < SAC_SET_MAXFUN; i += 1) {                                        \
            SAC_PF_OPS_PrintFunStats (SAC_PF_fun_name[i], SAC_PF_maxfunap[i],            \
                                      SAC_PF_ops[i]);                                    \
        }                                                                                \
    }

#define SAC_PF_OPS_INC_PRF( prf, type)                                                   \
    SAC_PF_OPS_IncPrf (T_int)


#else /* SAC_DO_PROFILE_OPS */


#define SAC_PF_DISPLAY_OPS 0

#define SAC_PF_OPS_DEFINE_EXTMOD()
#define SAC_PF_OPS_DEFINE_EXTLOC()
#define SAC_PF_OPS_DEFINE_LOC()
#define SAC_PF_OPS_INIT_RECORD(ops_record)
#define SAC_PF_OPS_SETUP()
#define SAC_PF_OPS_PRINT_STAT()

#define SAC_PF_OPS_INC_PRF( prf, type)

#endif /* SAC_DO_PROFILE_OPS */


#endif /* SAC_MUTC_MACROS */
#endif /* _SAC_RT_PROFILE_H */

