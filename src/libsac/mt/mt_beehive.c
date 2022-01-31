/*****************************************************************************
 *
 * file:   mt_beehive.c
 *
 * prefix: SAC_MT_
 *
 * description:
 *
 *
 * remark:
 *
 *
 *****************************************************************************/

#include "fun-attrs.h"

static UNUSED int _dummy_mt_beehive;

/* the code is only loaded into libsac.mt.pth and libsac.lpel.pth */
#if defined(SAC_MT_LIB_pthread) || defined(SAC_MT_LIB_lpel)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_COMPILE_MODULE 1
#define SAC_SET_NUM_SCHEDULERS 10 /* ?? */
#define SAC_DO_MT_BEEHIVE 1
#define SAC_DO_PHM 1

// #define SAC_DO_THREADS_STATIC 1      /* ?? */

#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC

#include "mt_beehive.h"
#include "runtime/phm_h/phm.h"         // SAC_CALLOC
#include "libsac/essentials/message.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE
// #undef SAC_SET_NUM_SCHEDULERS
#undef SAC_DO_MT_BEEHIVE

SAC_MT_DEFINE_LOCK (SAC_MT_propagate_lock)

SAC_MT_DEFINE_LOCK (SAC_MT_output_lock)

/* Global number of hives, worker bees and queens in the environment.
 * Used only for debugs.
 * ATOMIC ACCESSES ONLY! */
volatile unsigned int SAC_MT_cnt_hives = 0;
volatile unsigned int SAC_MT_cnt_worker_bees = 0;
volatile unsigned int SAC_MT_cnt_queen_bees = 0;

/* The global singleton queen-bee, used in ST functions in stand-alone programs.
 * In SEQ-only programs and when SAC is initialized as a library for external calls
 * it should be NULL and it won't be used.
 */
void *SAC_MT_singleton_queen = NULL;

/******************************************************************************
 *
 * function:
 *   static struct sac_hive_common_t *SAC_MT_Helper_AllocHiveCommons(
 *     unsigned num_bees, unsigned num_schedulers, unsigned sizeof_hive, unsigned
 *sizeof_bee)
 *
 * description:
 *
 *    Common helper function to allocate a hive and a bees in it.
 *    Allocates memory structures and links them together.
 *    Sets up basic info in the individual bees.
 *
 *    As this is a general code the actual byte sizes of the structures
 *    have to be passed in as arguments.
 *
 ******************************************************************************/
struct sac_hive_common_t *
SAC_MT_Helper_AllocHiveCommons (unsigned num_bees, unsigned num_schedulers,
                                unsigned sizeof_hive, unsigned sizeof_bee)
{
    /* allocate the Hive struct */
    struct sac_hive_common_t *hive
      = (struct sac_hive_common_t *)SAC_CALLOC (1, sizeof_hive);
    if (!hive) {
        SAC_RuntimeError ("Could not allocate memory for a hive.");
    }

    /* allocate an array of pointers to bees */
    hive->num_bees = num_bees;
    hive->bees = (struct sac_bee_common_t **)SAC_CALLOC (num_bees, sizeof (void *));
    if (!hive->bees) {
        SAC_RuntimeError ("Could not allocate memory for an array of ptrs to bees.");
    }

    // SAC_TR_LIBSAC_PRINT( ("Initializing Tasklocks."));
    if (num_schedulers > 0) {
        /* allocate scheduler's data structures in the hive */
        hive->SAC_MT_Tasklock = (pthread_mutex_t *)SAC_MALLOC (
          num_bees * num_schedulers * sizeof (pthread_mutex_t));
        hive->SAC_MT_Task = (int *)SAC_CALLOC (num_bees * num_schedulers, sizeof (int));
        hive->SAC_MT_LAST_Task
          = (int *)SAC_CALLOC (num_bees * num_schedulers, sizeof (int));
        hive->SAC_MT_rest_iterations = (int *)SAC_CALLOC (num_schedulers, sizeof (int));
        hive->SAC_MT_act_tasksize = (int *)SAC_CALLOC (num_schedulers, sizeof (int));
        hive->SAC_MT_last_taskend = (int *)SAC_CALLOC (num_schedulers, sizeof (int));
        hive->SAC_MT_TS_Tasklock
          = (pthread_mutex_t *)SAC_MALLOC (num_schedulers * sizeof (pthread_mutex_t));
        hive->SAC_MT_Taskcount = (int *)SAC_CALLOC (num_schedulers, sizeof (int));

        /* check success */
        if (!hive->SAC_MT_Tasklock || !hive->SAC_MT_Task || !hive->SAC_MT_LAST_Task
            || !hive->SAC_MT_rest_iterations || !hive->SAC_MT_act_tasksize
            || !hive->SAC_MT_last_taskend || !hive->SAC_MT_TS_Tasklock
            || !hive->SAC_MT_Taskcount) {
            SAC_RuntimeError (
              "Could not allocate memory for scheduling data in the hive!");
        }

        /* initialize the mutexes in the scheduler's data */
        for (unsigned n = 0; n < num_schedulers; ++n) {
            /* init pthread_mutex_t
             * SAC_MT_Tasklock[SAC_SET_THREADS_MAX*SAC_SET_NUM_SCHEDULERS]; */
            for (unsigned i = 0; i < num_bees; ++i) {
                pthread_mutex_init (&(SAC_MT_TASKLOCK_INIT (hive, n, i, num_schedulers)),
                                    NULL);
                // pthread_mutex_init( &(hive->SAC_MT_Tasklock[n + num_schedulers * i]),
                // NULL);
            }

            /* init pthread_mutex_t SAC_MT_TS_Tasklock[SAC_SET_NUM_SCHEDULERS]; */
            pthread_mutex_init (&(hive->SAC_MT_TS_Tasklock[n]), NULL);
        }
    }

    if (num_bees > 1) {
        /* allocate array of bees; one less than the total number of bees because
         * the bee #0 is the queen */
        /* use char* here because C++ doesn't like void* arithmetics but we want
         * do to byte-size pointer additions... */
        char *other_bees = SAC_CALLOC ((num_bees - 1), sizeof_bee);
        if (!other_bees) {
            SAC_RuntimeError ("Could not allocate memory for an array of bees.");
        }

        /* setup common info in bees */
        for (unsigned i = 1; i < num_bees; ++i) {
            /* NOTE: the index below is (i-1) because other_bees[] contains only slave
             * bees, not the queen bee itself */
            struct sac_bee_common_t *b
              = (struct sac_bee_common_t *)(other_bees + sizeof_bee * (i - 1));
            /* put a bee into hive */
            hive->bees[i] = b;
            /* set bee's data */
            b->local_id = i;
            b->thread_id
              = SAC_HM_THREADID_INVALID; /* must be assigned within the thread! */
            b->hive = hive;
        }
    }

    /* Computing task class of the queen bee. */

    for (hive->queen_class = 1; hive->queen_class < num_bees; hive->queen_class <<= 1)
        ;

    hive->queen_class >>= 1;

    /* increment the number of hives in the system */
    __sync_add_and_fetch (&SAC_MT_cnt_hives, 1);
    __sync_add_and_fetch (&SAC_MT_cnt_worker_bees, num_bees - 1);

    return hive;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_Helper_FreeHiveCommons(struct sac_hive_common_t *hive)
 *
 * description:
 *    Common helper function to free a hive memory.
 *
 ******************************************************************************/
void
SAC_MT_Helper_FreeHiveCommons (struct sac_hive_common_t *hive)
{
    /* decrement the number of hives in the environment */
    __sync_sub_and_fetch (&SAC_MT_cnt_hives, 1);
    __sync_sub_and_fetch (&SAC_MT_cnt_worker_bees, hive->num_bees - 1);

    if (hive->num_bees > 1) {
        /* the other_bees ptr below is equal to the identically-named variable in
         * SAC_MT_Helper_AllocHiveCommons() */
        struct sac_bee_common_t *other_bees = hive->bees[1];
        SAC_FREE (other_bees);
    }

    /* free the array of pointers */
    SAC_FREE (hive->bees);
    hive->bees = NULL;

    /* free the scheduler's data; typecast due to volatile */
    SAC_FREE ((void *)hive->SAC_MT_Tasklock);
    SAC_FREE ((void *)hive->SAC_MT_Task);
    SAC_FREE ((void *)hive->SAC_MT_LAST_Task);
    SAC_FREE ((void *)hive->SAC_MT_rest_iterations);
    SAC_FREE ((void *)hive->SAC_MT_act_tasksize);
    SAC_FREE ((void *)hive->SAC_MT_last_taskend);
    SAC_FREE ((void *)hive->SAC_MT_TS_Tasklock);
    SAC_FREE ((void *)hive->SAC_MT_Taskcount);

    /* free the hive itself */
    SAC_FREE (hive);
}

/****************************************************************************
 *
 * @fn void SAC_MT_Generic_AttachHive(struct sac_hive_common_t* hive,
 *                                       struct sac_bee_common_t *queen)
 *
 * @brief Attach the hive to the queen.
 *  A generic code that works both for LPEL and PTH backends.
 *
 *****************************************************************************/
void
SAC_MT_Generic_AttachHive (struct sac_hive_common_t *hive, struct sac_bee_common_t *queen)
{
    /* check: the hive must not be already attached */
    if (hive->bees[0]) {
        SAC_RuntimeError ("AttachHive: Cannot attach a hive which is already attached!"
                          " Call DetachHive() first.");
    }

    if (queen->hive) {
        /* destroy the current hive */
        SAC_MT_ReleaseHive (SAC_MT_DetachHive ());
    }

    /* now we're a queen without any hive */
    assert (!queen->hive);

    /* put the queen in the hive */
    hive->bees[0] = queen;
    queen->hive = hive;
    queen->b_class = hive->queen_class;
}

/**************************************************************************
 *
 * @fn struct sac_hive_common_t* SAC_MT_Generic_DetachHive(struct sac_bee_common_t *queen)
 *
 * @brief Detach a hive from the queen bee and return a handle to it.
 *
 *****************************************************************************/
struct sac_hive_common_t *
SAC_MT_Generic_DetachHive (struct sac_bee_common_t *queen)
{
    if (!queen) {
        /* no queen, no hive */
        return NULL;
    }

    if (!queen->hive) {
        /* no hive attached */
        return NULL;
    }

    struct sac_hive_common_t *hive = queen->hive;
    /* check that we're the queen */
    assert (hive->bees[0] == queen);

    /* remove the queen from the hive */
    hive->bees[0] = NULL;
    queen->hive = NULL;
    queen->b_class = 0;

    return hive;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_BEEHIVE_SetupInitial( int argc, char *argv[],
 *                                 unsigned int num_threads,
 *                                 unsigned int max_threads)
 *
 * description:
 *    Common initializations for a bee-hive system.
 *
 ******************************************************************************/
void
SAC_MT_BEEHIVE_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                             unsigned int max_threads)
{
}

#else /* defined(PTH) || defined(LPEL) else */

/*
 * The following symbols are provided even in the sequential case because
 * each module contains SEQ, ST and MT versions of each function. The latter
 * make reference to the following symbols. Even though they will never be
 * called when programs are compiled for sequential execution, the linker
 * nevertheless wants to see them.
 *
 * If we compile for mt_trace.o, we don't need the global variables since
 * these always remain in mt.o.
 */

#include "mt_beehive.h"

int SAC_MT_propagate_lock;                        /* dummy */
int SAC_MT_output_lock;                           /* dummy */
volatile unsigned int SAC_MT_cnt_hives = 0;       /* dummy */
volatile unsigned int SAC_MT_cnt_worker_bees = 0; /* dummy */
volatile unsigned int SAC_MT_cnt_queen_bees = 0;  /* dummy */
void *SAC_MT_singleton_queen = 0;                 /* dummy */

#endif /* defined(PTH) || defined(LPEL) */
