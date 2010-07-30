#ifndef DISABLE_MT

#ifdef MT

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1

#include "sac.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

unsigned int SAC_MT_threads;
volatile unsigned int SAC_MT_not_yet_parallel = 1;
pthread_key_t SAC_MT_threadid_key;

void
SAC_COMMON_MT_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                            unsigned int max_threads)
{
    int i;

    if (num_threads == 0) {
        for (i = 1; i < argc - 1; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'm') && (argv[i][2] == 't')
                && (argv[i][3] == '\0')) {
                SAC_MT_threads = atoi (argv[i + 1]);
                break;
            }
        }
        if ((SAC_MT_threads <= 0) || (SAC_MT_threads > max_threads)) {
            SAC_RuntimeError ("Number of threads is unspecified or exceeds legal"
                              " range (1 to %d).\n"
                              "    Use option '-mt <num>'.",
                              max_threads);
        }
    } else {
        SAC_MT_threads = num_threads;
    }

    SAC_TR_PRINT (("Number of threads determined as %u.", SAC_MT_threads));
}

#else /* MT */

volatile unsigned int SAC_MT_not_yet_parallel;

#endif /* MT */

#endif /* DISABLE_MT */