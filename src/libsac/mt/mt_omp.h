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

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif

/*****************************************************************************/

#if SAC_DO_MULTITHREAD

#if SAC_DO_MT_OMP

SAC_C_EXTERN void SAC_OMP_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                        unsigned int max_threads);

#endif /* SAC_DO_MT_OMP */

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD */

#endif /* _SAC_MT_OMP_H_ */
