/*****************************************************************************
 *
 * file:   mt_pth.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the Pthread multi-threading
 *   facilities.
 *
 *****************************************************************************/

#ifndef _SAC_RT_MT_PTH_H_
#define _SAC_RT_MT_PTH_H_

#if SAC_DO_MULTITHREAD && SAC_DO_MT_PTHREAD

/*****************************************************************************/

/* Inserted at the beginning of the ST and SEQ functions to define SAC_MT_self.
 * In SEQ the value will not be used and shall be NULL.
 * In ST the value should point to the single global queen-bee created for the
 * standalone program execution. */
#define SAC_MT_DEFINE_ST_SELF()                                                          \
    struct sac_bee_pth_t *const SAC_MT_self                                              \
      = (struct sac_bee_pth_t *)SAC_MT_singleton_queen;

/*****************************************************************************/

/*
 *  Macros for setting and clearing the synchronisation barrier
 *  These are tigthly related to the code in mt_beehive.h
 */
/* similar to lock */
#define SAC_MT_CLEAR_BARRIER(spmdfun, loc_id)                                            \
    SAC_MT_PTH_acquire_lck (                                                             \
      &CAST_BEE_COMMON_TO_PTH (SAC_MT_self->c.hive->bees[loc_id])->stop_lck);

/* similar to unlock */
#define SAC_MT_SET_BARRIER(spmdfun, loc_id)                                              \
    SAC_MT_PTH_release_lck (                                                             \
      &CAST_BEE_COMMON_TO_PTH (SAC_MT_self->c.hive->bees[loc_id])->stop_lck);

/* similar to is-unlocked */
#define SAC_MT_CHECK_BARRIER(spmdfun, loc_id)                                            \
    SAC_MT_PTH_may_acquire_lck (                                                         \
      &CAST_BEE_COMMON_TO_PTH (SAC_MT_self->c.hive->bees[loc_id])->stop_lck)

/*****************************************************************************/

/*
 *  Macros for implementing and calling spmd-functions
 */

#define SAC_MT_SPMD_EXECUTE(name)                                                        \
    {                                                                                    \
        SAC_TR_MT_PRINT (("Parallel PTH execution of spmd-block %s started.", #name));   \
        CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->spmd_fun = &name;                 \
        SAC_MT_self->c.hive->framedata = frame;                                          \
        SAC_MT_self->c.hive->retdata = rdata;                                            \
        SAC_MT_PTH_do_spmd_execute (SAC_MT_self);                                        \
        SAC_TR_MT_PRINT (("Parallel execution of spmd-block %s finished.", #name));      \
    }

/* Certainly no relaxation during barrier sync for PTHREADS! */
#define SAC_MT_SYNC_RELAX() /* empty */

/******************************************************************************/

/*
 *  Macros for setting up the multi-threaded runtime system
 */

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_barrier_type = SAC_SET_BARRIER_TYPE;                                          \
    SAC_MT_cpu_bind_strategy = SAC_SET_CPU_BIND_STRATEGY;                                \
    SAC_MT_do_trace = SAC_DO_TRACE_MT;                                                   \
    SAC_MT_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);          \
    SAC_MT_PTH_INIT_BARRIER (SAC_MT_GLOBAL_THREADS ());                                  \
    SAC_MT_SMART_INIT (SAC_MT_global_threads);

#define SAC_MT_SETUP() SAC_MT_PTH_SetupStandalone (SAC_SET_NUM_SCHEDULERS);

#define SAC_MT_FINALIZE()                                                                \
    SAC_MT_ReleaseHive (SAC_MT_DetachHive ());                                           \
    SAC_MT_ReleaseQueen ();                                                              \
    SAC_MT_singleton_queen = NULL;                                                       \
    SAC_MT_PTH_DESTROY_BARRIER ();                                                       \
    SAC_MT_SMART_FINALIZE ();

#define SAC_INVOKE_MAIN_FUN(fname, arg) fname (arg)

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD && SAC_DO_MULTITHREAD && SAC_DO_MT_PTHREAD */

#endif /* _SAC_RT_MT_PTH_H_ */
