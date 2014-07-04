/*****************************************************************************
 *
 * file:   mt_lpel.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *    LPEL MT backend header file. Uses the bee-hive abstraction.
 *
 * remark:
 *
 *
 *****************************************************************************/

#ifndef _SAC_LPEL_H_
#define _SAC_LPEL_H_

/* NOTE: SAC_DO_MT_BEEHIVE is defined only when including from mt_beehive.c */
#if SAC_DO_MULTITHREAD && SAC_DO_MT_LPEL

/*****************************************************************************/

#include <lpel.h>

struct sac_bee_lpel_t;
struct sac_hive_lpel_t;

/* Signature of all SPMD functions. Must match that used in wrapper code. */
#define SAC_MT_SPMDFUN_REAL_PARAM_LIST() struct sac_bee_lpel_t *SAC_MT_self

/* inserted by compiler in MT */
#define SAC_MT_MYTHREAD_PARAM() struct sac_bee_lpel_t *SAC_MT_self

/* The spmd function a hive will execute concurrently. */
typedef SAC_MT_SPMDFUN_REAL_RETTYPE () (*sac_hive_spmd_fun_lpel_t) (
  struct sac_bee_lpel_t *SAC_MT_self);

/**
 * The LPEL-specific bee type structure.
 * Inherits common stuff from mt_beehive.h.
 */
struct sac_bee_lpel_t {
    /* Common stuff. Must be the first field in the record */
    struct sac_bee_common_t c;

    /** LPEL-specific: */
    /* worker_id: Tasks are never moved across workers, hence the worker_id
     * can be statically set upon task creation. This is used for mapping. */
    unsigned int worker_id;
    /* tsk: The LPEL Task this bee is riding on. */
    lpel_task_t *tsk;
    /* start_lck: bees will block on this lock when waiting for an SPMD invocation to
     * happen. */
    lpel_bisema_t start_lck;
    /* stop_lck: stop-barrier synchro lock */
    lpel_bisema_t stop_lck;
    /* magic constant, for assertion checks */
    unsigned int magic1;
};

#define SAC_MT_LPEL_MAGIC_1 0xBeefCace     /* when alive */
#define SAC_MT_LPEL_BAD_MAGIC_1 0xDeadBeef /* when dead (freed) */

/**
 * The LPEL-specific hive type structure.
 * Inherits common stuff from mt_beehive.h.
 */
struct sac_hive_lpel_t {
    /* Common stuff. Must be the first field in the record. */
    struct sac_hive_common_t c;
    /* spmd_fun: the function the hive shall execute in the SPMD fashion */
    sac_hive_spmd_fun_lpel_t spmd_fun;
};

/* Up-cast a pointer from struct sac_bee_common_t to sac_bee_lpel_t.
 * An inline fun instead of a macro is used to ensure the argument is of a proper type. */
static inline struct sac_bee_lpel_t *
CAST_BEE_COMMON_TO_LPEL (struct sac_bee_common_t *cp)
{
    return (struct sac_bee_lpel_t *)cp;
}

/* Up-cast a pointer from struct sac_hive_common_t to sac_hive_lpel_t.
 * An inline fun instead of a macro is used to ensure the argument is of a proper type. */
static inline struct sac_hive_lpel_t *
CAST_HIVE_COMMON_TO_LPEL (struct sac_hive_common_t *cp)
{
    return (struct sac_hive_lpel_t *)cp;
}

/* init bee's start_lck in a locked state */
#define SAC_MT_INIT_START_LCK(bee)                                                       \
    LpelBiSemaInit (&(bee)->start_lck);                                                  \
    LpelBiSemaWait (&(bee)->start_lck);

#define SAC_MT_RELEASE_START_LCK(bee) LpelBiSemaSignal (&(bee)->start_lck)

#define SAC_MT_ACQUIRE_START_LCK(bee) LpelBiSemaWait (&(bee)->start_lck)

/* Inserted at the beginning of the ST and SEQ functions to define SAC_MT_self.
 * In SEQ the value will not be used and shall be NULL.
 * In ST the value should point to the single global queen-bee created for the
 * standalone program execution. */
#define SAC_MT_DEFINE_ST_SELF()                                                          \
    struct sac_bee_lpel_t *const SAC_MT_self = SAC_MT_singleton_queen;

/* LPEL Task stack size. There seems to be no rule to choose this,
 * and things may go horribly wrong if the stack overflows.
 * FIXME: check for overflows (using a canary)
 */
#define SAC_MT_LPEL_STACK_SIZE (8 * 1024 * 1024) /* 8MB should be enough for anybody */

/*****************************************************************************/

/*
 *  Macros for setting and clearing the synchronisation barrier
 */

/* init stop_lck in a bee in a locked state */
#define SAC_MT_INIT_BARRIER(bee)                                                         \
    LpelBiSemaInit (&bee->stop_lck);                                                     \
    LpelBiSemaWait (&bee->stop_lck);

/* similar to unlock */
#define SAC_MT_SET_BARRIER(spmdfun, loc_id)                                              \
    LpelBiSemaSignal (                                                                   \
      &CAST_BEE_COMMON_TO_LPEL (SAC_MT_self->c.hive->bees[loc_id])->stop_lck);

// #undef LPEL_HAS_BISEMA_COUNT_WAITING
/* if the symbol LPEL_HAS_BISEMA_COUNT_WAITING is defined, the LPEL library has
 * the LpelBiSemaCountWaiting() function which enables us to do active waiting
 * in the stop-sync barrier. Otwerwise we use a passive wait.
 * (Active waiting is more aggresive and may harm other non-sac tasks in the system.)
 * Note that the definition of macros below is tightly related to the sync code
 * in mt_beehive.h */
#ifdef LPEL_HAS_BISEMA_COUNT_WAITING
/* use active waiting in a barrier */
/* is-unlocked? */
#define SAC_MT_CHECK_BARRIER(spmdfun, loc_id)                                            \
    !LpelBiSemaCountWaiting (                                                            \
      &CAST_BEE_COMMON_TO_LPEL (SAC_MT_self->c.hive->bees[loc_id])->stop_lck)

/* similar to lock */
#define SAC_MT_CLEAR_BARRIER(spmdfun, loc_id)                                            \
    LpelBiSemaWait (                                                                     \
      &CAST_BEE_COMMON_TO_LPEL (SAC_MT_self->c.hive->bees[loc_id])->stop_lck);

/* If we or someone accidently schedules two SAC tasks onto the same LPEL worker thread,
 * we will deadlock during sync barrier because we will wait indefinitely without
 * preemption. As a quick-fix, we've introduced a relax that yields the task now and then
 * to allow a progress.
 */
#define SAC_MT_SYNC_RELAX() LpelTaskYield ();

#else /* def LPEL_HAS_BISEMA_COUNT_WAITING else */
/* doesn't have LpelBiSemaCountWaiting. Use passive wait. */

/* can-lock? Lock and say yes. */
#define SAC_MT_CHECK_BARRIER(spmdfun, loc_id)                                            \
    (LpelBiSemaWait (                                                                    \
       &CAST_BEE_COMMON_TO_LPEL (SAC_MT_self->c.hive->bees[loc_id])->stop_lck),          \
     1)

/* similar to lock; do nothing, because already locked */
#define SAC_MT_CLEAR_BARRIER(spmdfun, loc_id) /* empty */

#define SAC_MT_SYNC_RELAX() /* empty */

#endif

/*****************************************************************************/

/*
 *  Macros for implementing and calling spmd-functions
 */

#define SAC_MT_SPMD_EXECUTE(name)                                                        \
    {                                                                                    \
        SAC_TR_MT_PRINT (("Parallel LPEL execution of spmd-block %s started.", #name));  \
        CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive)->spmd_fun = &name;                \
        SAC_MT_self->c.hive->framedata = frame;                                          \
        SAC_MT_self->c.hive->retdata = rdata;                                            \
        SAC_MT_LPEL_do_spmd_execute (SAC_MT_self);                                       \
        SAC_TR_MT_PRINT (("Parallel execution of spmd-block %s finished.", #name));      \
    }

/* This is used in SAC_MT_SPMD_EXECUTE() to wake up bees in the hive
 * and make them execute the SPMD function. The queen bee follows the suit. */
static inline void
SAC_MT_LPEL_do_spmd_execute (struct sac_bee_lpel_t *const SAC_MT_self)
{
    /* we're not single thread any more */
    unsigned old_globally_single = SAC_MT_globally_single;
    if (SAC_MT_globally_single) {
        SAC_MT_globally_single = 0;
    }
    /* start non-queen bees */
    for (unsigned i = 1; i < SAC_MT_self->c.hive->num_bees; ++i) {
        struct sac_bee_lpel_t *b = CAST_BEE_COMMON_TO_LPEL (SAC_MT_self->c.hive->bees[i]);
        SAC_MT_RELEASE_START_LCK (b);
    }
    /* run ourself */
    CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive)->spmd_fun (SAC_MT_self);
    /* clean up */
    CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive)->spmd_fun = NULL;
    SAC_MT_self->c.hive->framedata = NULL;
    SAC_MT_self->c.hive->retdata = NULL;
    /* restore global single thread mode only if we're the only hive */
    if (old_globally_single) {
        SAC_MT_globally_single = 1;
    }
}

/******************************************************************************/

/*
 *  Macros for setting up the multi-threaded runtime system
 */

#define SAC_MT_SETUP()

#if SAC_DO_TRACE_MT

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_TR_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);

#define SAC_INVOKE_MAIN_FUN(fname, arg)                                                  \
    SAC_MT_LPEL_TR_SetupAndRunStandalone (fname, arg, SAC_SET_NUM_SCHEDULERS);

/* NOTE: SAC_MT_ReleaseHive() is called via SAC_MT_LPEL_SetupAndRunStandalone()
 * because we have to be inside a hive to be able to release it */
#define SAC_MT_FINALIZE() /* empty */

#else /* SAC_DO_TRACE_MT */

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_MT_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);

#define SAC_INVOKE_MAIN_FUN(fname, arg)                                                  \
    SAC_MT_LPEL_SetupAndRunStandalone (fname, arg, SAC_SET_NUM_SCHEDULERS);

/* NOTE: SAC_MT_ReleaseHive() is called via SAC_MT_LPEL_SetupAndRunStandalone()
 * because we have to be inside a hive to be able to release it */
#define SAC_MT_FINALIZE() /* empty */

#endif

/*****************************************************************************/

/**
 *  Declarations of global variables and functions defined in libsac/mt.c
 */

typedef void (*SAC_main_fun_t) (int *);

/** Functions **/

SAC_C_EXTERN void SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                       unsigned int max_threads);
SAC_C_EXTERN void SAC_MT_TR_SetupInitial (int argc, char *argv[],
                                          unsigned int num_threads,
                                          unsigned int max_threads);

SAC_C_EXTERN void SAC_MT_TR_SetupAsLibraryInitial (void);
SAC_C_EXTERN void SAC_MT_SetupAsLibraryInitial (void);

SAC_C_EXTERN void SAC_MT_LPEL_SetupAndRunStandalone (SAC_main_fun_t main_fn,
                                                     int *main_arg, int num_schedulers);
SAC_C_EXTERN void SAC_MT_LPEL_TR_SetupAndRunStandalone (SAC_main_fun_t main_fn,
                                                        int *main_arg,
                                                        int num_schedulers);

SAC_C_EXTERN unsigned int SAC_Get_CurrentBee_GlobalID (void);

#endif /* SAC_DO_MULTITHREAD && SAC_DO_MT_LPEL */
#endif /* ndef _SAC_LPEL_H_ */
