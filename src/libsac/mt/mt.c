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

#if ENABLE_HWLOC
#include "../runtime/mt_h/hwloc_data.h"
#define SAC_HWLOC_NUM_SOCKETS_VAR_NAME "SAC_NUM_SOCKETS"
#define SAC_HWLOC_NUM_CORES_VAR_NAME "SAC_NUM_CORES"
#define SAC_HWLOC_NUM_PUS_VAR_NAME "SAC_NUM_PUS"
#endif

unsigned int SAC_MT_threads;
volatile unsigned int SAC_MT_not_yet_parallel = 1;
pthread_key_t SAC_MT_threadid_key;

void
SAC_COMMON_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                            unsigned int max_threads)
{
    int i;
    bool mt_option_exists = FALSE;
    char *sac_parallel = NULL;

    if (num_threads == 0) {
        for (i = 1; i < argc - 1; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'm') && (argv[i][2] == 't')
                && (argv[i][3] == '\0')) {
                SAC_MT_threads = atoi (argv[i + 1]);
                mt_option_exists = TRUE;
                break;
            }
        }
        if (!mt_option_exists) {
            sac_parallel = getenv (SAC_PARALLEL_ENV_VAR_NAME);
            SAC_MT_threads = (sac_parallel != NULL) ? atoi (sac_parallel) : 0;
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
#if ENABLE_HWLOC
    char *char_dump;
    unsigned int sockets;
    unsigned int cores;
    unsigned int pus;

    char_dump = getenv (SAC_HWLOC_NUM_SOCKETS_VAR_NAME);
    if (char_dump == NULL)
        goto SAC_HWLOC_DONT_BIND_LBL;
    sockets = atoi (char_dump);

    char_dump = getenv (SAC_HWLOC_NUM_CORES_VAR_NAME);
    if (char_dump == NULL)
        goto SAC_HWLOC_DONT_BIND_LBL;
    cores = atoi (char_dump);

    char_dump = getenv (SAC_HWLOC_NUM_PUS_VAR_NAME);
    if (char_dump == NULL)
        goto SAC_HWLOC_DONT_BIND_LBL;
    pus = atoi (char_dump);

    if (sockets != 0 && cores != 0 && pus != 0) {
        if (SAC_MT_threads > sockets * cores * pus)
            SAC_RuntimeError (
              "sockets*cores*PUs is less than the number of threads desired");
        SAC_TR_PRINT (("Pinning on %u sockets, %u cores and %u processing units. Total "
                       "processing units used: %u",
                       sockets, cores, pus, sockets * cores * pus));
        SAC_HWLOC_init (sockets, cores, pus);
    } else {
    SAC_HWLOC_DONT_BIND_LBL:
        SAC_HWLOC_dont_bind ();
    }
#endif
}

#else /* MT */

volatile unsigned int SAC_MT_not_yet_parallel;

#endif /* MT */

#endif /* ENABLE_MT */
