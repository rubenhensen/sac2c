
/*****************************************************************************
 *
 * file:   cuda.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _SAC_CUDA_H_
#define _SAC_CUDA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "runtime/essentials_h/cuda_transfer_methods.h"
#include "runtime/essentials_h/cuda_sync_methods.h"
#include "libsac/cuda/sync.h"
#include "libsac/hwloc/cudabind.h"
#include "runtime/extras_h/rt_profile.h"

/*****************************************************************************
 *
 * ICMs for CUDA startup and finalization
 * =================
 *
 *****************************************************************************/


#if SAC_DO_CUDA_FORCE_INIT
#define SAC_CUDA_FORCE_INIT() {                                                          \
        SAC_TR_GPU_PRINT ("forcing CUDA device/context to be initilised with "           \
                          "dummmy call");                                                \
        size_t force_cuda_context_init;                                                  \
        cudaDeviceGetLimit (&force_cuda_context_init, cudaLimitMallocHeapSize);          \
    }
#else
#define SAC_CUDA_FORCE_INIT() SAC_NOOP ();
#endif /* SAC_DO_CUDA_FORCE_INIT */

#if SAC_SET_CPU_BIND_STRATEGY > 0
#define SAC_CUDA_BIND_SETUP() {                                                          \
    char _cuda_hwloc_status[1024];                                                       \
    SAC_TR_GPU_PRINT ("(hwloc) init CPU pinning for GPU card %d",                        \
                      SAC_SET_CPU_BIND_STRATEGY, 0);                                     \
    if (!SAC_CUDA_HWLOC_init (0, _cuda_hwloc_status, sizeof(_cuda_hwloc_status))) {      \
        SAC_RuntimeError ("HWLOC failure: unable to pin the closest CPU core to GPU!");  \
    }                                                                                    \
    SAC_TR_GPU_PRINT ("(hwloc) bound to %s", _cuda_hwloc_status);                        \
}
#else
#define SAC_CUDA_BIND_SETUP() SAC_NOOP ();
#endif /* SAC_SET_CPU_BIND_STRATEGY */

/**
 * At the moment we assume that we are dealing with only *one* stream, the default
 * steam (0). If want to support multple streams, we need to change this ICM. Additionally
 * we *DO NOT* handle any MT use-cases!!!
 */
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#if !defined(SAC_DO_CUDA_SYNC) || SAC_DO_CUDA_SYNC == SAC_CS_none                        \
    || SAC_DO_CUDA_SYNC == SAC_CS_device
#define SAC_CUDA_ASYNC_INIT() SAC_NOOP ();
#define SAC_CUDA_ASYNC_FINISH() SAC_NOOP ();
#elif SAC_DO_CUDA_SYNC == SAC_CS_stream
#define SAC_CUDA_ASYNC_INIT()                                                            \
    cudaEventCreateWithFlags (&cuda_sync_event,                                          \
                              cudaEventBlockingSync | cudaEventDisableTiming);           \
    SAC_CUDA_GET_LAST_ERROR ("CREATE SYNC EVENT");
#define SAC_CUDA_ASYNC_FINISH()                                                          \
    cudaEventDestroy (cuda_sync_event);                                                  \
    SAC_CUDA_GET_LAST_ERROR ("DESTROY SYNC EVENT");
#elif SAC_DO_CUDA_SYNC == SAC_CS_callback
#define SAC_CUDA_ASYNC_INIT()                                                            \
    cuda_async_spinlock_init (&cuda_sync_struct);
#define SAC_CUDA_ASYNC_FINISH()                                                          \
    cuda_async_spinlock_destroy (&cuda_sync_struct);
#endif /* SAC_DO_CUDA_SYNC */
#else
#define SAC_CUDA_ASYNC_INIT() SAC_NOOP ();
#define SAC_CUDA_ASYNC_FINISH() SAC_NOOP ();
#endif /* SAC_DO_CUDA_ALLOC */

#define SAC_CUDA_SETUP()                                                                 \
    SAC_CUDA_BIND_SETUP ();                                                              \
    SAC_CUDA_FORCE_INIT ();                                                              \
    SAC_CUDA_ASYNC_INIT ();
#define SAC_CUDA_FINALIZE()                                                              \
    SAC_CUDA_ASYNC_FINISH ();

/*****************************************************************************
 *
 * ICMs for CUDA related memory allocation and deallocation
 * =================
 *
 *****************************************************************************/

// Allocate on the device
#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system                    \
 || SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_CUDA_ALLOC(var_NT, basetype)                                                 \
    SAC_TR_GPU_PRINT ("Allocating CUDA device memory: %s", NT_STR (var_NT));             \
    SAC_PF_MEM_CUDA_INC_ALLOC (sizeof (basetype) * SAC_ND_A_SIZE (var_NT));              \
    cudaMalloc ((void **)&SAC_ND_A_FIELD (var_NT),                                       \
                SAC_ND_A_SIZE (var_NT) * sizeof (basetype));                             \
    SAC_GET_CUDA_MALLOC_ERROR ();
#else // Managed case
#define SAC_CUDA_ALLOC(var_NT, basetype)                                                 \
    SAC_TR_GPU_PRINT ("Allocating CUDA device memory (managed): %s", NT_STR (var_NT));   \
    SAC_PF_MEM_CUDA_INC_ALLOC (sizeof (basetype) * SAC_ND_A_SIZE (var_NT));              \
    cudaMallocManaged ((void **)&SAC_ND_A_FIELD (var_NT),                                \
                       SAC_ND_A_SIZE (var_NT) * sizeof (basetype));                      \
    SAC_GET_CUDA_MALLOC_ERROR ();
#endif

// Free device memory
#if SAC_DO_CUDA_ALLOC == SAC_CA_cuman || SAC_DO_CUDA_ALLOC == SAC_CA_cumanp
#define SAC_CUDA_FREE(var_NT, freefun)                                                   \
    SAC_TR_GPU_PRINT ("Freeing CUDA device memory (managed): %s", NT_STR (var_NT));      \
    SAC_PF_MEM_CUDA_INC_FREE (sizeof (*SAC_ND_A_FIELD (var_NT))                          \
                              * SAC_ND_A_SIZE (var_NT));                                 \
    cudaFree (SAC_ND_A_FIELD (var_NT));                                                  \
    SAC_GET_CUDA_FREE_ERROR ();
#else
#define SAC_CUDA_FREE(var_NT, freefun)                                                   \
    SAC_TR_GPU_PRINT ("Freeing CUDA device memory: %s", NT_STR (var_NT));                \
    SAC_PF_MEM_CUDA_INC_FREE (sizeof (*SAC_ND_A_FIELD (var_NT))                          \
                              * SAC_ND_A_SIZE (var_NT));                                 \
    cudaFree (SAC_ND_A_FIELD (var_NT));                                                  \
    SAC_GET_CUDA_FREE_ERROR ();
#endif

#define SAC_CUDA_ALLOC_BEGIN__DAO(var_NT, rc, dim, basetype)                             \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))     \
        SAC_CUDA_ALLOC__DATA (var_NT, basetype)                                          \
        SAC_ND_SET__RC (var_NT, rc)

#define SAC_CUDA_ALLOC_BEGIN__NO_DAO(var_NT, rc, dim, basetype)                          \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_PF_MEM_INC_ALLOC_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))     \
        SAC_ND_SET__RC (var_NT, rc)

#define SAC_CUDA_ALLOC_END__DAO(var_NT, rc, dim, basetype) }

#define SAC_CUDA_ALLOC_END__NO_DAO(var_NT, rc, dim, basetype)                            \
        SAC_CUDA_ALLOC__DATA (var_NT, basetype)                                          \
    }

#define SAC_CUDA_ALLOC__DATA__NOOP(var_NT, basetype) SAC_NOOP ()

#define SAC_CUDA_ALLOC__DATA__AKS(var_NT, basetype) SAC_CUDA_ALLOC (var_NT, basetype)

#define SAC_CUDA_ALLOC__DATA__AKD_AUD(var_NT, basetype) SAC_CUDA_ALLOC (var_NT, basetype)

#define SAC_CUDA_DEC_RC_FREE__UNQ(var_NT, rc, freefun)                                   \
    {                                                                                    \
        SAC_PF_MEM_INC_FREE_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))      \
        SAC_CUDA_FREE (var_NT, freefun)                                                  \
    }

#define SAC_CUDA_DEC_RC_FREE__NOOP(var_NT, rc, freefun) SAC_NOOP ()

#define SAC_CUDA_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                               \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("CUDA_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))              \
        if ((SAC_ND_A_RC (var_NT) -= rc) == 0) {                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_PF_MEM_INC_FREE_DESC (BYTE_SIZE_OF_DESC (SAC_ND_A_MIRROR_DIM (var_NT)))  \
            SAC_CUDA_FREE (var_NT, freefun)                                              \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

/*****************************************************************************
 *
 * ICMs for CUDA memory transfers
 * =================
 *
 *****************************************************************************/
/*
 * Transfer scalar on host to an array position on device
 */
#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system
#define SAC_CUDA_MEM_TRANSFER_SxA(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, &from_NT, sizeof (basetype),            \
                cudaMemcpyHostToDevice);                                                 \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_CUDA_MEM_TRANSFER_SxA(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpyAsync (SAC_ND_A_FIELD (to_NT) + offset, &from_NT, sizeof (basetype),       \
                     cudaMemcpyHostToDevice, 0);                                         \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#else /* managed */
#define SAC_CUDA_MEM_TRANSFER_SxA(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, &from_NT, sizeof (basetype),            \
                cudaMemcpyDefault);                                                      \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#endif

/*
 * Transfer data element in an array position on device to a host scalar
 */
#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system
#define SAC_CUDA_MEM_TRANSFER_AxS(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (&SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT) + offset,              \
                sizeof (basetype), cudaMemcpyDeviceToHost);                              \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_CUDA_MEM_TRANSFER_AxS(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpyAsync (&SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT) + offset,         \
                     sizeof (basetype), cudaMemcpyDeviceToHost, 0);                      \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#else /* managed */
#define SAC_CUDA_MEM_TRANSFER_AxS(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (&SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT) + offset,              \
                sizeof (basetype), cudaMemcpyDefault);                                   \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#endif

/*
 * Transfer data from a device array to another device array
 */
#define SAC_CUDA_MEM_TRANSFER_D2D(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, SAC_ND_A_FIELD (from_NT),               \
                sizeof (basetype) * SAC_ND_A_SIZE (from_NT), cudaMemcpyDeviceToDevice);  \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();

// This ICM is used most often for doing cudaMemcpy
#if !defined(SAC_DO_CUDA_ALLOC) || SAC_DO_CUDA_ALLOC == SAC_CA_system
#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype), direction);          \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    cudaMemcpyAsync (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                   \
                     SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype), direction, 0);  \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#elif SAC_DO_CUDA_ALLOC == SAC_CA_cureg
#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    cudaMemcpyAsync (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                   \
                     SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype), direction, 0);  \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#else /* managed */
/* We have two choices here, either we use CUDA's memcpy with cudaMemcpyDefault which
 * fully supports UVA but is an API call, or we perform a swap of the pointers.
 * Keep in mind that in the latter case, that from_NT will be immediately freed. Either
 * option seems not to affect number of page-faults or have any noticable difference in
 * performance. NOTE: with memcpy, the implementation can choose to use an actual memcpy,
 * rather than UVA, which might affect performance. This cannot be known a-priori!
 */
#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    std::swap (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT));                        \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();
#endif

/* CUDA synchronise methods when using asynchronous transfers */
#if SAC_DO_CUDA_ALLOC == SAC_CA_cureg || SAC_DO_CUDA_ALLOC == SAC_CA_cualloc
#if !defined(SAC_DO_CUDA_SYNC) || SAC_DO_CUDA_SYNC == SAC_CS_none
#define SAC_CUDA_MEM_TRANSFER_SYNC_START()
#define SAC_CUDA_MEM_TRANSFER_SYNC_END(var_NT)
#elif SAC_DO_CUDA_SYNC == SAC_CS_device
#define SAC_CUDA_MEM_TRANSFER_SYNC_START()
#define SAC_CUDA_MEM_TRANSFER_SYNC_END(var_NT)                                           \
    SAC_TR_GPU_PRINT ("Syncing for array: %s", NT_STR (var_NT));                         \
    cudaDeviceSynchronize ();                                                            \
    SAC_CUDA_GET_LAST_ERROR ("GPU SYNC END")
#elif SAC_DO_CUDA_SYNC == SAC_CS_stream
#define SAC_CUDA_MEM_TRANSFER_SYNC_START()                                               \
    cudaEventRecord (cuda_sync_event, 0);                                                \
    SAC_CUDA_GET_LAST_ERROR ("GPU SYNC START");
#define SAC_CUDA_MEM_TRANSFER_SYNC_END(var_NT)                                           \
    SAC_TR_GPU_PRINT ("Syncing for array: %s", NT_STR (var_NT));                         \
    cudaEventSynchronize (cuda_sync_event);                                              \
    SAC_CUDA_GET_LAST_ERROR ("GPU SYNC END")
#elif SAC_DO_CUDA_SYNC == SAC_CS_callback
#define SAC_CUDA_MEM_TRANSFER_SYNC_START()                                               \
    cudaLaunchHostFunc (0, cuda_rt_unlock, (void*)&cuda_sync_struct);                    \
    SAC_CUDA_GET_LAST_ERROR ("GPU SYNC START");
#define SAC_CUDA_MEM_TRANSFER_SYNC_END(var_NT)                                           \
    SAC_TR_GPU_PRINT ("Syncing for array: %s", NT_STR (var_NT));                         \
    cuda_async_spinlock_wait (&cuda_sync_struct);
#endif /* SAC_DO_CUDA_SYNC */
#else
#define SAC_CUDA_MEM_TRANSFER_SYNC_START()
#define SAC_CUDA_MEM_TRANSFER_SYNC_END(var_NT)
#endif /* SAC_DO_CUDA_ALLOC */

#define SAC_CUDA_MEM_PREFETCH(var_NT, basetype, device)                                  \
    SAC_TR_GPU_PRINT ("Prefetching CUDA memory: %s", NT_STR (var_NT));                   \
    cudaMemPrefetchAsync (SAC_ND_A_FIELD (var_NT),                                       \
                          SAC_ND_A_SIZE (var_NT) * sizeof (basetype),                    \
                          device, 0);                                                    \
    SAC_CUDA_GET_LAST_ERROR ("GPU MEMORY PREFETCH")

#define SAC_CUDA_COPY__ARRAY(to_NT, from_NT, basetype, freefun)                          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype),                      \
                cudaMemcpyDeviceToDevice);                                               \
    SAC_GET_CUDA_MEM_TRANSFER_ERROR ();

#define SAC_CUDA_FUN_DEF_END(...)

/*****************************************************************************
 *
 * ICMs for CUDA kernels
 * =================
 *
 *****************************************************************************/

/*
 * CUDA kernel parameters and arguments
 */
#define SAC_CUDA_PARAM_in__SCL(var_NT, basetype) basetype NT_NAME (var_NT)

#define SAC_CUDA_PARAM_in__AKS_AKD(var_NT, basetype) basetype *NT_NAME (var_NT)

#define SAC_CUDA_ARG_in__SCL(var_NT, basetype) NT_NAME (var_NT)

#define SAC_CUDA_ARG_in__AKS_AKD(var_NT, basetype) NT_NAME (var_NT)

#define SAC_CUDA_PARAM_out__SCL(var_NT, basetype)                                        \
    basetype *SAC_NAMEP (SAC_ND_A_FIELD (var_NT))

#define SAC_CUDA_PARAM_out__AKS_AKD(var_NT, basetype) basetype *NT_NAME (var_NT)

#define SAC_CUDA_ARG_out__SCL(var_NT, basetype) &NT_NAME (var_NT)

#define SAC_CUDA_ARG_out__AKS_AKD(var_NT, basetype) NT_NAME (var_NT)

#define SAC_CUDA_PARAM_inout__SCL(var_NT, basetype)                                      \
    basetype *SAC_NAMEP (SAC_ND_A_FIELD (var_NT))

#define SAC_CUDA_PARAM_inout__AKS_AKD(var_NT, basetype) basetype *NT_NAME (var_NT)

#define SAC_CUDA_ARG_inout__SCL(var_NT, basetype) &NT_NAME (var_NT)

#define SAC_CUDA_ARG_inout__AKS_AKD(var_NT, basetype) NT_NAME (var_NT)

/*
 * CUDA built-in index variables
 */
#define BLOCKIDX_X blockIdx.x
#define BLOCKIDX_Y blockIdx.y

#define BLOCKDIM_X blockDim.x
#define BLOCKDIM_Y blockDim.y
#define BLOCKDIM_Z blockDim.z

#define GRIDDIM_X gridDim.x
#define GRIDDIM_Y gridDim.y

#define THREADIDX_X threadIdx.x
#define THREADIDX_Y threadIdx.y
#define THREADIDX_Z threadIdx.z

#define SAC_CUDA_BLOCKIDX_X(to_nt) NT_NAME (to_nt) = blockIdx.x;
#define SAC_CUDA_BLOCKIDX_Y(to_nt) NT_NAME (to_nt) = blockIdx.y;

#define SAC_CUDA_BLOCKDIM_X(to_nt) NT_NAME (to_nt) = blockDim.x;
#define SAC_CUDA_BLOCKDIM_Y(to_nt) NT_NAME (to_nt) = blockDim.y;

#define SAC_CUDA_THREADIDX_X(to_nt) NT_NAME (to_nt) = threadIdx.x;
#define SAC_CUDA_THREADIDX_Y(to_nt) NT_NAME (to_nt) = threadIdx.y;
#define SAC_CUDA_THREADIDX_Z(to_nt) NT_NAME (to_nt) = threadIdx.z;

#define SAC_CUDA_GRIDDIM_X(to_nt) NT_NAME (to_nt) = gridDim.x;
#define SAC_CUDA_GRIDDIM_Y(to_nt) NT_NAME (to_nt) = gridDim.y;

/*
 * CUDA sync thread primitive
 */
#define SAC_CUDA_SYNCTHREADS() syncthreads ();

#define SAC_CUDA_KERNEL_TERMINATE() return;

#define SAC_CUDA_PRF_SYNCIN(to_nt, from_nt) NT_NAME (to_nt) = NT_NAME (from_nt);

#define SAC_CUDA_PRF_SYNCOUT(to_nt, from_nt) NT_NAME (to_nt) = NT_NAME (from_nt);

/*****************************************************************************
 *
 * ICMs for CUDA bound computation and checking
 * =================
 *
 *****************************************************************************/

/*********************** 1D case (Without Step/Width) ***********************/
#define SAC_CUDA_WLIDS_1D_0(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X + lb_var;                                  \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

/*********************** 1D case (With Step/Width) ***********************/
#define SAC_CUDA_WLIDS_1D_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X;       \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

/*********************** 2D case (Without Step/Width) ***********************/
#define SAC_CUDA_WLIDS_2D_0(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_Y * BLOCKDIM_Y + THREADIDX_Y + lb_var;                                  \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_2D_1(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X + lb_var;                                  \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

/*********************** 2D case (With Step/Width) ***********************/
#define SAC_CUDA_WLIDS_2D_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y * BLOCKDIM_Y + THREADIDX_Y;       \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_2D_SW_1(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X;       \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

/*********************** ND case (Without Step/Width) ***********************/
#define SAC_CUDA_WLIDS_ND_0(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_1(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_2(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_3(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_4(wlids_NT, wlids_NT_dim, lb_var, ub_var)                      \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Z + lb_var;

/*********************** ND case (With Step/Width) ***********************/
#define SAC_CUDA_WLIDS_ND_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    if (step_var > 1 && (BLOCKIDX_Y % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_1(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    if (step_var > 1 && (BLOCKIDX_X % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_2(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    if (step_var > 1 && (THREADIDX_X % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_3(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    if (step_var > 1 && (THREADIDX_Y % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_4(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var, \
                               ub_var)                                                   \
    if (step_var > 1 && (THREADIDX_Z % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Z + lb_var;

/*****************************************************************************
 *
 * NEW ICMs for WLIDS
 * =================
 *
 *****************************************************************************/

#define SAC_CUDA_WLIDS(wlids_NT, wlids_NT_dim, blockidx, blockdim, threadidx, step_var, \
                       width_var, lb_var, ub_var)                                        \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = blockidx * blockdim + threadidx + lb_var;    \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_SW(wlids_NT, wlids_NT_dim, blockidx, blockdim, threadidx, \
                          step_var, width_var, lb_var, ub_var)                           \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = blockidx * blockdim + threadidx;             \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_HD(wlids_NT, wlids_NT_dim, index, step_var, width_var, lb_var, \
                          ub_var)                                                        \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = index + lb_var;

#define SAC_CUDA_WLIDS_HD_SW(wlids_NT, wlids_NT_dim, index, step_var, width_var, lb_var, \
                             ub_var)                                                     \
    if (step_var > 1 && (index % step_var > (width_var - 1)))                            \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = index + lb_var;

/*****************************************************************************
 *
 * ICMs for CUDA kernel mappings on the host
 * =================
 *
 *****************************************************************************/

#define SAC_GKCO_OPD_DECLARE(var)                                                                           \
    int var;

#define SAC_GKCO_OPD_REDEFINE(from, to)                                                                     \
    to = from;

#define SAC_GKCO_HOST_OPD_SHIFT_LB(lb, ub)                                                                  \
    ub = ub - lb;

#define SAC_GKCO_HOST_OPD_COMPRESS_S(ub_r, ub_w, st)                                                        \
    ub_w = ub_r / st                                                                                        \
            + (ub_r % st != 0);

#define SAC_GKCO_HOST_OPD_COMPRESS_SW(ub_r, ub_w, st, wi, tmp)                                              \
    tmp = ub_r % st;                                                                                        \
    ub_w = ub_r / st * wi                                                                                   \
            + (tmp < wi ? tmp : wi);

#define SAC_GKCO_HOST_OPM_SET_GRID(max_x, max_y, max_z, max_total, ...)                                     \
    dim3 grid (__VA_ARGS__);                                                                                \
    SAC_TR_GPU_PRINT("CUDA XYZ grid dimension of %u x %u x %u", grid.x, grid.y, grid.z);                    \
                                                                                                            \
    if (grid.x <= 0)                                                                                        \
        SAC_RuntimeError("CUDA x grid dimension must be bigger then zero. Current value is %u", grid.x);    \
    if (grid.y <= 0)                                                                                        \
        SAC_RuntimeError("CUDA y grid dimension must be bigger then zero. Current value is %u", grid.y);    \
    if (grid.z <= 0)                                                                                        \
        SAC_RuntimeError("CUDA z grid dimension must be bigger then zero. Current value is %u", grid.z);    \
                                                                                                            \
    if (grid.x > max_x || grid.y > max_y, || grid.z > max_z)                                                \
        SAC_RuntimeError("CUDA XYZ grid dimension of %u x %u x %u exceeds "                                 \
                         "the compute capability's max value of %u x %u x %u",                              \
                         grid.x, grid.y, grid.z, max_x, max_y, max_z);

#define SAC_GKCO_HOST_OPM_SET_BLOCK(max_x, max_y, max_z, max_total, ...)                                    \
    dim3 block (__VA_ARGS__);                                                                               \
    SAC_TR_GPU_PRINT("CUDA XYZ block dimension of %u x %u x %u", block.x, block.y, block.z);                \
                                                                                                            \
    if (block.x <= 0)                                                                                       \
        SAC_RuntimeError("CUDA x block dimension must be bigger then zero. Current value is %u", block.x);  \
    if (block.y <= 0)                                                                                       \
        SAC_RuntimeError("CUDA y block dimension must be bigger then zero. Current value is %u", block.y);  \
    if (block.z <= 0)                                                                                       \
        SAC_RuntimeError("CUDA z block dimension must be bigger then zero. Current value is %u", block.z);  \
                                                                                                            \
    if (block.x > max_x || block.y > max_y, || block.z > max_z)                                             \
        SAC_RuntimeError("CUDA XYZ block dimension of %u x %u x %u exceeds "                                \
                         "the compute capability's max value of %u x %u x %u",                              \
                         block.x, block.y, block.z, max_x, max_y, max_z);                                   \
    if (block.x * block.y * block.z > max_total)                                                            \
        SAC_RuntimeError("CUDA XYZ block dimension of %u x %u x %u = %u exceeds compute capability's "      \
                         "max number of threads per block: %u",                                             \
                         block.x, block.y, block.z, block.x * block.y * block.z, max_total);

/*****************************************************************************
 *
 * ICMs for CUDA kernel mappings on the device
 * =================
 *
 *****************************************************************************/

#define SAC_GKCO_GPUD_OPD_UNSHIFT_LB(lb, idx)                                                               \
    idx = idx + lb;

#define SAC_GKCO_GPUD_OPD_UNCOMPRESS_S(st, idx)                                                             \
    idx = idx * st;

#define SAC_GKCO_GPUD_OPD_UNCOMPRESS_SW(st, wi, idx)                                                        \
    idx = idx / wi * st                                                                                     \
            + idx % wi;

#define SAC_GKCO_GPUD_OPM_DECLARE_IV(iv_var, iv_length) \
    int iv_var[iv_length];

#define SAC_GKCO_GPUD_OPD_DEF_IV(iv_var, dimension, value_var)                                              \
    iv_var[dimension] = value_var;


/*****************************************************************************
 *
 * ICMs for CUDA kernel array decalaration
 * =================
 *
 *****************************************************************************/
/*
#define SAC_CUDA_DECL_KERNEL_ARRAY( var_NT, basetype, dim) \
  basetype[dim] SAC_ND_A_FIELD( var_NT);
*/

#define SAC_CUDA_FORLOOP_BEGIN(iterator, upper_bound) for (; iterator < upper_bound;) {

#define SAC_CUDA_FORLOOP_END() }

#ifdef __cplusplus
}
#endif

#endif /* _SAC_CUDA_H */
