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

/** Global Variables */

/* The global number of threads in the environment, or at least the maximum.
 * The special case SAC_MT_global_threads==0 indicates a purely sequentil
 * program (SEQ). That comes handy when an MT-compiled library is used in
 * the SEQ mode and the code needs to dynamically distinguish the two.
 */
unsigned int SAC_MT_global_threads;

/* number of additional hidden (auxiliary) threads that the
 * heap manager has to deal with */
unsigned int SAC_MT_hm_aux_threads = 0;

/* Only a single thread in the environment?
 * This is used to optimize the PHM; when executing ST it does not have
 * to take so many precautions (locks).
 * FIXME: Probably not correct for SAC-as-library MT. But PHM is not used
 *        in that case anyway.
 */
unsigned int SAC_MT_globally_single = 1;

/** /Global Variables/ */

/******************************************************************************
 *
 * function:
 *   void SAC_COMMON_MT_SetupInitial( int argc, char *argv[],
 *                                    unsigned int num_threads,
 *                                    unsigned int max_threads)
 *
 * description:
 *  Parse the command line and determine the number of threads.
 *  Looks for the -mt cmdline option, sets SAC_MT_global_threads.
 *
 ******************************************************************************/
void
SAC_COMMON_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                            unsigned int max_threads)
{
    int i;
    bool mt_option_exists = FALSE;
    char *sac_parallel = NULL;

    if (num_threads == 0) {
        if (argv) {
            for (i = 1; i < argc - 1; i++) {
                if ((argv[i][0] == '-') && (argv[i][1] == 'm') && (argv[i][2] == 't')
                    && (argv[i][3] == '\0')) {
                    SAC_MT_global_threads = atoi (argv[i + 1]);
                    mt_option_exists = TRUE;
                    break;
                }
            }
        }
        if (!mt_option_exists) {
            sac_parallel = getenv (SAC_PARALLEL_ENV_VAR_NAME);
            SAC_MT_global_threads = (sac_parallel != NULL) ? atoi (sac_parallel) : 0;
        }
        if ((SAC_MT_global_threads <= 0) || (SAC_MT_global_threads > max_threads)) {
            SAC_RuntimeError ("Number of threads is unspecified or exceeds legal"
                              " range (1 to %d).\n"
                              "    Use the '%s' environment variable or the option"
                              " -mt <num>' (which override the environment variable).",
                              max_threads, SAC_PARALLEL_ENV_VAR_NAME);
        }
    } else {
        SAC_MT_global_threads = num_threads;
    }

    SAC_TR_PRINT (("Number of threads determined as %u.", SAC_MT_global_threads));
}

#else /* MT */
/* !MT */

/* global (maximal) number of threads in the environment */
unsigned int SAC_MT_global_threads = 0;

unsigned int SAC_MT_globally_single = 1;

/* number of additional hidden (auxiliary) threads that the
 * heap manager has to deal with */
unsigned int SAC_MT_hm_aux_threads = 0;

#endif /* MT */

#endif /* ENABLE_MT */
