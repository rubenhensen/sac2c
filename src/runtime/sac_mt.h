/*
 *
 * $Log$
 * Revision 3.48  2003/09/15 13:00:34  dkr
 * SAC_MT_MIN, SAC_MT_MAX replaced by SAC_MIN, SAC_MAX
 *
 * Revision 3.47  2003/04/14 17:38:34  sbs
 * in SAC_MT_SCHEDULER_Block_DIM0: casted iterations_rest into unsigned int
 * for compatibility with SAC_MT_MYTHREAD() in comparison.
 *
 * Revision 3.46  2003/03/20 13:53:50  sbs
 * config.h included; NEED_REENTRANT used.
 *
 * Revision 3.45  2002/07/16 12:51:49  dkr
 * code beautified
 *
 * Revision 3.43  2002/03/07 20:28:15  dkr
 * ';' added for SAC_MT_DECL_MYTHREAD
 *
 * Revision 3.42  2001/11/28 13:58:26  sbs
 * definition of global arrays of length 0 avoided.
 * (required for OSX_MAC).
 *
 * Revision 3.41  2001/08/01 20:09:53  ben
 * one minor bug fixed (Affinity_NEXT_TASK)
 *
 * Revision 3.40  2001/07/24 11:53:21  ben
 * some warnings for factoring removed
 *
 * Revision 3.39  2001/07/10 09:22:13  ben
 * Affinity : SAC_MT_maxloadthread, SAC_MT_mintask are now local variables
 *
 * Revision 3.38  2001/07/06 10:16:33  ben
 * one minor bug in SAC_SET_TASKS removed
 *
 * Revision 3.37  2001/07/04 10:02:32  ben
 * code beautiefied
 *
 * Revision 3.36  2001/06/29 11:10:06  sbs
 * application of CAT1 macro eliminated to prevent from
 * gcc 3.0 warning.
 *
 * Revision 3.35  2001/06/29 08:21:53  ben
 * one small optimation added to TS_Even, but nit yet tested
 *
 * Revision 3.34  2001/06/28 11:28:44  ben
 * one warning using TS_Even removed
 *
 * Revision 3.33  2001/06/28 11:15:13  ben
 * one minor optimation added to TS_Even
 *
 * Revision 3.32  2001/06/28 09:51:20  sbs
 * prep concat eliminated in definition of SAC_MT_SPMD_SETARG_out_rc.
 *
 * Revision 3.31  2001/06/27 14:36:09  ben
 * modified for cooperation with tasksel-pragma
 *
 * Revision 3.30  2001/06/20 12:24:14  ben
 * some tracing output added for Selfscheduling
 *
 * Revision 3.29  2001/06/19 12:32:34  ben
 * SCHEDULER_SET_TASKS modified with parameter max_tasks
 *
 * Revision 3.28  2001/06/15 12:42:24  ben
 * one minor bug in Factoring_INIT fixed
 *
 * Revision 3.27  2001/06/15 12:33:52  ben
 * Factoring debbuged and optimized
 *
 * Revision 3.26  2001/06/12 11:01:53  ben
 *   SAC_MT_SCHEDULER_Self called without tasks_per_thread now
 *
 * Revision 3.25  2001/06/05 09:54:02  ben
 *  SAC_MT_SCHEDULER_TS_Even modified
 * SAC_MT_SCHEDULER_Affinity_INIT added
 *
 * Revision 3.24  2001/05/30 12:24:11  ben
 * SAC_MT_SCHEDULER_TS_Factoring_INIT and
 *  SAC_MT_SCHEDULER_TS_Factoring implemented
 *
 * Revision 3.23  2001/05/23 09:44:24  ben
 * some minor bugs in .._NEXT_TASK fixed
 *
 * Revision 3.22  2001/05/21 12:49:24  ben
 * SAC_MT_TASK and SAC_MT_TASK_LOCK modified for using
 * SAC_MT_SET_NUM_SCHEDULERS
 *
 * Revision 3.21  2001/05/16 10:20:11  ben
 * RESET_TASKS changed to SET_TASKS
 *
 * Revision 3.20  2001/05/04 11:51:01  ben
 * Select_block renamed to SAC_MT_SCHEDULER_TS_Even
 * SAC_MT_SCHEDULER_..._FIRST_TASK/NEXT_TASK added
 * SAC_MT_SCHEDULER_TS_Even optimized (% eliminated,...)
 * SAC_MT_SCHEDULER_Afs_next_task renamed to Affinity_NEXT_TASK
 * in SAC_MT_SCHEDULER_Affinity_NEXT_TASK some paramters renamed for better
 *   understanding
 *
 * Revision 3.19  2001/04/26 15:12:28  dkr
 * declarations of volatile pointers to non-volatile objects are correct
 * now
 *
 * Revision 3.18  2001/04/04 07:47:29  ben
 * SAC_MT_SCHEDULER_Select_Block modified for better tasksizes
 *
 */

/*****************************************************************************
 *
 * file:   sac_mt.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the multi-threading
 *   facilities.
 *
 *****************************************************************************/

#ifndef _SAC_MT_H_
#define _SAC_MT_H_

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

/***
 ***   Definitions and declarations for the multi-threaded runtime system
 ***/

#ifndef SAC_SET_MAX_SYNC_FOLD
#define SAC_SET_MAX_SYNC_FOLD 0
#endif

#ifndef SAC_SET_CACHE_1_LINE
#define SAC_SET_CACHE_1_LINE 0
#endif

#ifndef SAC_SET_CACHE_2_LINE
#define SAC_SET_CACHE_2_LINE 0
#endif

#ifndef SAC_SET_CACHE_3_LINE
#define SAC_SET_CACHE_3_LINE 0
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#include "config.h"

/*
 * On some systems _REENTRANT has to be defined before including system libs
 * when intending to create thread-safe code. POSIX does not require this.
 * However, it usually does not harm!
 * One exception is the ALPHA, where _REENTRANT is defined within pthread.h
 * which conflicts a prior definition. Therefore, we better do not preset
 * _REENTRANT on that system.
 */
#ifdef NEED_REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>

/*
 * Definition of synchronisation barrier data type.
 */

typedef struct ARRAYRC {
    /*
     * REMARK:
     *
     * volatile void * ptr
     * void volatile * ptr
     *    'ptr' is a non-volatile pointer to a volatile object
     *
     * 'void * volatile ptr'
     *    'ptr' is a volatile pointer to a non-volatile object
     */
    void *volatile ptr;
    int *volatile rc;
} arrayrc_t;

typedef union {
    union {
        volatile int result_int;
        volatile float result_float;
        volatile double result_double;
        volatile char result_char;
        void *volatile result_array;
        volatile arrayrc_t result_array_rc;
        void *volatile result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_dummy_t;

#define SAC_MT_CACHE_LINE_MAX()                                                          \
    SAC_MAX (SAC_SET_CACHE_1_LINE,                                                       \
             SAC_MAX (SAC_SET_CACHE_2_LINE, SAC_MAX (SAC_SET_CACHE_3_LINE, 1)))

#define SAC_MT_BARRIER_OFFSET()                                                          \
    ((SAC_MT_CACHE_LINE_MAX () >= sizeof (SAC_MT_barrier_dummy_t))                       \
       ? SAC_MT_CACHE_LINE_MAX ()                                                        \
       : (SAC_MT_CACHE_LINE_MAX ()                                                       \
          * (((sizeof (SAC_MT_barrier_dummy_t) - 1) / SAC_MT_CACHE_LINE_MAX ()) + 1)))

typedef union {
    char dummy[SAC_MT_BARRIER_OFFSET ()];
    union {
        /*
         * REMARK:
         *
         * volatile void * ptr
         * void volatile * ptr
         *    'ptr' is a non-volatile pointer to a volatile object
         *
         * 'void * volatile ptr'
         *    'ptr' is a volatile pointer to a non-volatile object
         */
        volatile int result_int;
        volatile float result_float;
        volatile double result_double;
        volatile char result_char;
        void *volatile result_array;
        volatile arrayrc_t result_array_rc;
        void *volatile result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_t;

/*
 *  Basic access macros for synchronisation barrier.
 */

#define SAC_MT_BARRIER_READY(barrier, n) (barrier[n].b[0].result_int)

#define SAC_MT_BARRIER_RESULT(barrier, n, m, type) (barrier[n].b[m].result_##type)

#define SAC_MT_BARRIER_RC_RESULT_PTR(barrier, n, m, type)                                \
    (CAT1 (barrier[n].b[m].result_, type).ptr)
#define SAC_MT_BARRIER_RC_RESULT_RC(barrier, n, m, type)                                 \
    (CAT1 (barrier[n].b[m].result_, type).rc)

/*
 *  Advanced access macros for synchronisation barrier.
 */

#define SAC_MT_CLEAR_BARRIER(n) SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = 0;

#define SAC_MT_SET_BARRIER(n, m) SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = m;

#define SAC_MT_CHECK_BARRIER(n) (SAC_MT_BARRIER_READY (SAC_MT_barrier, n))

#define SAC_MT_SET_BARRIER_RESULT(n, m, type, res)                                       \
    {                                                                                    \
        SAC_MT_BARRIER_RESULT (SAC_MT_barrier, n, m, type) = res;                        \
        SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = m;                                    \
    }

#define SAC_MT_SET_BARRIER_RC_RESULT(n, m, type, res)                                    \
    {                                                                                    \
        SAC_MT_BARRIER_RC_RESULT_PTR (SAC_MT_barrier, n, m, type) = res;                 \
        SAC_MT_BARRIER_RC_RESULT_RC (SAC_MT_barrier, n, m, type) = CAT1 (res, __rc);     \
        SAC_MT_BARRIER_READY (SAC_MT_barrier, n) = m;                                    \
    }

#define SAC_MT_GET_BARRIER_RESULT(n, m, type)                                            \
    (SAC_MT_BARRIER_RESULT (SAC_MT_barrier, n, m, type))

#define SAC_MT_GET_BARRIER_RESULT(n, m, type)                                            \
    (SAC_MT_BARRIER_RESULT (SAC_MT_barrier, n, m, type))

#define SAC_MT_GET_BARRIER_RC_RESULT_PTR(n, m, type)                                     \
    (SAC_MT_BARRIER_RC_RESULT_PTR (SAC_MT_barrier, n, m, type))
#define SAC_MT_GET_BARRIER_RC_RESULT_RC(n, m, type)                                      \
    (SAC_MT_BARRIER_RC_RESULT_RC (SAC_MT_barrier, n, m, type))

/*
 *  Definition of macro implemented ICMs for global symbol definitions
 */

#if SAC_SET_NUM_SCHEDULERS

#define SAC_MT_DEFINE()                                                                  \
    SAC_MT_DEFINE_BARRIER ()                                                             \
    SAC_MT_DEFINE_SPMD_FRAME ()                                                          \
    SAC_MT_DEFINE_TASKLOCKS ()                                                           \
    SAC_MT_DEFINE_TS_TASKLOCKS ()                                                        \
    SAC_MT_DEFINE_TASKS ()                                                               \
    SAC_MT_DEFINE_LAST_TASKS ()                                                          \
    SAC_MT_DEFINE_REST_ITERATIONS ()                                                     \
    SAC_MT_DEFINE_ACT_TASKSIZE ()                                                        \
    SAC_MT_DEFINE_LAST_TASKEND ()                                                        \
    SAC_MT_DEFINE_TASKCOUNT ()

#else /* SAC_SET_NUM_SCHEDULERS == 0 */

/*
 * If (SAC_SET_NUM_SCHEDULERS == 0) the macros SAC_MT_DEFINE_TASKS(), etc.
 * would lead to definitions of global arrays of size 0. On OSX_MAC this
 * leads to a linker error!!
 * Unfortunately, libsac_mt.a requires SAC_MT_Tasklock and SAC_MT_TS_Tasklock
 * to be defined. Therefore, we have to insert dummy definitions for them.
 */

#define SAC_MT_DEFINE()                                                                  \
    SAC_MT_DEFINE_BARRIER ()                                                             \
    SAC_MT_DEFINE_SPMD_FRAME ()                                                          \
    SAC_MT_DEFINE_DUMMY_TASKLOCKS ()                                                     \
    SAC_MT_DEFINE_DUMMY_TS_TASKLOCKS ()

#endif

#define SAC_MT_DEFINE_BARRIER()                                                          \
    volatile SAC_MT_barrier_t SAC_MT_barrier_space[SAC_SET_THREADS_MAX + 1];

#define SAC_MT_DEFINE_SPMD_FRAME()                                                       \
    static volatile union SAC_SET_SPMD_FRAME SAC_MT_spmd_frame;

/*
 * Macros for datastructures of scheduling
 */

#define SAC_MT_DEFINE_TASKLOCKS()                                                        \
    pthread_mutex_t SAC_MT_Tasklock[SAC_SET_THREADS_MAX * SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_DEFINE_DUMMY_TASKLOCKS() pthread_mutex_t SAC_MT_Tasklock[1];

#define SAC_MT_TASKLOCK(sched_id, num)                                                   \
    SAC_MT_Tasklock[num + SAC_SET_NUM_SCHEDULERS * sched_id]

#define SAC_MT_TASKLOCK_INIT(sched_id, num, num_sched)                                   \
    SAC_MT_Tasklock[num + num_sched * sched_id]

#define SAC_MT_DEFINE_TASKS()                                                            \
    volatile int SAC_MT_Task[SAC_SET_THREADS_MAX * SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_TASK(sched_id, num) SAC_MT_Task[num + SAC_SET_NUM_SCHEDULERS * sched_id]

#define SAC_MT_DEFINE_LAST_TASKS()                                                       \
    volatile int SAC_MT_LAST_Task[SAC_SET_THREADS_MAX * SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_LAST_TASK(sched_id, num)                                                  \
    SAC_MT_LAST_Task[num + SAC_SET_NUM_SCHEDULERS * sched_id]

#define SAC_MT_DEFINE_REST_ITERATIONS()                                                  \
    volatile int SAC_MT_rest_iterations[SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_REST_ITERATIONS(sched_id) SAC_MT_rest_iterations[sched_id]

#define SAC_MT_DEFINE_ACT_TASKSIZE()                                                     \
    volatile int SAC_MT_act_tasksize[SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_ACT_TASKSIZE(sched_id) SAC_MT_act_tasksize[sched_id]

#define SAC_MT_DEFINE_LAST_TASKEND()                                                     \
    volatile int SAC_MT_last_taskend[SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_LAST_TASKEND(sched_id) SAC_MT_last_taskend[sched_id]

#define SAC_MT_DEFINE_TS_TASKLOCKS()                                                     \
    pthread_mutex_t SAC_MT_TS_Tasklock[SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_DEFINE_DUMMY_TS_TASKLOCKS() pthread_mutex_t SAC_MT_TS_Tasklock[1];

#define SAC_MT_TS_TASKLOCK(sched_id) SAC_MT_TS_Tasklock[sched_id]

#define SAC_MT_DEFINE_TASKCOUNT() volatile int SAC_MT_Taskcount[SAC_SET_NUM_SCHEDULERS];

#define SAC_MT_TASKCOUNT(sched_id) SAC_MT_Taskcount[sched_id]

#define SAC_MT_FUN_FRAME(name, blocks) struct blocks name;

#define SAC_MT_BLOCK_FRAME(name, args) struct args name;

#define SAC_MT_BLOCK_FRAME_DUMMY() int dummy;

#define I_POST(arg) arg
#define O_POST(arg) CAT0 (arg, _out)
#define S_POST(arg) CAT0 (arg, _shared)

#define SAC_MT_SPMD_ARG_in(type, name) type I_POST (name);

#define SAC_MT_SPMD_ARG_out(type, name) type *O_POST (name);

#define SAC_MT_SPMD_ARG_shared(type, name) type S_POST (name);

#define SAC_MT_SPMD_ARG_in_rc(type, name)                                                \
    type I_POST (name);                                                                  \
    int SAC_ND_A_RC (I_POST (name));

#define SAC_MT_SPMD_ARG_out_rc(type, name)                                               \
    type *O_POST (name);                                                                 \
    int *SAC_ND_A_RC (O_POST (name));

#define SAC_MT_SPMD_ARG_shared_rc(type, name)                                            \
    type S_POST (name);                                                                  \
    int SAC_ND_A_RC (S_POST (name));

#define SAC_MT_SPMD_ARG_preset(type, name) type name;

#define SAC_MT_SPMD_ARG_preset_rc(type, name)                                            \
    type name;                                                                           \
    int SAC_ND_A_RC (name);

/*
 *  Definition of macro implemented ICMs for setting up the multi-threaded
 *  runtime system.
 */

#if SAC_DO_TRACE_MT
#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_TR_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);
#else
#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);
#endif

#if SAC_DO_TRACE_MT
#define SAC_MT_SETUP()                                                                   \
    SAC_MT_TR_Setup (SAC_MT_CACHE_LINE_MAX (), SAC_MT_BARRIER_OFFSET (),                 \
                     SAC_SET_NUM_SCHEDULERS);
#else
#define SAC_MT_SETUP()                                                                   \
    SAC_MT_Setup (SAC_MT_CACHE_LINE_MAX (), SAC_MT_BARRIER_OFFSET (),                    \
                  SAC_SET_NUM_SCHEDULERS);
#endif

/*
 *  Definition of macro implemented ICMs for handling of spmd-function
 */

#define SAC_MT_SPMD_EXECUTE(name)                                                        \
    {                                                                                    \
        SAC_TR_MT_PRINT (("Parallel execution of spmd-block %s started.", #name));       \
        SAC_MT_spmd_function = &name;                                                    \
        SAC_MT_not_yet_parallel = 0;                                                     \
        SAC_MT_START_WORKERS ();                                                         \
        name (0, SAC_MT_MASTERCLASS (), 0);                                              \
        SAC_MT_not_yet_parallel = 1;                                                     \
        SAC_TR_MT_PRINT (("Parallel execution of spmd-block %s finished.", #name));      \
    }

#define SAC_MT_START_WORKERS() SAC_MT_master_flag = 1 - SAC_MT_master_flag;

#define SAC_MT_WORKER_WAIT()                                                             \
    {                                                                                    \
        while (SAC_MT_worker_flag == SAC_MT_master_flag)                                 \
            ;                                                                            \
        SAC_MT_worker_flag = SAC_MT_master_flag;                                         \
    }

#define SAC_MT_SPMD_FUN_REAL_PARAM_LIST()                                                \
    const unsigned int SAC_MT_mythread, const unsigned int SAC_MT_myworkerclass,         \
      unsigned int SAC_MT_worker_flag

#define SAC_MT_SPMD_FUN_REAL_RETURN() return (SAC_MT_worker_flag);

#define SAC_MT_SPMD_FUN_REAL_RETTYPE() static volatile unsigned int

#define SAC_MT_MYTHREAD() SAC_MT_mythread

#define SAC_MT_MYWORKERCLASS() SAC_MT_myworkerclass

#define SAC_MT_DECL_MYTHREAD() const unsigned int SAC_MT_mythread = 0;

#if 0
#define SAC_MT_DETERMINE_THREAD_ID()                                                     \
    const unsigned int SAC_MT_mythread                                                   \
      = *((unsigned int *)pthread_getspecific (SAC_MT_threadid_key));
#else
#define SAC_MT_DETERMINE_THREAD_ID()
#endif

#define SAC_MT_SPMD_SPECIAL_FRAME(spmdname)                                              \
    SAC_MT_spmd_frame.SAC_MT_CURRENT_FUN ().spmdname

#define SAC_MT_SPMD_CURRENT_FRAME                                                        \
    SAC_MT_spmd_frame.SAC_MT_CURRENT_FUN ().SAC_MT_CURRENT_SPMD ()

#define SAC_MT_SPMD_PARAM_in(type, param)                                                \
    type param = SAC_MT_SPMD_CURRENT_FRAME.I_POST (param);

#define SAC_MT_SPMD_PARAM_in_rc(type, param)                                             \
    type I_POST (param) = SAC_MT_SPMD_CURRENT_FRAME.I_POST (param);                      \
    int SAC_ND_A_RC (I_POST (param))                                                     \
      = SAC_MT_SPMD_CURRENT_FRAME.SAC_ND_A_RCP (I_POST (param));

#define SAC_MT_SPMD_PARAM_out(type, param)

#define SAC_MT_SPMD_PARAM_out_rc(type, param)

#define SAC_MT_SPMD_SETARG_in(spmdname, arg)                                             \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).I_POST (arg) = arg;

#define SAC_MT_SPMD_SETARG_in_rc(spmdname, arg)                                          \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).I_POST (arg) = arg;                             \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).SAC_ND_A_RCP (I_POST (arg)) = SAC_ND_A_RCP (arg);

#define SAC_MT_SPMD_SETARG_out(spmdname, arg)                                            \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).O_POST (arg) = &arg;

#define SAC_MT_SPMD_SETARG_out_rc(spmdname, arg)                                         \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).O_POST (arg) = &arg;                            \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).SAC_ND_A_RCP (O_POST (arg))                     \
      = &SAC_ND_A_RCP (arg);

#define SAC_MT_SPMD_SETARG_shared(spmdname, arg)                                         \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).S_POST (arg) = arg;

#define SAC_MT_SPMD_SETARG_shared_rc(spmdname, arg)                                      \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).S_POST (arg) = arg;                             \
    SAC_MT_SPMD_SPECIAL_FRAME (spmdname).SAC_ND_A_RCP (S_POST (arg)) = SAC_ND_A_RCP (arg);

#define SAC_MT_SPMD_RET_out(param) *(SAC_MT_SPMD_CURRENT_FRAME.O_POST (param)) = param;

#define SAC_MT_SPMD_RET_out_rc(param)                                                    \
    *(SAC_MT_SPMD_CURRENT_FRAME.O_POST (param)) = param;                                 \
    *(SAC_MT_SPMD_CURRENT_FRAME.SAC_ND_A_RCP (O_POST (param))) = SAC_ND_A_RCP (param);

#define SAC_MT_SPMD_RET_shared(param) SAC_MT_SPMD_CURRENT_FRAME.S_POST (param) = param;

#define SAC_MT_SPMD_RET_shared_rc(param)                                                 \
    SAC_MT_SPMD_CURRENT_FRAME.S_POST (param) = param;                                    \
    SAC_MT_SPMD_CURRENT_FRAME.SAC_ND_A_RCP (S_POST (param)) = SAC_ND_A_RCP (param);

#define SAC_MT_SPMD_GET_shared(param) param = SAC_MT_SPMD_CURRENT_FRAME.S_POST (param);

#define SAC_MT_SPMD_GET_shared_rc(param)                                                 \
    param = SAC_MT_SPMD_CURRENT_FRAME.S_POST (param);                                    \
    SAC_ND_A_RCP (param) = SAC_MT_SPMD_CURRENT_FRAME.SAC_ND_A_RCP (S_POST (param));

/*
 *  Macros for body of value exchange parts
 */

#define SAC_MT_MASTER_BEGIN(nlabel)                                                      \
    {                                                                                    \
        label_master_continue_##nlabel:

#define SAC_MT_MASTER_END(nlabel)                                                        \
    goto label_continue_##nlabel;                                                        \
    }

#define SAC_MT_WORKER_BEGIN(nlabel)                                                      \
    {                                                                                    \
        label_worker_continue_##nlabel:

#define SAC_MT_WORKER_END(nlabel) }

#define SAC_MT_RESTART(nlabel) label_continue_##nlabel:

/*
 *  Definition of macro implemented ICMs for synchronisation
 */

#define SAC_MT_SYNC_NONFOLD_1(id)                                                        \
    {                                                                                    \
        unsigned int i;                                                                  \
                                                                                         \
        for (i = 1; i <= SAC_MT_MYWORKERCLASS (); i <<= 1) {                             \
            SAC_TR_MT_PRINT (                                                            \
              ("Waiting for worker thread #%u.", SAC_MT_MYTHREAD () + i));               \
            while (!SAC_MT_CHECK_BARRIER (SAC_MT_MYTHREAD () + i))                       \
                ;                                                                        \
            SAC_MT_CLEAR_BARRIER (SAC_MT_MYTHREAD () + i);                               \
        }                                                                                \
                                                                                         \
        SAC_MT_SET_BARRIER (SAC_MT_MYTHREAD (), 1);                                      \
                                                                                         \
        SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));                     \
        if (SAC_MT_MYTHREAD ())                                                          \
            goto label_worker_continue_##id;                                             \
    }

#define SAC_MT_SYNC_ONEFOLD_1(type, accu_var, tmp_var, id)                               \
    {                                                                                    \
        SAC_TR_MT_PRINT_FOLD_RESULT (type, accu_var, "Pure thread fold result:");        \
                                                                                         \
        if (!SAC_MT_MYWORKERCLASS ()) {                                                  \
            SAC_MT_SET_BARRIER_RESULT (SAC_MT_MYTHREAD (), 1, type, accu_var);           \
            SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));                 \
            SAC_TR_MT_PRINT_FOLD_RESULT (type, accu_var, "Partial fold result:");        \
            if (SAC_MT_MYTHREAD ()) {                                                    \
                goto label_worker_continue_##id;                                         \
            }                                                                            \
            goto label_master_continue_##id;                                             \
        }                                                                                \
                                                                                         \
        if (SAC_MT_MYTHREAD ()) {                                                        \
            unsigned int SAC_MT_ready_count = SAC_MT_MYWORKERCLASS () >> 1;              \
            unsigned int SAC_MT_son_id;                                                  \
            unsigned int SAC_MT_i;                                                       \
                                                                                         \
            for (;;) {                                                                   \
                SAC_MT_i = SAC_MT_MYWORKERCLASS ();                                      \
                                                                                         \
                do {                                                                     \
                    SAC_MT_son_id = SAC_MT_MYTHREAD () + SAC_MT_i;                       \
                                                                                         \
                    if (SAC_MT_CHECK_BARRIER (SAC_MT_son_id)) {                          \
                        SAC_MT_CLEAR_BARRIER (SAC_MT_son_id);                            \
                        tmp_var = SAC_MT_GET_BARRIER_RESULT (SAC_MT_son_id, 1, type);

#define SAC_MT_SYNC_ONEFOLD_2(type, accu_var, tmp_var, id)                               \
    if (!SAC_MT_ready_count) {                                                           \
        SAC_MT_SET_BARRIER_RESULT (SAC_MT_MYTHREAD (), 1, type, accu_var);               \
        SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));                     \
        SAC_TR_MT_PRINT_FOLD_RESULT (type, accu_var, "Partial fold result:");            \
        goto label_worker_continue_##id;                                                 \
    }                                                                                    \
    SAC_MT_ready_count >>= 1;                                                            \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_i >>= 1)                                                               \
        ;                                                                                \
    }                                                                                    \
    }                                                                                    \
    else                                                                                 \
    {                                                                                    \
        unsigned int SAC_MT_ready_count = SAC_MT_MASTERCLASS () >> 1;                    \
        unsigned int SAC_MT_son_id;                                                      \
                                                                                         \
        for (;;) {                                                                       \
            SAC_MT_son_id = SAC_MT_MASTERCLASS ();                                       \
                                                                                         \
            do {                                                                         \
                if (SAC_MT_CHECK_BARRIER (SAC_MT_son_id)) {                              \
                    SAC_MT_CLEAR_BARRIER (SAC_MT_son_id);                                \
                    tmp_var = SAC_MT_GET_BARRIER_RESULT (SAC_MT_son_id, 1, type);

#define SAC_MT_SYNC_ONEFOLD_3(type, accu_var, tmp_var, id)                               \
    if (!SAC_MT_ready_count) {                                                           \
        SAC_MT_SET_BARRIER_RESULT (SAC_MT_MYTHREAD (), 1, type, accu_var);               \
        SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));                     \
        SAC_TR_MT_PRINT_FOLD_RESULT (type, accu_var, "Partial fold result:");            \
        goto label_master_continue_##id;                                                 \
    }                                                                                    \
    SAC_MT_ready_count >>= 1;                                                            \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_son_id >>= 1)                                                          \
        ;                                                                                \
    }                                                                                    \
    }                                                                                    \
    }

#define SAC_MT_SYNC_MULTIFOLD_1A(id) {

#define SAC_MT_SYNC_MULTIFOLD_1B(id) if (!SAC_MT_MYWORKERCLASS ()) {

#define SAC_MT_SYNC_MULTIFOLD_1C(id)                                                     \
    SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));

#define SAC_MT_SYNC_MULTIFOLD_1D(id)                                                     \
    if (SAC_MT_MYTHREAD ()) {                                                            \
        goto label_worker_continue_##id;                                                 \
    }                                                                                    \
    goto label_master_continue_##id;                                                     \
    }                                                                                    \
                                                                                         \
    if (SAC_MT_MYTHREAD ()) {                                                            \
        unsigned int SAC_MT_ready_count = SAC_MT_MYWORKERCLASS () >> 1;                  \
        unsigned int SAC_MT_son_id;                                                      \
        unsigned int SAC_MT_i;                                                           \
                                                                                         \
        for (;;) {                                                                       \
            SAC_MT_i = SAC_MT_MYWORKERCLASS ();                                          \
                                                                                         \
            do {                                                                         \
                SAC_MT_son_id = SAC_MT_MYTHREAD () + SAC_MT_i;                           \
                                                                                         \
                if (SAC_MT_CHECK_BARRIER (SAC_MT_son_id)) {                              \
                    SAC_MT_CLEAR_BARRIER (SAC_MT_son_id);

#define SAC_MT_SYNC_MULTIFOLD_2A(id) if (!SAC_MT_ready_count) {

#define SAC_MT_SYNC_MULTIFOLD_2B(id)                                                     \
    SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));

#define SAC_MT_SYNC_MULTIFOLD_2C(id)                                                     \
    goto label_worker_continue_##id;                                                     \
    }                                                                                    \
    SAC_MT_ready_count >>= 1;                                                            \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_i >>= 1)                                                               \
        ;                                                                                \
    }                                                                                    \
    }                                                                                    \
    else                                                                                 \
    {                                                                                    \
        unsigned int SAC_MT_ready_count = SAC_MT_MASTERCLASS () >> 1;                    \
        unsigned int SAC_MT_son_id;                                                      \
                                                                                         \
        for (;;) {                                                                       \
            SAC_MT_son_id = SAC_MT_MASTERCLASS ();                                       \
                                                                                         \
            do {                                                                         \
                if (SAC_MT_CHECK_BARRIER (SAC_MT_son_id)) {                              \
                    SAC_MT_CLEAR_BARRIER (SAC_MT_son_id);

#define SAC_MT_SYNC_MULTIFOLD_3A(id) if (!SAC_MT_ready_count) {

#define SAC_MT_SYNC_MULTIFOLD_3B(id)                                                     \
    SAC_TR_MT_PRINT (("Synchronisation block %d finished", id));

#define SAC_MT_SYNC_MULTIFOLD_3C(id)                                                     \
    goto label_master_continue_##id;                                                     \
    }                                                                                    \
    SAC_MT_ready_count >>= 1;                                                            \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_son_id >>= 1)                                                          \
        ;                                                                                \
    }                                                                                    \
    }                                                                                    \
    }

/*
 * Definition of macros implementing a general locking mechanism
 */

#define SAC_MT_DEFINE_LOCK(name) pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER;

#define SAC_MT_DECLARE_LOCK(name) extern pthread_mutex_t name;

#define SAC_MT_ACQUIRE_LOCK(name) pthread_mutex_lock (&name);

#define SAC_MT_RELEASE_LOCK(name) pthread_mutex_unlock (&name);

/*
 *  Definitions of macro-implemented ICMs for scheduling
 */

#define SAC_MT_ADJUST_SCHEDULER_BOUND(diff, bound, lower, upper, unrolling)              \
    {                                                                                    \
        diff = 0;                                                                        \
        if (((bound) > (lower)) && ((bound) < (upper))) {                                \
            diff = ((bound) - (lower)) % (unrolling);                                    \
                                                                                         \
            if (diff) {                                                                  \
                diff = (unrolling)-diff;                                                 \
                                                                                         \
                if ((bound) + diff >= (upper)) {                                         \
                    diff = (upper) - (bound);                                            \
                    (bound) = (upper);                                                   \
                } else {                                                                 \
                    (bound) += diff;                                                     \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define SAC_MT_ADJUST_SCHEDULER__OFFSET(to_nt, dims, dim, lower, upper, unrolling,       \
                                        offset)                                          \
    {                                                                                    \
        int diff_start, diff_stop;                                                       \
                                                                                         \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_start, SAC_WL_MT_SCHEDULE_START (dim),       \
                                       lower, upper, unrolling)                          \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_stop, SAC_WL_MT_SCHEDULE_STOP (dim), lower,  \
                                       upper, unrolling)                                 \
                                                                                         \
        SAC_WL_OFFSET (to_nt) += diff_start * (offset);                                  \
        SAC_TR_MT_PRINT (("Scheduler Adjustment: dim %d: %d -> %d", dim,                 \
                          SAC_WL_MT_SCHEDULE_START (dim),                                \
                          SAC_WL_MT_SCHEDULE_STOP (dim)));                               \
    }

#define SAC_MT_ADJUST_SCHEDULER(to_nt, dims, dim, lower, upper, unrolling)               \
    {                                                                                    \
        int diff_start, diff_stop;                                                       \
                                                                                         \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_start, SAC_WL_MT_SCHEDULE_START (dim),       \
                                       lower, upper, unrolling)                          \
        SAC_MT_ADJUST_SCHEDULER_BOUND (diff_stop, SAC_WL_MT_SCHEDULE_STOP (dim), lower,  \
                                       upper, unrolling)                                 \
                                                                                         \
        SAC_TR_MT_PRINT (("Scheduler Adjustment: dim %d: %d -> %d", dim,                 \
                          SAC_WL_MT_SCHEDULE_START (dim),                                \
                          SAC_WL_MT_SCHEDULE_STOP (dim)));                               \
    }

#define SAC_MT_SCHEDULER_Static_BEGIN()

#define SAC_MT_SCHEDULER_Static_END()

#define SAC_MT_SCHEDULER_Block_DIM0(lower, upper, unrolling)                             \
    {                                                                                    \
        const int iterations = (upper - lower) / unrolling;                              \
        const int iterations_per_thread = (iterations / SAC_MT_THREADS ()) * unrolling;  \
        const int iterations_rest = iterations % SAC_MT_THREADS ();                      \
                                                                                         \
        if (iterations_rest && (SAC_MT_MYTHREAD () < (unsigned int)iterations_rest)) {   \
            SAC_WL_MT_SCHEDULE_START (0)                                                 \
              = lower + SAC_MT_MYTHREAD () * (iterations_per_thread + unrolling);        \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + (iterations_per_thread + unrolling);      \
        } else {                                                                         \
            SAC_WL_MT_SCHEDULE_START (0) = (lower + iterations_rest * unrolling)         \
                                           + SAC_MT_MYTHREAD () * iterations_per_thread; \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + iterations_per_thread;                    \
        }                                                                                \
                                                                                         \
        SAC_TR_MT_PRINT (("Scheduler 'Block': dim 0: %d -> %d",                          \
                          SAC_WL_MT_SCHEDULE_START (0), SAC_WL_MT_SCHEDULE_STOP (0)));   \
    }

#define SAC_MT_SCHEDULER_BlockVar_DIM0(lower, upper, unrolling)                          \
    {                                                                                    \
        SAC_MT_SCHEDULER_Block_DIM0 (lower, upper, unrolling)                            \
    }

/*
 * TS_Even is the same as Block_DIM0 with two changes
 * first it gets the dimension for dividing tasks
 * second it divides in num_tasks*SAC_MT_THREADS() tasks
 */

#define SAC_MT_SCHEDULER_TS_Even(sched_id, tasks_on_dim, lower, upper, unrolling,        \
                                 num_tasks, taskid, worktodo)                            \
    {                                                                                    \
        const int number_of_tasks = num_tasks * SAC_MT_THREADS ();                       \
        const int iterations = (upper - lower) / unrolling;                              \
        const int iterations_per_thread = (iterations / number_of_tasks) * unrolling;    \
        const int iterations_rest = iterations % number_of_tasks;                        \
        SAC_WL_MT_SCHEDULE_START (tasks_on_dim) = 0;                                     \
        SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim) = 0;                                      \
                                                                                         \
        worktodo = (taskid < number_of_tasks);                                           \
        if (worktodo) {                                                                  \
            if (iterations_rest && (taskid < iterations_rest)) {                         \
                SAC_WL_MT_SCHEDULE_START (tasks_on_dim)                                  \
                  = lower + (iterations_per_thread + unrolling) * (taskid);              \
                SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                   \
                  = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + iterations_per_thread      \
                    + unrolling;                                                         \
            } else {                                                                     \
                SAC_WL_MT_SCHEDULE_START (tasks_on_dim)                                  \
                  = (lower + iterations_rest * unrolling)                                \
                    + taskid * iterations_per_thread;                                    \
                SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                   \
                  = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + iterations_per_thread;     \
            }                                                                            \
            SAC_TR_MT_PRINT (("'TS_Even': dim %d: %d -> %d, Task: %d", tasks_on_dim,     \
                              SAC_WL_MT_SCHEDULE_START (tasks_on_dim),                   \
                              SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim), taskid));          \
        }                                                                                \
    }

#define SAC_MT_SCHEDULER_TS_Factoring_INIT(sched_id, lower, upper)                       \
    {                                                                                    \
        SAC_MT_REST_ITERATIONS (sched_id) = (upper - lower);                             \
        SAC_MT_LAST_TASKEND (sched_id) = 0;                                              \
        SAC_MT_TASKCOUNT (sched_id) = 0;                                                 \
        SAC_MT_ACT_TASKSIZE (sched_id)                                                   \
          = ((SAC_MT_REST_ITERATIONS (sched_id)) / (2 * SAC_MT_threads)) + 1;            \
    }

/*
 * TS_Factoring divides the With Loop in Blocks with decreasing size
 * every SAC_MT_THREADS taskss have the same size and then a new tasksize
 * will be computed
 */

#define SAC_MT_SCHEDULER_TS_Factoring(sched_id, tasks_on_dim, lower, upper, unrolling,   \
                                      taskid, worktodo)                                  \
    {                                                                                    \
        int tasksize;                                                                    \
        int restiter;                                                                    \
        SAC_WL_MT_SCHEDULE_START (tasks_on_dim) = 0;                                     \
        SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim) = 0;                                      \
                                                                                         \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TS_TASKLOCK (sched_id));                             \
                                                                                         \
        restiter = SAC_MT_REST_ITERATIONS (sched_id);                                    \
        worktodo = (restiter > 0);                                                       \
                                                                                         \
        if (worktodo) {                                                                  \
            if (SAC_MT_TASKCOUNT (sched_id) == SAC_MT_threads) {                         \
                SAC_MT_ACT_TASKSIZE (sched_id)                                           \
                  = ((restiter) / (2 * SAC_MT_threads)) + 1;                             \
                SAC_MT_TASKCOUNT (sched_id) = 0;                                         \
            }                                                                            \
            tasksize = SAC_MT_ACT_TASKSIZE (sched_id);                                   \
            (SAC_MT_TASKCOUNT (sched_id))++;                                             \
                                                                                         \
            (SAC_WL_MT_SCHEDULE_START (tasks_on_dim)) = SAC_MT_LAST_TASKEND (sched_id);  \
                                                                                         \
            if (restiter < tasksize)                                                     \
                tasksize = restiter;                                                     \
            restiter -= tasksize;                                                        \
                                                                                         \
            SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim)                                       \
              = SAC_WL_MT_SCHEDULE_START (tasks_on_dim) + tasksize;                      \
                                                                                         \
            SAC_MT_ACT_TASKSIZE (sched_id) = tasksize;                                   \
            SAC_MT_REST_ITERATIONS (sched_id) = restiter;                                \
            SAC_MT_LAST_TASKEND (sched_id) = SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim);     \
                                                                                         \
            SAC_TR_MT_PRINT (("'TS_Factoring': dim %d: %d -> %d, Task: %d, Size: %d",    \
                              tasks_on_dim, SAC_WL_MT_SCHEDULE_START (tasks_on_dim),     \
                              SAC_WL_MT_SCHEDULE_STOP (tasks_on_dim),                    \
                              SAC_MT_TASKCOUNT (sched_id) - 1,                           \
                              SAC_MT_ACT_TASKSIZE (sched_id)));                          \
        }                                                                                \
        SAC_MT_RELEASE_LOCK (SAC_MT_TS_TASKLOCK (sched_id));                             \
    }

/*
 * SET_TASKS initiziale the first max_task taskqueues
 */

#define SAC_MT_SCHEDULER_SET_TASKS(sched_id, max_task)                                   \
    {                                                                                    \
        int i;                                                                           \
                                                                                         \
        for (i = 0; i <= max_task; i++)                                                  \
            SAC_MT_TASK (sched_id, i) = 0;                                               \
        SAC_TR_MT_PRINT (("SAC_MT_TASK set for sched_id %d", sched_id));                 \
    }

#define SAC_MT_SCHEDULER_Static_FIRST_TASK(taskid)                                       \
    {                                                                                    \
        taskid = SAC_MT_MYTHREAD ();                                                     \
    }

#define SAC_MT_SCHEDULER_Static_NEXT_TASK(taskid)                                        \
    {                                                                                    \
        taskid += SAC_MT_THREADS ();                                                     \
    }

#define SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC(sched_id, taskid)                        \
    {                                                                                    \
        taskid = SAC_MT_MYTHREAD ();                                                     \
        SAC_TR_MT_PRINT (("SAC_MT_SCHEDULER_Self_FIRST_TASK_STATIC"));                   \
    }

#define SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC(sched_id, taskid)                       \
    {                                                                                    \
        SAC_MT_SCHEDULER_Self_NEXT_TASK (sched_id, taskid);                              \
        SAC_TR_MT_PRINT (("SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC task=%d", taskid));  \
    }

#define SAC_MT_SCHEDULER_Self_NEXT_TASK(sched_id, taskid)                                \
    {                                                                                    \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, 0));                             \
                                                                                         \
        taskid = SAC_MT_TASK (sched_id, 0);                                              \
        (SAC_MT_TASK (sched_id, 0))++;                                                   \
                                                                                         \
        SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, 0));                             \
    }

/*
 * Affinity_INIT initialize the taskqueues for each thread for the
 * affinity scheduling
 */
#define SAC_MT_SCHEDULER_Affinity_INIT(sched_id, tasks_per_thread)                       \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < SAC_MT_THREADS (); i++) {                                        \
            SAC_MT_TASK (sched_id, i) = i * tasks_per_thread;                            \
            SAC_MT_LAST_TASK (sched_id, i) = (i + 1) * tasks_per_thread;                 \
        }                                                                                \
    }

#define SAC_MT_SCHEDULER_Affinity_FIRST_TASK(sched_id, tasks_per_thread, taskid,         \
                                             worktodo)                                   \
    {                                                                                    \
        SAC_MT_SCHEDULER_Affinity_NEXT_TASK (sched_id, tasks_per_thread, taskid,         \
                                             worktodo)                                   \
    }

#define SAC_MT_SCHEDULER_Affinity_NEXT_TASK(sched_id, tasks_per_thread, taskid,          \
                                            worktodo)                                    \
    {                                                                                    \
        int queueid, maxloadthread, mintask;                                             \
        worktodo = 0;                                                                    \
        taskid = 0;                                                                      \
                                                                                         \
        /* first look if MYTHREAD has work to do */                                      \
        SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_MYTHREAD ()));            \
                                                                                         \
        if (SAC_MT_TASK (sched_id, SAC_MT_MYTHREAD ())                                   \
            < SAC_MT_LAST_TASK (sched_id, SAC_MT_MYTHREAD ())) {                         \
            taskid = SAC_MT_TASK (sched_id, SAC_MT_MYTHREAD ());                         \
            (SAC_MT_TASK (sched_id, SAC_MT_MYTHREAD ()))++;                              \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_MYTHREAD ()));        \
            worktodo = 1;                                                                \
        } else {                                                                         \
            /*                                                                           \
             * if there was no work to do, find the task, which has done, the            \
             * smallest work till now                                                    \
             */                                                                          \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, SAC_MT_MYTHREAD ()));        \
            maxloadthread = 0;                                                           \
            mintask = SAC_MT_LAST_TASK (sched_id, 0) - SAC_MT_TASK (sched_id, 0);        \
            for (queueid = 1; queueid < SAC_MT_THREADS (); queueid++)                    \
                if (SAC_MT_LAST_TASK (sched_id, queueid)                                 \
                      - SAC_MT_TASK (sched_id, queueid)                                  \
                    > mintask) {                                                         \
                    mintask = SAC_MT_TASK (sched_id, queueid)                            \
                              - SAC_MT_LAST_TASK (sched_id, queueid);                    \
                    maxloadthread = queueid;                                             \
                }                                                                        \
                                                                                         \
            /* if there was a thread with work to do,get his next task */                \
            SAC_MT_ACQUIRE_LOCK (SAC_MT_TASKLOCK (sched_id, maxloadthread));             \
            if (SAC_MT_TASK (sched_id, maxloadthread)                                    \
                < SAC_MT_LAST_TASK (sched_id, maxloadthread)) {                          \
                taskid = SAC_MT_LAST_TASK (sched_id, maxloadthread) - 1;                 \
                (SAC_MT_LAST_TASK (sched_id, maxloadthread))--;                          \
                worktodo = 1;                                                            \
            }                                                                            \
            SAC_MT_RELEASE_LOCK (SAC_MT_TASKLOCK (sched_id, maxloadthread));             \
        }                                                                                \
    }

#if 0
/* These macros are probably no longer used. */

#if SAC_DO_THREADS_STATIC

#define SAC_MT_SCHEDULER_Block_DIM0_PREDICATE(iterations_rest) iterations_rest

#else

#define SAC_MT_SCHEDULER_Block_DIM0_PREDICATE(iterations_rest) 1

#endif
#endif /* 0 */

/*
 *  Declarations of global variables and functions defined in libsac/mt.c
 */

extern pthread_attr_t SAC_MT_thread_attribs;

extern volatile SAC_MT_barrier_t *SAC_MT_barrier;

extern volatile unsigned int SAC_MT_master_flag;

extern volatile unsigned int SAC_MT_not_yet_parallel;

extern unsigned int SAC_MT_masterclass;

extern unsigned int SAC_MT_threads;

extern volatile unsigned int (*SAC_MT_spmd_function) (const unsigned int,
                                                      const unsigned int, unsigned int);

extern void SAC_MT_Setup (int cache_line_max, int barrier_offset, int num_schedulers);

extern void SAC_MT_TR_Setup (int cache_line_max, int barrier_offset, int num_schedulers);

extern void SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                 unsigned int max_threads);

extern void SAC_MT_TR_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                    unsigned int max_threads);

extern int atoi (const char *str);

extern pthread_key_t SAC_MT_threadid_key;

extern unsigned int SAC_MT_master_id;

SAC_MT_DECLARE_LOCK (SAC_MT_output_lock)

SAC_MT_DECLARE_LOCK (SAC_MT_init_lock)

/*****************************************************************************/

#if SAC_DO_THREADS_STATIC

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is known statically.
 ***/

#define SAC_MT_THREADS() SAC_SET_THREADS

#define SAC_MT_MASTERCLASS() SAC_SET_MASTERCLASS

/*****************************************************************************/

#else /* SAC_DO_THREADS_STATIC */

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is determined dynamically.
 ***/

#define SAC_MT_THREADS() SAC_MT_threads

#define SAC_MT_MASTERCLASS() SAC_MT_masterclass

#endif /* SAC_DO_THREADS_STATIC */

/*****************************************************************************/

#else /* SAC_DO_MULTITHREAD */

/***
 ***   Definitions and declarations for sequential execution (dummies)
 ***/

#define SAC_MT_THREADS() 1

#define SAC_MT_MYTHREAD() 0

#define SAC_MT_DECL_MYTHREAD() const unsigned int SAC_MT_mythread = 0;

#define SAC_MT_SETUP()
#define SAC_MT_SETUP_INITIAL()

#define SAC_MT_DEFINE()

#define SAC_MT_DEFINE_ARG_BUFFER_BEGIN()

#define SAC_MT_DEFINE_ARG_BUFFER_END()

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_BEGIN(name)

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_END(name)

#define SAC_MT_DEFINE_ARG_BUFFER_ENTRY_ITEM(type, item)

#define SAC_MT_DEFINE_LOCK(name)

#define SAC_MT_DECLARE_LOCK(name)

#define SAC_MT_ACQUIRE_LOCK(name)

#define SAC_MT_RELEASE_LOCK(name)

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* _SAC_MT_H_ */
