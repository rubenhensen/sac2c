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

#if SAC_MT_MODE > 0

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1

#include "mt.h"
#include "runtime/hwloc_h/hwloc.h"
#include "hwloc_data.h"
#include "runtime/essentials_h/bool.h"
#include "runtime/essentials_h/std.h"
#include "libsac/essentials/message.h"
#include "libsac/essentials/trace.h"

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

/* Only a single thread in the environment?
 * This is used to optimize the PHM; when executing ST it does not have
 * to take so many precautions (locks).
 * This is True (1) only during parts of execution in stand-alone applications.
 * In library applications after initialization this is always False (0).
 */
unsigned int SAC_MT_globally_single = 1;

/* The barrier type encodes which barrier to use. Since this is configurable
 * by sac2c, only the executable knows about it. It is initialised in mt_xx.h
 * in the definition of the macro SAC_MT_SETUP_INITIAL.
 */
unsigned int SAC_MT_barrier_type;

/* The cpu_bind_strategy encodes whether/ how to use hwloc for cpu binding.
 * Again, configurability through sac2c dictates its existance. It is initialised in
 * mt_xx.h in the definition of the macro SAC_MT_SETUP_INITIAL.
 */
unsigned int SAC_MT_cpu_bind_strategy;

/* Whether DO_TRACE_MT is enabled or not.
 * Again, configurability through sac2c dictates its existance. It is initialised in
 * mt_xx.h in the definition of the macro SAC_MT_SETUP_INITIAL.
 */
unsigned int SAC_MT_do_trace;

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

    SAC_TR_LIBSAC_PRINT (("Number of threads determined as %u.", SAC_MT_global_threads));

#if ENABLE_HWLOC
    if (SAC_MT_cpu_bind_strategy != 0) { // HWLOC is not OFF!
        SAC_MT_HWLOC_init (SAC_MT_global_threads);
    } else {
        SAC_TR_LIBSAC_PRINT (("cpu binding turned off"));
    }
#endif
}

#else /* MT */
/* !MT */

#include "mt.h"
#include "runtime/rtspec_h/rtspec.h" // SAC_RTSPEC_CURRENT_THREAD_ID

/******************************************************************************
 * function:
 *   unsigned int SAC_MT_Internal_CurrentThreadId(void)
 *
 * description:
 *  Return the Thread ID of the current thread.
 *  Called from PHM if it does not maintain its own thread ids.
 ******************************************************************************/
unsigned int
SAC_MT_Internal_CurrentThreadId (void)
{
    /* this actually won't be used, but satisfy the linker */
    return SAC_RTSPEC_CURRENT_THREAD_ID ();
}

/* global (maximal) number of threads in the environment */
unsigned int SAC_MT_global_threads = 0;

unsigned int SAC_MT_globally_single = 1;

#endif /* MT */
