/*
 * $Id: omp.c 16606 2009-11-19 18:33:30Z cg $
 */

/*****************************************************************************
 *
 * file:   mt_omp.c
 *
 * prefix: SAC
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains routines and global identifiers required by the
 *   OpenMP multi-threaded runtime system.
 *
 *
 *****************************************************************************/

#include "config.h"

/******************************************************************************
 *
 * If all defines are falls this source file is "emptry".
 * Emptry source files are not allowed by the C99 standard
 * For this reason a dummy variable is declared.
 *
 ******************************************************************************/
int dummy_mt_omp;

/*
 * In case we do not have mt available, we have to make sure this file
 * does not cause any problems (e.g. when running implicit dependency
 * inference system). Therefore, we render this file empty iff MT compilation
 * is disabled!
 */

#if ENABLE_MT && ENABLE_OMP

#ifdef OMP /* the code is only loaded into libsac.mt.omp */

#include <pthread.h>
/* #include <omp.h> */
#include <stdio.h>
#include <stdlib.h>

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_MT_OMP 1
#define SAC_DO_THREADS_STATIC 1

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_MT_OMP
#undef SAC_DO_THREADS_STATIC

/******************************************************************************
 *
 * description:
 *
 *   This function implements an initial setup of the runtime system for
 *   OpenMP multi-threaded program execution.
 *
 *   These setups need to be performed *before* the heap is initialised
 *   because heap initialisation requires the number of threads. Moreover,
 *   OpenMP standard library omp_set_num_threads needs the number of threads
 *   to initialize the OpenMP multi-threaded environment
 *
 ******************************************************************************/

void
SAC_OMP_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                      unsigned int max_threads)
{
    SAC_COMMON_MT_SetupInitial (argc, argv, num_threads, max_threads);
}

unsigned int
SAC_Get_Global_ThreadID (void)
{
    /* return omp_get_thread_num(); */
    return 0;
}

#endif /* OMP */

#else /* ENABLE_MT && ENABLE_OMP */

#endif /*  ENABLE_MT && ENABLE_OMP  */
