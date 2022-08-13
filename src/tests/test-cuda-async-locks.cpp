#include <cstdlib>

#include "gtest/gtest.h"
#include "config.h"

#if ENABLE_CUDA

#include <cuda_runtime.h>

extern "C" {
#include "libsac/cuda/sync.h"
}

TEST (CUDARuntime, CUDAAsyncLock)
{
    cuda_sync_t lock = {0, false};

    cuda_async_spinlock_init (&lock);

    ASSERT_TRUE (lock.locked);

    cuda_async_spinlock_destroy (&lock);

    ASSERT_FALSE (lock.locked);
}

#else /* ENABLE_CUDA */

static const int empty_unit = 0xdeadbeef;

#endif /* ENABLE_CUDA */
