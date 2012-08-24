
/*****************************************************************************
 *
 * file:   cudahybrid.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _SAC_CUDAHYBRID_H_
#define _SAC_CUDAHYBRID_H_

/*****************************************************************************
 *
 * ICMs for distributed variables allocation and deallocation
 * =================
 *
 *****************************************************************************/
#define SAC_DIST_ALLOC(var_NT, rc, dim, basetype)                                        \
    SAC_ND_ALLOC__DESC (var_NT, dim)                                                     \
    SAC_ND_A_FIELD (var_NT)                                                              \
      = dist_var_alloc (SAC_ND_A_SHAPE (var_NT, 0), SAC_ND_A_SIZE (var_NT),              \
                        sizeof (basetype));                                              \
    SAC_ND_SET__RC (var_NT, rc)

#define SAC_DIST_FREE(var_NT, freefun) dist_var_free (SAC_ND_A_FIELD (var_NT));

#define SAC_DIST_DEC_RC_FREE(var_NT, rc, freefun)                                        \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("DIST_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))              \
        if ((SAC_ND_A_RC (var_NT) -= rc) == 0) {                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_DIST_FREE (var_NT, freefun)                                              \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

/*****************************************************************************
 *
 * ICMs for DIST memory transfers
 * =================
 *
 *****************************************************************************/
#define SAC_DIST_DEV2DIST(to_NT, orig_NT, devnumber_NT, start_NT, stop_NT)               \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = conc2dist (SAC_ND_A_FIELD (orig_NT), SAC_ND_A_FIELD (start_NT),                  \
                   SAC_ND_A_FIELD (stop_NT), SAC_ND_A_FIELD (devnumber_NT));

#define SAC_DIST_HOST2DIST_SPMD(to_NT, orig_NT)                                          \
    SAC_ND_A_FIELD (to_NT) = conc2dist (SAC_ND_A_FIELD (orig_NT), SAC_schedule_start0,   \
                                        SAC_schedule_stop0, 0);

#define SAC_DIST_HOST2DIST_ST(to_NT, orig_NT, start, stop)                               \
    SAC_ND_A_FIELD (to_NT) = conc2dist (SAC_ND_A_FIELD (orig_NT), start, stop, 0);

#define SAC_DIST_DIST2HOST_ABS(to_NT, from_NT, start, stop, setOwner, basetype)          \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = (basetype *)dist2conc (SAC_ND_A_FIELD (from_NT), start, stop, 0, setOwner,       \
                               (cudaStream_t *)0);

#define SAC_DIST_DIST2DEV_ABS(to_NT, from_NT, start, stop, device_NT, basetype)          \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = (basetype *)dist2conc (SAC_ND_A_FIELD (from_NT), start, stop,                    \
                               SAC_ND_A_FIELD (device_NT), FALSE, stream);

#define SAC_DIST_DIST2HOST_REL(to_NT, from_NT, start, stop, setOwner, basetype)          \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = (basetype *)dist2conc (SAC_ND_A_FIELD (from_NT), start + SAC_schedule_start0,    \
                               stop + SAC_schedule_stop0, 0, setOwner,                   \
                               (cudaStream_t *)0);

#define SAC_DIST_DIST2DEV_REL(to_NT, from_NT, start, stop, device_NT, basetype)          \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = (basetype *)dist2conc (SAC_ND_A_FIELD (from_NT), start + SAC_schedule_start0,    \
                               stop + SAC_schedule_stop0, SAC_ND_A_FIELD (device_NT),    \
                               TRUE, stream);

#define SAC_DIST_DIST2DEV_AVAIL(to_NT, from_NT, start, stop, device_NT, start_NT,        \
                                stop_NT, basetype)                                       \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = (basetype *)dist2conc (SAC_ND_A_FIELD (from_NT),                                 \
                               start + SAC_ND_A_FIELD (start_NT),                        \
                               stop + SAC_ND_A_FIELD (stop_NT),                          \
                               SAC_ND_A_FIELD (device_NT), FALSE, stream);

#define SAC_DIST_DISTCONTBLOCK(to_NT, from_NT, start, stop, device_NT, availstart_NT,    \
                               availstop_NT)                                             \
    SAC_ND_A_FIELD (to_NT)                                                               \
      = dist_end_contiguous_block (SAC_ND_A_FIELD (from_NT),                             \
                                   start + SAC_ND_A_FIELD (availstart_NT),               \
                                   stop + SAC_ND_A_FIELD (availstop_NT),                 \
                                   SAC_ND_A_FIELD (device_NT));                          \
    SAC_ND_A_FIELD (to_NT) = (SAC_ND_A_FIELD (to_NT) == SAC_ND_A_SHAPE (from_NT, 0)      \
                                ? SAC_ND_A_FIELD (to_NT)                                 \
                                : SAC_ND_A_FIELD (to_NT) - stop);                        \
    SAC_ND_A_FIELD (availstop_NT)                                                        \
      = min (SAC_ND_A_FIELD (availstop_NT),                                              \
             max (SAC_ND_A_FIELD (availstart_NT) + 1, SAC_ND_A_FIELD (to_NT)));

/*****************************************************************************
 *
 * Other ICMs for CUDA
 * =================
 *
 *****************************************************************************/

#define SAC_DIST_GETCUDATHREAD(var_NT)                                                   \
    SAC_ND_A_FIELD (var_NT) = getDeviceNumber (SAC_MT_SELF_LOCAL_ID ());

#define SAC_CUDA_SET_DEVICE(var_NT)                                                      \
    cudaSetDevice (SAC_ND_A_FIELD (var_NT) - 1);                                         \
    SAC_CUDA_GET_STREAM (var_NT)

#define SAC_CUDA_DEVICE_SYNC(devnumber_NT)                                               \
    cudaError_t error = cudaDeviceSynchronize ();                                        \
    if (error != cudaSuccess)                                                            \
        SAC_RuntimeError ("CUDA error synchronizing devices: %s\n",                      \
                          cudaGetErrorString (error));                                   \
    cache_stream_destroy (SAC_ND_A_FIELD (devnumber_NT) - 1);

#define SAC_CUDA_GET_STREAM(var_NT)                                                      \
    cudaStream_t *stream = cache_stream_create (SAC_ND_A_FIELD (var_NT) - 1);

/*****************************************************************************
 *
 * ICMs for scheduling
 * =================
 *
 *****************************************************************************/

#define SAC_SCHED_START(var_NT, dim) SAC_ND_A_FIELD (var_NT) = SAC_schedule_start##dim;

#define SAC_SCHED_STOP(var_NT, dim) SAC_ND_A_FIELD (var_NT) = SAC_schedule_stop##dim;

#endif
