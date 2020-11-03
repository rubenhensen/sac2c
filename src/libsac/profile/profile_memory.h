/**
 * Header file for profile_memory.c
 */
#ifndef _SAC_PROFILE_MEM_H
#define _SAC_PROFILE_MEM_H

#include <stddef.h>
#include "runtime/essentials_h/bool.h"

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/* struct to capture memory allocs/frees in scope */

typedef struct memory_record {
    unsigned long alloc_mem_count;
    unsigned long free_mem_count;
    unsigned long alloc_desc_count;
    unsigned long free_desc_count;
    unsigned long reuse_mem_count;
#ifdef SAC_BACKEND_CUDA
    unsigned long alloc_mem_cuda_count;
    unsigned long free_mem_cuda_count;
    unsigned long alloc_desc_cuda_count;
    unsigned long free_desc_cuda_count;
#endif /* SAC_BACKEND_CUDA */
} SAC_PF_MEMORY_RECORD;

SAC_C_EXTERN void SAC_PF_MEM_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_PrintRecordStats (SAC_PF_MEMORY_RECORD record);
SAC_C_EXTERN void SAC_PF_MEM_PrintFunStats (const char *func_name, unsigned num_ap,
                                            const SAC_PF_MEMORY_RECORD *records);
SAC_C_EXTERN bool SAC_PF_MEM_IsRecordZero (SAC_PF_MEMORY_RECORD record);
SAC_C_EXTERN void SAC_PF_MEM_AllocMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_FreeMemcnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_AllocDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_FreeDescnt (size_t size);
SAC_C_EXTERN void SAC_PF_MEM_ReuseMemcnt (void);
SAC_C_EXTERN void SAC_PF_MEM_AddToMax (size_t size);

#endif /* _SAC_PROFILE_MEM_H */
