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

typedef enum {
    pf_cuda_knl,
    pf_cuda_htod,
    pf_cuda_dtoh,
    pf_cuda_hmal,
    pf_cuda_dmal,
    pf_cuda_hfree,
    pf_cuda_dfree
} cuda_count_t;

typedef struct cuda_timer {
    cudaEvent_t start;
    cudaEvent_t stop;
    struct cuda_timer *parent;
    struct cuda_timer *next;
} cuda_timer_t;

SAC_C_EXTERN cuda_timer_t *cuda_timer_knl;
SAC_C_EXTERN cuda_timer_t *cuda_timer_htod;
SAC_C_EXTERN cuda_timer_t *cuda_timer_dtoh;
SAC_C_EXTERN cuda_timer_t *cuda_timer_hmal;
SAC_C_EXTERN cuda_timer_t *cuda_timer_dmal;
SAC_C_EXTERN cuda_timer_t *cuda_timer_hfree;
SAC_C_EXTERN cuda_timer_t *cuda_timer_dfree;

SAC_C_EXTERN cuda_timer_t *SAC_PF_TIMER_CUDA_Add (cuda_count_t type, cuda_timer_t *timer);
SAC_C_EXTERN float SAC_PF_TIMER_CUDA_Sum (cuda_timer_t *timer);
SAC_C_EXTERN void SAC_PF_TIMER_CUDA_Destroy (cuda_timer_t *timer);
SAC_C_EXTERN void SAC_PF_TIMER_CUDA_Inc_Counter (cuda_count_t type);
SAC_C_EXTERN size_t SAC_PF_TIMER_CUDA_Get_Counter (cuda_count_t type);
SAC_C_EXTERN void SAC_PF_TIMER_CUDA_PrintTimePercentage (const char *title, const char *space,
                                              cuda_count_t type, const float time1, const SAC_PF_TIMER *time2);

#endif /* ENABLE_CUDA */

SAC_C_EXTERN void SAC_PF_MEM_CUDA_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AddToMax (size_t size);

#endif /* _SAC_PROFILE_CUDA_H */
