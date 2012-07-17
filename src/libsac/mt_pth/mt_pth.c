/*
 * $Id: mt.c 16774 2010-03-10 17:10:12Z sah $
 */

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
 *   By means of the make tool, this source file is used to produce two
 *   different object files:
 *   1. mt_pth.o        which contains the normal routines
 *   2. mt_pth.trace.o  which contains variants of the normal routines along
 *                      with some extra routines and identifiers for tracing
 *                      program execution.
 *
 *   The compiler produces different object files by setting or unsetting
 *   the preprocessor flag TRACE.
 *
 *****************************************************************************/

#include "config.h"

/******************************************************************************
 *
 * If all defines are falls this source file is "emptry".
 * Emptry source files are not allowed by the C99 standard
 * For this reason a dummy variable is declared.
 *
 ******************************************************************************/
static int dummy_mt_pth;

/*
 * In case we do not have mt available, we have to make sure this file
 * does not cause any problems (e.g. when running implicit dependency
 * inference system). Therefore, we render this file empty iff MT compilation
 * is disabled!
 */

#if ENABLE_MT

#ifdef PTH /* the code is only loaded into libsac.mt.pth */

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
#define SAC_DO_MT_PTHREAD 1
#define SAC_DO_THREADS_STATIC 1
#define SAC_DO_COMPILE_MODULE 1
#define SAC_SET_NUM_SCHEDULERS 10

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE
#undef SAC_SET_NUM_SCHEDULERS

#if ENABLE_HWLOC
#include <../runtime/mt_h/hwloc_data.h>
#endif

#if TRACE
#define __ReleaseHive SAC_MT_TR_ReleaseHive
#define __AllocHive SAC_MT_TR_AllocHive
#define __ThreadServeLoop SAC_MT_LPEL_TR_ThreadServeLoop
#define __AttachHive SAC_MT_TR_AttachHive
#define __DetachHive SAC_MT_TR_DetachHive
#define __ReleaseQueen SAC_MT_TR_ReleaseQueen
#else
/* no trace */
#define __ReleaseHive SAC_MT_ReleaseHive
#define __AllocHive SAC_MT_AllocHive
#define __ThreadServeLoop SAC_MT_LPEL_ThreadServeLoop
#define __AttachHive SAC_MT_AttachHive
#define __DetachHive SAC_MT_DetachHive
#define __ReleaseQueen SAC_MT_ReleaseQueen
#endif

/*
 *  Definition of global variables.
 *
 *    These variables define the multi-threaded runtime system.
 */
#if !TRACE

/*
 * If we compile for mt_trace.o, we don't need the global variables since
 * these always remain in mt.o.
 */

/* pthreads default thread attributes (const after init) */
pthread_attr_t SAC_MT_thread_attribs;

/* TLS key to retrieve the Thread Self Bee Ptr */
pthread_key_t SAC_MT_self_bee_key;

#else

/* defined when !TRACE, declared when TRACE */

#endif /* TRACE */

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
    return 0;
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

    __ReleaseQueen ();

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
    for (;;) {
        SAC_TR_PRINT (("Worker thread G:%d, L:%d ready.", SAC_MT_self->c.global_id,
                       SAC_MT_self->c.local_id));

        /* wait on start lock: the queen will release it when all is ready
         * for an SPMD execution */
        // SAC_MT_PTH_acquire_lck(&SAC_MT_self->start_lck);
        SAC_MT_PTH_wait_on_barrier (&SAC_MT_self->start_barr_locfl,
                                    &CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive)
                                       ->start_barr_sharedfl);

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
    SAC_MT_self->c.thread_id = SAC_MT_CurrentThreadId ();

    /* correct worker class */
    while ((SAC_MT_self->c.local_id + SAC_MT_self->c.b_class)
           >= SAC_MT_self->c.hive->num_bees) {
        SAC_MT_self->c.b_class >>= 1;
    }

    SAC_TR_PRINT (("This is worker thread G:%u, L:%u, T:%u with class %u.",
                   SAC_MT_self->c.global_id, SAC_MT_self->c.local_id,
                   SAC_MT_self->c.thread_id, SAC_MT_self->c.b_class));

    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive);

    /* create other */
    for (unsigned i = SAC_MT_self->c.b_class; i; i >>= 1) {
        assert ((SAC_MT_self->c.local_id + i) < hive->c.num_bees);
        struct sac_bee_pth_t *bee
          = CAST_BEE_COMMON_TO_PTH (hive->c.bees[SAC_MT_self->c.local_id + i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_PRINT (("Creating thread L:%u with maximum class %u.", bee->c.local_id,
                       bee->c.b_class));

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
                                 SAC_HWLOC_cpu_sets[SAC_MT_self->c.global_id],
                                 HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
        if (ret == -1) {
            SAC_RuntimeError (("Could not bind thread"));
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
    SAC_MT_self->c.thread_id = SAC_MT_CurrentThreadId ();

    SAC_TR_PRINT (("This is worker thread L:1, G:%d, T:%d with class 0.",
                   SAC_MT_self->c.global_id, SAC_MT_self->c.thread_id));

    /* start creating other bees */
    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (SAC_MT_self->c.hive);
    const unsigned queen_class = hive->c.queen_class;

    for (unsigned i = queen_class; i > 1; i >>= 1) {
        assert (i < hive->c.num_bees);
        struct sac_bee_pth_t *bee = CAST_BEE_COMMON_TO_PTH (hive->c.bees[i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_PRINT (("Creating thread #%u with maximum class %u.", i, bee->c.b_class));

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
                                 SAC_HWLOC_cpu_sets[SAC_MT_self->c.global_id],
                                 HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
        if (ret == -1) {
            SAC_RuntimeError (("Could not bind thread"));
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
    SAC_TR_PRINT (("Setting up POSIX thread attributes"));

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
 *   void SAC_MT_PTH_TR_SetupInitial( int argc, char *argv[],
 *                                unsigned int num_threads,
 *                                unsigned int max_threads)
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

#if TRACE
void
SAC_MT_TR_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                        unsigned int max_threads)
#else
void
SAC_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                     unsigned int max_threads)
#endif
{
    SAC_MT_BEEHIVE_SetupInitial (argc, argv, num_threads, max_threads);

    do_setup_pth ();

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (argc, argv, num_threads, max_threads);

    /* init thread-id assignment array, after we know the number of threads. */
    SAC_MT_InitThreadRegistry (SAC_MT_global_threads);
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_TR_SetupAsLibraryInitial(void)
 *
 * description:
 *
 *  This is a limited initialization code needed when sac is executed as a library
 *  via sac4c. As we do not know the total number of threads, we just pass
 *  some defaults, and do not initialize the thread-id registry.
 *
 ******************************************************************************/
#if TRACE
void
SAC_MT_TR_SetupAsLibraryInitial (void)
#else
void
SAC_MT_SetupAsLibraryInitial (void)
#endif
{
    SAC_MT_BEEHIVE_SetupInitial (0, NULL, 1024, 1024);

    do_setup_pth ();

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (0, NULL, 1024, 1024);

    /* mark the thread registry as unused: SAC_MT_CurrentThreadId() will always return 0.
     * The thread_id field in bees will be invalid (always zero). */
    SAC_MT_UnusedThreadRegistry ();
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

    /* init */
    // memset(self, 0, sizeof(struct sac_bee_pth_t));

    /* set bee's data */
    self->c.local_id = 0;
    self->c.thread_id = SAC_MT_CurrentThreadId ();
    /* init locks */
    // SAC_MT_INIT_START_LCK(self);
    SAC_MT_INIT_BARRIER (self);

    if (SAC_MT_AssignBeeGlobalId (&self->c)) {
        /* oops! */
        SAC_RuntimeError ("Could not register the bee!");
    }

    /* set key value in this thread */
    if (0 != pthread_setspecific (SAC_MT_self_bee_key, self)) {
        SAC_RuntimeError (
          "Unable to initialize thread specific data (SAC_MT_self_bee_key).");
    }

    assert (SAC_MT_PTH_determine_self () == self);

    return self;
}

/******************************************************************************
 *
 * function:
 *    void SAC_MT_TR_ReleaseHive(struct sac_hive_common_t* h)
 *    void SAC_MT_ReleaseHive(struct sac_hive_common_t* h)
 *
 * description:
 *
 *  Release the given hive. The hive should be already detached (no queen in it).
 *  All bees in the hive will be killed and the memory deallocated.
 *
 ******************************************************************************/
#if TRACE
void
SAC_MT_TR_ReleaseHive (struct sac_hive_common_t *h)
#else
void
SAC_MT_ReleaseHive (struct sac_hive_common_t *h)
#endif
{
    if (!h) {
        /* no hive, no work */
        return;
    }
    if (h->bees[0]) {
        /* there is a queen! */
        SAC_RuntimeError ("SAC_MT_*_ReleaseHive: Cannot release a hive with a queen."
                          " Call DetachHive() first.");
        return;
    }

    struct sac_hive_pth_t *const hive = CAST_HIVE_COMMON_TO_PTH (h);
    /* setup spmd function which kills each bee */
    hive->spmd_fun = spmd_kill_pth_bee;

    /* start slave bees */
    SAC_MT_PTH_signal_barrier (NULL, &hive->start_barr_sharedfl);

    //   for (unsigned i = 1; i < hive->c.num_bees; ++i) {
    //     struct sac_bee_pth_t *b = CAST_BEE_COMMON_TO_PTH(hive->c.bees[i]);
    //     SAC_MT_RELEASE_START_LCK(b);
    //   }

    /* wait on bees until done */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        pthread_join (CAST_BEE_COMMON_TO_PTH (hive->c.bees[i])->pth, NULL);
    }

    /* now, all slave bees should be dead; release data */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        SAC_MT_ReleaseBeeGlobalId (hive->c.bees[i]);

        // SAC_MT_PTH_destroy_lck(&CAST_BEE_COMMON_TO_PTH(hive->c.bees[i])->start_lck);
        SAC_MT_PTH_destroy_lck (&CAST_BEE_COMMON_TO_PTH (hive->c.bees[i])->stop_lck);
    }

    /* release the memory */
    SAC_MT_Helper_FreeHiveCommons (&hive->c);

    /* decrement the number of hives in the environment */
    /* FIXME: in a library sac setting we probably don't want to decrement the number of
     * hives anytime */
    __sync_sub_and_fetch (&SAC_MT_global_num_hives, 1);
}

/******************************************************************************
 *
 * function:
 *   struct sac_hive_common_t* SAC_MT_TR_AllocHive( unsigned int num_bees, int
 *num_schedulers, const int *places, void *thdata)
 *
 * description:
 *
 *  Allocate a new hive. The hive will contain (num_bees-1) new bees (threads).
 *  The places and thdata arguments are ignored.
 *
 ******************************************************************************/
#if TRACE
struct sac_hive_common_t *
SAC_MT_TR_AllocHive (unsigned int num_bees, int num_schedulers, const int *places,
                     void *thdata)
#else
struct sac_hive_common_t *
SAC_MT_AllocHive (unsigned int num_bees, int num_schedulers, const int *places,
                  void *thdata)
#endif
{
    SAC_TR_PRINT (("Initializing the bee data structure."));

    if (places) {
        SAC_RuntimeWarning ("SAC_MT_AllocHive: places not used in the PTH backed.");
    }
    if (thdata) {
        SAC_RuntimeWarning ("SAC_MT_AllocHive: thdata not used in the PTH backed.");
    }

    assert (num_bees >= 1);

    /* increment the number of hives in the system */
    if (__sync_add_and_fetch (&SAC_MT_global_num_hives, 1) > 1) {
        /* we're not single thread any more */
        SAC_MT_globally_single = 0;
    }

    struct sac_hive_pth_t *hive = CAST_HIVE_COMMON_TO_PTH (
      SAC_MT_Helper_AllocHiveCommons (num_bees, num_schedulers,
                                      sizeof (struct sac_hive_pth_t),
                                      sizeof (struct sac_bee_pth_t)));

    /* setup extended info in bees */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        struct sac_bee_pth_t *b = CAST_BEE_COMMON_TO_PTH (hive->c.bees[i]);
        /* init locks */
        // SAC_MT_INIT_START_LCK(b);
        SAC_MT_INIT_BARRIER (b);
    }

    SAC_TR_PRINT (("Thread class of master thread is %d.", (int)hive->c.queen_class));

    if (hive->c.num_bees > 1) {
        SAC_TR_PRINT (("Creating worker thread #1 of class 0"));

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
            SAC_RuntimeError (("Could not bind thread"));
        }
    }
#endif

    return &hive->c;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_TR_ReleaseQueen(void)
 *   void SAC_MT_ReleaseQueen(void)
 *
 * description:
 *
 *  Release the queen-bee stub from the current thread context (TLS).
 *  If a hive is attached, it will be released as well.
 *
 ******************************************************************************/
#if TRACE
void
SAC_MT_TR_ReleaseQueen (void)
#else
void
SAC_MT_ReleaseQueen (void)
#endif
{
    SAC_TR_PRINT (("Finalizing hive."));

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
        __ReleaseHive (__DetachHive ());
    }

    /* the queen must be free of a hive now */
    assert (self->c.hive == NULL);

    /* release the queen bee structure associated with this thread */
    SAC_MT_ReleaseBeeGlobalId (&self->c);
    // SAC_MT_PTH_destroy_lck(&self->start_lck);
    SAC_MT_PTH_destroy_lck (&self->stop_lck);
    SAC_FREE (self);

    /* set self bee ptr */
    pthread_setspecific (SAC_MT_self_bee_key, NULL);
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_TR_AttachHive(struct sac_hive_common_t* h)
 *   void SAC_MT_AttachHive(struct sac_hive_common_t* h)
 *
 * description:
 *
 *  Attach a hive to the current queen-bee.
 *  The generic version from mt_beehive is used.
 *
 ******************************************************************************/
#if TRACE
void
SAC_MT_TR_AttachHive (struct sac_hive_common_t *h)
#else
void
SAC_MT_AttachHive (struct sac_hive_common_t *h)
#endif
{
    SAC_TR_PRINT (("Attaching hive to a queen."));

    if (!h) {
        SAC_RuntimeError ("__AttachHive called with a NULL hive!");
        return;
    }

    /* allocate a bee for the current thread, if needed */
    struct sac_bee_pth_t *queen = EnsureThreadHasBee ();
    /* generic attach func */
    SAC_MT_Generic_AttachHive (h, &queen->c);
}

/******************************************************************************
 *
 * function:
 *   struct sac_hive_common_t* SAC_MT_TR_DetachHive()
 *   struct sac_hive_common_t* SAC_MT_DetachHive()
 *
 * description:
 *
 *  Detach the hive from the current queen-bee.
 *  The generic version from mt_beehive is used.
 *
 ******************************************************************************/
#if TRACE
struct sac_hive_common_t *
SAC_MT_TR_DetachHive ()
#else
struct sac_hive_common_t *
SAC_MT_DetachHive ()
#endif
{
    SAC_TR_PRINT (("Detaching hive from a queen."));

    struct sac_bee_pth_t *queen = SAC_MT_PTH_determine_self ();
    /* generic detach func */
    return SAC_MT_Generic_DetachHive (&queen->c);
}

/** =====================================================================================
 */

/******************************************************************************
 *
 * function:
 *   void SAC_MT_PTH_TR_SetupStandalone( int num_schedulers)
 *   void SAC_MT_PTH_SetupStandalone( int num_schedulers)
 *
 * description:
 *
 *  Initializes stand-alone SAC program environment.
 *  Allocates and attaches a new hive to the calling thread.
 *
 ******************************************************************************/
#if TRACE
void
SAC_MT_PTH_TR_SetupStandalone (int num_schedulers)
#else
void
SAC_MT_PTH_SetupStandalone (int num_schedulers)
#endif
{
    struct sac_hive_common_t *hive
      = __AllocHive (SAC_MT_global_threads, num_schedulers, NULL, NULL);
    __AttachHive (hive);
    /* In standalone programs there is only a single global queen-bee. Place her in the
     * global variable. All the ST functions will take it from there. */
    SAC_MT_singleton_queen = hive->bees[0];
}

#if !TRACE
/******************************************************************************
 * function:
 *   struct sac_bee_common_t* SAC_MT_CurrentBee()
 *
 * description:
 *
 ******************************************************************************/
struct sac_bee_common_t *
SAC_MT_CurrentBee ()
{
    return &SAC_MT_PTH_determine_self ()->c;
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_Get_Global_ThreadID(void)
 *
 * description:
 *
 *  Return the Global Thread ID of the current thread.
 *  Used in PHM.
 *
 ******************************************************************************/
unsigned int
SAC_Get_Global_ThreadID (void)
{
    return SAC_MT_CurrentThreadId ();
}

/******************************************************************************
 * function:
 *   unsigned int SAC_Get_CurrentBee_GlobalID(void)
 *
 * description:
 *
 *   Return the Global Bee ID of the current thread, if possible.
 *   This is only used for trace prints.
 *
 ******************************************************************************/
unsigned int
SAC_Get_CurrentBee_GlobalID (void)
{
    unsigned int result = SAC_MT_INVALID_GLOBAL_ID;
    struct sac_bee_pth_t *self = SAC_MT_PTH_determine_self ();

    if (self) {
        result = self->c.global_id;
    }

    return result;
}
#endif

#if 0
/******************************************************************************
 *
 * function:
 *   void SAC_MT1_Setup( int num_schedulers)
 *   void SAC_MT1_TR_Setup( int num_schedulers)
 *
 * description:
 *
 *   This function initializes the runtime system for multi-threaded
 *   program execution. The basic steps performed are
 *   - Initialisation of the Scheduler Mutexlocks SAC_MT_TASKLOCKS
 *   - determining the thread class of the master thread,
 *   - creation and initialization of POSIX thread attributes,
 *
 ******************************************************************************/
#if TRACE
void SAC_MT1_TR_Setup( int num_schedulers)
#else
void SAC_MT1_Setup( int num_schedulers)
#endif
{
  int i,n;

  SAC_TR_PRINT( ("Initializing Tasklocks."));

  for (n=0;n< num_schedulers;n++){
    pthread_mutex_init( &(SAC_MT_TS_TASKLOCK(n)),NULL);
    for (i = 0; (unsigned int)i < SAC_MT_threads; i++) {
      pthread_mutex_init( &(SAC_MT_TASKLOCK_INIT( n, i,num_schedulers)), NULL);
    }
  }

  SAC_TR_PRINT( ("Computing thread class of master thread."));

  for (SAC_MT_masterclass = 1;
       SAC_MT_masterclass < SAC_MT_threads;
       SAC_MT_masterclass <<= 1);

  SAC_MT_masterclass >>= 1;

  SAC_TR_PRINT( ("Thread class of master thread is %d.",
                (int) SAC_MT_masterclass));

  if (SAC_MT_threads > 1) {

    SAC_MT1_internal_id = (pthread_t*)SAC_MALLOC( SAC_MT_threads * sizeof(pthread_t));

    if (SAC_MT1_internal_id == NULL) {
      SAC_RuntimeError( "Unable to allocate memory for internal thread identifiers");
    }

    SAC_TR_PRINT( ("Setting up POSIX thread attributes"));

    if (0 != pthread_attr_init( &SAC_MT_thread_attribs)) {
      SAC_RuntimeError( "Unable to initialize POSIX thread attributes");
    }

    if (0 != pthread_attr_setscope( &SAC_MT_thread_attribs,
                                    PTHREAD_SCOPE_SYSTEM)) {
      SAC_RuntimeWarning( "Unable to set POSIX thread attributes to "
                          "PTHREAD_SCOPE_SYSTEM.\n"
                          "Probably, your PTHREAD implementation does "
                          "not support system \n"
                          "scope threads, i.e. threads are likely not "
                          "to be executed in \n"
                          "parallel, but in time-sharing mode.");
    }

#if ENABLE_HWLOC
  if (SAC_HWLOC_topology)
  {
    int ret;
    ret = hwloc_set_cpubind(SAC_HWLOC_topology, SAC_HWLOC_cpu_sets[SAC_MT_master_id], HWLOC_CPUBIND_THREAD | HWLOC_CPUBIND_STRICT);
    if (ret == -1)
    {
      SAC_RuntimeError(("Could not bind thread"));
    }
  }
#endif

  }
}

#if !TRACE

static
void ThreadControl_MT1(void *arg)
{
  const unsigned int my_thread_id = (unsigned long int) arg;
  volatile unsigned int worker_flag = 0;

  pthread_setspecific(SAC_MT_threadid_key, &my_thread_id);

  worker_flag = (*SAC_MT_spmd_function)(my_thread_id, 0, worker_flag);
}


void SAC_MT1_StartWorkers()
{
  unsigned long int i;

  for (i=1; i<SAC_MT_threads; i++) {
    if (0 != pthread_create( &SAC_MT1_internal_id[i],
                             &SAC_MT_thread_attribs,
                             (void *(*) (void *)) ThreadControl_MT1,
                             (void *) (i))) {

      SAC_RuntimeError("Master thread failed to create worker thread #%u", i);
    }
  }
}

#endif
#endif

#else /* MT */

#if !TRACE
/* !MT && !TRACE */
int SAC_MT_self_bee_key; /* dummy */

#endif /* TRACE */

#endif /* PTH */

#endif /* ENABLE_MT */
