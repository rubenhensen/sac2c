/**
 * @brief General header file for the profiling system of SaC
 *
 */
#ifndef _SAC_PROFILE_H
#define _SAC_PROFILE_H

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*
 * Type definitions
 */

#include <sys/time.h>
/*
 * for struct timeval
 */

#include <sys/resource.h>
/*
 * for struct rusage
 */

#include <stdlib.h>
/*
 * for size_t
 */

typedef struct timeval SAC_PF_TIMER;

typedef enum {
    PF_ow_fun = 0,
    PF_ow_genarray = 1,
    PF_ow_modarray = 2,
    PF_ow_fold = 3,
    PF_iw_fun = 4,
    PF_iw_genarray = 5,
    PF_iw_modarray = 6,
    PF_iw_fold = 7
    /* Cuda profiling options */
    ,
    PF_cuda_knl = 8
    /* Distributed memory backend profiling options */
    ,
    PF_distmem_exec_rep = 9,
    PF_distmem_exec_dist = 10,
    PF_distmem_exec_side_effects = 11,
    PF_distmem_rep_barrier = 12,
    PF_distmem_dist_barrier = 13,
    PF_distmem_side_effects_barrier = 14,
    PF_distmem_comm = 15

} SAC_PF_record_type;

typedef struct profile_record {
    int funno;
    int funapno;
    SAC_PF_record_type record_type;
    struct profile_record *parent;
    int *cycle_tag;
} SAC_PROFILE_RECORD;

#endif /* _SAC_PROFILE_H */
