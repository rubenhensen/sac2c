/**
 * This header file allows us to more _cleanly_ deal with `-Wpedantic` related
 * warnings coming from CUDA. Here we use GCC pragmas to control what diagnostics
 * are active.
 */
#ifndef _OVERRIDE_CUDA_RUNTIME_H_
#define _OVERRIDE_CUDA_RUNTIME_H_

/* This is to disable the warning being issued by CUDA 11.2:
 * /opt/cuda/include/driver_types.h:1826:36: warning: ISO C restricts enumerator values to range of ‘int’ [-Wpedantic]
 *    1826 |     cudaMemAllocationTypeMax     = 0xFFFFFFFF
 *         |                                    ^~~~~~~~~~
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <cuda_runtime.h>
#pragma GCC diagnostic pop

#endif // _OVERRIDE_CUDA_RUNTIME_H_
