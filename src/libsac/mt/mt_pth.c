/*****************************************************************************
 *
 * file:   mt_pth.c
 *
 * prefix: SAC
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains routines and global identifiers required by the Pthread
 *   multi-threaded runtime system.
 *
 * remark:
 *
 *****************************************************************************/

#include "fun-attrs.h"

/******************************************************************************
 *
 * If all defines are falls this source file is "emptry".
 * Emptry source files are not allowed by the C99 standard
 * For this reason a dummy variable is declared.
 *
 ******************************************************************************/
static UNUSED int dummy_mt_pth;

/*
 * In case we do not have mt available, we have to make sure this file
 * does not cause any problems (e.g. when running implicit dependency
 * inference system). Therefore, we render this file empty iff MT compilation
 * is disabled!
 */

#if defined(SAC_MT_LIB_pthread) /* the code is only loaded into libsac.mt.pth */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// FIXME: sort out the mess of using sac.h from code as well as from libsac !
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_PTHREAD 1
#define SAC_DO_THREADS_STATIC 1
#define SAC_DO_COMPILE_MODULE 1
#define SAC_SET_NUM_SCHEDULERS 10
// FIXME: do we need this? doesn't that undermine noPHM?
//        well, SAC_HM_DiscoversThreads requires this....
#define SAC_DO_PHM    1

#include "mt.h"
#include "mt_pth.h"
#include "runtime/mt_h/rt_mt_barriers.h" // SAC_MT_PTH_SIGNAL_BARRIER
#include "libsac/hwloc/cpubind.h"
#include "libsac/essentials/trace.h" // SAC_TR_LIBSAC_PRINT
#include "runtime/phm_h/phm.h" // SAC_HM_DiscoversThreads
#include "runtime/rtspec_h/rtspec.h" // SAC_RTSPEC_CURRENT_THREAD_ID
#include "libsac/essentials/message.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE
#undef SAC_SET_NUM_SCHEDULERS

/*
 *  Definition of global variables.
 *
 *    These variables define the multi-threaded runtime system.
 */

/* pthreads default thread attributes (const after init) */
pthread_attr_t SAC_MT_thread_attribs;

/* TLS key to retrieve the Thread Self Bee Ptr */
pthread_key_t SAC_MT_self_bee_key;

/******************************************************************************
 *
 * function:
 *   static inline struct sac_bee_pth_t *SAC_MT_PTH_determine_self()
 *
 * description:
 *
 *  find the self-bee pointer using task local storage in pthreads
 *
 ******************************************************************************/
static inline struct sac_bee_pth_t *
SAC_MT_PTH_determine_self (void)
{
    return (struct sac_bee_pth_t *)pthread_getspecific (SAC_MT_self_bee_key);
}

/******************************************************************************
 *
 * function:
 *   static unsigned int spmd_kill_pth_bee(struct sac_bee_pth_t * const SAC_MT_self)
 *
 * description:
 *
 *  This SPMD function causes a bee to perform a suicide.
 *
 ******************************************************************************/
static unsigned int
spmd_kill_pth_bee (struct sac_bee_pth_t *const SAC_MT_self)
{
    SAC_MT_PTH_release_lck (&SAC_MT_self->stop_lck);
    /* by resetting the key value we prevent pthreads from calling a destructor hook
     * (tls_destroy_self_bee_key) on the thread */
    pthread_setspecific (SAC_MT_self_bee_key, NULL);
    pthread_exit (NULL);
    /* never reached here */
}

/******************************************************************************
 *
 * function:
 *   static void tls_destroy_self_bee_key(void *data)
 *
 * description:
 *
 *  This hook is automatically called by pthreads whenever thread with
 *  non-NULL SAC_MT_self_bee_key associated with it terminates.
 *  If this happens for a queen-bee, we must release it.
 *  Note that slave bees will be deallocated elsewhere.
 *
 ******************************************************************************/
static void
tls_destroy_self_bee_key (void *data)
{
    /* NOTE: this is only called when data != NULL */
    assert (data);
    /* This is not nice. Pthreads have already reset the key value to NULL
     * and then passed us the original value in the data argument.
     * To make our live easier we put things back, then perform a standard
     * cleanup. */
    pthread_setspecific (SAC_MT_self_bee_key, data);

    struct sac_bee_pth_t *self = (struct sac_bee_pth_t *)data;

    /* slave bees should be cleaned up via spmd_kill_pth_bee */
    assert (self->c.local_id == 0);

    SAC_MT_ReleaseQueen ();

    /* check that no value was left in there */
    assert (pthread_getspecific (SAC_MT_self_bee_key) == NULL);
}

/******************************************************************************
 *
 * function:
 *   static void ThreadServeLoop(struct sac_bee_pth_t *SAC_MT_self)
 *
 * description:
 *
 *   This thread function is executed by worker threads to server the queen
 *   bee in a loop.
 *
 ******************************************************************************/
static void
ThreadServeLoop (struct sac_bee_pth_t *SAC_MT_self)
{
    volatile unsigned *sharedfl;
    unsigned *locfl;

    for (;;) {
        sharedfl = &CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->start_barr_sharedfl;
        locfl = &SAC_MT_self->start_barr_locfl;

        SAC_TR_LIBSAC_PRINT (("Worker thread H:%p, L:%d ready.", SAC_MT_self->c.hive,
                              SAC_MT_self->c.local_id));
        SAC_TR_LIBSAC_PRINT (("Worker thread L:%d takes barrier type: %i.",
                              SAC_MT_self->c.local_id, SAC_MT_barrier_type));

        /* wait on start lock: the queen will release it when all is ready
         * for an SPMD execution */
        switch (SAC_MT_barrier_type) {
        case 1:
            take_mutex_barrier ();
            break;
        case 2:
            wait_on_cond_barrier (sharedfl, locfl);
            break;
#ifndef __APPLE__
        case 3:
            take_pthread_barrier ();
            break;
#endif
#ifdef __linux__
        case 4:
            wait_on_futex_barrier (sharedfl, locfl);
            break;
#endif
        default:
            SAC_MT_PTH_wait_on_barrier (locfl, sharedfl);
        }

        /* check there is a hive */
        assert (SAC_MT_self->c.hive);

        /* run SPMD; the barrier is in the function */
        CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)->spmd_fun (SAC_MT_self);
    }
}

/******************************************************************************
 *
 * function:
 *   void ThreadControl(void *arg)
 *
 * description:
 *
 *   This thread control function is executed by worker threads immediately
 *   after creation. After some thread specific initalizations, further worker
 *   threads are created depending on the worker class.
 *
 *   Afterwards, the worker thread enters a non-terminating loop where it waits
 *   at a barrier for work to be assigned and returns after finishing its task.
 *
 ******************************************************************************/
static void *
ThreadControl (void *arg)
{
    /* This is executed in the bee >1 of a new hive */
    struct sac_bee_pth_t *const SAC_MT_self = (struct sac_bee_pth_t *)arg;
    assert (SAC_MT_self && SAC_MT_self->c.hive);
    assert (SAC_MT_self->c.local_id >= 2);

    pthread_setspecific (SAC_MT_self_bee_key, SAC_MT_self);
    SAC_MT_self->c.thread_id = (SAC_HM_DiscoversThreads ()) ? SAC_HM_CurrentThreadId ()
                                                            : SAC_MT_self->c.local_id;

    /* correct worker class */
    while ((SAC_MT_self->c.local_id + SAC_MT_self->c.b_class)
           >= SAC_MT_self->c.hive->num_bees) {
        SAC_MT_self->c.b_class >>= 1;
    }

    SAC_TR_LIBSAC_PRINT (("This is worker thread H:%p, L:%u, T:%u with class %u.",
                          SAC_MT_self->c.hive, SAC_MT_self->c.local_id,
                          SAC_MT_self->c.thread_id, SAC_MT_self->c.b_class));

    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive);

    /* create other */
    for (unsigned i = SAC_MT_self->c.b_class; i; i >>= 1) {
        assert ((SAC_MT_self->c.local_id + i) < hive->c.num_bees);
        struct sac_bee_pth_t *bee
          = CAST_BEE_COMMON_TO_PTH (hive->c.bees[SAC_MT_self->c.local_id + i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_LIBSAC_PRINT (("Creating thread L:%u with maximum class %u.",
                              bee->c.local_id, bee->c.b_class));

        if (0 != pthread_create (&bee->pth, &SAC_MT_thread_attribs, ThreadControl, bee)) {

            SAC_RuntimeError ("Multi Thread Error: worker thread L:%u failed to create"
                              "worker thread L:%u",
                              SAC_MT_self->c.local_id, bee->c.local_id);
        }
    }

#if ENABLE_HWLOC
    if (SAC_HWLOC_topology) {
        int ret;
        ret = hwloc_set_cpubind (SAC_HWLOC_topology,
                                 SAC_HWLOC_cpu_sets[SAC_MT_self->c.local_id],
                                 HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
        if (ret == -1) {
            SAC_RuntimeError (("Could not bind thread; turn -mt_bind off"));
        } else {
            SAC_TR_LIBSAC_PRINT (("bound thread %d", SAC_MT_self->c.local_id));
        }
    }
#endif

    ThreadServeLoop (SAC_MT_self);
    return SAC_MT_self;
}

/******************************************************************************
 *
 * function:
 *   void* ThreadControlInitialWorker(void *arg)
 *
 * description:
 *
 *   This function is a special thread control function for the first worker
 *   thread. This thread creates additional threads on behalf of the master
 *   thread. As an optimization technique, the master thread does not create
 *   worker threads according to its thread class. Instead, the master thread
 *   only creates the first worker thread which in turn creates additional
 *   worker threads.
 *
 ******************************************************************************/
static void *
ThreadControlInitialWorker (void *arg)
{
    /* This is executed in the bee #1 of a new hive */
    struct sac_bee_pth_t *const SAC_MT_self = (struct sac_bee_pth_t *)arg;
    assert (SAC_MT_self && SAC_MT_self->c.hive);
    assert (SAC_MT_self->c.local_id == 1);

    pthread_setspecific (SAC_MT_self_bee_key, SAC_MT_self);
    SAC_MT_self->c.b_class = 0;
    SAC_MT_self->c.thread_id = (SAC_HM_DiscoversThreads ()) ? SAC_HM_CurrentThreadId ()
                                                            : SAC_MT_self->c.local_id;

    SAC_TR_LIBSAC_PRINT (("This is worker thread L:1, H:%p, T:%d with class 0.",
                          SAC_MT_self->c.hive, SAC_MT_self->c.thread_id));

    /* start creating other bees */
    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive);
    const unsigned queen_class = hive->c.queen_class;

    for (unsigned i = queen_class; i > 1; i >>= 1) {
        assert (i < hive->c.num_bees);
        struct sac_bee_pth_t *bee = CAST_BEE_COMMON_TO_PTH (hive->c.bees[i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_LIBSAC_PRINT (
          ("Creating thread #%u with maximum class %u.", i, bee->c.b_class));

        if (0 != pthread_create (&bee->pth, &SAC_MT_thread_attribs, ThreadControl, bee)) {

            SAC_RuntimeError ("Multi Thread Error: worker thread #1 failed to create"
                              "worker thread #%u",
                              i);
        }
    }

#if ENABLE_HWLOC
    if (SAC_HWLOC_topology) {
        int ret;
        ret = hwloc_set_cpubind (SAC_HWLOC_topology,
                                 SAC_HWLOC_cpu_sets[SAC_MT_self->c.local_id],
                                 HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
        if (ret == -1) {
            SAC_RuntimeError (("Could not bind thread; turn -mt_bind off"));
        } else {
            SAC_TR_LIBSAC_PRINT (("bound thread %d", SAC_MT_self->c.local_id));
        }
    }
#endif

    ThreadServeLoop (SAC_MT_self);
    return SAC_MT_self;
}

/******************************************************************************
 *
 * function:
 *   static void do_setup_pth(void)
 *
 * description:
 *
 *   Common PTH set-up needed both in standalone and librarized setting.
 *
 ******************************************************************************/
static void
do_setup_pth (void)
{
    SAC_TR_LIBSAC_PRINT (("Setting up POSIX thread attributes"));

    if (0 != pthread_key_create (&SAC_MT_self_bee_key, tls_destroy_self_bee_key)) {
        SAC_RuntimeError (
          "Unable to create thread specific data key (SAC_MT_self_bee_key).");
    }

    if (0 != pthread_attr_init (&SAC_MT_thread_attribs)) {
        SAC_RuntimeError ("Unable to initialize POSIX thread attributes");
    }

    if (0 != pthread_attr_setscope (&SAC_MT_thread_attribs, PTHREAD_SCOPE_SYSTEM)) {
        SAC_RuntimeWarning ("Unable to set POSIX thread attributes to "
                            "PTHREAD_SCOPE_SYSTEM.\n"
                            "Probably, your PTHREAD implementation does "
                            "not support system \n"
                            "scope threads, i.e. threads are likely not "
                            "to be executed in \n"
                            "parallel, but in time-sharing mode.");
    }

#if 0 /* we join threads in __ReleaseHive() */
  if (0 != pthread_attr_setdetachstate( &SAC_MT_thread_attribs,
                                        PTHREAD_CREATE_DETACHED)) {
    SAC_RuntimeWarning( "Unable to set POSIX thread attributes to "
                        "PTHREAD_CREATE_DETACHED."
                        "Probably, your PTHREAD implementation does "
                        "not support detached \n"
                        "threads. This may cause some runtime "
                        "overhead.");
  }
#endif
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_PTH_SetupInitial( int argc, char *argv[],
 *                             unsigned int num_threads,
 *                             unsigned int max_threads)
 *
 * description:
 *
 *   This function implements an initial setup of the runtime system for
 *   multi-threaded program execution. Here initializations are made which
 *   may not wait till worker thread creation. Basically, these are
 *   - the creation of the thread specific data key which holds the thread ID,
 *   - the initialization of the thread specific data for the master thread,
 *   - the evaluation of the -mt command line option.
 *
 *   These setups need to be performed *before* the heap is initialised
 *   because heap initialisation requires the number of threads. Moreover,
 *   we need the thread specific data key to access the current thread id
 *   for legacy memory allocations.
 *
 ******************************************************************************/

void
SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                     unsigned int max_threads)
{
    SAC_MT_BEEHIVE_SetupInitial (argc, argv, num_threads, max_threads);

    do_setup_pth ();

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (argc, argv, num_threads, max_threads);
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_SetupAsLibraryInitial(void)
 *
 * description:
 *
 *  This is a limited initialization code needed when sac is executed as a library
 *  via sac4c. As we do not know the total number of threads, we just pass
 *  some defaults, and do not initialize the thread-id registry.
 *
 ******************************************************************************/
void
SAC_MT_SetupAsLibraryInitial (void)
{
    SAC_MT_BEEHIVE_SetupInitial (0, NULL, 1024, 1024);

    do_setup_pth ();

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (0, NULL, 1024, 1024);

    /* In a library we're never alone. */
    SAC_MT_globally_single = 0;
}

/******************************************************************************
 *
 * function:
 *   struct sac_bee_pth_t *EnsureThreadHasBee(void)
 *
 * description:
 *
 *  Make sure the current thread has the bee stub assigned to it.
 *  Creates a new queen stub if there is none.
 *
 ******************************************************************************/
static struct sac_bee_pth_t *
EnsureThreadHasBee (void)
{
    struct sac_bee_pth_t *self = SAC_MT_PTH_determine_self ();

    if (self) {
        /* ok, there's a bee already */
        return self;
    }

    /* allocate the bee structure */
    self = (struct sac_bee_pth_t *)SAC_CALLOC (1, sizeof (struct sac_bee_pth_t));
    if (!self) {
        SAC_RuntimeError ("Could not allocate memory for the first bee.");
    }

    /* set bee's data */
    self->c.local_id = 0;
    self->c.thread_id
      = (SAC_HM_DiscoversThreads ()) ? SAC_HM_CurrentThreadId () : self->c.local_id;
    /* init locks */
    // SAC_MT_INIT_START_LCK(self);
    SAC_MT_INIT_BARRIER (self);

    /* set key value in this thread */
    if (0 != pthread_setspecific (SAC_MT_self_bee_key, self)) {
        SAC_RuntimeError (
          "Unable to initialize thread specific data (SAC_MT_self_bee_key).");
    }

    assert (SAC_MT_PTH_determine_self () == self);

    /* increment the number of queens */
    __sync_add_and_fetch (&SAC_MT_cnt_queen_bees, 1);

    return self;
}

/******************************************************************************
 *
 * function:
 *    void SAC_MT_ReleaseHive(struct sac_hive_common_t* h)
 *
 * description:
 *
 *  Release the given hive. The hive should be already detached (no queen in it).
 *  All bees in the hive will be killed and the memory deallocated.
 *
 ******************************************************************************/
void
SAC_MT_ReleaseHive (struct sac_hive_common_t *h)
{
    if (!h) {
        /* no hive, no work */
        return;
    }
    if (h->bees[0]) {
        /* there is a queen! */
        SAC_RuntimeError ("SAC_MT_ReleaseHive: Cannot release a hive with a queen."
                          " Call DetachHive() first.");
    }

    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (h);
    /* setup spmd function which kills each bee */
    hive->spmd_fun = spmd_kill_pth_bee;

    volatile unsigned *sharedfl = &hive->start_barr_sharedfl;

    /* start slave bees */
    switch (SAC_MT_barrier_type) {
    case 1:
        take_mutex_barrier ();
        break;
    case 2:
        lift_cond_barrier (sharedfl);
        break;
#ifndef __APPLE__
    case 3:
        take_pthread_barrier ();
        break;
#endif
#ifdef __linux__
    case 4:
        lift_futex_barrier (sharedfl);
        break;
#endif
    default:
        SAC_MT_PTH_signal_barrier (NULL, sharedfl);
    }

    /* wait on bees until done */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        pthread_join (CAST_BEE_COMMON_TO_PTH (hive->c.bees[i])->pth, NULL);
    }

    /* now, all slave bees should be dead; release data */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        SAC_MT_PTH_destroy_lck (&CAST_BEE_COMMON_TO_PTH (hive->c.bees[i])->stop_lck);
    }

    /* release the memory */
    SAC_MT_Helper_FreeHiveCommons (&hive->c);
}

/******************************************************************************
 *
 * function:
 *   struct sac_hive_common_t* SAC_MT_AllocHive( unsigned int num_bees, int
 *num_schedulers, const int *places, void *thdata)
 *
 * description:
 *
 *  Allocate a new hive. The hive will contain (num_bees-1) new bees (threads).
 *  The places and thdata arguments are ignored.
 *
 ******************************************************************************/

struct sac_hive_common_t *
SAC_MT_AllocHive (unsigned int num_bees, int num_schedulers, const int *places,
                  void *thdata)
{
    SAC_TR_LIBSAC_PRINT (("Initializing the bee data structure."));

    if (places) {
        SAC_RuntimeWarning ("SAC_MT_AllocHive: places not used in the PTH backed.");
    }
    if (thdata) {
        SAC_RuntimeWarning ("SAC_MT_AllocHive: thdata not used in the PTH backed.");
    }

    assert (num_bees >= 1);

    struct sac_hive_pth_t *hive = CAST_HIVE_COMMON_TO_PTH (
      SAC_MT_Helper_AllocHiveCommons (num_bees, num_schedulers,
                                      sizeof (struct sac_hive_pth_t),
                                      sizeof (struct sac_bee_pth_t)));

    /* setup extended info in bees */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        struct sac_bee_pth_t *b = CAST_BEE_COMMON_TO_PTH (hive->c.bees[i]);
        /* init locks */
        SAC_MT_INIT_BARRIER (b);
    }

    SAC_TR_LIBSAC_PRINT (
      ("Thread class of master thread is %d.", (int)hive->c.queen_class));

    if (hive->c.num_bees > 1) {
        SAC_TR_LIBSAC_PRINT (("Creating worker thread #1 of class 0"));

        if (0
            != pthread_create (&CAST_BEE_COMMON_TO_PTH (hive->c.bees[1])->pth,
                               &SAC_MT_thread_attribs, ThreadControlInitialWorker,
                               hive->c.bees[1])) {
            SAC_RuntimeError ("Unable to create (initial) worker thread #1");
        }
    }

#if ENABLE_HWLOC
    if (SAC_HWLOC_topology) {
        int ret;
        ret = hwloc_set_cpubind (SAC_HWLOC_topology, SAC_HWLOC_cpu_sets[0],
                                 HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
        if (ret == -1) {
            SAC_RuntimeError (("Could not bind thread; turn -mt_bind off"));
        } else {
            SAC_TR_LIBSAC_PRINT (("bound thread 0"));
        }
    }
#endif

    return &hive->c;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_ReleaseQueen(void)
 *
 * description:
 *
 *  Release the queen-bee stub from the current thread context (TLS).
 *  If a hive is attached, it will be released as well.
 *
 ******************************************************************************/
void
SAC_MT_ReleaseQueen (void)
{
    SAC_TR_LIBSAC_PRINT (("Finalizing hive."));

    struct sac_bee_pth_t *self = SAC_MT_PTH_determine_self ();

    if (!self) {
        /* no queen, ok */
        return;
    }

    if (self->c.hive) {
        /* ensure we're a queen in the hive */
        if (self->c.hive->bees[0] != &self->c) {
            SAC_RuntimeError ("Only the queen can tear down a hive!");
        }

        /* destroy the hive */
        SAC_MT_ReleaseHive (SAC_MT_DetachHive ());
    }

    /* the queen must be free of a hive now */
    assert (self->c.hive == NULL);

    /* release the queen bee structure associated with this thread */
    SAC_MT_PTH_destroy_lck (&self->stop_lck);
    SAC_FREE (self);

    /* set self bee ptr */
    pthread_setspecific (SAC_MT_self_bee_key, NULL);

    /* decrement the number of queens */
    __sync_sub_and_fetch (&SAC_MT_cnt_queen_bees, 1);
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_AttachHive(struct sac_hive_common_t* h)
 *
 * description:
 *
 *  Attach a hive to the current queen-bee.
 *  The generic version from mt_beehive is used.
 *
 ******************************************************************************/
void
SAC_MT_AttachHive (struct sac_hive_common_t *h)
{
    SAC_TR_LIBSAC_PRINT (("Attaching hive to a queen."));

    if (!h) {
        SAC_RuntimeError ("SAC_MT_AttachHive called with a NULL hive!");
    }

    /* allocate a bee for the current thread, if needed */
    struct sac_bee_pth_t *queen = EnsureThreadHasBee ();
    /* generic attach func */
    SAC_MT_Generic_AttachHive (h, &queen->c);
}

/******************************************************************************
 *
 * function:
 *   struct sac_hive_common_t* SAC_MT_DetachHive(void)
 *
 * description:
 *
 *  Detach the hive from the current queen-bee.
 *  The generic version from mt_beehive is used.
 *
 ******************************************************************************/
struct sac_hive_common_t *
SAC_MT_DetachHive (void)
{
    SAC_TR_LIBSAC_PRINT (("Detaching hive from a queen."));

    struct sac_bee_pth_t *queen = SAC_MT_PTH_determine_self ();
    /* generic detach func */
    return SAC_MT_Generic_DetachHive (&queen->c);
}

/** =====================================================================================
 */

/******************************************************************************
 *
 * function:
 *   void SAC_MT_PTH_SetupStandalone( int num_schedulers)
 *
 * description:
 *
 *  Initializes stand-alone SAC program environment.
 *  Allocates and attaches a new hive to the calling thread.
 *
 ******************************************************************************/
void
SAC_MT_PTH_SetupStandalone (int num_schedulers)
{
    struct sac_hive_common_t *hive
      = SAC_MT_AllocHive (SAC_MT_global_threads, num_schedulers, NULL, NULL);
    SAC_MT_AttachHive (hive);
    /* In standalone programs there is only a single global queen-bee. Place her in the
     * global variable. All the ST functions will take it from there. */
    SAC_MT_singleton_queen = hive->bees[0];
}

/******************************************************************************
 * function:
 *   struct sac_bee_common_t* SAC_MT_CurrentBee(void)
 *
 * description:
 *
 ******************************************************************************/
struct sac_bee_common_t *
SAC_MT_CurrentBee (void)
{
    struct sac_bee_pth_t *bee = SAC_MT_PTH_determine_self ();

    if (bee == NULL) {
        return NULL;
    } else {
        return &bee->c;
    }
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_MT_Internal_CurrentThreadId(void)
 *
 * description:
 *
 *  Return the Thread ID of the current thread.
 *  Called from PHM if it does not maintain its own thread ids.
 *
 ******************************************************************************/
unsigned int
SAC_MT_Internal_CurrentThreadId (void)
{
    if (SAC_MT_globally_single) {
        return SAC_RTSPEC_CURRENT_THREAD_ID ();
    } else {
        struct sac_bee_common_t *bee = SAC_MT_CurrentBee ();

        if (bee == NULL) {
            return SAC_RTSPEC_CURRENT_THREAD_ID ();
        } else {
            return SAC_MT_CurrentBee ()
              ->local_id; // LPEL may require this to be thread_id...
        }
    }
}

#else /* MT */

#include "mt_pth.h"

int SAC_MT_self_bee_key; /* dummy */

#endif /* PTH */
