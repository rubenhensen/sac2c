/*
 * $Id$
 */

/*****************************************************************************
 *
 * file:   mt.h
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

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#ifndef SAC_SIMD_COMPILATION

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

/***
 ***   Definitions and declarations for the multi-threaded runtime system
 ***/

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

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <alloca.h>

#ifndef memset
extern void *memset (void *s, int c, size_t n);
#endif

#ifndef memcpy
extern void *memcpy (void *dest, const void *src, size_t n);
#endif

/*
 * We need the above extern declarations here rather than including
 * the corresponding header files because the further declarations in
 * string.h conflict with SAC generated headers in the SAC string module.
 *
 * The check for a previous definition of a macro of equal name are required
 * for operating systems that try to overload memset or memcpy, e.g. MAC OS.
 */

/*****************************************************************************/

/*
 *  Macros for global symbol definitions
 */

#if SAC_SET_NUM_SCHEDULERS

#define SAC_MT_DEFINE()                                                                  \
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
    SAC_MT_DEFINE_DUMMY_TASKLOCKS ()                                                     \
    SAC_MT_DEFINE_DUMMY_TS_TASKLOCKS ()

#endif

/*****************************************************************************/

/*
 * Macros for cache line adjustment of data structure
 */

#define SAC_MT_CACHE_LINE_MAX()                                                          \
    SAC_MAX (SAC_SET_CACHE_1_LINE,                                                       \
             SAC_MAX (SAC_SET_CACHE_2_LINE, SAC_MAX (SAC_SET_CACHE_3_LINE, 1)))

/*****************************************************************************/

/*
 * Definition of macros implementing a general locking mechanism
 */

#define SAC_MT_DEFINE_LOCK(name) pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER;

#define SAC_MT_DECLARE_LOCK(name) SAC_C_EXTERN pthread_mutex_t name;

#define SAC_MT_ACQUIRE_LOCK(name) pthread_mutex_lock (&name);

#define SAC_MT_RELEASE_LOCK(name) pthread_mutex_unlock (&name);

/*****************************************************************************/

/*
 * Macros for implementing the SPMD frame used to move data into SPMD function
 *
 * SAC MT FRAME, used to pass in and inout arguments to SPMD functions.
 * Defined as a union of structs, one struct for every SPMD function.
 */

#define SAC_MT_SPMD_FRAME_BEGIN() static volatile union {

#define SAC_MT_SPMD_FRAME_END()                                                          \
    }                                                                                    \
    SAC_spmd_frame;

#define SAC_MT_SPMD_FRAME_ELEMENT_BEGIN(spmdfun) struct {

#define SAC_MT_SPMD_FRAME_ELEMENT_END(spmdfun)                                           \
    }                                                                                    \
    spmdfun;

#define SAC_MT_FRAME_ELEMENT_in__NODESC(name, num, basetype, var_NT)                     \
    SAC_ND_TYPE (var_NT, basetype) in_##num;

#define SAC_MT_FRAME_ELEMENT_in__DESC(name, num, basetype, var_NT)                       \
    SAC_ND_TYPE (var_NT, basetype) in_##num;                                             \
    SAC_ND_DESC_TYPE (var_NT) in_##num##_desc;

#define SAC_MT_FRAME_ELEMENT_inout__NODESC(name, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype) * in_##num;

#define SAC_MT_FRAME_ELEMENT_inout__DESC(name, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype) * in_##num;                                           \
    SAC_ND_DESC_TYPE (var_NT) * in_##num##_desc;

#define SAC_MT_FRAME_ELEMENT__NOOP(name, num, basetype, var_NT)

/*****************************************************************************/

/*
 * Macros for sending data to the SPMD frame
 */

#define SAC_MT_SEND_PARAM_in__NODESC(spmdfun, num, var_NT)                               \
    SAC_spmd_frame.spmdfun.in_##num = SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_PARAM_in__DESC(spmdfun, num, var_NT)                                 \
    SAC_spmd_frame.spmdfun.in_##num = SAC_ND_A_FIELD (var_NT);                           \
    SAC_spmd_frame.spmdfun.in_##num##_desc = SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM_inout__NODESC(spmdfun, num, var_NT)                            \
    SAC_spmd_frame.spmdfun.in_##num = &SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_PARAM_inout__DESC(spmdfun, num, var_NT)                              \
    SAC_spmd_frame.spmdfun.in_##num = &SAC_ND_A_FIELD (var_NT);                          \
    SAC_spmd_frame.spmdfun.in_##num##_desc = &SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM__NOOP(spmdfun, num, var_NT) SAC_NOOP ()

/*****************************************************************************/

/*
 * Macros for receiving data from the SPMD frame
 */

#define SAC_MT_RECEIVE_PARAM_in__NODESC(spmdfun, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_spmd_frame.spmdfun.in_##num;

#define SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC(spmdfun, num, basetype, var_NT)          \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_spmd_frame.spmdfun.in_##num;                           \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT)                                                               \
      = (SAC_ND_DESC_TYPE (var_NT))alloca (FIXED_SIZE_OF_DESC * sizeof (int));           \
    memset (SAC_ND_A_DESC (var_NT), '\0', FIXED_SIZE_OF_DESC * sizeof (int));

#define SAC_MT_RECEIVE_PARAM_in__DESC(spmdfun, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_spmd_frame.spmdfun.in_##num;                           \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT) = (SAC_ND_DESC_TYPE (var_NT))alloca (                         \
      BYTE_SIZE_OF_DESC (DESC_DIM (SAC_spmd_frame.spmdfun.in_##num##_desc)));            \
    memcpy (SAC_ND_A_DESC (var_NT), SAC_spmd_frame.spmdfun.in_##num##_desc,              \
            BYTE_SIZE_OF_DESC (DESC_DIM (SAC_spmd_frame.spmdfun.in_##num##_desc)));

#define SAC_MT_RECEIVE_PARAM_inout__NODESC(spmdfun, num, basetype, var_NT)               \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_spmd_frame.spmdfun.in_##num;

#define SAC_MT_RECEIVE_PARAM_inout__DESC(spmdfun, num, basetype, var_NT)                 \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_spmd_frame.spmdfun.in_##num;                                                 \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    CAT0 (SAC_ND_A_DESC (var_NT), __s) = (SAC_ND_DESC_TYPE (var_NT))alloca (             \
      BYTE_SIZE_OF_DESC (DESC_DIM (*SAC_spmd_frame.spmdfun.in_##num##_desc)));           \
    memcpy (CAT0 (SAC_ND_A_DESC (var_NT), __s), *SAC_spmd_frame.spmdfun.in_##num##_desc, \
            BYTE_SIZE_OF_DESC (DESC_DIM (*SAC_spmd_frame.spmdfun.in_##num##_desc)));     \
    SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))                       \
      = &CAT0 (SAC_ND_A_DESC (var_NT), __s);

#define SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC(spmdfun, num, basetype, var_NT)       \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_spmd_frame.spmdfun.in_##num;                                                 \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    CAT0 (SAC_ND_A_DESC (var_NT), __s)                                                   \
      = (SAC_ND_DESC_TYPE (var_NT))alloca (FIXED_SIZE_OF_DESC * sizeof (int));           \
    memset (CAT0 (SAC_ND_A_DESC (var_NT), __s), '\0',                                    \
            FIXED_SIZE_OF_DESC * sizeof (int));                                          \
    SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))                       \
      = &CAT0 (SAC_ND_A_DESC (var_NT), __s);

#define SAC_MT_RECEIVE_PARAM__NOOP(spmdfun, num, basetype, var_NT) SAC_NOOP ()

/*****************************************************************************/

/*
 * Macros for defining the synchronisation barrier
 *
 * SAC MT BARRIER, used to pass out parameters out of SPMD functions.
 * Defined as a union of structs, one struct for every SPMD function.
 * We provide dummy entries on each level in case a program does not
 * contain a single SPMD function or an SPMD function has no arguments.
 */

#define SAC_MT_SPMD_BARRIER_BEGIN()                                                      \
    static volatile struct {                                                             \
        int ready;                                                                       \
        union {                                                                          \
            struct {                                                                     \
                char cache_align_buffer[SAC_MT_CACHE_LINE_MAX () - sizeof (int)];        \
            } dummy;

#define SAC_MT_SPMD_BARRIER_ELEMENT_BEGIN(spmdfun) struct {

#define SAC_MT_BARRIER_ELEMENT__NOOP(name, num, basetype, var_NT)

#define SAC_MT_BARRIER_ELEMENT_out__NODESC(name, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype) in_##num;

#define SAC_MT_BARRIER_ELEMENT_out__DESC(name, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype) in_##num;                                             \
    SAC_ND_DESC_TYPE (var_NT) in_##num##_desc;

#define SAC_MT_SPMD_BARRIER_ELEMENT_END(spmdfun)                                         \
    }                                                                                    \
    spmdfun;

#define SAC_MT_SPMD_BARRIER_END()                                                        \
    }                                                                                    \
    data;                                                                                \
    }                                                                                    \
    SAC_spmd_barrier[SAC_SET_THREADS_MAX + 1];

/*****************************************************************************/

/*
 *  Macros for setting and clearing the synchronisation barrier
 */

#define SAC_MT_CLEAR_BARRIER(spmdfun, thread) SAC_spmd_barrier[thread].ready = 0;

#define SAC_MT_SET_BARRIER(spmdfun, thread) SAC_spmd_barrier[thread].ready = 1;

#define SAC_MT_CHECK_BARRIER(spmdfun, thread) (SAC_spmd_barrier[thread].ready)

/*****************************************************************************/

/*
 *  Macros for sending data to the synchronisation barrier
 */

#define SAC_MT_SEND_RESULT__NOOP(spmdfun, thread, num, var_NT) SAC_NOOP ()

#define SAC_MT_SEND_RESULT_out__NODESC(spmdfun, thread, num, var_NT)                     \
    SAC_spmd_barrier[thread].data.spmdfun.in_##num = SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_RESULT_out__DESC(spmdfun, thread, num, var_NT)                       \
    SAC_spmd_barrier[thread].data.spmdfun.in_##num = SAC_ND_A_FIELD (var_NT);            \
    SAC_spmd_barrier[thread].data.spmdfun.in_##num##_desc = SAC_ND_A_DESC (var_NT);

/*****************************************************************************/

/*
 *  Macros for receiving data from the synchronisation barrier
 */

#define SAC_MT_RECEIVE_RESULT__NOOP(spmdfun, thread, num, var_NT) SAC_NOOP ()

#define SAC_MT_RECEIVE_RESULT_out__NODESC(spmdfun, thread, num, var_NT)                  \
    SAC_ND_A_FIELD (var_NT) = SAC_spmd_barrier[thread].data.spmdfun.in_##num;

#define SAC_MT_RECEIVE_RESULT_out__DESC(spmdfun, thread, num, var_NT)                    \
    SAC_ND_A_FIELD (var_NT) = SAC_spmd_barrier[thread].data.spmdfun.in_##num;            \
    SAC_ND_A_DESC (var_NT) = SAC_spmd_barrier[thread].data.spmdfun.in_##num##_desc;

/*****************************************************************************/

/*
 * Macros for implementing the barrier synchronisation
 */

#define SAC_MT_SYNC_BEGIN(spmdfun)                                                       \
    {                                                                                    \
        unsigned int SAC_MT_ready_count = SAC_MT_MYWORKERCLASS ();                       \
        unsigned int SAC_MT_son_id;                                                      \
        unsigned int SAC_MT_i;                                                           \
                                                                                         \
        while (SAC_MT_ready_count > 0) {                                                 \
            SAC_MT_i = SAC_MT_MYWORKERCLASS ();                                          \
                                                                                         \
            do {                                                                         \
                SAC_MT_son_id = SAC_MT_MYTHREAD () + SAC_MT_i;                           \
                                                                                         \
                if (SAC_MT_CHECK_BARRIER (spmdfun, SAC_MT_son_id)) {

#define SAC_MT_SYNC_CONT(spmdfun)                                                        \
    SAC_MT_CLEAR_BARRIER (spmdfun, SAC_MT_son_id)                                        \
    SAC_MT_ready_count >>= 1;                                                            \
    if (SAC_MT_ready_count == 0) {                                                       \
        break;                                                                           \
    }                                                                                    \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_i >>= 1)                                                               \
        ;                                                                                \
    }

#define SAC_MT_SYNC_END(spmdfun)                                                         \
    SAC_MT_SET_BARRIER (spmdfun, SAC_MT_MYTHREAD ())                                     \
    }

#define SAC_MT_SYNC_FOLD__NOOP(spmdfun, num, accu_NT, val_NT, basetype, tag, foldfun)    \
    SAC_NOOP ()

#define SAC_MT_SYNC_FOLD_out__NODESC(spmdfun, num, accu_NT, val_NT, basetype, tag,       \
                                     foldfun)                                            \
    SAC_MT_RECEIVE_RESULT_out__NODESC (spmdfun, SAC_MT_son_id, num, val_NT);             \
    SAC_##tag##_FUNAP2 (foldfun, SAC_ND_ARG_out (accu_NT, basetype),                     \
                        SAC_ND_ARG_in (accu_NT, basetype),                               \
                        SAC_ND_ARG_in (val_NT, basetype));

#define SAC_MT_SYNC_FOLD_out__DESC(spmdfun, num, accu_NT, val_NT, basetype, tag,         \
                                   foldfun)                                              \
    SAC_MT_RECEIVE_RESULT_out__DESC (spmdfun, SAC_MT_son_id, num, val_NT);               \
    SAC_##tag##_FUNAP2 (foldfun, SAC_ND_ARG_out (accu_NT, basetype),                     \
                        SAC_ND_ARG_in (accu_NT, basetype),                               \
                        SAC_ND_ARG_in (val_NT, basetype));

/*****************************************************************************/

/*
 *  Macros for implementing and calling spmd-functions
 */

#define SAC_MT_SPMD_EXECUTE(name)                                                        \
    {                                                                                    \
        SAC_TR_MT_PRINT (("Parallel execution of spmd-block %s started.", #name));       \
        SAC_MT_spmd_function = &name;                                                    \
        SAC_MT_not_yet_parallel = 0;                                                     \
        SAC_MT_START_WORKERS ()                                                          \
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

#define SAC_MT_SPMDFUN_REAL_PARAM_LIST()                                                 \
    const unsigned int SAC_MT_mythread, const unsigned int SAC_MT_myworkerclass,         \
      unsigned int SAC_MT_worker_flag

#define SAC_MT_SPMDFUN_REAL_RETURN() return (SAC_MT_worker_flag);

#define SAC_MT_SPMDFUN_REAL_RETTYPE() unsigned int

#define SAC_MT_MYTHREAD() SAC_MT_mythread

#define SAC_MT_MYTHREAD_PARAM() const unsigned int SAC_MT_mythread

#define SAC_MT_MYWORKERCLASS() SAC_MT_myworkerclass

#define SAC_MT_DECL_MYTHREAD() const unsigned int SAC_MT_mythread = 0;

#define SAC_MT_DETERMINE_THREAD_ID()

/*****************************************************************************/

#if SAC_DO_THREADS_STATIC

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is known statically.
 ***/

#define SAC_MT_THREADS() SAC_SET_THREADS

#define SAC_MT_MASTERCLASS() SAC_SET_MASTERCLASS

#else /* SAC_DO_THREADS_STATIC */

/***
 ***   Definitions and declarations specific to the case where the exact number
 ***   of threads is determined dynamically.
 ***/

#define SAC_MT_THREADS() SAC_MT_threads

#define SAC_MT_MASTERCLASS() SAC_MT_masterclass

#endif /* SAC_DO_THREADS_STATIC */

/*****************************************************************************/

/*
 * SAC_PRF_RUNMT* primitive functions. These decide whether to execute the
 * following SPMD block sequentially or in parallel.
 */

#define SAC_ND_PRF_RUNMT_GENARRAY__DATA(var_NT, args) SAC_ND_WRITE (var_NT, 0) = 0;

#define SAC_ND_PRF_RUNMT_MODARRAY__DATA(var_NT, args) SAC_ND_WRITE (var_NT, 0) = 0;

#define SAC_ND_PRF_RUNMT_FOLD__DATA(var_NT, args) SAC_ND_WRITE (var_NT, 0) = 0;

/*****************************************************************************/

/*
 * Macros for data structures used by scheduling disciplines
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

/*****************************************************************************/

/*
 *  Definitions of macro-implemented ICMs for scheduling
 */

#define SAC_MT_ADJUST_SCHEDULER_BOUND(diff, bound, lower, upper, unrolling)              \
    {                                                                                    \
        (diff) = 0;                                                                      \
        if (((bound) > (lower)) && ((bound) < (upper))) {                                \
            (diff) = ((bound) - (lower)) % (unrolling);                                  \
                                                                                         \
            if (diff) {                                                                  \
                (diff) = (unrolling) - (diff);                                           \
                                                                                         \
                if ((bound) + (diff) >= (upper)) {                                       \
                    (diff) = (upper) - (bound);                                          \
                    (bound) = (upper);                                                   \
                } else {                                                                 \
                    (bound) += (diff);                                                   \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }

#define SAC_MT_ADJUST_SCHEDULER__BEGIN(to_NT, dims, dim, lower, upper, unrolling)        \
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
                          SAC_WL_MT_SCHEDULE_STOP (dim)));
/* Here is no closing bracket missing -> SAC_MT_ADJUST_SCHEDULER__END */

#define SAC_MT_ADJUST_SCHEDULER__OFFSET(off_NT, to_NT, dim)                              \
    SAC_ND_WRITE (off_NT, 0)                                                             \
      = SAC_ND_READ (off_NT, 0) + diff_start * SAC_WL_SHAPE_FACTOR (to_NT, dim);

#define SAC_MT_ADJUST_SCHEDULER__END(to_NT, dims, dim, lower, upper, unrolling) }

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
                                                                                         \
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
            if (restiter < tasksize) {                                                   \
                tasksize = restiter;                                                     \
            }                                                                            \
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
        for (i = 0; i <= max_task; i++) {                                                \
            SAC_MT_TASK (sched_id, i) = 0;                                               \
        }                                                                                \
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
        SAC_TR_MT_PRINT (                                                                \
          ("SAC_MT_SCHEDULER_Self_FIRST_TASK_DYNAMIC task = %d", taskid));               \
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

/******************************************************************************/

/*
 *  Macros for setting up the multi-threaded runtime system
 */

#if SAC_DO_TRACE_MT

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_TR_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);

#define SAC_MT_SETUP() SAC_MT_TR_Setup (SAC_SET_NUM_SCHEDULERS);

#else /* SAC_DO_TRACE_MT */

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);

#define SAC_MT_SETUP() SAC_MT_Setup (SAC_SET_NUM_SCHEDULERS);

#endif

/*****************************************************************************/

/*
 *  Macros for object access synchronisation within SPMD functions
 */

#define SAC_ND_PROP_OBJ_IN() SAC_MT_ACQUIRE_LOCK (SAC_MT_propagate_lock);

#define SAC_ND_PROP_OBJ_OUT() SAC_MT_RELEASE_LOCK (SAC_MT_propagate_lock);

#define SAC_ND_PROP_OBJ_UNBOX(unboxed, boxed)                                            \
    SAC_ND_A_FIELD (unboxed) = *SAC_NAMEP (SAC_ND_A_FIELD (boxed));

#define SAC_ND_PROP_OBJ_BOX(boxed, unboxed)                                              \
    *SAC_NAMEP (SAC_ND_A_FIELD (boxed)) = SAC_ND_A_FIELD (unboxed);

/*****************************************************************************/

/*
 *  Declarations of global variables and functions defined in libsac/mt.c
 */

SAC_C_EXTERN pthread_attr_t SAC_MT_thread_attribs;

SAC_C_EXTERN volatile unsigned int SAC_MT_master_flag;

SAC_C_EXTERN volatile unsigned int SAC_MT_not_yet_parallel;

SAC_C_EXTERN unsigned int SAC_MT_masterclass;

SAC_C_EXTERN unsigned int SAC_MT_threads;

SAC_C_EXTERN pthread_t *SAC_MT1_internal_id;

/*
 * REMARK:
 *
 * no volatile for the function return value here, as volatile has
 * no effect for rvalues! And a function return value is a rvalue.
 */
SAC_C_EXTERN unsigned int (*SAC_MT_spmd_function) (const unsigned int, const unsigned int,
                                                   unsigned int);

SAC_C_EXTERN void SAC_MT_Setup (int num_schedulers);

SAC_C_EXTERN void SAC_MT_TR_Setup (int num_schedulers);

SAC_C_EXTERN void SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                       unsigned int max_threads);

SAC_C_EXTERN void SAC_MT_TR_SetupInitial (int argc, char *argv[],
                                          unsigned int num_threads,
                                          unsigned int max_threads);

SAC_C_EXTERN pthread_key_t SAC_MT_threadid_key;

SAC_C_EXTERN unsigned int SAC_MT_master_id;

SAC_MT_DECLARE_LOCK (SAC_MT_propagate_lock)

SAC_MT_DECLARE_LOCK (SAC_MT_output_lock)

SAC_MT_DECLARE_LOCK (SAC_MT_init_lock)

SAC_C_EXTERN void SAC_MT1_TR_Setup (int num_schedulers);

SAC_C_EXTERN void SAC_MT1_Setup (int num_schedulers);

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

#define SAC_ND_PROP_OBJ_IN()

#define SAC_ND_PROP_OBJ_OUT()

#define SAC_ND_PROP_OBJ_UNBOX(unboxed, boxed)

#define SAC_ND_PROP_OBJ_BOX(boxed, unboxed)                                              \
    SAC_ND_A_FIELD (boxed) = SAC_ND_A_FIELD (unboxed);

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* SAC_SIMD_COMPILATION */

#endif /* _SAC_MT_H_ */
