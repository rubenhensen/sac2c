/**
 * Header file for memory.c
 */

#ifndef _SAC_PF_MEM_H
#define _SAC_PF_MEM_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

SAC_C_EXTERN void SAC_PF_MEM_PrintStats (void);
SAC_C_EXTERN void SAC_PF_MEM_AllocMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeMemcnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_AllocDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_FreeDescnt (int size, int typesize);
SAC_C_EXTERN void SAC_PF_MEM_ReuseMemcnt (void);

#endif /* _SAC_PF_MEM_H */
