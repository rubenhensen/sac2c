/*
 *
 * $Log$
 * Revision 2.13  2000/02/07 09:51:59  cg
 * Changed setting of semicolons in definitions and declarations of
 * mutex locks in order to avoid nasty warnings from cc.
 *
 * Revision 2.12  2000/01/17 16:25:58  cg
 * Reorganized initialization of the runtime system for
 * multithreaded program execution.
 *
 * Revision 2.11  1999/11/18 10:13:29  jhs
 * Added a very special volatile ...
 *
 * Revision 2.10  1999/09/01 17:13:38  jhs
 * Added rc's in barriers.
 *
 * Revision 2.9  1999/07/30 13:51:14  jhs
 * Deleted unused macros.
 *
 * Revision 2.8  1999/07/28 13:02:56  jhs
 * Barriers are static now.
 *
 * Revision 2.7  1999/07/20 16:57:01  jhs
 * Added SAC_MT_SYNC_MULTIFOLD_[1|2|3][A|B].
 *
 * Revision 2.6  1999/07/01 13:05:00  jhs
 * Added macros SAC_MT_[MASTER|WORKER]_[BEGIN|END] and SAC_MT_RESTART being
 * part of the new values exchange ICMseries between SYNC-BLOCKS.
 * Done the beautification on the fly.
 *
 * Revision 2.5  1999/06/25 15:40:55  jhs
 * Just to provide compilabilty.
 *
 * Revision 2.4  1999/06/03 13:31:27  jhs
 * Deleted extra frame, all that can be done with the existing frame.
 *
 * Revision 2.3  1999/06/03 13:20:14  jhs
 * Added extra spmd-frame to exchange data between master & workers.
 *
 * Revision 2.2  1999/04/06 13:43:33  cg
 * SAC_MT_THREADS() is now set to 1 in case of single threaded execution.
 *
 * Revision 2.1  1999/02/23 12:43:55  sacbase
 * new release made
 *
 * Revision 1.17  1999/02/19 09:31:22  cg
 * Support for MIT-threads discarded.
 *
 * Revision 1.16  1999/01/18 08:15:18  sbs
 * changed SAC_MT_SETUP_PTHREAD:
 * Errorchecking plus non-NULL first arg for pthread_create
 *
 * Revision 1.15  1998/12/10 12:39:05  cg
 * Bug fixed in definition of _MIT_POSIX_THREADS.
 *
 * Revision 1.14  1998/12/07 10:00:11  cg
 * added #define _MIT_POSIX_THREADS to please Linux
 *
 * Revision 1.13  1998/08/27 14:47:40  cg
 * bugs fixed in implementations of ICMs ADJUST_SCHEDULER and
 * SCHEDULER_Block_DIM0.
 *
 * Revision 1.12  1998/08/03 10:50:52  cg
 * added implementation of new ICM MT_ADJUST_SCHEDULER
 *
 * Revision 1.11  1998/07/29 08:44:16  cg
 * bug fixed in one-fold-barrier: now programs produce correct
 * results even if started with -mt 1
 *
 * Revision 1.10  1998/07/03 10:21:33  cg
 * ICM MT_START_SPMD replaced by MT_SPMD_EXECUTE
 *
 * Revision 1.9  1998/07/02 09:27:39  cg
 * fixed several bugs
 * dynamic multi-threading option renamed to -mt <num>
 *
 * Revision 1.8  1998/06/29 08:57:13  cg
 * added tracing facilities
 *
 * Revision 1.7  1998/06/25 08:07:56  cg
 * various small syntactic bugs fixed
 *
 * Revision 1.6  1998/06/23 12:54:42  cg
 * various bug fixes but state of development is still preliminary.
 *
 * Revision 1.5  1998/06/03 14:56:04  cg
 * some bugs removed
 *
 * Revision 1.4  1998/05/15 15:43:56  cg
 * first bugs removed
 *
 * Revision 1.3  1998/05/15 09:22:57  cg
 * first complete version
 *
 * Revision 1.2  1998/05/07 11:15:41  cg
 * minor errors removed
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_mt.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the multi-threading
 *   facilities.
 *
 *
 *
 *****************************************************************************/

#ifndef SAC_MT_H

#define SAC_MT_H

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

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>

/*
 * Definition of synchronisation barrier data type.
 */

typedef struct ARRAYRC {
    /* volatile */ void *ptr;
    /* volatile */ int *rc;
} arrayrc_t;

typedef union {
    union {
        volatile int result_int;
        volatile float result_float;
        volatile double result_double;
        volatile char result_char;
        volatile void *result_array;
        volatile arrayrc_t result_array_rc;
        volatile void *result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_dummy_t;

#define SAC_MT_MAX(a, b) ((a) > (b) ? (a) : (b))

#define SAC_MT_CACHE_LINE_MAX()                                                          \
    SAC_MT_MAX (SAC_SET_CACHE_1_LINE,                                                    \
                SAC_MT_MAX (SAC_SET_CACHE_2_LINE, SAC_MT_MAX (SAC_SET_CACHE_3_LINE, 1)))

#define SAC_MT_BARRIER_OFFSET()                                                          \
    ((SAC_MT_CACHE_LINE_MAX () >= sizeof (SAC_MT_barrier_dummy_t))                       \
       ? SAC_MT_CACHE_LINE_MAX ()                                                        \
       : (SAC_MT_CACHE_LINE_MAX ()                                                       \
          * (((sizeof (SAC_MT_barrier_dummy_t) - 1) / SAC_MT_CACHE_LINE_MAX ()) + 1)))

typedef union {
    char dummy[SAC_MT_BARRIER_OFFSET ()];
    union {
        volatile int result_int;
        volatile float result_float;
        volatile double result_double;
        volatile char result_char;
        volatile void *result_array;
        volatile arrayrc_t result_array_rc;
        volatile void *result_hidden;
    } b[SAC_SET_MAX_SYNC_FOLD + 1];
} SAC_MT_barrier_t;

/*
 *  Basic access macros for synchronisation barrier.
 */

#define CONCAT(a, b) a##b
#define CATCAT(a, b) CONCAT (a, b)

#define SAC_MT_BARRIER_READY(barrier, n) (barrier[n].b[0].result_int)

#define SAC_MT_BARRIER_RESULT(barrier, n, m, type) (barrier[n].b[m].result_##type)

#define SAC_MT_BARRIER_RC_RESULT_PTR(barrier, n, m, type)                                \
    (CATCAT (CATCAT (barrier[n].b[m].result_, type), .ptr))
#define SAC_MT_BARRIER_RC_RESULT_RC(barrier, n, m, type)                                 \
    (CATCAT (CATCAT (barrier[n].b[m].result_, type), .rc))

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
        SAC_MT_BARRIER_RC_RESULT_RC (SAC_MT_barrier, n, m, type) = CONCAT (res, __rc);   \
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

#define SAC_MT_DEFINE()                                                                  \
    SAC_MT_DEFINE_BARRIER ()                                                             \
    SAC_MT_DEFINE_SPMD_FRAME ()

#define SAC_MT_DEFINE_BARRIER()                                                          \
    volatile SAC_MT_barrier_t SAC_MT_barrier_space[SAC_SET_THREADS_MAX + 1];

#define SAC_MT_DEFINE_SPMD_FRAME()                                                       \
    static volatile union SAC_SET_SPMD_FRAME SAC_MT_spmd_frame;

#define SAC_MT_FUN_FRAME(name, blocks) struct blocks name;

#define SAC_MT_BLOCK_FRAME(name, args) struct args name;

#define SAC_MT_BLOCK_FRAME_DUMMY() int dummy;

#define I_POST(arg) arg
#define O_POST(arg) CAT (arg, _out)
#define S_POST(arg) CAT (arg, _shared)

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
    SAC_MT_TR_Setup (SAC_MT_CACHE_LINE_MAX (), SAC_MT_BARRIER_OFFSET ());
#else
#define SAC_MT_SETUP() SAC_MT_Setup (SAC_MT_CACHE_LINE_MAX (), SAC_MT_BARRIER_OFFSET ());
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

#define SAC_MT_DETERMINE_THREAD_ID()                                                     \
    const unsigned int SAC_MT_mythread                                                   \
      = *((unsigned int *)pthread_getspecific (SAC_MT_threadid_key));

#define SAC_MT_SPMD_SPECIAL_FRAME(spmdname)                                              \
    SAC_MT_spmd_frame.SAC_MT_CURRENT_FUN ().##spmdname

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
 *  Definitions of macro-implemented ICMs for scheduling
 */

#define SAC_MT_ADJUST_SCHEDULER(array, dim, lower, upper, unrolling, offset)             \
    {                                                                                    \
        if ((SAC_WL_MT_SCHEDULE_START (dim) > lower)                                     \
            && (SAC_WL_MT_SCHEDULE_START (dim) < upper)) {                               \
            int tmp = (SAC_WL_MT_SCHEDULE_START (dim) - lower) % unrolling;              \
                                                                                         \
            if (tmp) {                                                                   \
                tmp = unrolling - tmp;                                                   \
                SAC_WL_MT_SCHEDULE_START (dim) += tmp;                                   \
                array##__destptr += tmp * (offset);                                      \
            }                                                                            \
        }                                                                                \
    }

#define SAC_MT_SCHEDULER_Static_BEGIN()

#define SAC_MT_SCHEDULER_Static_END()

#define SAC_MT_SCHEDULER_Block_DIM0(lower, upper, unrolling)                             \
    {                                                                                    \
        const int iterations = (upper - lower) / unrolling;                              \
        const int iterations_per_thread = (iterations / SAC_MT_THREADS ()) * unrolling;  \
        const int iterations_rest = iterations % SAC_MT_THREADS ();                      \
                                                                                         \
        if (iterations_rest && (SAC_MT_MYTHREAD () < iterations_rest)) {                 \
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

#if 0
/* These macros are probably no longer used. */

#if SAC_DO_THREADS_STATIC

#define SAC_MT_SCHEDULER_Block_DIM0_PREDICATE(iterations_rest) iterations_rest

#else

#define SAC_MT_SCHEDULER_Block_DIM0_PREDICATE(iterations_rest) 1

#endif
#endif /* 0 */

/*
 * Definition of macros implementing a general locking mechanism
 */

#define SAC_MT_DEFINE_LOCK(name) pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER;

#define SAC_MT_DECLARE_LOCK(name) extern pthread_mutex_t name;

#define SAC_MT_ACQUIRE_LOCK(name) pthread_mutex_lock (&name);

#define SAC_MT_RELEASE_LOCK(name) pthread_mutex_unlock (&name);
/*
 * Warning:
 *
 * At least on Solaris 2.5 - 2.7 the static initialization of mutex locks causes
 * warnings upon compilation. The reason is that in the corresponding file
 * pthread.h, the definition of PTHREAD_MUTEX_INITIALIZER does NOT fit the
 * type definition of pthread_mutex_t.
 *
 * However, so far it works  :-)))
 */

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

extern void SAC_MT_Setup (int cache_line_max, int barrier_offset);
extern void SAC_MT_TR_Setup (int cache_line_max, int barrier_offset);

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

#endif /* SAC_MT_H */
