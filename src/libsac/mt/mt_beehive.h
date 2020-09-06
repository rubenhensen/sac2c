/*****************************************************************************
 *
 * file:   mt_beehive.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *    This file is part of the SAC standard header file sac.h
 *    Common parts for the bee-hives based MT backends: LPEL and PThreads (PTH).
 *
 *
 *****************************************************************************/

#ifndef _SAC_BEEHIVE_H_
#define _SAC_BEEHIVE_H_

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/* NOTE: SAC_DO_MT_BEEHIVE is defined only when including from mt_beehive.c */
#if SAC_DO_MULTITHREAD && (SAC_DO_MT_BEEHIVE || SAC_DO_MT_PTHREAD || SAC_DO_MT_LPEL)

#include "runtime/mt_h/schedule.h" // SAC_MT_DEFINE_*, ...
#include "runtime/mt_h/rt_mt.h"    //SAC_MT_DECLARE_LOCK, ...

struct sac_bee_common_t;
struct sac_hive_common_t;

#define SAC_MT_SPMDFUN_REAL_RETURN() return (0);

#define SAC_MT_SPMDFUN_REAL_RETTYPE() unsigned int

/**
 * The common (base) part of a bee inheritance hierarchy.
 * Bees are worker threads or tasks that can execute an SPMD function.
 */
struct sac_bee_common_t {
    /* local_id: a local (L:) bee ID within its bee-hive.
     * The bee L:0 is always the queen-bee (the master) in the hive. */
    unsigned local_id;
    /* hive: ptr to the hive this bee is in */
    struct sac_hive_common_t *hive;
    /* b_class: bee class, it is used in the barrier synchronisation */
    unsigned b_class;

    /* The thread_id field is valid only if the thread registry has been
     * initialized with the correct maximal number of threads in the system.
     * It is needed for the PHM.
     * In the sac-as-a-library setting the PHM is not supported, and so we
     * do not initialize the thread registry. Hence in that case the thread_id
     * will be zero. */
    unsigned thread_id;
};

/**
 * The common (base) part of a bee-hive inheritance hierarchy.
 * A hive is an ordered collection of bees.
 */
struct sac_hive_common_t {
    /* num_bees: the number of bees in the hive; length of the `bees' array. */
    unsigned num_bees;
    /* bees: array of num_bees size of pointers to bees.
     * The position [0] is reserved for the queen (master) bee.  */
    struct sac_bee_common_t **bees;
    /* queen_class: the MT class of the queen (master) bee */
    unsigned queen_class;
    /* framedata: pointer to the activation frame data of the current SPMD execution.
     * It is NULL otherwise when the hive is not in MT code
     * (see SAC_MT_NOT_LOCALLY_PARALLEL below). */
    void *framedata;
    /* retdata: pointer to the return data array (SPMD specific) */
    void *retdata;
    /* instantiate scheduler's variables as fields in the hive */
    SAC_MT_SCHEDULER_FIELD_TASKLOCKS ();
    SAC_MT_SCHEDULER_FIELD_TASKS ();
    SAC_MT_SCHEDULER_FIELD_LAST_TASKS ();
    SAC_MT_SCHEDULER_FIELD_REST_ITERATIONS ();
    SAC_MT_SCHEDULER_FIELD_ACT_TASKSIZE ();
    SAC_MT_SCHEDULER_FIELD_LAST_TASKEND ();
    SAC_MT_SCHEDULER_FIELD_TS_TASKLOCKS ();
    SAC_MT_SCHEDULER_FIELD_TASKCOUNT ();
};

/* Get current bee's local ID. Used for task scheduling. */
#define SAC_MT_SELF_LOCAL_ID() (SAC_MT_self->c.local_id)

/* Get current bee's thread ID. Used for PHM and thread-level locking (even in LPEL). */
#define SAC_MT_SELF_THREAD_ID() (SAC_MT_self->c.thread_id)

/* #define SAC_MT_GLOBAL_THREADS()       in mt.h */
/* Get current hive's local number of thread. Used for task scheduling. */
#define SAC_MT_LOCAL_THREADS()                                                           \
    (current_nr_threads ? current_nr_threads : SAC_MT_self->c.hive->num_bees)

/* Ptr to the current hive. Used in schedule.h to get the variables. */
#define SAC_MT_HIVE() (SAC_MT_self->c.hive)

#define SAC_MT_MYTHREAD() SAC_MT_self

#if 0 /* seemingly not needed */
/* SAC_MT_NOT_LOCALLY_PARALLEL: true when the local hive is in seq mode,
 * false when hive is executing and SPMD fun. */
#define SAC_MT_NOT_LOCALLY_PARALLEL() (!SAC_MT_self->c.hive->framedata)
#endif


/*****************************************************************************/

/**
 *  Declarations of global variables and functions defined in libsac/mt.c
 */

/* Global number of hives in the environment;
 * Atomic variable! */
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_hives;
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_worker_bees;
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_queen_bees;

/* The global singleton queen-bee, used in ST functions in stand-alone programs.
 * In SEQ-only programs and when SAC is initialized as a library for external calls
 * it should be NULL and it won't be used.
 */
SAC_C_EXTERN void *SAC_MT_singleton_queen;

// FIXME: does this need to be isolated??
SAC_MT_DECLARE_LOCK (SAC_MT_propagate_lock)

/* for message prints */
SAC_MT_DECLARE_LOCK (SAC_MT_output_lock)

/** Public Functions **/
/* NOTE: these are in fact defined in specific threading libs (LPEL, PTH) */

SAC_C_EXTERN void SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                       unsigned int max_threads);

SAC_C_EXTERN void SAC_MT_SetupAsLibraryInitial (void);

SAC_C_EXTERN struct sac_hive_common_t *SAC_MT_AllocHive (unsigned int num_bees,
                                                         int num_schedulers,
                                                         const int *places, void *thdata);

SAC_C_EXTERN void SAC_MT_ReleaseHive (struct sac_hive_common_t *h);

SAC_C_EXTERN void SAC_MT_AttachHive (struct sac_hive_common_t *h);

SAC_C_EXTERN struct sac_hive_common_t *SAC_MT_DetachHive (void);

SAC_C_EXTERN void SAC_MT_ReleaseQueen (void);

/** Internal interface functions */

SAC_C_EXTERN void SAC_MT_BEEHIVE_SetupInitial (int argc, char *argv[],
                                               unsigned int num_threads,
                                               unsigned int max_threads);

SAC_C_EXTERN struct sac_hive_common_t *
SAC_MT_Helper_AllocHiveCommons (unsigned num_bees, unsigned num_schedulers,
                                unsigned sizeof_hive, unsigned sizeof_bee);

SAC_C_EXTERN void SAC_MT_Helper_FreeHiveCommons (struct sac_hive_common_t *hive);

SAC_C_EXTERN void SAC_MT_Generic_AttachHive (struct sac_hive_common_t *hive,
                                             struct sac_bee_common_t *queen);
SAC_C_EXTERN struct sac_hive_common_t *
SAC_MT_Generic_DetachHive (struct sac_bee_common_t *queen);

SAC_C_EXTERN struct sac_bee_common_t *SAC_MT_CurrentBee (void);

/** -- */

/*****************************************************************************/
#else /* else SAC_DO_MULTITHREAD && (SAC_DO_MT_BEEHIVE || SAC_DO_MT_PTHREAD ||           \
         SAC_DO_MT_LPEL) */

#define SAC_MT_SELF_THREAD_ID() 0

/* inserted at the beginning of non-spmd functions to define SAC_MT_self */
#define SAC_MT_DEFINE_ST_SELF() /*empty*/

/* for compatability in seq needed:
 */
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_hives;
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_worker_bees;
SAC_C_EXTERN volatile unsigned int SAC_MT_cnt_queen_bees;
SAC_C_EXTERN void *SAC_MT_singleton_queen;

SAC_C_EXTERN int SAC_MT_propagate_lock;
SAC_C_EXTERN int SAC_MT_output_lock;

#endif /* SAC_DO_MULTITHREAD && (SAC_DO_MT_BEEHIVE || SAC_DO_MT_PTHREAD ||               \
          SAC_DO_MT_LPEL) */
#endif /* ndef _SAC_BEEHIVE_H_ */
