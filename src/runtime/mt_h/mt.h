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

#define SAC_MT_SETUP()
#define SAC_MT_SETUP_INITIAL()

#define SAC_MT_DEFINE()

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
