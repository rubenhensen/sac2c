/**
 * Header file for profile_memory.c
 */

#ifndef _SAC_PROFILE_MEM_H
#define _SAC_PROFILE_MEM_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

SAC_C_EXTERN void SAC_PF_MEM_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_AllocMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_AllocDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_ReuseMemcnt (void);

/* struct to capture memory allocs/frees in scope */

typedef struct memory_record {
    unsigned long alloc_mem_count;
    unsigned long free_mem_count;
    unsigned long alloc_desc_count;
    unsigned long free_desc_count;
    unsigned long reuse_mem_count;
} SAC_PF_MEMORY_RECORD;

#endif /* _SAC_PROFILE_MEM_H */
