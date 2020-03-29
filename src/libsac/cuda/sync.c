/**
 * @file
 *
 * @brief implements synchronisation methods for CUDA_ASYNC_MODE
 *        option within the sac-compiler
 *
 */

#include <stdlib.h>
#include <pthread.h>

#include "sync.h"
#include "libsac/essentials/message.h"    // SAC_RuntimeError,...

#if ENABLE_CUDA
#include <cuda_runtime.h>
#else /* ENABLE_CUDA */
#define CUDART_CB
#endif /* ENABLE_CUDA */

/* locking using spinlock */

void cuda_async_spinlock_init (cuda_lock_t *locks, int num_streams)
{
    int ret;

    if ((ret = pthread_spin_init (locks, PTHREAD_PROCESS_SHARED)))
    {
        SAC_RuntimeError ("Unable to initialise the lock for CUDA synchronisation!!!");
    }

    // we then lock the spinlock
    pthread_spin_lock (locks);
}

void cuda_async_spinlock_destroy (cuda_lock_t *locks, int num_streams)
{
    int ret;

    if ((ret = pthread_spin_destroy (locks)))
    {
        SAC_RuntimeWarning ("Unable to destroy the lock for CUDA synchronisation!!!");
    }
}

void cuda_async_spinlock_wait (cuda_lock_t *locks, int num)
{
    pthread_spin_lock (locks);
}

void cuda_async_spinlock_unlock (cuda_lock_t *locks, int num)
{
    pthread_spin_unlock (locks);
}

void CUDART_CB cuda_rt_unlock (void *data)
{
    cuda_sync_t *cuda_struct = (cuda_sync_t*)data;
    cuda_async_spinlock_unlock (&cuda_struct->lock, 0);
}
