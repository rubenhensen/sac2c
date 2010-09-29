
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

/*****************************************************************************
 *
 * ICMs for CUDA realated memory allocation and deallocation
 * =================
 *
 *****************************************************************************/
#define SAC_CUDA_ALLOC_BEGIN__DAO(var_NT, rc, dim, basetype)                             \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_CUDA_ALLOC__DATA (var_NT, basetype)                                          \
        SAC_ND_SET__RC (var_NT, rc)

#define SAC_CUDA_ALLOC_BEGIN__NO_DAO(var_NT, rc, dim, basetype)                          \
    {                                                                                    \
        SAC_ND_ALLOC__DESC (var_NT, dim)                                                 \
        SAC_ND_SET__RC (var_NT, rc)

#define SAC_CUDA_ALLOC_END__DAO(var_NT, rc, dim, basetype) }

#define SAC_CUDA_ALLOC_END__NO_DAO(var_NT, rc, dim, basetype)                            \
    SAC_CUDA_ALLOC__DATA (var_NT, basetype)                                              \
    }

#define SAC_CUDA_FREE(var_NT, freefun) cudaFree (SAC_ND_A_FIELD (var_NT));

#define SAC_CUDA_ALLOC__DATA__NOOP(var_NT, basetype) SAC_NOOP ()

#define SAC_CUDA_ALLOC__DATA__AKS(var_NT, basetype)                                      \
    cudaMalloc ((void **)&SAC_ND_A_FIELD (var_NT),                                       \
                SAC_ND_A_SIZE (var_NT) * sizeof (basetype));

#define SAC_CUDA_ALLOC__DATA__AKD_AUD(var_NT, basetype)                                  \
    cudaMalloc ((void **)&SAC_ND_A_FIELD (var_NT),                                       \
                SAC_ND_A_SIZE (var_NT) * sizeof (basetype));

#define SAC_CUDA_DEC_RC_FREE__UNQ(var_NT, rc, freefun) SAC_CUDA_FREE (var_NT, freefun)

#define SAC_CUDA_DEC_RC_FREE__NOOP(var_NT, rc, freefun) SAC_NOOP ()

#define SAC_CUDA_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                               \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("CUDA_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))              \
        if ((SAC_ND_A_RC (var_NT) -= rc) == 0) {                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
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
#define SAC_CUDA_MEM_TRANSFER_SxA(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, &from_NT, sizeof (basetype),            \
                cudaMemcpyHostToDevice);

/*
 * Transfer data element in an array position on deive to a host scalar
 */
#define SAC_CUDA_MEM_TRANSFER_AxS(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (&SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT) + offset,              \
                sizeof (basetype), cudaMemcpyDeviceToHost);

/*
 * Transfer data from a device array to another device array
 */
#define SAC_CUDA_MEM_TRANSFER_D2D(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, SAC_ND_A_FIELD (from_NT),               \
                sizeof (basetype) * SAC_ND_A_SIZE (from_NT), cudaMemcpyDeviceToDevice);

#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype), direction);

#define SAC_CUDA_COPY__ARRAY(to_NT, from_NT, basetype, freefun)                          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype),                      \
                cudaMemcpyDeviceToDevice);

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

#define THREADIDX_X threadIdx.x
#define THREADIDX_Y threadIdx.y
#define THREADIDX_Z threadIdx.z

#define SAC_CUDA_BLOCKIDX_X() blockIdx.x;
#define SAC_CUDA_BLOCKIDX_Y() blockIdx.y;

#define SAC_CUDA_BLOCKDIM_X() blockDim.x;
#define SAC_CUDA_BLOCKDIM_Y() blockDim.y;

#define SAC_CUDA_THREADIDX_X() threadIdx.x;
#define SAC_CUDA_THREADIDX_Y() threadIdx.y;
#define SAC_CUDA_THREADIDX_Z() threadIdx.z;

/*
 * CUDA sync thread primitive
 */
#define SAC_CUDA_SYNCTHREADS() syncthreads ();

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
#define SAC_CUDA_WLIDS_1D_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
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
#define SAC_CUDA_WLIDS_2D_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
                               ub_var)                                                   \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y * BLOCKDIM_Y + THREADIDX_Y;       \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_2D_SW_1(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
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
#define SAC_CUDA_WLIDS_ND_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
                               ub_var)                                                   \
    if (step_var > 1 && (BLOCKIDX_Y % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_1(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
                               ub_var)                                                   \
    if (step_var > 1 && (BLOCKIDX_X % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_2(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
                               ub_var)                                                   \
    if (step_var > 1 && (THREADIDX_X % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_3(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
                               ub_var)                                                   \
    if (step_var > 1 && (THREADIDX_Y % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_4(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var,      \
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

#define SAC_CUDA_WLIDS(wlids_NT, wlids_NT_dim, blockidx, blockdim, threadidx, step_var,  \
                       width_var, lb_var, ub_var)                                        \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = blockidx * blockdim + threadidx + lb_var;    \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_SW(wlids_NT, wlids_NT_dim, blockidx, blockdim, threadidx,         \
                          step_var, width_var, lb_var, ub_var)                           \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = blockidx * blockdim + threadidx;             \
    if (step_var > 1                                                                     \
        && (SAC_ND_READ (wlids_NT, wlids_NT_dim) % step_var > (width_var - 1)))          \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) += lb_var;                                     \
    if (SAC_ND_READ (wlids_NT, wlids_NT_dim) >= ub_var)                                  \
        return;

#define SAC_CUDA_WLIDS_HD(wlids_NT, wlids_NT_dim, index, step_var, width_var, lb_var,    \
                          ub_var)                                                        \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = index + lb_var;

#define SAC_CUDA_WLIDS_HD_SW(wlids_NT, wlids_NT_dim, index, step_var, width_var, lb_var, \
                             ub_var)                                                     \
    if (step_var > 1 && (index % step_var > (width_var - 1)))                            \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = index + lb_var;

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

#endif
