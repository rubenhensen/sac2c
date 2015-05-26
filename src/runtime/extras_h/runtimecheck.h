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
        SAC_RuntimeError message;                                                        \
    }
/* yes, in C '0;' is indeed a legal statement 8-)) */

#define SAC_ASSURE_TYPE_LINE(cond, line, message)                                        \
    if (!(cond)) {                                                                       \
        SAC_RuntimeErrorLine (line, message);                                            \
    }

#else /* SAC_DO_CHECK_TYPE */

#define SAC_ASSURE_TYPE(cond, message) /* nothing */

#define SAC_ASSURE_TYPE_LINE(cond, line, message) /* nothing */

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
 * SAC_DISTMEM_CHECK_WRITE_ALLOWED ( ptr, var_NT, pos)
 * SAC_DISTMEM_CHECK_READ_ALLOWED ( ptr, var_NT, pos)
 *
 ******************************************************************************/

#if SAC_DO_CHECK_DISTMEM

#define _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_LOCAL_WRITE(ptr)                                 \
    ((SAC_DISTMEM_are_dist_writes_allowed && SAC_DISTMEM_IS_VALID_WRITE_PTR (ptr))       \
       ? TRUE                                                                            \
       : FALSE)

#define _SAC_DISTMEM_CHECK_IS_LEGAL_FOR_CACHE_WRITE(ptr)                                 \
    ((SAC_DISTMEM_are_cache_writes_allowed && SAC_DISTMEM_IS_VALID_CACHE_PTR (ptr))      \
       ? TRUE                                                                            \
       : FALSE)

#define SAC_DISTMEM_CHECK_WRITE_ALLOWED(ptr, var_NT, pos)                                \
    ((_SAC_DISTMEM_CHECK_IS_LEGAL_FOR_LOCAL_WRITE (ptr)                                  \
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
    (SAC_DISTMEM_IS_VALID_READ_PTR (ptr)                                                 \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Illegal read access to distributed array %s"                \
                            ", index position %d at %p",                                 \
                            NT_STR (var_NT), pos, ptr),                                  \
          0)),

#define SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ(ptr)                                    \
    (SAC_DISTMEM_IS_VALID_READ_PTR (ptr)                                                 \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Illegal read access to distributed array at %p", ptr), 0));

#else /* SAC_DO_CHECK_DISTMEM */

#define SAC_DISTMEM_CHECK_WRITE_ALLOWED(ptr, var_NT, pos)

#define SAC_DISTMEM_CHECK_READ_ALLOWED(ptr, var_NT, pos)

#define SAC_DISTMEM_CHECK_POINTER_VALID_FOR_READ(ptr)

#endif /* SAC_DO_CHECK_DISTMEM */

#endif /* _SAC_RUNTIMECHECK_H_ */
