/**
 * @file
 * @brief Header file for profile_cuda.c
 */
#ifndef _SAC_PROFILE_CUDA_H
#define _SAC_PROFILE_CUDA_H

#include "libsac/profile/profile.h"

#include <stddef.h>

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#if ENABLE_CUDA || SAC_CUDA_MACROS
#include <cuda_runtime_api.h>

typedef struct cuda_timer {
    cudaEvent_t start;
    cudaEvent_t stop;
    struct cuda_timer *next;
} cuda_timer_t;


SAC_C_EXTERN cuda_timer_t *cuda_timer;

SAC_C_EXTERN cuda_timer_t *SAC_PF_TIMER_CUDA_Add (cuda_timer_t *timer);
SAC_C_EXTERN float SAC_PF_TIMER_CUDA_Sum (void);
SAC_C_EXTERN void SAC_PF_TIMER_CUDA_Destroy (void);

#endif /* ENABLE_CUDA */

SAC_C_EXTERN void SAC_PF_TIMER_CUDA_PrintTimePercentage (const char *title, const char *space,
                                              const float time1, const SAC_PF_TIMER *time2);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AddToMax (size_t size);

#endif /* _SAC_PROFILE_CUDA_H */
