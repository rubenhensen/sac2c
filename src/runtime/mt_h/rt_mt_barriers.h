/*****************************************************************************
 *
 * file:   rt_mt_barriers.h
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

#ifndef _SAC_RT_MT_BARRIERS_H_
#define _SAC_RT_MT_BARRIERS_H_

#include "libsac/mt/mt_barriers.h" // init_mutex_barrier

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

#endif // SAC_SET_BARRIER_TYPE

#endif // _SAC_RT_MT_BARRIERS_H_

