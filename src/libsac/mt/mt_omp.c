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

#include "fun-attrs.h"

/******************************************************************************
 *
 * If all defines are falls this source file is "empty".
 * Empty source files are not allowed by the C99 standard
 * For this reason a dummy variable is declared.
 *
 ******************************************************************************/
static UNUSED int dummy_mt_omp;

/*
 * In case we do not have mt available, we have to make sure this file
 * does not cause any problems (e.g. when running implicit dependency
 * inference system). Therefore, we render this file empty iff MT compilation
 * is disabled!
 */

#ifdef SAC_BACKEND_omp /* the code is only loaded into libsac.mt.omp */

#include "mt_omp.h"

#include <pthread.h>
/* #include <omp.h> */
#include <stdio.h>
#include <stdlib.h>

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

/* called from PHM if it does not maintain its own thread ids */
unsigned int
SAC_MT_Internal_CurrentThreadId (void)
{
    /* return omp_get_thread_num(); */
    return 0;
}

#endif /* OMP */
