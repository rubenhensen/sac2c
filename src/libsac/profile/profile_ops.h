/**
 * Header file for profile_memory.c
 */

#ifndef _SAC_PROFILE_OPS_H
#define _SAC_PROFILE_OPS_H

#include "runtime/essentials_h/bool.h"

/**
 * Here the prfs that are currently supported.
 * The funny names stem from the ICM that is
 * generated for their applications in 
 * codegen/compile.c
 * It is essential to make sure that all those
 * prf's are covered that are compiled by 
 * COMPprfOp_SxS. To find out, please ccheck
 * tree/prf_info.mac
 *
 * P_undef is unused and marks the end of
 * them.
 */
enum pf_ops {
  P_SAC_ND_PRF_ADD,
  P_SAC_ND_PRF_SUB,
  P_SAC_ND_PRF_MUL,
  P_SAC_ND_PRF_DIV,
  P_SAC_ND_PRF_MOD,
  P_SAC_ND_PRF_APLMOD,
  P_SAC_ND_PRF_MIN,
  P_SAC_ND_PRF_MAX,
  P_SAC_ND_PRF_EQ,
  P_SAC_ND_PRF_NE,
  P_SAC_ND_PRF_LE,
  P_SAC_ND_PRF_LT,
  P_SAC_ND_PRF_GE,
  P_SAC_ND_PRF_GT,
  P_SAC_ND_PRF_AND,
  P_SAC_ND_PRF_OR,
  P_undef
} ;

/**
 * Here the "simpletypes" that are currently supported.
 * Since simpltype itself is not available at runtime, 
 * we generate this pseudo-encoding. When extending
 * them to further simpletypes, COMPprfOp_SxS
 * in codegen/compile.c needs to be adjusted accordingly.
 * In order to avoid runtime checks, we add a pseudo
 * type T_ignore which serves as pool for all simpletypes
 * we are currently not interested in/
 *
 * Similar as in simpletypes, T_nothing is unused and
 * marks the end of them.
 */
enum pf_types {
  T_int,
  T_float,
  T_double,
  T_ignore,
  T_nothing
} ;

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/* struct to capture memory allocs/frees in scope */

typedef struct ops_record {
    unsigned long type_ops_count[T_nothing][P_undef];
} SAC_PF_OPS_RECORD;

SAC_C_EXTERN void SAC_PF_OPS_PrintStats (void);
SAC_C_EXTERN void SAC_PF_OPS_PrintRecordStats (SAC_PF_OPS_RECORD record);
SAC_C_EXTERN void SAC_PF_OPS_PrintFunStats (const char *func_name, unsigned num_ap,
                                            const SAC_PF_OPS_RECORD *records);
SAC_C_EXTERN void SAC_PF_OPS_PrintOpsStats (unsigned long *p_ops);
SAC_C_EXTERN bool SAC_PF_OPS_IsRecordZero (SAC_PF_OPS_RECORD record);
SAC_C_EXTERN void SAC_PF_OPS_IncPrf (enum pf_types s, enum pf_ops o);

#endif /* _SAC_PROFILE_OPS_H */
