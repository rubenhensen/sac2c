/*****************************************************************************
 *
 * file:   mt_lpel.inc.c
 *
 * prefix: SAC_MT_
 *
 * description:
 *  LPEL MT backend.
 *
 * remark:
 *  This is included in mt_lpel_std.c and mt_lpel_trace.c.
 *
 *****************************************************************************/

#include "config.h"

int dummy_mt_lpel;

#if ENABLE_MT && ENABLE_MT_LPEL

/* the code is only loaded into libsac.mt.lpel */
#if defined(LPEL)

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
#define SAC_DO_MT_LPEL 1
// #define SAC_DO_THREADS_STATIC 1
#define SAC_DO_COMPILE_MODULE 1
#define SAC_SET_NUM_SCHEDULERS 10

#undef SAC_DO_MT_PTHREAD
#undef SAC_DO_THREADS_STATIC

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_LPEL
#undef SAC_DO_THREADS_STATIC
#undef SAC_DO_COMPILE_MODULE
#undef SAC_SET_NUM_SCHEDULERS

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

#if !TRACE

/* Indicates the control flow has been migrated under the LPEL system.
 * This is because LPEL does not want to tell us if we're riding on an LPEL task
 * or not. And as it may happen that some of our unfortunate code may be called
 * in both context we'd better know if we're allowed to call LPEL task functions. */
int SAC_MT_hopefully_under_lpel = 0;

#else

extern int SAC_MT_hopefully_under_lpel;

#endif

/******************************************************************************
 *
 * function:
 *   static inline struct sac_bee_lpel_t *SAC_MT_LPEL_determine_self()
 *
 * description:
 *
 *  Find the self-bee pointer using task local storage in LPEL.
 *
 ******************************************************************************/
static inline struct sac_bee_lpel_t *
SAC_MT_LPEL_determine_self ()
{
    return LpelGetUserData (LpelTaskSelf ());
}

/******************************************************************************
 *
 * function:
 *   static unsigned int spmd_kill_lpel_bee(struct sac_bee_lpel_t * const SAC_MT_self)
 *
 * description:
 *
 *  This SPMD function causes a bee to perform a suicide.
 *
 ******************************************************************************/
static unsigned int
spmd_kill_lpel_bee (struct sac_bee_lpel_t *const SAC_MT_self)
{
    SAC_TR_PRINT (("Bee G:%d, L:%d, W:%d is terminating.", SAC_MT_self->c.global_id,
                   SAC_MT_self->c.local_id, SAC_MT_self->worker_id));
    LpelBiSemaSignal (&SAC_MT_self->stop_lck);
    LpelSetUserData (SAC_MT_self->tsk, NULL);
    LpelTaskExit (NULL);
    /* never reached here */
    abort ();
}

/******************************************************************************
 *
 * function:
 *   static void AutoReleaseQueen(lpel_task_t *tsk, void *usrdata)
 *
 * description:
 *
 *  This fun releases queen-bee stub. It is called via LPEL Task Local Data
 *  destructor hook.
 *
 ******************************************************************************/
static void
AutoReleaseQueen (lpel_task_t *tsk, void *usrdata)
{
    __ReleaseQueen ();
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_LPEL_TR_ThreadServeLoop(struct sac_bee_lpel_t *SAC_MT_self)
 *
 * description:
 *
 *   This function is executed by worker bees to server the queen
 *   bee until killed.
 *
 ******************************************************************************/
static
#if TRACE
  void
  SAC_MT_LPEL_TR_ThreadServeLoop (struct sac_bee_lpel_t *SAC_MT_self)
#else
  void
  SAC_MT_LPEL_ThreadServeLoop (struct sac_bee_lpel_t *SAC_MT_self)
#endif
{
    for (;;) {
        SAC_TR_PRINT (("LPEL-based bee G:%d, L:%d, W:%d ready.", SAC_MT_self->c.global_id,
                       SAC_MT_self->c.local_id, SAC_MT_self->worker_id));

        /* wait on start lock: the queen will release it when all is ready
         * for an SPMD execution */
        LpelBiSemaWait (&SAC_MT_self->start_lck);

        /* check there is a hive */
        assert (SAC_MT_self->c.hive);

        /* run SPMD; the barrier is in the function */
        CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive)->spmd_fun (SAC_MT_self);
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
    struct sac_bee_lpel_t *const SAC_MT_self = arg;
    assert (SAC_MT_self && SAC_MT_self->c.hive);
    assert (SAC_MT_self->c.local_id >= 2);

    /* set self bee ptr */
    LpelSetUserData (LpelTaskSelf (), SAC_MT_self);
    /* no destructor for userdata in a slave bee */
    SAC_MT_self->c.thread_id = SAC_MT_CurrentThreadId ();

    /* correct worker class */
    while ((SAC_MT_self->c.local_id + SAC_MT_self->c.b_class)
           >= SAC_MT_self->c.hive->num_bees) {
        SAC_MT_self->c.b_class >>= 1;
    }

    SAC_TR_PRINT (("This is bee G:%u, L:%u with class %u at LPEL worker W:%d.",
                   SAC_MT_self->c.global_id, SAC_MT_self->c.local_id,
                   SAC_MT_self->c.b_class, SAC_MT_self->worker_id));

    struct sac_hive_lpel_t *const hive = CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive);

    /* create other */
    for (unsigned i = SAC_MT_self->c.b_class; i; i >>= 1) {
        assert ((SAC_MT_self->c.local_id + i) < hive->c.num_bees);

        struct sac_bee_lpel_t *bee
          = CAST_BEE_COMMON_TO_LPEL (hive->c.bees[SAC_MT_self->c.local_id + i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_PRINT (
          ("Creating task L:%u with maximum class %u at LPEL worker W:%d, %d.",
           bee->c.local_id, bee->c.b_class, bee->worker_id, SAC_MT_global_threads));

        bee->tsk
          = LpelTaskCreate (bee->worker_id, ThreadControl, bee, SAC_MT_LPEL_STACK_SIZE);

        if (!bee->tsk) {
            SAC_RuntimeError ("Multi Thread Error: worker thread #%u failed to create"
                              "worker thread #%u",
                              SAC_MT_self->c.local_id, bee->c.local_id);
        }

        LpelTaskRun (bee->tsk);
    }

    __ThreadServeLoop (SAC_MT_self);
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
    struct sac_bee_lpel_t *const SAC_MT_self = arg;
    assert (SAC_MT_self && SAC_MT_self->c.hive);
    assert (SAC_MT_self->c.local_id == 1);

    /* set self bee ptr */
    LpelSetUserData (LpelTaskSelf (), SAC_MT_self);
    /* no destructor for userdata in a slave bee */
    SAC_MT_self->c.thread_id = SAC_MT_CurrentThreadId ();

    SAC_TR_PRINT (("This is bee G:%d, L:1 with class 0 at LPEL worker W:%d.",
                   SAC_MT_self->c.global_id, SAC_MT_self->worker_id));
    SAC_MT_self->c.b_class = 0;

    /* start creating other bees */
    struct sac_hive_lpel_t *const hive = CAST_HIVE_COMMON_TO_LPEL (SAC_MT_self->c.hive);
    const unsigned queen_class = hive->c.queen_class;

    for (unsigned i = queen_class; i > 1; i >>= 1) {
        assert (i < hive->c.num_bees);

        struct sac_bee_lpel_t *bee = CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i]);
        bee->c.b_class = (i >> 1);

        SAC_TR_PRINT (("Creating task L:%u with maximum class %u on LPEL worker W:%d.", i,
                       bee->c.b_class, bee->worker_id));

        bee->tsk
          = LpelTaskCreate (bee->worker_id, ThreadControl, bee, SAC_MT_LPEL_STACK_SIZE);

        if (!bee->tsk) {
            SAC_RuntimeError ("Multi Thread Error: worker thread #1 failed to create"
                              "worker thread #%u",
                              i);
        }

        LpelTaskRun (bee->tsk);
    }

    __ThreadServeLoop (SAC_MT_self);
    return SAC_MT_self;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_LPEL_TR_SetupInitial( int argc, char *argv[],
 *                                unsigned int num_threads,
 *                                unsigned int max_threads)
 *   void SAC_MT_LPEL_SetupInitial( int argc, char *argv[],
 *                             unsigned int num_threads,
 *                             unsigned int max_threads)
 *
 * description:
 *
 *   This function implements an initial setup of the runtime system for
 *   multi-threaded program execution. Here initializations are made which
 *   may not wait till worker thread creation.
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

    SAC_TR_PRINT (("SAC/LPEL Init"));

    /* none */

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (argc, argv, num_threads, max_threads);

    /* init thread-id assignment array, after we know the number of threads. */
    /* In LPEL we have SAC_MT_global_threads number of workers, plus the main thread.
     * The main thread layes dormant, the queen is a task running on one of the workers.
     * Hence, we have (SAC_MT_global_threads+1) threads in total visible to PHM,
     * but only SAC_MT_global_threads workers/bees.
     */
    /* NOTE: SAC_MT_hm_aux_threads used also in the SAC_HM_SETUP() macro */
    SAC_MT_hm_aux_threads = 1;
    SAC_MT_InitThreadRegistry (SAC_MT_global_threads + SAC_MT_hm_aux_threads);
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

    SAC_TR_PRINT (("SAC/LPEL Init as library"));

    /* none */

    /* common setup: determine the actual number of threads (cmd line/env var)
     * and set SAC_MT_global_threads */
    SAC_COMMON_MT_SetupInitial (0, NULL, 1024, 1024);

    /* do not initialize the heap manager here */
    /* mark the thread registry as unused: SAC_MT_CurrentThreadId() will always return 0.
     * The thread_id field in bees will be invalid (always zero). */
    SAC_MT_UnusedThreadRegistry ();
}

/******************************************************************************
 *
 * function:
 *   struct sac_bee_lpel_t *EnsureThreadHasBee(void)
 *
 * description:
 *
 *  Make sure the current LPEL task has the bee stub assigned to it.
 *  Creates a new queen stub if there is none.
 *
 ******************************************************************************/
static struct sac_bee_lpel_t *
EnsureThreadHasBee (void)
{
    struct sac_bee_lpel_t *self = SAC_MT_LPEL_determine_self ();

    if (self) {
        /* ok, there's a bee already */
        return self;
    }

    SAC_TR_PRINT (("Creating a queen bee in an existing LPEL task."));

    /* allocate the bee structure */
    self = SAC_CALLOC (1, sizeof (struct sac_bee_lpel_t));
    if (!self) {
        SAC_RuntimeError ("Could not allocate memory for the first bee.");
    }

    /* init */
    // memset(self, 0, sizeof(struct sac_bee_lpel_t));

    /* set bee's data */
    self->c.local_id = 0;
    self->c.thread_id = SAC_MT_CurrentThreadId ();
    self->tsk = LpelTaskSelf ();
    self->worker_id = LpelTaskGetWorkerId (self->tsk);
    /* init locks */
    SAC_MT_INIT_START_LCK (self);
    SAC_MT_INIT_BARRIER (self);

    if (SAC_MT_AssignBeeGlobalId (&self->c)) {
        /* oops! */
        SAC_RuntimeError ("Could not register the bee!");
    }

    SAC_TR_PRINT (
      ("The queen bee is registered as G:%d, W:%d.", self->c.global_id, self->worker_id));

    /* set self bee ptr, and destructor */
    LpelSetUserData (LpelTaskSelf (), self);
    LpelSetUserDataDestructor (LpelTaskSelf (), AutoReleaseQueen);

    /* postcondition */
    assert (SAC_MT_LPEL_determine_self () == self);

    return self;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_TR_ReleaseHive(struct sac_hive_common_t* h)
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

    struct sac_hive_lpel_t *const hive = CAST_HIVE_COMMON_TO_LPEL (h);
    SAC_TR_PRINT (("Releasing hive with %d bees.", hive->c.num_bees));

    /* setup spmd function which kills each bee */
    hive->spmd_fun = spmd_kill_lpel_bee;

    /* start slave bees */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        struct sac_bee_lpel_t *b = CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i]);
        SAC_MT_RELEASE_START_LCK (b);
    }

    /* wait on bees until done */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        LpelBiSemaWait (&CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i])->stop_lck);
        /* is there a task join in LPEL? or free? */
    }

    /* now, all slave bees should be dead; release data */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        SAC_MT_ReleaseBeeGlobalId (hive->c.bees[i]);

        LpelBiSemaDestroy (&CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i])->start_lck);
        LpelBiSemaDestroy (&CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i])->stop_lck);
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
 *  Allocate a new hive. The hive will contain (num_bees-1) new bees (LPEL tasks).
 *  The places[] array optionally specifies worker IDs where the tasks will be placed.
 *  The thdata argument is opaque data passed into LPEL.
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
    /* Evil I am, and it will end up in tears. But when used as a library,
     * when can I be sure that we've been moved under LPEL? */
    SAC_MT_hopefully_under_lpel = 1;

    SAC_TR_PRINT (("Allocating a hive with %d bees.", num_bees));
    assert (num_bees >= 1);

    /* increment the number of hives in the system */
    if (__sync_add_and_fetch (&SAC_MT_global_num_hives, 1) > 1) {
        /* we're not single thread any more */
        SAC_MT_globally_single = 0;
    }

    struct sac_hive_lpel_t *hive = CAST_HIVE_COMMON_TO_LPEL (
      SAC_MT_Helper_AllocHiveCommons (num_bees, num_schedulers,
                                      sizeof (struct sac_hive_lpel_t),
                                      sizeof (struct sac_bee_lpel_t)));

    /* the place of the queen; used only when places are not explicitly given */
    const unsigned callers_wkid = (places) ? 0 : LpelTaskGetWorkerId (LpelTaskSelf ());
    const unsigned wk_count = (places) ? 0 : LpelWorkerCount ();

    /* setup extended info in bees */
    for (unsigned i = 1; i < hive->c.num_bees; ++i) {
        struct sac_bee_lpel_t *b = CAST_BEE_COMMON_TO_LPEL (hive->c.bees[i]);
        /* init locks */
        SAC_MT_INIT_START_LCK (b);
        SAC_MT_INIT_BARRIER (b);
        /* set place */
        if (places) {
            b->worker_id = places[i];
        } else {
            /* default placement: round-robin starting at the successor of the caller's
             * position */
            b->worker_id = (i + callers_wkid) % wk_count;
        }
    }

    SAC_TR_PRINT (("Thread class of the queen task is %d.", (int)hive->c.queen_class));

    if (hive->c.num_bees > 1) {
        struct sac_bee_lpel_t *const bee = CAST_BEE_COMMON_TO_LPEL (hive->c.bees[1]);

        SAC_TR_PRINT (
          ("Creating bee L:1 of class 0 on LPEL worker W:%d", bee->worker_id));

        bee->tsk = LpelTaskCreate (bee->worker_id, ThreadControlInitialWorker,
                                   hive->c.bees[1], SAC_MT_LPEL_STACK_SIZE);

        if (!bee->tsk) {
            SAC_RuntimeError ("Unable to create (initial) worker thread #1");
        }

        LpelTaskRun (CAST_BEE_COMMON_TO_LPEL (hive->c.bees[1])->tsk);
    }

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
 *  Release the queen-bee stub from the current LPEL task context (TLS).
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
    SAC_TR_PRINT (("Finalizing the queen."));

    struct sac_bee_lpel_t *self = SAC_MT_LPEL_determine_self ();

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
    LpelBiSemaDestroy (&self->start_lck);
    LpelBiSemaDestroy (&self->stop_lck);
    SAC_FREE (self);

    /* set self bee ptr */
    LpelSetUserData (LpelTaskSelf (), NULL);
    LpelSetUserDataDestructor (LpelTaskSelf (), NULL);
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
    struct sac_bee_lpel_t *queen = EnsureThreadHasBee ();
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

    struct sac_bee_lpel_t *queen = SAC_MT_LPEL_determine_self ();
    /* generic detach func */
    return SAC_MT_Generic_DetachHive (&queen->c);
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
    return &SAC_MT_LPEL_determine_self ()->c;
}

/******************************************************************************
 * function:
 *   unsigned int SAC_Get_Global_ThreadID(void)
 *
 * description:
 *  Return the Global Thread ID of the current thread.
 *  Public function called from the private heap manager.
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
 *   Return the Global Bee ID of the current thread, if possible.
 *   This is only used for trace prints.
 *
 ******************************************************************************/
unsigned int
SAC_Get_CurrentBee_GlobalID (void)
{
    /* We need to determine the self bee, but cannot simply call
     * SAC_MT_LPEL_determine_self(), because if we're actually not in a LPEL task context,
     * it will segfault. Hence we must first check if SAC execution has been already moved
     * under LPEL. */
    unsigned int result = SAC_MT_INVALID_GLOBAL_ID;

    if (SAC_MT_hopefully_under_lpel) {
        struct sac_bee_lpel_t *self = SAC_MT_LPEL_determine_self ();
        if (self) {
            result = self->c.global_id;
        }
    }

    return result;
}
#endif

/** =====================================================================================
 */
/**
 * The following code is needed when this MT LPEL backend is used in a standalone program.
 * This means we have to initialize and babysit LPEL by ourselves.
 *
 * One of the annoying things is that after initializing LPEL we must move our own butt
 * over from the initial thread to the initial LPEL task.
 * This is accomplished via the struct sac_lpel_main_fn_cont and fun.
 * sac_lpel_run_main_wrapper(), and it also requires a little bit of pthreads wizardry.
 */

/* continuation data struct */
struct sac_lpel_main_fn_cont {
    SAC_main_fun_t main_fn;
    int *main_arg;
    int num_schedulers;

    /* condition to signal when SAC is done with LPEL */
    pthread_cond_t done_sig;
};

/** continuation from SAC_MT_LPEL_SetupAndRunStandalone, but inside an LPEL task */
static void *
sac_lpel_run_main_wrapper (void *arg)
{
    struct sac_lpel_main_fn_cont *c = arg;

    SAC_MT_hopefully_under_lpel = 1;

    struct sac_hive_common_t *hive
      = __AllocHive (SAC_MT_global_threads, c->num_schedulers, NULL, NULL);
    __AttachHive (hive);
    /* In standalone programs there is only a single global queen-bee. Place her in the
     * global variable. All the ST functions will take it from there. */
    SAC_MT_singleton_queen = hive->bees[0];

    c->main_fn (c->main_arg);

    __ReleaseHive (__DetachHive ());
    __ReleaseQueen ();
    SAC_MT_singleton_queen = NULL;

    /* This will switch off global_ids in SAC_Get_CurrentBee_GlobalID */
    SAC_MT_hopefully_under_lpel = 0;

    SAC_TR_PRINT (("SAC_Main*() has finished inside its LPEL wrapper task."));
    /* signal the main thread it may start shuttind down LPEL.
     * It just means that unused LPEL workes will be removed. */
    pthread_cond_signal (&c->done_sig);

    return NULL;
}

#if TRACE
void
SAC_MT_LPEL_TR_SetupAndRunStandalone (SAC_main_fun_t main_fn, int *main_arg,
                                      int num_schedulers)
#else
void
SAC_MT_LPEL_SetupAndRunStandalone (SAC_main_fun_t main_fn, int *main_arg,
                                   int num_schedulers)
#endif
{
    /** the problem at hand here is that not only we have to initialize LPEL, but the SAC
     * main function has to be run *inside* an LPEL task. Thus, we create a single LPEL
     * task, move ourselves into it, then continue with AllocHive() (which does most of
     * the initializations) and then we run the SAC main function.
     */
    SAC_TR_PRINT (("Starting LPEL: num_workers=%d", SAC_MT_global_threads));

    /* init LPEL */
    lpel_config_t cfg;
    memset (&cfg, 0, sizeof (lpel_config_t));
    cfg.proc_workers = cfg.num_workers = SAC_MT_global_threads;
    cfg.proc_others = 0;
    cfg.flags = 0;

    LpelInit (&cfg);
    LpelStart ();

    /* continuation */
    struct sac_lpel_main_fn_cont c;
    c.main_fn = main_fn;
    c.main_arg = main_arg;
    c.num_schedulers = num_schedulers;

    if (pthread_cond_init (&c.done_sig, NULL)) {
        SAC_RuntimeError ("Could not create a condition signal!");
    }

    /* create task for the main bee, put it on worker #0 */
    lpel_task_t *main_tsk
      = LpelTaskCreate (0, sac_lpel_run_main_wrapper, &c, SAC_MT_LPEL_STACK_SIZE);

    /* run it */
    LpelTaskRun (main_tsk);

    pthread_mutex_t done_lock; /* dummy lock */
    pthread_mutex_init (&done_lock, NULL);
    pthread_mutex_lock (&done_lock);
    /* wait until sac has completed its LPEL work */
    pthread_cond_wait (&c.done_sig, &done_lock);

    SAC_TR_PRINT (("Stopping LPEL."));

    pthread_cond_destroy (&c.done_sig);
    pthread_mutex_destroy (&done_lock);

    /* wait for all workers to be free of tasks and then terminate */
    LpelStop ();
    LpelCleanup ();

    SAC_TR_PRINT (("LPEL stopped."));
}

#else /* defined(LPEL) else */

#endif /* defined(LPEL) */

#endif /* ENABLE_MT */
