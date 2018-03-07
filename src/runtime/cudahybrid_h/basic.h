/*****************************************************************************
 *
 * file:   basic.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions for basic functions of the CUDAHYBRID backend
 *
 *****************************************************************************/

#ifndef _SAC_CUDAHYBRID_BASIC_H_
#define _SAC_CUDAHYBRID_BASIC_H_

#include "runtime/cudahybrid_h/dist_var_memsave.h"
#include "runtime/cudahybrid_h/dist_var_nomemsave.h"

#ifdef __CUDACC__

#define getDeviceNumber(SAC_MT_mythread)                                                 \
    SAC_MT_mythread - (SAC_MT_LOCAL_THREADS () - SAC_CUDA_DEVICES) + 1

#define SAC_CUDA_ENV_VAR_NAME "SAC_CUDA"
#define SAC_CUDA_SHARE_ENV_VAR_NAME "SAC_CUDA_SHARE"

#define SAC_DIST_SETUP() SAC_DIST_setup (__argc, __argv)

double SAC_CUDA_SHARE;

void
SAC_DIST_setup (int argc, char **argv)
{
    // multiple cuda setup
    bool option_exists = FALSE;
    char *env_res = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if ((argv[i][0] == '-') && (argv[i][1] == 'c') && (argv[i][2] == 'u')
            && (argv[i][3] == 'd') && (argv[i][4] == 'a') && (argv[i][5] == '\0')) {
            SAC_CUDA_DEVICES = atoi (argv[i + 1]);
            option_exists = TRUE;
            break;
        }
    }
    if (!option_exists) {
        env_res = getenv (SAC_CUDA_ENV_VAR_NAME);
        SAC_CUDA_DEVICES = (env_res != NULL) ? atoi (env_res) : 1;
    }

    for (int i = 1; i < argc - 1; i++) {
        if ((argv[i][0] == '-') && (argv[i][1] == 's') && (argv[i][2] == 'h')
            && (argv[i][3] == 'a') && (argv[i][4] == 'r') && (argv[i][5] == 'e')
            && (argv[i][6] == '\0')) {
            SAC_CUDA_SHARE = atof (argv[i + 1]);
            option_exists = TRUE;
            break;
        }
    }
    if (!option_exists) {
        env_res = getenv (SAC_CUDA_SHARE_ENV_VAR_NAME);
        SAC_CUDA_SHARE = (env_res != NULL) ? atof (env_res) : 1;
    }

    int deviceCount;
    cudaGetDeviceCount (&deviceCount);
    if (SAC_CUDA_DEVICES > (unsigned int)deviceCount) {
        SAC_RuntimeError ("Number of CUDA devices requested exceeds those"
                          " available (%d).\n"
                          "    Use the '%s' environment variable or the option"
                          " -cuda <num>' (which overrides the environment variable).",
                          deviceCount, SAC_CUDA_ENV_VAR_NAME);
    }
    if (SAC_CUDA_DEVICES > SAC_MT_GLOBAL_THREADS ()) {
        SAC_RuntimeError ("Number of cuda devices requested exceeds threads "
                          " available (%d).\n"
                          "    Use the '%s' environment variable or the option"
                          " -cuda <num>' (which overrides the environment variable)"
                          " to reduce number of cuda devices, or increase"
                          " number of threads.",
                          deviceCount, SAC_CUDA_ENV_VAR_NAME);
    }
    SAC_MT_DEVICES = SAC_CUDA_DEVICES + 1;

    cache_init ();
}

#define SAC_DIST_SCHEDULER_Block_DIM0(lower, upper, unrolling)                           \
    {                                                                                    \
                                                                                         \
        int devices = SAC_MT_LOCAL_THREADS ();                                           \
        int ub = upper;                                                                  \
        int lb = lower;                                                                  \
        int device_n = getDeviceNumber (SAC_MT_SELF_LOCAL_ID ());                        \
                                                                                         \
        if (device_n > 0) {                                                              \
            if (SAC_MT_LOCAL_THREADS () != SAC_CUDA_DEVICES) {                           \
                ub = SAC_CUDA_SHARE * upper;                                             \
                devices = SAC_CUDA_DEVICES;                                              \
            }                                                                            \
            device_n--;                                                                  \
        } else {                                                                         \
            device_n = SAC_MT_SELF_LOCAL_ID ();                                          \
            if (SAC_CUDA_DEVICES > 0) {                                                  \
                lb = upper * SAC_CUDA_SHARE;                                             \
                devices = SAC_MT_LOCAL_THREADS () - SAC_CUDA_DEVICES;                    \
            }                                                                            \
        }                                                                                \
                                                                                         \
        const int iterations = (ub - lb) / unrolling;                                    \
        const int iterations_per_thread = (iterations / devices) * unrolling;            \
        const int iterations_rest = iterations % SAC_MT_LOCAL_THREADS ();                \
                                                                                         \
        if (iterations_rest && (device_n < (unsigned int)iterations_rest)) {             \
            SAC_WL_MT_SCHEDULE_START (0)                                                 \
              = lb + device_n * (iterations_per_thread + unrolling);                     \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + (iterations_per_thread + unrolling);      \
        } else {                                                                         \
            SAC_WL_MT_SCHEDULE_START (0)                                                 \
              = (lb + iterations_rest * unrolling) + device_n * iterations_per_thread;   \
            SAC_WL_MT_SCHEDULE_STOP (0)                                                  \
              = SAC_WL_MT_SCHEDULE_START (0) + iterations_per_thread;                    \
        }                                                                                \
                                                                                         \
        SAC_TR_MT_PRINT (("Scheduler 'Block': dim 0: %d -> %d",                          \
                          SAC_WL_MT_SCHEDULE_START (0), SAC_WL_MT_SCHEDULE_STOP (0)));   \
    }

#define SAC_DIST_SCHEDULER_BlockVar_DIM0(lower, upper, unrolling)                        \
    {                                                                                    \
        SAC_DIST_SCHEDULER_Block_DIM0 (lower, upper, unrolling)                          \
    }

#endif

#endif
