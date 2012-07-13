/*****************************************************************************
 *
 * file:   mt_autothid.c
 *
 * prefix: SAC_MT_
 *
 * description:
 *    Automatic thread registry.
 *    The registry assigns unique IDs (numbers) from the range [0, num_threads)
 *    to any thread in the environment. This is needed for the Heap Manager
 *    when used in LPEL or library (sac4c) setting.
 *
 * remark:
 *    This heavily relies on PThreads and their Threads Local Storage (TLS).
 *    We create a TLS variable SAC_MT_phmthr_registry. Whenever a new thread
 *    is created this variable is initialized to NULL. When we are asked for
 *    a thread ID of the current thread we look at the TLS variable. When it is
 *    NULL a new thread ID is assigned from the pool to the thread
 *    and the TLS variable is set to point to an integer element holding the ID.
 *    Later whenever the ID of the same thread is required we just return the
 *    contents of the pointer.
 *
 *    The TLS variable has a destructur assigned to it. Whenever thread terminates
 *    the pthreads library will call our destructor and we will release the thread ID
 *    of the terminated thread into a pool.
 *
 *****************************************************************************/

#include "config.h"

int _dummy_mt_autothid;

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
 * Thread ID Registry structure
 */
struct sac_phm_thread_registry_t {
    /* Write lock */
    pthread_mutex_t lock;
    /* array of thread ids */
    unsigned int *th_numbers;

    /* size of the array */
    unsigned int th_size;
    /* last alloc'ed or free'd position (an optimization) */
    unsigned int last_pos;

    /* TLS key to retrieve the Thread ID for PHM */
    pthread_key_t threadid_key;
};

/* global registry */
static struct sac_phm_thread_registry_t SAC_MT_phmthr_registry;

/******************************************************************************
 *
 * function:
 *   static void tls_destroy_threadid_key(void *data)
 *
 * description:
 *
 *  This hook is automatically called by pthreads whenever thread with
 *  non-NULL threadid_key associated with it terminates.
 *
 ******************************************************************************/
static void
tls_destroy_threadid_key (void *data)
{
    /* NOTE: Pthreads has already reset the key value to NULL. */
    /* Check that we've been initialized properly. */
    assert (SAC_MT_phmthr_registry.th_numbers);

    pthread_mutex_lock (&SAC_MT_phmthr_registry.lock);

    /* the thread number */
    const unsigned th_num = *((unsigned *)data);
    /* position in the array */
    const unsigned th_pos = (unsigned *)data - SAC_MT_phmthr_registry.th_numbers;

    /* sanity checks */
    assert (th_num != SAC_PHM_THREADID_INVALID);
    assert (th_pos < SAC_MT_phmthr_registry.th_size);
    assert (SAC_MT_phmthr_registry.th_numbers[th_pos] == th_num);

    /* release the position */
    SAC_MT_phmthr_registry.last_pos = th_pos;
    SAC_MT_phmthr_registry.th_numbers[th_pos] = SAC_PHM_THREADID_INVALID;

    pthread_mutex_unlock (&SAC_MT_phmthr_registry.lock);
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_MT_AutoAssignThreadId()
 *
 * description:
 *
 *  Allocate and assign a thread_id number to the calling thread.
 *
 ******************************************************************************/
static unsigned int
SAC_MT_AutoAssignThreadId (void)
{
    /* Check that we've been initialized properly. */
    assert (SAC_MT_phmthr_registry.th_numbers);

    pthread_mutex_lock (&SAC_MT_phmthr_registry.lock);

    unsigned th_pos = SAC_MT_phmthr_registry.last_pos;
    int found = 0;

    for (unsigned i = 0; i < SAC_MT_phmthr_registry.th_size; ++i) {
        if (SAC_MT_phmthr_registry.th_numbers[th_pos] == SAC_PHM_THREADID_INVALID) {
            /* found a free position */
            found = 1;
            break;
        }
        /* inc th_pos, go round the size */
        th_pos = (th_pos + 1) % SAC_MT_phmthr_registry.th_size;
    }

    if (found) {
        /* th_pos is the free position; take it */
        SAC_MT_phmthr_registry.th_numbers[th_pos] = th_pos;
        SAC_MT_phmthr_registry.last_pos = th_pos;
        /* set the TLS key to point into the th_numbers array at the assigned position */
        pthread_setspecific (SAC_MT_phmthr_registry.threadid_key,
                             &SAC_MT_phmthr_registry.th_numbers[th_pos]);
    }

    /* unlock the mutex in any case */
    pthread_mutex_unlock (&SAC_MT_phmthr_registry.lock);

    /* it is a critical error if there's no free space */
    if (!found) {
        SAC_RuntimeError ("SAC-MT-PHM Automatic thread registry:"
                          " I was told there will no more than %d threads, and now you "
                          "have more!",
                          SAC_MT_phmthr_registry.th_size);
        abort ();
    }

    return th_pos;
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_InitThreadRegistry(unsigned int num_threads)
 *
 * description:
 *
 *  Initialize.
 *
 ******************************************************************************/
void
SAC_MT_InitThreadRegistry (unsigned int num_threads)
{
    SAC_TR_PRINT (("Initializing automatic thread registry, expecting max. %d threads.",
                   num_threads));

    struct sac_phm_thread_registry_t *const r = &SAC_MT_phmthr_registry;
    assert (!r->th_numbers && "Thread registry has been already initialized!");

    /* clear */
    memset (r, 0, sizeof (struct sac_phm_thread_registry_t));

    /* alloc an array of ids */
    r->th_numbers = (unsigned *)SAC_MALLOC (sizeof (unsigned) * num_threads);
    if (!r->th_numbers) {
        SAC_RuntimeError ("Could not allocate memory for the thread registry array.");
    }

    r->th_size = num_threads;
    r->last_pos = 0;
    pthread_mutex_init (&r->lock, NULL);

    for (int i = 0; i < num_threads; ++i) {
        r->th_numbers[i] = SAC_PHM_THREADID_INVALID;
    }

    if (pthread_key_create (&r->threadid_key, tls_destroy_threadid_key)) {
        SAC_FREE (r->th_numbers);
        r->th_numbers = NULL;
        SAC_RuntimeError ("Unable to create thread specific data key "
                          "(SAC_MT_phmthr_registry.threadid_key).");
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_MT_UnusedThreadRegistry(void)
 *
 * description:
 *
 *  Initialize as unused: SAC_MT_CurrentThreadId() will always return 0.
 *
 ******************************************************************************/
void
SAC_MT_UnusedThreadRegistry (void)
{
    SAC_TR_PRINT (("Initializing automatic thread registry as unused."));

    struct sac_phm_thread_registry_t *const r = &SAC_MT_phmthr_registry;
    assert (!r->th_numbers && "Thread registry has been already initialized!");

    memset (r, 0, sizeof (struct sac_phm_thread_registry_t));

    /* alloc an array of ids */
    r->th_numbers = (unsigned *)SAC_MALLOC (sizeof (unsigned) * 1);
    if (!r->th_numbers) {
        SAC_RuntimeError ("Could not allocate memory for the thread registry array.");
    }

    r->th_size = 0;
    r->last_pos = 0;
    pthread_mutex_init (&r->lock, NULL);

    for (int i = 0; i < 1; ++i) {
        r->th_numbers[i] = SAC_PHM_THREADID_INVALID;
    }
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_MT_CurrentThreadId(void)
 *
 * description:
 *
 *  Return the unique thread ID of the current thread.
 *  Assign a new one if there is none.
 *
 ******************************************************************************/
unsigned int
SAC_MT_CurrentThreadId (void)
{
    /* Check that we've been initialized properly. */
#if 0
  assert(SAC_MT_phmthr_registry.th_numbers
        && "SAC-MT-PHM Automatic thread registry: was not initialized!");
#else
//   static int warned = 0;
#endif

    if (SAC_MT_phmthr_registry.th_numbers) {
        if (SAC_MT_phmthr_registry.th_size > 0) {
            /* normal mode */
            unsigned *p_th_num
              = (unsigned *)pthread_getspecific (SAC_MT_phmthr_registry.threadid_key);

            if (p_th_num) {
                /* already assigned */
                return *p_th_num;
            } else {
                /* invent new */
                return SAC_MT_AutoAssignThreadId ();
            }
        } else {
            /* marked as won't-use; return 0 always */
            return 0;
        }
    } else {
        /* err... */
        //     if (!warned) {
        SAC_RuntimeWarning (
          "SAC-MT-PHM Automatic thread registry: accessed while not initialized (yet)!");
        //       warned = 1;
        //     }
        return 0;
    }
}

#endif /* defined(PTH) || defined(LPEL) */

#endif /* ENABLE_MT */
