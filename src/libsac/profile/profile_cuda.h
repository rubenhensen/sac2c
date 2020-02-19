/**
 * @file
 * @brief Header file for profile_cuda.c
 */
#ifndef _SAC_PROFILE_CUDA_H
#define _SAC_PROFILE_CUDA_H

#include <stddef.h>

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

SAC_C_EXTERN void SAC_PF_MEM_CUDA_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AllocDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_FreeDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_CUDA_AddToMax (size_t size);

#endif /* _SAC_PROFILE_CUDA_H */
