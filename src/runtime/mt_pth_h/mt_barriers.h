/*****************************************************************************
 *
 * file:   mt_barriers.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *    This file is part of the SAC standard header file sac.h
 *    It adds a couple of new start barriers to the SAC runtime
 *    library. Those barriers can be used in stead of the default spin
 *    lock barrier. The new barriers are designed to be more energy
 *    efficient then the original spin lock barrier. Instead of wasting
 *    energy by staying active during a spin lock, the new barriers
 *    are paused during the lock. The -mt_barrier_type compile option
 *    can be used to select which start barrier should be used. The
 *    default option is 'spin', which will select the original spin lock
 *    barrier.
 *
 * important files:
 *    - mt_pth.h
 *      (initialization of barrier: SAC_MT_SETUP_INITIAL macro
 *       signal barrier: SAC_MT_PTH_do_spmd_execute function
 *       destruction of barrier: SAC_MT_FINALIZE macro)
 *    - mt_pth.c
 *      (lock barrier: SAC_MT_ReleaseHive function)
 *
 *****************************************************************************/

#ifndef _MT_BARRIERS
#define _MT_BARRIERS

#include <pthread.h>
//#include <unistd.h>

// futex barrier libraries. The futex barrier is only made available on linux
// systems, because not all systems support this technique.
#ifdef __linux__
#include <limits.h>
#include <sys/syscall.h>
#include <linux/futex.h>

long syscall (long number, ...);
#endif

// make spin lock the default barrier type
#define SAC_MT_PTH_INIT_BARRIER(nrthreads)
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) SAC_MT_PTH_signal_barrier (NULL, glflag)
#define SAC_MT_PTH_DESTROY_BARRIER()

// if the user has selected a barrier type, the default barrier is replaced by
// one of the other options
#ifdef SAC_SET_BARRIER_TYPE

#ifndef __APPLE__
#define ACCEPT_PTHREAD 1
#else
#define ACCEPT_PTHREAD 0
#endif

// the futex barrier is only made available on linux systems
#ifdef __linux__
#define ACCEPT_FUTEX 1
#else
#define ACCEPT_FUTEX 0
#endif

// unset default barrier type
#undef SAC_MT_PTH_INIT_BARRIER
#undef SAC_MT_PTH_SIGNAL_BARRIER
#undef SAC_MT_PTH_DESTROY_BARRIER

// set barrier type based on the users choice
#if SAC_SET_BARRIER_TYPE == 1
#define SAC_MT_PTH_INIT_BARRIER(nrthreads) init_mutex_barrier (nrthreads)
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) take_mutex_barrier ()
#define SAC_MT_PTH_DESTROY_BARRIER() destroy_mutex_barrier ()
#elif SAC_SET_BARRIER_TYPE == 2
#define SAC_MT_PTH_INIT_BARRIER(nrthreads) init_cond_barrier ()
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) lift_cond_barrier (glflag)
#define SAC_MT_PTH_DESTROY_BARRIER() destroy_cond_barrier ()
#elif SAC_SET_BARRIER_TYPE == 3 && ACCEPT_PTHREAD == 1
#define SAC_MT_PTH_INIT_BARRIER(nrthreads) init_pthread_barrier (nrthreads)
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) take_pthread_barrier ()
#define SAC_MT_PTH_DESTROY_BARRIER() destroy_pthread_barrier ()
#elif SAC_SET_BARRIER_TYPE == 4 && ACCEPT_FUTEX == 1
#define SAC_MT_PTH_INIT_BARRIER(nrthreads)
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) lift_futex_barrier (glflag)
#define SAC_MT_PTH_DESTROY_BARRIER()
#else
#define SAC_MT_PTH_INIT_BARRIER(nrthreads)
#define SAC_MT_PTH_SIGNAL_BARRIER(glflag) SAC_MT_PTH_signal_barrier (NULL, glflag)
#define SAC_MT_PTH_DESTROY_BARRIER()
#endif

#undef ACCEPT_FUTEX
#undef ACCEPT_PTHREAD

#endif

// this header file is included in many c files, however the variables below
// should only be declared once. The variables should also be globally available.
// The construct below guarantees that the variables are declared once (as part
// of the main sac file) and are seen as extern variables by all other c files.
#ifdef SAC_SET_BARRIER_TYPE
#define SAC_MT_PTH_EXTERN
#else
#define SAC_MT_PTH_EXTERN SAC_C_EXTERN
#endif

SAC_MT_PTH_EXTERN int mutex_nr_threads;
SAC_MT_PTH_EXTERN int mutex_thread_count;
SAC_MT_PTH_EXTERN pthread_mutex_t mutex_sacred;
SAC_MT_PTH_EXTERN pthread_mutex_t mutex_lock_sacred;
SAC_MT_PTH_EXTERN pthread_mutex_t mutex_barrier;

SAC_MT_PTH_EXTERN pthread_cond_t cond_barrier;
SAC_MT_PTH_EXTERN pthread_mutex_t cond_mutex;

#ifndef __APPLE__
SAC_MT_PTH_EXTERN pthread_barrier_t pthread_barrier;
#endif

#undef SAC_MT_PTH_EXTERN

/******************************************************************************
 *
 * function:
 *    void init_mutex_barrier(int nr_threads)
 *
 * description:
 *    Initialization function for the mutex barrier. This barrier is internally
 *    only using pthread mutex locks.
 *
 * arguments:
 *    - int nr_threads: the total number of threads that are being used
 *
 ******************************************************************************/
static inline void
init_mutex_barrier (int nr_threads)
{
    mutex_nr_threads = nr_threads;
    pthread_mutex_init (&mutex_sacred, NULL);
    pthread_mutex_init (&mutex_lock_sacred, NULL);
    pthread_mutex_init (&mutex_barrier, NULL);
}

/******************************************************************************
 *
 * function:
 *    void take_mutex_barrier()
 *
 * description:
 *    This function is used to both barrier lock and barrier signal of the
 *    mutex barrier. Any thread that calls this function will be locked, except
 *    for the last thread. When the last thread calls this function, all
 *    threads will be released from there lock.
 *
 ******************************************************************************/
static inline void
take_mutex_barrier (void)
{
    int lock = 0;

    pthread_mutex_lock (&mutex_lock_sacred);
    pthread_mutex_unlock (&mutex_lock_sacred);
    pthread_mutex_lock (&mutex_sacred);
    mutex_thread_count++;
    if (mutex_thread_count == mutex_nr_threads) {
        pthread_mutex_lock (&mutex_lock_sacred);
        pthread_mutex_unlock (&mutex_barrier);
    } else {
        lock = 1;
    }
    if (mutex_thread_count == 1) {
        pthread_mutex_trylock (&mutex_barrier);
    }
    pthread_mutex_unlock (&mutex_sacred);

    if (lock) {
        pthread_mutex_lock (&mutex_barrier);
        mutex_thread_count--;
        if (mutex_thread_count == 1) {
            mutex_thread_count = 0;
            pthread_mutex_unlock (&mutex_lock_sacred);
        } else {
            pthread_mutex_unlock (&mutex_barrier);
        }
    }
}

/******************************************************************************
 *
 * function:
 *    void destroy_mutex_barrier()
 *
 * description:
 *    Distruction function for the mutex barrier.
 *
 ******************************************************************************/
static inline void
destroy_mutex_barrier (void)
{
    pthread_mutex_destroy (&mutex_sacred);
    pthread_mutex_destroy (&mutex_lock_sacred);
    pthread_mutex_destroy (&mutex_barrier);
}

/******************************************************************************
 *
 * function:
 *    void init_cond_barrier()
 *
 * description:
 *    Initialization function for the cond barrier. This barrier is internally
 *    only using pthread cond locks.
 *
 ******************************************************************************/
static inline void
init_cond_barrier (void)
{
    pthread_cond_init (&cond_barrier, NULL);
    pthread_mutex_init (&cond_mutex, NULL);
}

/******************************************************************************
 *
 * function:
 *    void wait_on_cond_barrier(volatile unsigned int *glflag, unsigned int *lcflag)
 *
 * description:
 *    This function is used to lock the cond barrier.
 *
 * arguments:
 *    - volatile unsigned int *glflag: pointer to the global flag. This should
 *        be an integer that is shared by all threads.
 *    - unsigned int *lcflag: pointer to local flag. This should be an integer
 *        that is owned by the current thread.
 *
 ******************************************************************************/
static inline void
wait_on_cond_barrier (volatile unsigned int *glflag, unsigned int *lcflag)
{
    pthread_mutex_lock (&cond_mutex);
    if (*glflag == *lcflag) {
        pthread_cond_wait (&cond_barrier, &cond_mutex);
    }
    pthread_mutex_unlock (&cond_mutex);
    (*lcflag)++;
}

/******************************************************************************
 *
 * function:
 *    void lift_cond_barrier(volatile unsigned int *glflag)
 *
 * description:
 *    This function is used to signal the cond barrier.
 *
 * arguments:
 *    - volatile unsigned int *glflag: pointer to the global flag. This should
 *        be an integer that is shared by all threads.
 *
 ******************************************************************************/
static inline void
lift_cond_barrier (volatile unsigned int *glflag)
{
    pthread_mutex_lock (&cond_mutex);
    (*glflag)++;
    pthread_cond_broadcast (&cond_barrier);
    pthread_mutex_unlock (&cond_mutex);
}

/******************************************************************************
 *
 * function:
 *    void lift_cond_barrier(volatile unsigned int *glflag)
 *
 * description:
 *    This function is used to destroy the cond barrier.
 *
 ******************************************************************************/
static inline void
destroy_cond_barrier (void)
{
    pthread_cond_destroy (&cond_barrier);
    pthread_mutex_destroy (&cond_mutex);
}

#ifndef __APPLE__
/******************************************************************************
 *
 * function:
 *    void init_pthread_barrier(int nr_threads)
 *
 * description:
 *    Initialization function for the pthread barrier. This function is just a
 *    wrapper for the pthread_barrier_init function.
 *
 * arguments:
 *    - int nr_threads: the total number of threads that are being used
 *
 ******************************************************************************/
static inline void
init_pthread_barrier (int nr_threads)
{
    pthread_barrier_init (&pthread_barrier, NULL, nr_threads);
}

/******************************************************************************
 *
 * function:
 *    void take_pthread_barrier()
 *
 * description:
 *    This function is used to both barrier lock and barrier signal of the
 *    pthread barrier. Any thread that calls this function will be locked, except
 *    for the last thread. When the last thread calls this function, all
 *    threads will be released from there lock. This function is just a wrapper
 *    for the pthread_barrier_wait function.
 *
 ******************************************************************************/
static inline void
take_pthread_barrier (void)
{
    pthread_barrier_wait (&pthread_barrier);
}

/******************************************************************************
 *
 * function:
 *    void destroy_pthread_barrier()
 *
 * description:
 *    This function is used to destroy the pthread barrier. The function is
 *    just a wrapper for the pthread_barrier_destroy function.
 *
 ******************************************************************************/
static inline void
destroy_pthread_barrier (void)
{
    pthread_barrier_destroy (&pthread_barrier);
}
#endif

#ifdef __linux__
/******************************************************************************
 *
 * function:
 *    void wait_on_futex_barrier(volatile unsigned int *glflag, unsigned int *lcflag)
 *
 * description:
 *    This function is used to lock the futex barrier.
 *
 * arguments:
 *    - volatile unsigned int *glflag: pointer to the global flag. This should
 *        be an integer that is shared by all threads.
 *    - unsigned int *lcflag: pointer to local flag. This should be an integer
 *        that is owned by the current thread.
 *
 ******************************************************************************/
static inline void
wait_on_futex_barrier (volatile unsigned int *glflag, unsigned int *lcflag)
{
    syscall (SYS_futex, glflag, FUTEX_WAIT, *lcflag, NULL, NULL, 0);
    (*lcflag)++;
}

/******************************************************************************
 *
 * function:
 *    void lift_futex_barrier(volatile unsigned int *glflag)
 *
 * description:
 *    This function is used to signal the futex barrier.
 *
 * arguments:
 *    - volatile unsigned int *glflag: pointer to the global flag. This should
 *        be an integer that is shared by all threads.
 *
 ******************************************************************************/
static inline void
lift_futex_barrier (volatile unsigned int *glflag)
{
    (*glflag)++;
    syscall (SYS_futex, glflag, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
}
#endif

#endif