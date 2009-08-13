

#ifndef _SAC_CUDA_H_
#define _SAC_CUDA_H_

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

/* Transfer scalar on host to an array position on deive */
#define SAC_CUDA_MEM_TRANSFER_SxA(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, &from_NT, sizeof (basetype),            \
                cudaMemcpyHostToDevice);

/* Transfer data element in an array position on deive to a host scalar*/
#define SAC_CUDA_MEM_TRANSFER_AxS(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (&SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT) + offset,              \
                sizeof (basetype), cudaMemcpyDeviceToHost);

/* Transfer data from a device array to another device array */
#define SAC_CUDA_MEM_TRANSFER_D2D(to_NT, offset, from_NT, basetype)                      \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT) + offset, SAC_ND_A_FIELD (from_NT),               \
                sizeof (basetype) * SAC_ND_A_SIZE (from_NT), cudaMemcpyDeviceToDevice);

#define SAC_CUDA_MEM_TRANSFER__AKS_AKD_AUD(to_NT, from_NT, basetype, direction)          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype), direction);

//------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------------
#define SAC_CUDA_COPY__ARRAY(to_NT, from_NT, basetype, freefun)                          \
    cudaMemcpy (SAC_ND_A_FIELD (to_NT), SAC_ND_A_FIELD (from_NT),                        \
                SAC_ND_A_MIRROR_SIZE (from_NT) * sizeof (basetype),                      \
                cudaMemcpyDeviceToDevice);

//--------------------------------------------------------------

/* parameters and arguments */

#define SAC_CUDA_PARAM__SCL(var_NT, basetype) basetype NT_NAME (var_NT)
#define SAC_CUDA_PARAM__AKS_AKD(var_NT, basetype) basetype *NT_NAME (var_NT)

#define SAC_CUDA_ARG__SCL(var_NT, basetype) NT_NAME (var_NT)
#define SAC_CUDA_ARG__AKS_AKD(var_NT, basetype) NT_NAME (var_NT)

#define BLOCKIDX_X blockIdx.x
#define BLOCKIDX_Y blockIdx.y

#define BLOCKDIM_X blockDim.x
#define BLOCKDIM_Y blockDim.y

#define THREADIDX_X threadIdx.x
#define THREADIDX_Y threadIdx.y
#define THREADIDX_Z threadIdx.z

//------------------ 1D case ---------------------
#define SAC_CUDA_WLIDS_1D_0(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X + lb_var;

//------------------ 2D case ---------------------
#define SAC_CUDA_WLIDS_2D_0(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_Y * BLOCKDIM_Y + THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_2D_1(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim)                                                \
      = BLOCKIDX_X * BLOCKDIM_X + THREADIDX_X + lb_var;

//------------------ ND case with step and width ---------------------
#define SAC_CUDA_WLIDS_ND_SW_0(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var)      \
    if (step_var > 1 && (BLOCKIDX_X % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_1(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var)      \
    if (step_var > 1 && (BLOCKIDX_Y % step_var > (width_var - 1)))                       \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_2(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var)      \
    if (step_var > 1 && (THREADIDX_X % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_3(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var)      \
    if (step_var > 1 && (THREADIDX_Y % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_SW_4(wlids_NT, wlids_NT_dim, step_var, width_var, lb_var)      \
    if (step_var > 1 && (THREADIDX_Z % step_var > (width_var - 1)))                      \
        return;                                                                          \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Z + lb_var;

//------------------ ND case without step and width ---------------------

#define SAC_CUDA_WLIDS_ND_0(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_1(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = BLOCKIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_2(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_X + lb_var;

#define SAC_CUDA_WLIDS_ND_3(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Y + lb_var;

#define SAC_CUDA_WLIDS_ND_4(wlids_NT, wlids_NT_dim, lb_var)                              \
    SAC_ND_WRITE (wlids_NT, wlids_NT_dim) = THREADIDX_Z + lb_var;

#endif