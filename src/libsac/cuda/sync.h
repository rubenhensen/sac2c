#ifndef _LIBSAC_CUDA_SYNC_H_
#define _LIBSAC_CUDA_SYNC_H_

#include <pthread.h>
#if ENABLE_CUDA
#include <cuda_runtime.h>
#else /* ENABLE_CUDA */
#define CUDART_CB
#endif /* ENABLE_CUDA */

typedef volatile pthread_spinlock_t cuda_lock_t;
typedef struct { cuda_lock_t lock; } cuda_sync_t;

void cuda_async_spinlock_init (cuda_lock_t *locks, int num_streams);
void cuda_async_spinlock_destroy (cuda_lock_t *locks, int num_streams);
void cuda_async_spinlock_wait (cuda_lock_t *locks, int num);
void cuda_async_spinlock_unlock (cuda_lock_t *locks, int num);
void CUDART_CB cuda_rt_unlock (void *data);

#endif /* _LIBSAC_CUDA_SYNC_H_ */
