/*****************************************************************************
 *
 * file:   mt.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *
 *   It is the major header file of the implementation of the common multi-threading
 *   facilities, both for OpenMP solution and Pthread solution.
 *
 *****************************************************************************/

#ifndef _SAC_MT_H_
#define _SAC_MT_H_

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*****************************************************************************/

#if SAC_DO_MULTITHREAD || (SAC_MT_MODE > 0)

/* number of total threads in the environment */
SAC_C_EXTERN unsigned int SAC_MT_global_threads;

/* barrier implementation to use for mt-sync */
SAC_C_EXTERN unsigned int SAC_MT_barrier_type;

/* how to use hwloc for cpu binding */
SAC_C_EXTERN unsigned int SAC_MT_cpu_bind_strategy;

/* Whether DO_TRACE_MT is enabled or not */
SAC_C_EXTERN unsigned int SAC_MT_do_trace;

/* Only a single thread in the environment?
 * Used for PHM optimizations.
 */
SAC_C_EXTERN unsigned int SAC_MT_globally_single;

SAC_C_EXTERN void SAC_COMMON_MT_SetupInitial (int argc, char *argv[],
                                              unsigned int num_threads,
                                              unsigned int max_threads);

/*  Called from PHM if it does not maintain its own thread ids. */
SAC_C_EXTERN unsigned int SAC_MT_Internal_CurrentThreadId (void);

#else

SAC_C_EXTERN unsigned int SAC_MT_global_threads;
SAC_C_EXTERN unsigned int SAC_MT_globally_single;
SAC_C_EXTERN unsigned int SAC_MT_Internal_CurrentThreadId (void);

#endif

#endif /* _SAC_MT_H_ */
