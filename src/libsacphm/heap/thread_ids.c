/*****************************************************************************
 *
 * file:   thread_ids.c
 *
 * prefix: SAC_HM_
 *
 * description:
 *    Autonomous thread discovery.
 *    The registry assigns unique IDs (numbers) from the range [0, num_threads)
 *    to any thread in the environment.
 *
 * remark:
 *    This heavily relies on PThreads and their Threads Local Storage (TLS).
 *    We create a TLS variable SAC_HM_threads. Whenever a new thread
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
 *    This is compiled in only when SAC_DO_HM_DISCOVER_THREADS is 1.
 *    When disabled we resubmit all requests back to the MT layer.
 *    The MT layer normally keeps the IDs for all threads created by SAC runtime.
 *
 *****************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "heapmgr.h"

#if SAC_DO_HM_DISCOVER_THREADS

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
static struct sac_phm_thread_registry_t SAC_HM_threads;

/******************************************************************************
 *
 * function:
 *   static void tls_destroy_threadid_key(void *data)
 *
 * description:
 *
 *  This hook is automatically called by pthreads whenever a thread with
 *  non-NULL threadid_key terminates.
 *
 ******************************************************************************/
static void
tls_destroy_threadid_key (void *data)
{
    /* NOTE: Pthreads has already reset the key value to NULL. */
    /* Check that we've been initialized properly. */
    assert (SAC_HM_threads.th_numbers);

    pthread_mutex_lock (&SAC_HM_threads.lock);

    /* the thread number */
    const unsigned th_num = *((unsigned *)data);
    /* position in the array */
    const unsigned th_pos = (unsigned *)data - SAC_HM_threads.th_numbers;

    /* sanity checks */
    assert (th_num != SAC_HM_THREADID_INVALID);
    assert (th_pos < SAC_HM_threads.th_size);
    assert (SAC_HM_threads.th_numbers[th_pos] == th_num);

    /* release the position */
    SAC_HM_threads.last_pos = th_pos;
    SAC_HM_threads.th_numbers[th_pos] = SAC_HM_THREADID_INVALID;

    pthread_mutex_unlock (&SAC_HM_threads.lock);
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_HM_AutoAssignThreadId()
 *
 * description:
 *
 *  Allocate and assign a thread_id number to the calling thread.
 *
 ******************************************************************************/
static unsigned int
SAC_HM_AutoAssignThreadId (void)
{
    /* Check that we've been initialized properly. */
    assert (SAC_HM_threads.th_numbers);

    pthread_mutex_lock (&SAC_HM_threads.lock);

    unsigned th_pos = SAC_HM_threads.last_pos;
    int found = 0;

    for (unsigned i = 0; i < SAC_HM_threads.th_size; ++i) {
        if (SAC_HM_threads.th_numbers[th_pos] == SAC_HM_THREADID_INVALID) {
            /* found a free position */
            found = 1;
            break;
        }
        /* inc th_pos, go round the size */
        th_pos = (th_pos + 1) % SAC_HM_threads.th_size;
    }

    if (found) {
        /* th_pos is the free position; take it */
        SAC_HM_threads.th_numbers[th_pos] = th_pos;
        SAC_HM_threads.last_pos = th_pos;
        /* set the TLS key to point into the th_numbers array at the assigned position */
        pthread_setspecific (SAC_HM_threads.threadid_key,
                             &SAC_HM_threads.th_numbers[th_pos]);
    }

    /* unlock the mutex in any case */
    pthread_mutex_unlock (&SAC_HM_threads.lock);

    /* it is a critical error if there's no free space */
    if (!found) {
        SAC_RuntimeError ("SAC-MT-HM Automatic thread registry:"
                          " More threads than expected (%d)!",
                          SAC_HM_threads.th_size);
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
SAC_HM_InitThreadRegistry (void)
{
    /* We cannot perform (don't want) any malloc() before the phm is initialized! */
    unsigned int num_threads = SAC_HM_ASSUME_THREADS_MAX;
    static unsigned prealloc_th_numbers[SAC_HM_ASSUME_THREADS_MAX];

    //   SAC_TR_PRINT( ("Initializing thread registry, expecting max. %d threads.",
    //                 num_threads));

    struct sac_phm_thread_registry_t *const r = &SAC_HM_threads;
    assert (!r->th_numbers && "Thread registry has been already initialized!");

    /* clear */
    memset (r, 0, sizeof (struct sac_phm_thread_registry_t));

    /* alloc an array of ids */
    //   r->th_numbers = (unsigned *) SAC_MALLOC(sizeof(unsigned) * num_threads);
    r->th_numbers = prealloc_th_numbers;
    if (!r->th_numbers) {
        SAC_RuntimeError ("Could not allocate memory for the thread registry array.");
    }

    r->th_size = num_threads;
    r->last_pos = 0;
    pthread_mutex_init (&r->lock, NULL);

    for (unsigned i = 0; i < num_threads; ++i) {
        r->th_numbers[i] = SAC_HM_THREADID_INVALID;
    }

    if (pthread_key_create (&r->threadid_key, tls_destroy_threadid_key)) {
        SAC_FREE (r->th_numbers);
        r->th_numbers = NULL;
        SAC_RuntimeError (
          "Unable to create thread specific data key (SAC_HM_threads.threadid_key).");
    }
}

#if 0
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
void SAC_MT_UnusedThreadRegistry(void)
{
  SAC_TR_PRINT( ("Initializing automatic thread registry as unused."));

  struct sac_phm_thread_registry_t *const r = &SAC_HM_threads;
  assert(!r->th_numbers && "Thread registry has been already initialized!");

  memset(r, 0, sizeof(struct sac_phm_thread_registry_t));

  /* alloc an array of ids */
  r->th_numbers = (unsigned *) SAC_MALLOC(sizeof(unsigned) * 1);
  if (!r->th_numbers) {
    SAC_RuntimeError( "Could not allocate memory for the thread registry array.");
  }

  r->th_size = 0;
  r->last_pos = 0;
  pthread_mutex_init(&r->lock, NULL);

  for (int i = 0; i < 1; ++i) {
    r->th_numbers[i] = SAC_HM_THREADID_INVALID;
  }
}
#endif

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_HM_CurrentThreadId(void)
 *
 * description:
 *
 *  Return the unique thread ID of the current thread.
 *  Assign a new one if there is none.
 *  This is external interface.
 *
 ******************************************************************************/
unsigned int
SAC_HM_CurrentThreadId (void)
{
    if (SAC_HM_threads.th_numbers == NULL) {
        /* initialize */
        SAC_HM_InitThreadRegistry ();
    }

    unsigned *p_th_num = (unsigned *)pthread_getspecific (SAC_HM_threads.threadid_key);

    if (p_th_num) {
        /* already assigned */
        return *p_th_num;
    } else {
        /* invent new */
        return SAC_HM_AutoAssignThreadId ();
    }
}

/******************************************************************************
 *
 * function:
 *   int SAC_HM_DiscoversThreads(void)
 *
 * description:
 *
 *  Returns 1 to indicate that the implementation provides the thread ids.
 *  This is external interface.
 *
 ******************************************************************************/
int
SAC_HM_DiscoversThreads (void)
{
    /* This informs the MT layer that we provide unique thread IDs to all threads
     * (both ours and foreign) in the environment. */
    return 1;
}

#else /* SAC_DO_HM_DISCOVER_THREADS */

/** ****************************************************************************** */

/* non-XT variant: The thread registry is not used. Rely on the MT library to provide
 * its own thread ids. */

/*  Called from PHM if it does not maintain its own thread ids. */
SAC_C_EXTERN unsigned int SAC_MT_Internal_CurrentThreadId (void); /* in mt layer */

unsigned int
SAC_HM_CurrentThreadId (void)
{
    /* ask the MT layer about the thread ID it has assigned */
    return SAC_MT_Internal_CurrentThreadId ();
}

int
SAC_HM_DiscoversThreads (void)
{
    /* This informs the MT layer that it has to assign threads IDs to its threads.
     * The downside is that no other threads than those created by the SAC runtime
     * are allowed in the environment. */
    return 0;
}

#endif /* SAC_DO_HM_DISCOVER_THREADS */
