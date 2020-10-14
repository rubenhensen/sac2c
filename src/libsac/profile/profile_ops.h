/**
 * Header file for profile_memory.c
 */

#ifndef _SAC_PROFILE_OPS_H
#define _SAC_PROFILE_OPS_H

#include "runtime/essentials_h/bool.h"

enum pf_types {
  T_int,
  T_float,
  T_double,
  T_nothing
} ;

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/* struct to capture memory allocs/frees in scope */

typedef struct ops_record {
    unsigned long ops_count[T_nothing];
} SAC_PF_OPS_RECORD;

SAC_C_EXTERN void SAC_PF_OPS_PrintStats (void);
SAC_C_EXTERN void SAC_PF_OPS_PrintRecordStats (SAC_PF_OPS_RECORD record);
SAC_C_EXTERN void SAC_PF_OPS_PrintFunStats (const char *func_name, unsigned num_ap,
                                            const SAC_PF_OPS_RECORD *records);
SAC_C_EXTERN bool SAC_PF_OPS_IsRecordZero (SAC_PF_OPS_RECORD record);
SAC_C_EXTERN void SAC_PF_OPS_IncPrf (enum pf_types s);

#endif /* _SAC_PROFILE_OPS_H */
