/**
 * Header file for profile_memory.c
 */

#ifndef _SAC_PROFILE_MEM_H
#define _SAC_PROFILE_MEM_H

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
} SAC_PF_MEMORY_RECORD;

SAC_C_EXTERN void SAC_PF_MEM_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_PrintRecordStats (SAC_PF_MEMORY_RECORD record);
SAC_C_EXTERN void SAC_PF_MEM_PrintFunStats (const char * func_name, int num_ap, const SAC_PF_MEMORY_RECORD * records);
SAC_C_EXTERN int  SAC_PF_MEM_IsRecordZero (SAC_PF_MEMORY_RECORD record);
SAC_C_EXTERN void SAC_PF_MEM_AllocMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_AllocDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_ReuseMemcnt (void);

#endif /* _SAC_PROFILE_MEM_H */
