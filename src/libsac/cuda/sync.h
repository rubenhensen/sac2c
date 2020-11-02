#ifndef _LIBSAC_CUDA_SYNC_H_
#define _LIBSAC_CUDA_SYNC_H_

#include "config.h"

#if ENABLE_CUDA

#include <pthread.h>
#include <stdbool.h>
#include <cuda_runtime.h>

typedef volatile pthread_spinlock_t cuda_lock_t;
typedef struct { cuda_lock_t lock; bool locked; } cuda_sync_t;

extern cudaEvent_t cuda_sync_event;
extern cuda_sync_t cuda_sync_struct;

extern void cuda_async_spinlock_init (cuda_sync_t *locks);
extern void cuda_async_spinlock_destroy (cuda_sync_t *locks);
extern void cuda_async_spinlock_wait (cuda_sync_t *locks);
extern void cuda_async_spinlock_unlock (cuda_sync_t *locks);
extern void CUDART_CB cuda_rt_unlock (void *data);

#endif /* ENABLE_CUDA */
#endif /* _LIBSAC_CUDA_SYNC_H_ */
