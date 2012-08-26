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

#ifdef __CUDACC__

#define getDeviceNumber(SAC_MT_mythread)                                                 \
    SAC_MT_mythread - (SAC_MT_LOCAL_THREADS () - SAC_CUDA_DEVICES) + 1

#define SAC_CUDA_ENV_VAR_NAME "SAC_CUDA"

#define SAC_DIST_SETUP() SAC_DIST_setup (__argc, __argv)

void
SAC_DIST_setup (int argc, char **argv)
{
    // multiple cuda setup
    bool cuda_option_exists = FALSE;
    char *sac_parallel = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if ((argv[i][0] == '-') && (argv[i][1] == 'c') && (argv[i][2] == 'u')
            && (argv[i][3] == 'd') && (argv[i][4] == 'a') && (argv[i][5] == '\0')) {
            SAC_CUDA_DEVICES = atoi (argv[i + 1]);
            cuda_option_exists = TRUE;
            break;
        }
    }
    if (!cuda_option_exists) {
        sac_parallel = getenv (SAC_CUDA_ENV_VAR_NAME);
        SAC_CUDA_DEVICES = (sac_parallel != NULL) ? atoi (sac_parallel) : 1;
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

#endif

#endif
