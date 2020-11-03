/**
 * @file
 *
 * @brief Implements a synchronisation method for CUDA backend
 *        that uses the CUDA Callback API.
 *
 * CUDA offers several ways of syncing operations. This is particularly,
 * critical in the case of asynchronous operation, as one needs to
 * prevent race conditions.
 *
 * There are three ways of doing this, which work at different levels
 * within the CUDA programming model. They are:
 * - device level synchronisation
 * - stream level syncrhonisation
 * - callback event within a stream
 *
 * The first two are the most common, and make use of GPU/CUDA-driver
 * based facility for synchronising with the host (or between streams).
 *
 * The last options allows us to create a callback within a given stream,
 * and rather then using GPU/CUDA-driver to do the syncing, we can do this
 * ourselves on the host.
 *
 * This module implements a possible way of doing this using pthread's
 * spinlock fascility.
 *
 * These function calls are resolved within runtime/cuda_h/cuda.h and only
 * materialise if the sac2c flag -cuda_async_mode=callback.
 *
 * The basic idea of this is, at a point when we which to sync, we introduce
 * the callback function into the stream. At somepoint within the code, at a
 * later stage we mark that lock as locked. At runtime that means, that if we
 * reach the lock, before the stream has reached the call back, the host waits.
 * Once the callback is called, the lock unlocks and the host can continue.
 * 
 * With CUAD/CUADE traversals, we add a large between sync-point and lock-point,
 * which means hopefully on the host we never have to wait.
 */
#include "config.h"

#if ENABLE_CUDA

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <cuda_runtime.h>

#include "sync.h"
#include "libsac/essentials/message.h"    // SAC_RuntimeError,...

/**
 * this variable is *not* part of the callback sync, but is used to
 * perform a stream based sync.
 */
cudaEvent_t cuda_sync_event;

/**
 * Struct that contains the reference to the spin lock.
 */
cuda_sync_t cuda_sync_struct = {0, false};

/**
 * @brief initialise the spinlock
 *
 * @param locks the struct holding locks
 */
void cuda_async_spinlock_init (cuda_sync_t *locks)
{
    if (pthread_spin_init (&locks->lock, PTHREAD_PROCESS_SHARED))
    {
        SAC_RuntimeError ("Unable to initialise the lock for CUDA synchronisation!!!");
    }

    // we then lock the spinlock
    cuda_async_spinlock_wait (locks);
}

/**
 * @brief destroy an existing lock(s)
 *
 * @param locks
 */
void cuda_async_spinlock_destroy (cuda_sync_t *locks)
{
    if (locks->locked)
    {
        cuda_async_spinlock_unlock (locks);
    }

    if (pthread_spin_destroy (&locks->lock))
    {
        SAC_RuntimeWarning ("Unable to destroy the lock for CUDA synchronisation!!!");
    }
}

/**
 * @brief cause lock to be activated, we wait
 *
 * @param locks
 */
void cuda_async_spinlock_wait (cuda_sync_t *locks)
{
    if (pthread_spin_lock (&locks->lock))
    {
        SAC_RuntimeError ("Unable to lock CUDA spin lock!!!");
    }
    locks->locked = true;
}

/**
 * @brief unlock lock
 *
 * @param locks
 */
void cuda_async_spinlock_unlock (cuda_sync_t *locks)
{
    if (pthread_spin_unlock (&locks->lock))
    {
        SAC_RuntimeError ("Unable to lock CUDA spin lock!!!");
    }
    locks->locked = false;
}

/**
 * @brief CUDA callback function
 *
 * Takes a reference of the lock, and when called unlocks it.
 * Must implement CUDART_CB return type.
 *
 * @param data lock
 */
void CUDART_CB cuda_rt_unlock (void *data)
{
    cuda_sync_t *cuda_struct = (cuda_sync_t*)data;
    cuda_async_spinlock_unlock (cuda_struct);
}

#else /* ENABLE_CUDA */

static const int emptymodule = 0xdeadbeef;

#endif /* ENABLE_CUDA */
