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

#include "config.h"

static int _dummy_mt_beehive;

#if ENABLE_MT

/* the code is only loaded into libsac.mt.pth and libsac.lpel.pth */
#if defined(PTH) || defined(LPEL)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if TRACE
#define SAC_DO_TRACE 1
#else
#define SAC_DO_TRACE 0
#endif

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_COMPILE_MODULE 1
#define SAC_SET_NUM_SCHEDULERS 10 /* ?? */
#define SAC_DO_MT_BEEHIVE 1

// #define SAC_DO_THREADS_STATIC 1      /* ?? */

#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE
// #undef SAC_SET_NUM_SCHEDULERS
#undef SAC_DO_MT_BEEHIVE

/**
 * Global array of ptrs to all bees in the environment.
 * This is required mainly to keep track which global_id's
 * are already assigned and which were freed
 */
struct sac_beehive_registry_t {
    /* Write lock for SAC_MT_all_bees */
    pthread_mutex_t lock;
    /* array of ptrs to bees */
    struct sac_bee_common_t **all_bees;
    /* size of the array */
    unsigned int ab_size;
};

struct sac_beehive_registry_t SAC_MT_beehive_registry;

SAC_MT_DEFINE_LOCK (SAC_MT_propagate_lock)

SAC_MT_DEFINE_LOCK (SAC_MT_output_lock)

/* Global number of hives in the environment;
 * ATOMIC ACCESSES ONLY! */
unsigned int SAC_MT_global_num_hives = 0;

/* The global singleton queen-bee, used in ST functions in stand-alone programs.
 * In SEQ-only programs and when SAC is initialized as a library for external calls
 * it should be NULL and it won't be used.
 */
void *SAC_MT_singleton_queen = NULL;

/******************************************************************************
 *
 * function:
 *   void SAC_MT_AssignBeeGlobalId(struct sac_bee_common_t *bee)
 *
 * description:
 *
 *   Assigns to the bee a unique global ID. Returns 0 on success, -1 on error.
 *   The global id is stored in the bee.
 *
 ******************************************************************************/
int
SAC_MT_AssignBeeGlobalId (struct sac_bee_common_t *bee)
{
    /* NOTE: It is not nice to touch the all_bees[] here without holding the lock,
     * but it will work fine as long as the pointer itself is always updated
     * atomically, i.e. there must not be any transient NULL states even when
     * the lock is held. */
    assert (SAC_MT_beehive_registry.all_bees
            && "The global SAC_MT_beehive_registry was not properly initialized.");

    pthread_mutex_lock (&SAC_MT_beehive_registry.lock);

    for (unsigned i = 0; i < SAC_MT_beehive_registry.ab_size; ++i) {
        if (SAC_MT_beehive_registry.all_bees[i] == NULL) {
            /* found a slot! */
            SAC_MT_beehive_registry.all_bees[i] = bee;
            bee->global_id = i;
            pthread_mutex_unlock (&SAC_MT_beehive_registry.lock);
            return 0; /* ok */
        }
    }
    /* failed - no empty slot */

    /* enlarge the array by 2X */
    unsigned int new_bs = 2 * SAC_MT_beehive_registry.ab_size;
    struct sac_bee_common_t **new_arr
      = (struct sac_bee_common_t **)realloc (SAC_MT_beehive_registry.all_bees,
                                             sizeof (void *) * new_bs);
    if (!new_arr) {
        /* cannot enlarge the array; fail */
        pthread_mutex_unlock (&SAC_MT_beehive_registry.lock);
        return -1;
    }
    unsigned int half = SAC_MT_beehive_registry.ab_size;
    SAC_MT_beehive_registry.all_bees = new_arr;
    SAC_MT_beehive_registry.ab_size = new_bs;
    /* clean the upper half of the array which has just been allocated */
    memset (&SAC_MT_beehive_registry.all_bees[half], 0, sizeof (void *) * half);

    /* the slot at [half] is certainly empty, return it */
    SAC_MT_beehive_registry.all_bees[half] = bee;
    bee->global_id = half;
    pthread_mutex_unlock (&SAC_MT_beehive_registry.lock);
    return 0; /* ok */
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_ReleaseBeeGlobalId(struct sac_bee_common_t *bee)
 *
 * description:
 *
 *   Releases a bee's unique global ID. Returns 0 on success, -1 on error.
 *
 ******************************************************************************/
int
SAC_MT_ReleaseBeeGlobalId (struct sac_bee_common_t *bee)
{
    pthread_mutex_lock (&SAC_MT_beehive_registry.lock);
    /* check */
    assert (SAC_MT_beehive_registry.all_bees[bee->global_id] == bee);
    /* release */
    SAC_MT_beehive_registry.all_bees[bee->global_id] = NULL;
    bee->global_id = SAC_MT_INVALID_GLOBAL_ID; /* set nonsense */
    /* unlock */
    pthread_mutex_unlock (&SAC_MT_beehive_registry.lock);
    return 0;
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_MT_BeesGrandTotal()
 *
 * description:
 *
 *   Return the total number of known bees in an environment.
 *
 ******************************************************************************/
unsigned int
SAC_MT_BeesGrandTotal ()
{
    pthread_mutex_lock (&SAC_MT_beehive_registry.lock);

    unsigned cnt = 0;
    for (unsigned i = 0; i < SAC_MT_beehive_registry.ab_size; ++i) {
        if (SAC_MT_beehive_registry.all_bees[i]) {
            ++cnt;
        }
    }

    pthread_mutex_unlock (&SAC_MT_beehive_registry.lock);
    return cnt;
}

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
    // memset(hive, 0, sizeof_hive);

    /* allocate an array of pointers to bees */
    hive->num_bees = num_bees;
    hive->bees = (struct sac_bee_common_t **)SAC_CALLOC (num_bees, sizeof (void *));
    if (!hive->bees) {
        SAC_RuntimeError ("Could not allocate memory for an array of ptrs to bees.");
    }
    // memset(hive->bees, 0, sizeof(void*) * num_bees);

    // SAC_TR_PRINT( ("Initializing Tasklocks."));
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
        for (int n = 0; n < num_schedulers; ++n) {
            /* init pthread_mutex_t
             * SAC_MT_Tasklock[SAC_SET_THREADS_MAX*SAC_SET_NUM_SCHEDULERS]; */
            for (int i = 0; (unsigned int)i < num_bees; ++i) {
                pthread_mutex_init (&(SAC_MT_TASKLOCK_INIT (hive, n, i, num_schedulers)),
                                    NULL);
                // pthread_mutex_init( &(hive->SAC_MT_Tasklock[n + num_schedulers * i]),
                // NULL);
            }

            /* init pthread_mutex_t SAC_MT_TS_Tasklock[SAC_SET_NUM_SCHEDULERS]; */
            pthread_mutex_init (&(hive->SAC_MT_TS_Tasklock[n]), NULL);
        }
    }

    /* allocate array of bees; one less than the total number of bees because
     * the bee #0 is the queen */
    void *other_bees = SAC_CALLOC ((num_bees - 1), sizeof_bee);
    if (!other_bees) {
        SAC_RuntimeError ("Could not allocate memory for an array of bees.");
    }
    // memset(other_bees, 0, sizeof_bee * (num_bees - 1));

    /* setup common info in bees */
    for (unsigned i = 1; i < num_bees; ++i) {
        /* NOTE: the index below is (i-1) because other_bees[] contains only slave bees,
         * not the queen bee itself */
        struct sac_bee_common_t *b
          = (struct sac_bee_common_t *)other_bees + sizeof_bee * (i - 1);
        /* put a bee into hive */
        hive->bees[i] = b;
        /* set b->c.global_id */
        if (SAC_MT_AssignBeeGlobalId (b)) {
            SAC_RuntimeError ("Could not assign a bee global ID.");
        }
        /* set bee's data */
        b->local_id = i;
        b->thread_id = SAC_PHM_THREADID_INVALID; /* must be assigned within the thread! */
        b->hive = hive;
    }

    /* Computing task class of the queen bee. */

    for (hive->queen_class = 1; hive->queen_class < num_bees; hive->queen_class <<= 1)
        ;

    hive->queen_class >>= 1;

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
    /* the other_bees ptr below is equal to the identically-named variable in
     * SAC_MT_Helper_AllocHiveCommons() */
    struct sac_bee_common_t *other_bees = hive->bees[1];
    SAC_FREE (other_bees);

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
 * @fn void SAC_MT_TR_Generic_AttachHive(struct sac_hive_common_t* hive,
 *                                       struct sac_bee_common_t *queen)
 *
 * @brief Attach the hive to the queen.
 *  A generic code that works both for LPEL and PTH backends.
 *
 *****************************************************************************/
#if TRACE
void
SAC_MT_TR_Generic_AttachHive (struct sac_hive_common_t *hive,
                              struct sac_bee_common_t *queen)
#else
void
SAC_MT_Generic_AttachHive (struct sac_hive_common_t *hive, struct sac_bee_common_t *queen)
#endif
{
    /* check: the hive must not be already attached */
    if (hive->bees[0]) {
        SAC_RuntimeError ("AttachHive: Cannot attach a hive which is already attached!"
                          " Call DetachHive() first.");
        return;
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
 * @fn struct sac_hive_common_t* SAC_MT_TR_Generic_DetachHive(struct sac_bee_common_t
 **queen)
 *
 * @brief Detach a hive from the queen bee and return a handle to it.
 *
 *****************************************************************************/
#if TRACE
struct sac_hive_common_t *
SAC_MT_TR_Generic_DetachHive (struct sac_bee_common_t *queen)
#else
struct sac_hive_common_t *
SAC_MT_Generic_DetachHive (struct sac_bee_common_t *queen)
#endif
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
 *    Initializes the global-id registry.
 *
 ******************************************************************************/
void
SAC_MT_BEEHIVE_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                             unsigned int max_threads)
{
    /* allocate the array, clear it, and initialize the lock */
    SAC_MT_beehive_registry.all_bees
      = (struct sac_bee_common_t **)SAC_CALLOC (max_threads, sizeof (void *));
    if (!SAC_MT_beehive_registry.all_bees) {
        SAC_RuntimeError ("Could not allocate memory for the global array of bee ptrs.");
    }
    // memset(SAC_MT_beehive_registry.all_bees, 0, sizeof(void*) * max_threads);
    SAC_MT_beehive_registry.ab_size = max_threads;
    pthread_mutex_init (&SAC_MT_beehive_registry.lock, NULL);
}

#else /* defined(PTH) || defined(LPEL) else */

#if !TRACE
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

int SAC_MT_propagate_lock;        /* dummy */
int SAC_MT_output_lock;           /* dummy */
int SAC_MT_global_num_hives = 0;  /* dummy */
void *SAC_MT_singleton_queen = 0; /* dummy */

#ifdef ENABLE_MT_LPEL
/**
 * If MT_LPEL is enabled in SAC, and the stdlib is compiled to use it,
 * but the actual program is SEQ, then we need to stub those Lpel functions
 * linked in the stdlib. The functions will not be used.
 */

#include "sac.h" /* for SAC_RuntimeError() */

#define STUB(fname)                                                                      \
    void fname (void)                                                                    \
    {                                                                                    \
        SAC_RuntimeError (                                                               \
          "Stub " #fname                                                                 \
          " called! You must not link against libsac.seq.so when using LPEL!");          \
    }

STUB (LpelTaskYield)
STUB (LpelBiSemaCountWaiting)
STUB (LpelBiSemaWait)
STUB (LpelBiSemaSignal)

#endif

#endif /* !TRACE */

#endif /* defined(PTH) || defined(LPEL) */

#endif /* ENABLE_MT */
