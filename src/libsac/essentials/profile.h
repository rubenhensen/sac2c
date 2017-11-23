/*****************************************************************************
 *
 * file:   profile.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides the interface to profile.c
 *
 *****************************************************************************/

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
    /* Distributed memory backend profiling options */
    ,
    PF_distmem_exec_rep = 8,
    PF_distmem_exec_dist = 9,
    PF_distmem_exec_side_effects = 10,
    PF_distmem_rep_barrier = 11,
    PF_distmem_dist_barrier = 12,
    PF_distmem_side_effects_barrier = 13,
    PF_distmem_comm = 14

} SAC_PF_timer_type;

typedef struct timer_record {
    int funno;
    int funapno;
    SAC_PF_timer_type timer_type;
    struct timer_record *parent;
    int *cycle_tag;
} SAC_PF_TIMER_RECORD;

/*
 * External declarations of C library functions needed
 */

SAC_C_EXTERN int getrusage (int who, struct rusage *rusage);

/*
 * External declarations of library functions defined in libsac
 */

SAC_C_EXTERN void SAC_PF_PrintHeader (char *title);
SAC_C_EXTERN void SAC_PF_PrintHeaderNode (char *title, size_t rank);
SAC_C_EXTERN void SAC_PF_PrintSubHeader (char *title, int lineno);
SAC_C_EXTERN void SAC_PF_PrintTime (char *title, char *space, SAC_PF_TIMER *time);
SAC_C_EXTERN void SAC_PF_PrintCount (char *title, char *space, unsigned long count);
SAC_C_EXTERN void SAC_PF_PrintTimePercentage (char *title, char *space,
                                              SAC_PF_TIMER *time1, SAC_PF_TIMER *time2);

/*
 * External declarations of global variables defined in libsac
 */

SAC_C_EXTERN int SAC_PF_act_funno;
SAC_C_EXTERN int SAC_PF_act_funapno;
SAC_C_EXTERN int SAC_PF_with_level;
SAC_C_EXTERN struct rusage SAC_PF_start_timer;
SAC_C_EXTERN struct rusage SAC_PF_stop_timer;

#endif /* _SAC_PROFILE_H */


