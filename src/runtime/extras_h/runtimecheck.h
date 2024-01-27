/*****************************************************************************
 *
 * file:   sac_runtimecheck.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h.
 *   It provides macros for runtime checks.
 *
 *****************************************************************************/

#ifndef _SAC_RUNTIMECHECK_H_
#define _SAC_RUNTIMECHECK_H_

/******************************************************************************
 *
 * ICMs for dynamic type checks
 * ============================
 *
 * ASSURE_TYPE( cond, message)
 *
 ******************************************************************************/

#if SAC_DO_CHECK_TYPE

#define SAC_ASSURE_TYPE(cond, message)                                                   \
    if (!(cond)) {                                                                       \
        SAC_RuntimeWarning (message);                                                    \
    }
/* yes, in C '0;' is indeed a legal statement 8-)) */

#define SAC_ASSURE_TYPE_LINE(filename, line, col, cond, message)                         \
    if (!(cond)) {                                                                       \
        SAC_RuntimeWarningLoc (filename, line, col, message);                            \
    }

#else /* SAC_DO_CHECK_TYPE */

#define SAC_ASSURE_TYPE(cond, message) /* nothing */

#define SAC_ASSURE_TYPE_LINE(filename, line, col, cond, message) /* nothing */

#endif /* SAC_DO_CHECK_TYPE */

/******************************************************************************
 *
 * ICMs for boundary checks
 * ========================
 *
 * BC_READ( var_NT, pos)
 * BC_WRITE( var_NT, pos)
 *
 ******************************************************************************
 *
 * REMARK:
 *   The current way of boundcchecking is not yet satisfying.
 *
 *   Bounds are not checked in each dimension, but only accesses
 *   are determined which exceed the memory allocated for the accessed
 *   array. However, doing boundary checking correctly would require
 *   much more effort.
 *
 ******************************************************************************/

#if SAC_DO_CHECK_BOUNDARY

#define SAC_BC_READ(var_NT, pos)                                                         \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (var_NT)))                                      \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Memory access violation on reading from array %s\n"         \
                            "*** with size %d at index position %d !\n",                 \
                            NT_STR (var_NT), SAC_ND_A_SIZE (var_NT), pos),               \
          0)),

#define SAC_BC_WRITE(var_NT, pos)                                                        \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (var_NT)))                                      \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Memory access violation on writing into array %s\n"         \
                            "*** with size %d at index position %d !\n",                 \
                            NT_STR (var_NT), SAC_ND_A_SIZE (var_NT), pos),               \
          0)),

#else /* SAC_DO_CHECK_BOUNDARY */

#define SAC_BC_WRITE(var_NT, pos)
#define SAC_BC_READ(var_NT, pos)

#endif /* SAC_DO_CHECK_BOUNDARY */

/******************************************************************************
 *
 * ICMs for distributed memory runtime checks
 * ========================
 *
 * #define SAC_DISTMEM_CHECK_PTR_FROM_CACHE( var_NT, pos, ptr)
 * SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED()
 * SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC_ALLOWED()
 * SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_EXEC_ALLOWED()
 * SAC_DISTMEM_CHECK_IS_SWITCH_TO_DIST_EXEC_ALLOWED()
 * SAC_DISTMEM_CHECK_IS_SWITCH_TO_SYNC_EXEC_ALLOWED()
 * SAC_DISTMEM_CHECK_WRITE_ALLOWED( ptr, var_NT, pos)
 * SAC_DISTMEM_CHECK_READ_ALLOWED( ptr, var_NT, pos)
 * SAC_DISTMEM_CHECK_IS_NON_DIST_ARR( var_NT)
 * SAC_DISTMEM_CHECK_IS_DSM_ARR( var_NT)
 *
 ******************************************************************************/

#if SAC_DO_CHECK_DISTMEM

#define SAC_DISTMEM_CHECK_PTR_FROM_CACHE(var_NT, pos, ptr)                               \
    ((ptr                                                                                \
      == SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (var_NT), SAC_NT_CBASETYPE (var_NT),    \
                                   SAC_ND_A_FIRST_ELEMS (var_NT), pos))                  \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Pointer to element %d of array %s from cache (%p) "         \
                            "does not match calculated pointer (%p). "                   \
                            "Offset: %" PRIuPTR ", first elems: %zd. "                   \
                            "Cache at %p: %d elements from %d.",                         \
                            pos, NT_STR (var_NT), ptr,                                   \
                            SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (var_NT),            \
                                                      SAC_NT_CBASETYPE (var_NT),         \
                                                      SAC_ND_A_FIRST_ELEMS (var_NT),     \
                                                      pos),                              \
                            SAC_ND_A_OFFS (var_NT), SAC_ND_A_FIRST_ELEMS (var_NT),       \
                            SAC_ND_A_MIRROR_PTR_CACHE (var_NT),                          \
                            SAC_ND_A_MIRROR_PTR_CACHE_COUNT (var_NT),                    \
                            SAC_ND_A_MIRROR_PTR_CACHE_FROM (var_NT)),                    \
          0)),

#define SAC_DISTMEM_CHECK_PTR_FROM_DESC(var_NT, pos, ptr)                                \
    ((ptr                                                                                \
      == SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (var_NT), SAC_NT_CBASETYPE (var_NT),    \
                                   SAC_ND_A_FIRST_ELEMS (var_NT), pos))                  \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Pointer to element %d of array %s from descriptor (%p) "    \
                            "does not match calculated pointer (%p). "                   \
                            "Offset: %" PRIuPTR ", first elems: %zd. "                   \
                            "Element local to: %d, start of array there: %p.",           \
                            pos, NT_STR (var_NT), ptr,                                   \
                            SAC_DISTMEM_ELEM_POINTER (SAC_ND_A_OFFS (var_NT),            \
                                                      SAC_NT_CBASETYPE (var_NT),         \
                                                      SAC_ND_A_FIRST_ELEMS (var_NT),     \
                                                      pos),                              \
                            SAC_ND_A_OFFS (var_NT), SAC_ND_A_FIRST_ELEMS (var_NT),       \
                            ((unsigned)pos / (unsigned)SAC_ND_A_FIRST_ELEMS (var_NT)),   \
                            SAC_ND_A_DESC_PTR (var_NT,                                   \
                                               ((unsigned)pos                            \
                                                / (unsigned)SAC_ND_A_FIRST_ELEMS (       \
                                                    var_NT)))),                          \
          0)),

#define SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED()                                         \
    if (!SAC_DISTMEM_are_dsm_allocs_allowed) {                                           \
        SAC_RuntimeError ("Allocations in the DSM segment are "                          \
                          "not allowed at this point.");                                 \
    }

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC_ALLOWED()                 \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_dist) {                           \
        SAC_RuntimeError (                                                               \
          "Cannot switch from distributed to side effects outer execution mode.");       \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects_outer) {      \
        SAC_RuntimeError ("Already in side effects outer execution mode.");              \
    }

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_EXEC_ALLOWED()                       \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_dist) {                           \
        SAC_RuntimeError (                                                               \
          "Cannot switch from distributed to side effects execution mode.");             \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects) {            \
        SAC_RuntimeError ("Already in side effects execution mode.");                    \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_sync) {                    \
        SAC_RuntimeError (                                                               \
          "Cannot switch from synchronous to side effects execution mode.");             \
    }

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_DIST_EXEC_ALLOWED()                               \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_dist) {                           \
        SAC_RuntimeError ("Already in distributed execution mode.");                     \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects) {            \
        SAC_RuntimeError (                                                               \
          "Cannot switch from side effects to distributed execution mode.");             \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects_outer) {      \
        SAC_RuntimeError (                                                               \
          "Cannot switch from side effects outer to distributed execution mode.");       \
    }

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SYNC_EXEC_ALLOWED()                               \
    if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_sync) {                           \
        SAC_RuntimeError ("Already in synchronous execution mode.");                     \
    } else if (SAC_DISTMEM_exec_mode == SAC_DISTMEM_exec_mode_side_effects) {            \
        SAC_RuntimeError (                                                               \
          "Cannot switch from side effects to synchronous execution mode.");             \
    }

#define _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_LOCAL_WRITE(ptr)                                 \
    ((SAC_DISTMEM_are_dist_writes_allowed && SAC_DISTMEM_IS_VALID_WRITE_PTR (ptr))       \
       ? TRUE                                                                            \
       : FALSE)

#define _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_CACHE_WRITE(ptr)                                 \
    ((SAC_DISTMEM_are_cache_writes_allowed && SAC_DISTMEM_IS_VALID_CACHE_PTR (ptr))      \
       ? TRUE                                                                            \
       : FALSE)

#define SAC_DISTMEM_CHECK_WRITE_ALLOWED(ptr, var_NT, pos)                                \
    ((!SAC_ND_A_IS_DIST (var_NT) || _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_LOCAL_WRITE (ptr)    \
      || _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_CACHE_WRITE (ptr))                              \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Illegal write access to distributed array %s"               \
                            ", index position %d at %p\n"                                \
                            "(legal pointer? %d, writes allowed? %d, "                   \
                            "legal cache pointer? %d cache writes allowed? %d)",         \
                            NT_STR (var_NT), pos, ptr,                                   \
                            SAC_DISTMEM_IS_VALID_WRITE_PTR (ptr),                        \
                            SAC_DISTMEM_are_dist_writes_allowed,                         \
                            SAC_DISTMEM_IS_VALID_CACHE_PTR (ptr),                        \
                            SAC_DISTMEM_are_cache_writes_allowed),                       \
          0)),

#define SAC_DISTMEM_CHECK_READ_ALLOWED(ptr, var_NT, pos)                                 \
    ((!SAC_ND_A_IS_DIST (var_NT) || SAC_DISTMEM_IS_VALID_READ_PTR (ptr))                 \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Illegal read access to distributed array %s"                \
                            ", index position %d at %p",                                 \
                            NT_STR (var_NT), pos, ptr),                                  \
          0)),

#define SAC_DISTMEM_CHECK_LOCAL_READ_ALLOWED(ptr, var_NT, pos)                           \
    ((!SAC_ND_A_IS_DIST (var_NT)                                                         \
      || (SAC_DISTMEM_IS_VALID_LOCAL_READ_IDX (var_NT, pos)                              \
          && SAC_DISTMEM_IS_VALID_LOCAL_READ_PTR (ptr)))                                 \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Illegal local read access to distributed array %s"          \
                            ", index position %d at %p",                                 \
                            NT_STR (var_NT), pos, ptr),                                  \
          0)),

#define SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ(ptr)                                    \
    if (!SAC_DISTMEM_IS_VALID_READ_PTR (ptr)) {                                          \
        SAC_RuntimeError ("Illegal read access to distributed array at %p", ptr);        \
    }

/* This macro checks that a array is non-distributed. */
#define SAC_DISTMEM_CHECK_IS_NON_DIST_ARR(var_NT)                                        \
    (SAC_ND_A_IS_DIST (var_NT)                                                           \
       ? (SAC_RuntimeError ("Array %s is marked as distributed but treated "             \
                            "as non-distributed.",                                       \
                            NT_STR (var_NT)),                                            \
          0)                                                                             \
       : ((!SAC_DISTMEM_IS_NON_DIST_PTR (SAC_ND_A_FIELD (var_NT)))                       \
            ? (SAC_RuntimeError ("Array %s points to DSM memory at %p but is "           \
                                 "treated as non-distributed.",                          \
                                 NT_STR (var_NT), SAC_ND_A_FIELD (var_NT)),              \
               0)                                                                        \
            : 0)),

/* This macro checks that a array is non-distributed but allocated in DSM memory iff it is
 * marked as such. */
#define SAC_DISTMEM_CHECK_IS_DSM_ARR(var_NT)                                             \
    ((SAC_ND_A_IS_DIST (var_NT)                                                          \
      && SAC_DISTMEM_IS_NON_DIST_PTR (SAC_ND_A_FIELD (var_NT)))                          \
       ? (SAC_RuntimeError (                                                             \
            "Array %s is marked as allocated in DSM memory but allocated "               \
            "in normal memory at %p.",                                                   \
            NT_STR (var_NT), SAC_ND_A_FIELD (var_NT)),                                   \
          0)                                                                             \
       : ((!SAC_ND_A_IS_DIST (var_NT)                                                    \
           && !SAC_DISTMEM_IS_NON_DIST_PTR (SAC_ND_A_FIELD (var_NT)))                    \
            ? (SAC_RuntimeError (                                                        \
                 "Array %s is marked as allocated in normal memory but is "              \
                 "allocated in DSM memory at %p.",                                       \
                 NT_STR (var_NT), SAC_ND_A_FIELD (var_NT)),                              \
               0)                                                                        \
            : 0)),

#else /* SAC_DO_CHECK_DISTMEM */

#define SAC_DISTMEM_CHECK_PTR_FROM_CACHE(var_NT, pos, ptr)

#define SAC_DISTMEM_CHECK_PTR_FROM_DESC(var_NT, pos, ptr)

#define SAC_DISTMEM_CHECK_IS_DSM_ALLOC_ALLOWED()

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_OUTER_EXEC_ALLOWED()

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SIDE_EFFECTS_EXEC_ALLOWED()

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_DIST_EXEC_ALLOWED()

#define SAC_DISTMEM_CHECK_IS_SWITCH_TO_SYNC_EXEC_ALLOWED()

#define SAC_DISTMEM_CHECK_WRITE_ALLOWED(ptr, var_NT, pos)

#define SAC_DISTMEM_CHECK_READ_ALLOWED(ptr, var_NT, pos)

#define SAC_DISTMEM_CHECK_LOCAL_READ_ALLOWED(ptr, var_NT, pos)

#define SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ(ptr)

#define SAC_DISTMEM_CHECK_IS_NON_DIST_ARR(var_NT)

#define SAC_DISTMEM_CHECK_IS_DSM_ARR(var_NT)

#endif /* SAC_DO_CHECK_DISTMEM */

/******************************************************************************
 *
 * ICMs for GPU runtime checks
 * ========================
 *
 ******************************************************************************/
#if SAC_DO_CHECK_GPU

#define SAC_CUDA_GET_LAST_ERROR_COND(ERROR_MSG, cu_status)                               \
    do {                                                                                 \
        cudaError_t error = cudaGetLastError ();                                         \
        if (error == cudaSuccess) {                                                      \
            SAC_NOOP ();                                                                 \
        } else if (error != cu_status) {                                                 \
            SAC_RuntimeError ("%s::%s(%d) [%s ERROR]: %s\n", __FILE__, __func__,         \
                              __LINE__, ERROR_MSG, cudaGetErrorString (error));          \
        }                                                                                \
    } while (0)

#define SAC_CUDA_GET_LAST_ERROR(ERROR_MSG)                                               \
    do {                                                                                 \
        cudaError_t error = cudaGetLastError ();                                         \
        if (error != cudaSuccess) {                                                      \
            SAC_RuntimeError ("%s::%s(%d) [%s ERROR]: %s\n", __FILE__, __func__,         \
                              __LINE__, ERROR_MSG, cudaGetErrorString (error));          \
        }                                                                                \
    } while (0)

#define SAC_CUDA_GET_LAST_KERNEL_ERROR()                                                 \
    cudaDeviceSynchronize ();                                                            \
    SAC_CUDA_GET_LAST_ERROR ("GPU KERNEL")

#define SAC_GET_CUDA_MEM_TRANSFER_ERROR() SAC_CUDA_GET_LAST_ERROR ("GPU MEMORY TRANSFER")

#define SAC_GET_CUDA_MALLOC_ERROR() SAC_CUDA_GET_LAST_ERROR ("GPU MALLOC")

#define SAC_GET_CUDA_FREE_ERROR() SAC_CUDA_GET_LAST_ERROR ("GPU FREE")

#define SAC_GET_CUDA_PIN_ERROR() SAC_CUDA_GET_LAST_ERROR ("GPU PINNING")

#define SAC_GET_CUDA_UNPIN_ERROR() SAC_CUDA_GET_LAST_ERROR ("GPU UNPINNING")

#define SAC_PRAGMA_GRID_CHECK(max_x, max_y, max_z, max_total)                                               \
    if (grid.x > max_x || grid.y > max_y || grid.z > max_z)                                                 \
        SAC_RuntimeError("CUDA XYZ grid dimension of %u x %u x %u exceeds "                                 \
                         "the compute capability's max value of %u x %u x %u",                              \
                         grid.x, grid.y, grid.z, max_x, max_y, max_z);

#define SAC_PRAGMA_BLOCK_CHECK(max_x, max_y, max_z, max_total)                                              \
    if (block.x > max_x || block.y > max_y || block.z > max_z)                                              \
        SAC_RuntimeError("CUDA XYZ block dimension of %u x %u x %u exceeds "                                \
                         "the compute capability's max value of %u x %u x %u",                              \
                         block.x, block.y, block.z, max_x, max_y, max_z);                                   \
    if (block.x * block.y * block.z > max_total)                                                            \
        SAC_RuntimeError("CUDA XYZ block dimension of %u x %u x %u = %u exceeds compute capability's "      \
                         "max number of threads per block: %u",                                             \
                         block.x, block.y, block.z, block.x * block.y * block.z, max_total);

#else

#define SAC_CUDA_GET_LAST_ERROR_COND(ERROR_MSG, cu_status)
#define SAC_CUDA_GET_LAST_ERROR(ERROR_MSG)
#define SAC_CUDA_GET_LAST_KERNEL_ERROR()
#define SAC_GET_CUDA_MEM_TRANSFER_ERROR()
#define SAC_GET_CUDA_MALLOC_ERROR()
#define SAC_GET_CUDA_FREE_ERROR()
#define SAC_GET_CUDA_PIN_ERROR()
#define SAC_GET_CUDA_UNPIN_ERROR()
#define SAC_PRAGMA_GRID_CHECK(max_x, max_y, max_z, max_total)
#define SAC_PRAGMA_BLOCK_CHECK(max_x, max_y, max_z, max_total)

#endif /* SAC_DO_CHECK_GPU */

#if SAC_DO_TRACE_GPU
#define SAC_PRAGMA_BITMASK_CHECK(krnl, is, ig, val, bitmask, flat, fmt, ...)                                \
    val = bitmask[flat];                                                                                    \
    fprintf(stderr, "%u ", val);
#define SAC_PRAGMA_BITMASK_CHECK_NL fprintf(stderr, "\n");
#else
#define SAC_PRAGMA_BITMASK_CHECK(krnl, is, ig, val, bitmask, flat, fmt, ...)                                \
    val = bitmask[flat];                                                                                    \
    if (is && ig) {                                                                                         \
        if (val == 0)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") inside grid was not hit inside kernel %u!",                   \
                             __VA_ARGS__, krnl);                                                            \
        if (val >= 2)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") inside grid was hit %u times inside kernel %u!",              \
                             __VA_ARGS__, val, krnl);                                                       \
    }                                                                                                       \
    else if (is) {                                                                                          \
        if (val == 1)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") outside the grid was hit inside kernel %u!",                  \
                             __VA_ARGS__, krnl);                                                            \
        if (val >= 2)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") outside the grid was hit %u times inside kernel %u!",         \
                             __VA_ARGS__, val, krnl);                                                       \
    }                                                                                                       \
    else {                                                                                                  \
        if (val == 1)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") below lowerbound was hit inside kernel %u!",                  \
                             __VA_ARGS__, krnl);                                                            \
        if (val >= 2)                                                                                       \
            SAC_RuntimeError("Index (" fmt ") below lowerbound was hit %u times inside kernel %u!",         \
                             __VA_ARGS__, val, krnl);                                                       \
    }
#define SAC_PRAGMA_BITMASK_CHECK_NL
#endif

#define SAC_PRAGMA_BITMASK_UB_CHECK(krnl, val, bitmask, size)                                               \
    val = bitmask[size];                                                                                    \
    if (val >= 1)                                                                                           \
        SAC_RuntimeError("%u indexes above upperbound have been hit inside kernel %u!", val, krnl);

#endif /* _SAC_RUNTIMECHECK_H_ */
