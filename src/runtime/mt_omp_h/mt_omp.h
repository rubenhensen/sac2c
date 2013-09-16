/*****************************************************************************
 *
 * file:   mt_omp.h
 *
 * prefix: SAC_MT_OMP_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the OpenMP multi-threading
 *   facilities.
 *
 *****************************************************************************/

#ifndef _SAC_MT_OMP_H_
#define _SAC_MT_OMP_H_

#ifndef SAC_SIMD_COMPILATION

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

#if SAC_DO_MT_OMP

SAC_C_EXTERN void SAC_OMP_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                        unsigned int max_threads);

#define SAC_MT_SETUP_INITIAL()                                                           \
    SAC_OMP_SetupInitial (__argc, __argv, SAC_SET_THREADS, SAC_SET_THREADS_MAX);

#define SAC_OMP_SET_NUM_THREADS() omp_set_num_threads (SAC_MT_GLOBAL_THREADS ())

#define SAC_HM_RTSPEC_THREADS() 0

#define SAC_OMP_SET_MAX_ACTIVE_LEVEL()                                                   \
    omp_set_max_active_levels (SAC_OMP_MAX_ACTIVE_LEVEL ())

#define SAC_OMP_MAX_ACTIVE_LEVEL() SAC_OMP_ACTIVE_LEVEL

#define SAC_MT_SETUP()

#define SAC_MT_DEFINE()

#define SAC_MT_FINALIZE()

#endif /* SAC_DO_MT_OMP */

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* SAC_SIMD_COMPILATION */

#endif /* _SAC_MT_OMP_H_ */
