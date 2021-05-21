/**
 * @file
 * @brief This file specifies a set of functions used by profile.c to profile memory
 *        operations, specifically count and size(s).
 */

#include <stdio.h>

#include "libsac/profile/profile_print.h"
#include "libsac/profile/profile_ops.h"
#include "runtime/essentials_h/bool.h"

/* Global Variables */

/**
 * We keep all information about arithmetic and relational 
 * function calls in a single two-dimensional array.
 * The first axis denotes the different types, and the second
 * the different operations. The values used here are the
 * final entries in the corresponding enum types provided
 * by libsac/profile/profile_ops.h.
 */
static unsigned long SAC_PF_OPS_type_ops_total[T_nothing][P_undef];

/**
 * @brief Increments the ops counters.
 *
 * @param s simpletype as encoded in profile_ops.h
 * @param o operation as encoded in profile_ops.h
 */
void
SAC_PF_OPS_IncPrf ( enum pf_types  s, enum pf_ops o)
{
    SAC_PF_OPS_type_ops_total[s][o] += 1;
}

/**
 * @brief Test if a record's fields are all zero.
 *        This allows us to prune the output.
 *
 * @param record the record to check
 */
inline bool
SAC_PF_OPS_IsRecordZero (SAC_PF_OPS_RECORD record)
{
    bool res = true;
    int i,j;
    for( i=0; i<T_ignore; i++) {
        for( j=0; j<P_undef; j++) {
            res = res && (record.type_ops_count[i][j] == 0);
        }
    }
    return res;
}

/**
 * @brief Helper function which presents the
 *        counter values for all different operations
 *        of a single given element type.
 *        Used globally as well as on a function level.
 *
 * @param p_ops pointer to the vector of counters per operation
 */
inline void
SAC_PF_OPS_PrintOpsStats (unsigned long *p_ops)
{
    int i;
    unsigned long ari_ops=0, rel_ops=0, minmax=0;
    for (i=P_SAC_ND_PRF_ADD; i<P_SAC_ND_PRF_MIN; i++) {
        ari_ops += p_ops[i];
    }
    for (; i<P_SAC_ND_PRF_EQ; i++) {
        minmax += p_ops[i];
    }
    for (; i<P_undef; i++) {
        rel_ops += p_ops[i];
    }
    SAC_PF_PrintCount ("arithmetic ops:", "", ari_ops);
    SAC_PF_PrintCount ("relational ops:", "", rel_ops);
    SAC_PF_PrintCount ("minmax ops    :", "", minmax);
    SAC_PF_PrintCount ("total ops     :", "", ari_ops+rel_ops+minmax);
    
}


/**
 * @brief Print memory profiling statistics for current record.
 *
 * @param record the record to print
 */
inline void
SAC_PF_OPS_PrintRecordStats (SAC_PF_OPS_RECORD record)
{
    
    SAC_PF_PrintSection ("Double precision FLOPS:");
    SAC_PF_OPS_PrintOpsStats (&record.type_ops_count[T_double][0]);
    SAC_PF_PrintSection ("Single precision FLOPS:");
    SAC_PF_OPS_PrintOpsStats (&record.type_ops_count[T_float][0]);
    SAC_PF_PrintSection ("int IOPS:");
    SAC_PF_OPS_PrintOpsStats (&record.type_ops_count[T_int][0]);
}

/**
 * @brief Print statistics for given function.
 *
 * @param func_name Name of function
 * @param num_ap Number of applications of function
 * @param records The records that store the statistics
 */
void
SAC_PF_OPS_PrintFunStats (const char *func_name, unsigned num_ap,
                          const SAC_PF_OPS_RECORD *records)
{
    unsigned i;
    bool zero;

    /* lets check that there is actually something useful to print */
    for (i = 0, zero = true; i < num_ap; i++) {
        zero &= SAC_PF_OPS_IsRecordZero (records[i]);
    }

    /* if at least one record is non-zero */
    if (!zero) {
        SAC_PF_PrintHeader (func_name);
        for (i = 0; i < num_ap; i++) {
            if (!SAC_PF_OPS_IsRecordZero (records[i])) {
                if (num_ap > 1) {
                    fprintf (stderr, "--- Application %d\n", i);
                }
                SAC_PF_OPS_PrintRecordStats (records[i]);
            }
        }
    }
}

/**
 * @brief Call to print arithmetic and relational profiling statistics.
 */
void
SAC_PF_OPS_PrintStats ()
{
    SAC_PF_PrintHeader ("Operations Profile");

    fprintf (stderr, "\n*** %-72s\n", "Operation counters:");
    SAC_PF_PrintSection ("Total double precision FLOPS:");
    SAC_PF_OPS_PrintOpsStats (&SAC_PF_OPS_type_ops_total[T_double][0]);
    SAC_PF_PrintSection ("Total single precision FLOPS:");
    SAC_PF_OPS_PrintOpsStats (&SAC_PF_OPS_type_ops_total[T_float][0]);
    SAC_PF_PrintSection ("Total int IOPS:");
    SAC_PF_OPS_PrintOpsStats (&SAC_PF_OPS_type_ops_total[T_int][0]);
}
