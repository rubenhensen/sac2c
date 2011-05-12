/*
 * $Id: omp.c 16606 2009-11-19 18:33:30Z cg $
 */

#include "config.h"

/*****************************************************************************
 *
 * file:   mt.c
 *
 * prefix: SAC
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains routines and global identifiers required by both the
 *   OpenMP and the PThread based multi-threaded runtime systems.
 *
 *
 *****************************************************************************/

#if ENABLE_MT

#ifdef MT

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

#define SAC_PARALLEL_ENV_VAR_NAME "SAC_PARALLEL"

unsigned int SAC_MT_threads;
volatile unsigned int SAC_MT_not_yet_parallel = 1;
pthread_key_t SAC_MT_threadid_key;

void
SAC_COMMON_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                            unsigned int max_threads)
{
    int i;

    if (num_threads == 0) {
        for (i = 1; i < argc; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'm') && (argv[i][2] == 't')
                && (argv[i][3] == '\0')) {
                SAC_MT_threads = atoi (argv[i + 1]);
                break;
            }
        }
        if (i == argc) { /* '-mt' option not found */
            SAC_MT_threads = atoi (getenv (SAC_PARALLEL_ENV_VAR_NAME));
        }
        if ((SAC_MT_threads <= 0) || (SAC_MT_threads > max_threads)) {
            SAC_RuntimeError ("Number of threads is unspecified or exceeds legal"
                              " range (1 to %d).\n"
                              "    Use the '%s' environment variable or the option"
                              " -mt <num>' (which override the environment variable).",
                              max_threads, SAC_PARALLEL_ENV_VAR_NAME);
        }
    } else {
        SAC_MT_threads = num_threads;
    }

    SAC_TR_PRINT (("Number of threads determined as %u.", SAC_MT_threads));
}

#else /* MT */

volatile unsigned int SAC_MT_not_yet_parallel;

#endif /* MT */

#endif /* ENABLE_MT */
