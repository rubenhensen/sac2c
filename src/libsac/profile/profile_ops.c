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

/* these counter are for all memory operations */
static size_t SAC_PF_OPS_total[T_nothing]; /**< Holds the total nmber of prfs on the given simpletype */

/**
 * @brief Increments the alloc counter.
 *
 * @param size Size in bytes
 */
void
SAC_PF_OPS_IncPrf ( enum pf_types  s)
{
    SAC_PF_OPS_total[s] += 1;
}

/**
 * @brief Test if a record's fields are all zero.
 *
 * @param record the record to check
 */
inline bool
SAC_PF_OPS_IsRecordZero (SAC_PF_OPS_RECORD record)
{
    bool res = true;
    unsigned int i;
    for( i=0; i<T_nothing; i++) {
      res = res && (record.ops_count[i] == 0);
    }
    return res;
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
    SAC_PF_PrintCount ("total", "", record.ops_count[T_double]);
    SAC_PF_PrintSection ("Single precision FLOPS:");
    SAC_PF_PrintCount ("total", "", record.ops_count[T_float]);
    SAC_PF_PrintSection ("int IOPS:");
    SAC_PF_PrintCount ("total", "", record.ops_count[T_int]);
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
 * @brief Call to print memory profiling statistics.
 */
void
SAC_PF_OPS_PrintStats ()
{
    SAC_PF_PrintHeader ("Memory Profile");

    fprintf (stderr, "\n*** %-72s\n", "Operation counters:");
    SAC_PF_PrintSection ("Double precision FLOPS:");
    SAC_PF_PrintCount ("total", "", SAC_PF_OPS_total[T_double]);
    SAC_PF_PrintSection ("Single precision FLOPS:");
    SAC_PF_PrintCount ("total", "", SAC_PF_OPS_total[T_float]);
    SAC_PF_PrintSection ("int IOPS:");
    SAC_PF_PrintCount ("total", "", SAC_PF_OPS_total[T_int]);
}
