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

#ifndef _SAC_MT_PTH_H_
#define _SAC_MT_PTH_H_

#if SAC_DO_MULTITHREAD && SAC_DO_MT_PTHREAD

/*****************************************************************************/

/* Global TLS key to retrieve the Thread Self Bee Ptr.
 * Defined in mt_pth.c */
SAC_C_EXTERN pthread_key_t SAC_MT_self_bee_key;

/* forwards */
struct sac_bee_pth_t;
struct sac_hive_pth_t;

SAC_C_EXTERN int barrier_type;

/**
 * Low-latency mutex/semaphore types, i.e. spinlocks.
 * The implementation is simplistic and assumes only a single locker and single unlocker,
 * which is all we need.
 * These locks are used in latency-critical code: the stopping barrier.
 */
struct sac_pth_llmutex_t {
    /* 0 = unlocked, 1 = locked */
    volatile unsigned flag;
};

/* Release the lock. */
static inline void
SAC_MT_PTH_release_lck (struct sac_pth_llmutex_t *const lck)
{
    lck->flag = 0;
}

/* Acquire the lock. Busy-wait until the lock is ready. */
static inline void
SAC_MT_PTH_acquire_lck (struct sac_pth_llmutex_t *const lck)
{
    /* WARNING: THIS IS *NOT* ATOMIC AND WILL FAIL IF MORE THAN ONE THREAD ATTEMPTS TO
     * LOCK AT THE SAME TIME ! */
    while (lck->flag != 0)
        ;
    lck->flag = 1;
}

/* Test if the lock is unlocked. */
static inline int
SAC_MT_PTH_may_acquire_lck (struct sac_pth_llmutex_t *const lck)
{
    return (lck->flag == 0);
}

/* Initialize the lock. */
static inline void
SAC_MT_PTH_init_lck (struct sac_pth_llmutex_t *const lck, int locked)
{
    lck->flag = locked;
}

/* Release the lock. */
static inline void
SAC_MT_PTH_destroy_lck (struct sac_pth_llmutex_t *const lck)
{
    /* nop */
}

/**
 * Barrier synchronisation: one signals and many are released.
 * This is used in the starting barrier.
 */

static inline void
SAC_MT_PTH_wait_on_barrier (unsigned *locfl, volatile unsigned *sharedfl)
{
    const unsigned old_fl = *locfl;
    unsigned new_fl;

    /* wait until the value in the sharedfl changes compared to what
     * we remember from the last synchro. */
    do {
        new_fl = *sharedfl;
    } while (new_fl == old_fl);

    /* remember the new value locally */
    *locfl = new_fl;
}

static inline void
SAC_MT_PTH_signal_barrier (unsigned *locfl, volatile unsigned *sharedfl)
{
    /* Read the current shared flag, invert it, and store back.
     * We are guaranteed to be the only writer, hence this is safe */
    unsigned new_fl = ~(*sharedfl);
    *sharedfl = new_fl;

    if (locfl != NULL) {
        *locfl = new_fl;
    }
}

/** ------------------------------------------------------------------------- */

/* Signature of all SPMD functions. Must match that used in wrapper code. */
#define SAC_MT_SPMDFUN_REAL_PARAM_LIST() struct sac_bee_pth_t *SAC_MT_self

/* inserted by compiler in MT */
#define SAC_MT_MYTHREAD_PARAM() struct sac_bee_pth_t *SAC_MT_self

/* The spmd function a hive will execute concurrently. */
typedef SAC_MT_SPMDFUN_REAL_RETTYPE () (*volatile sac_hive_spmd_fun_pth_t) (
  struct sac_bee_pth_t *SAC_MT_self);

/**
 * The PTH-specific bee type structure.
 * Inherits common stuff from mt_beehive.h.
 */
struct sac_bee_pth_t {
    /* Common stuff. Must be the first field in the record. */
    struct sac_bee_common_t c;

    /** pth-specific: */
    /* pth: pthreads handle to the worker thread.
     * Empty in the first bee. */
    pthread_t pth;
    /* start_barr_locfl: starting barrier local flag */
    unsigned start_barr_locfl;
    /* stop_lck: stop-barrier synchro lock */
    struct sac_pth_llmutex_t stop_lck;
};

/**
 * The PTH-specific hive type structure.
 * Inherits common stuff from mt_beehive.h.
 */
struct sac_hive_pth_t {
    /* Common stuff. Must be the first field in the record. */
    struct sac_hive_common_t c;
    /* spmd_fun: the function the hive shall execute in the SPMD fashion */
    sac_hive_spmd_fun_pth_t spmd_fun;
    /* start_barr_sharedfl: starting barrier shared flag */
    volatile unsigned start_barr_sharedfl;
};

/* Up-cast a pointer from struct sac_bee_common_t to sac_bee_pth_t.
 * An inline fun is used to ensure the argument is of a proper type. */
static inline struct sac_bee_pth_t *
CAST_BEE_COMMON_TO_PTH (struct sac_bee_common_t *cp)
{
    return (struct sac_bee_pth_t *)cp;
}

/* Up-cast a pointer from struct sac_hive_common_t to sac_hive_pth_t.
 * An inline fun is used to ensure the argument is of a proper type. */
static inline struct sac_hive_pth_t *
CAST_HIVE_COMMON_TO_PTH (struct sac_hive_common_t *cp)
{
    return (struct sac_hive_pth_t *)cp;
}

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

/* init stop_lck in a bee in a locked state */
#define SAC_MT_INIT_BARRIER(bee) SAC_MT_PTH_init_lck (&bee->stop_lck, 1)

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

/* This is used in SAC_MT_SPMD_EXECUTE() to wake up bees in the hive
 * and make them execute the SPMD function. The queen bee follows the suit. */
static inline void
SAC_MT_PTH_do_spmd_execute (struct sac_bee_pth_t *const SAC_MT_self)
{
    volatile unsigned *sharedfl
      = &CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->start_barr_sharedfl;

    /* we're not single thread any more */
    unsigned old_globally_single = SAC_MT_globally_single;
    if (SAC_MT_globally_single) {
        SAC_MT_globally_single = 0;
    }
    /* start all the slave bees by signaling the start-barrier */
    SAC_MT_PTH_SIGNAL_BARRIER (sharedfl);

    /* run ourself */
    CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->spmd_fun (SAC_MT_self);
    /* clean up */
    CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->spmd_fun = NULL;
    SAC_MT_self->c.hive->framedata = NULL;
    SAC_MT_self->c.hive->retdata = NULL;
    /* restore global single thread mode */
    if (old_globally_single) {
        SAC_MT_globally_single = 1;
    }
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

/**
 *  Declarations of global variables and functions defined in libsac/mt.c
 */

/* pthreads default thread attributes (const after init) */
SAC_C_EXTERN pthread_attr_t SAC_MT_thread_attribs;

/** Functions **/

SAC_C_EXTERN void SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                       unsigned int max_threads);

SAC_C_EXTERN void SAC_MT_PTH_SetupStandalone (int num_schedulers);

SAC_C_EXTERN void SAC_MT_SetupAsLibraryInitial (void);

SAC_C_EXTERN unsigned int SAC_Get_CurrentBee_GlobalID (void);

/*  Called from PHM if it does not maintain its own thread ids. */
SAC_C_EXTERN unsigned int SAC_MT_Internal_CurrentThreadId (void);

#endif /* SAC_DO_MULTITHREAD && SAC_DO_MULTITHREAD && SAC_DO_MT_PTHREAD */

#endif /* _SAC_MT_PTH_H_ */
