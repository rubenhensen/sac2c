
/*****************************************************************************
 *
 * file:   distmem_icm.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros for the distributed memory backend.
 *
 *****************************************************************************/

#ifndef _SAC_DISTMEM_ICM_H_
#define _SAC_DISTMEM_ICM_H_

/*
 * SAC_DISTMEM_MIRROR_IS_DIST implementations (referenced by distmem_icm_gen.h)
 */

#define SAC_DISTMEM_MIRROR_IS_DIST__SCL(var_NT) FALSE

#define SAC_DISTMEM_MIRROR_IS_DIST__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dm_isDist)

/*
 * SAC_DISTMEM_MIRROR_OFFS implementations (referenced by distmem_icm_gen.h)
 */

#define SAC_DISTMEM_MIRROR_OFFS__DEFAULT(var_NT) CAT12 (NT_NAME (var_NT), __dm_offs)

/*
 * SAC_DISTMEM_MIRROR_FIRST_ELEMS implementations (referenced by distmem_icm_gen.h)
 */

#define SAC_DISTMEM_MIRROR_FIRST_ELEMS__DEFAULT(var_NT)                                  \
    CAT12 (NT_NAME (var_NT), __dm_firstElems)

/*
 * SAC_DISTMEM_ALLOC__DATA implementations (referenced by distmem_icm_gen.h)
 */
#if 0
#define SAC_DISTMEM_ALLOC__DATA__AKS(var_NT)                                             \
    {                                                                                    \
        SAC_HM_MALLOC_FIXED_SIZE (SAC_ND_A_FIELD (var_NT),                               \
                                  SAC_ND_A_SIZE (var_NT)                                 \
                                    * sizeof (*SAC_ND_A_FIELD (var_NT)),                 \
                                  void)                                                  \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }
#endif

#define SAC_DISTMEM_MALLOC_FIXED_SIZE_WITH_DESC(var, var_desc, var_offs, size, dim,      \
                                                basetype, descbasetype)                  \
    {                                                                                    \
        var = (basetype *)SAC_DISTMEM_MALLOC (size, &var_offs);                          \
        SAC_HM_MALLOC_FIXED_SIZE (var_desc, BYTE_SIZE_OF_DESC (dim), descbasetype)       \
    }

/*
 * SAC_DISTMEM_ALLOC_DESC_AND_DATA implementations (referenced by distmem_icm_gen.h)
 */
#define SAC_DISTMEM_ALLOC__DESC_AND_DATA__AKS(var_NT, dim, basetype)                     \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_DISTMEM_MALLOC_FIXED_SIZE_WITH_DESC (SAC_ND_A_FIELD (var_NT),                \
                                                 SAC_ND_A_DESC (var_NT),                 \
                                                 SAC_DISTMEM_MIRROR_OFFS (var_NT),       \
                                                 SAC_DISTMEM_MIRROR_FIRST_ELEMS (var_NT) \
                                                   * sizeof (*SAC_ND_A_FIELD (var_NT)),  \
                                                 SAC_ND_A_MIRROR_DIM (var_NT), basetype, \
                                                 SAC_ND_DESC_BASETYPE (var_NT))          \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_TR_MEM_PRINT (("DISTMEM_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT),      \
                           SAC_ND_A_FIELD (var_NT)))                                     \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

/*
 * SAC_DISTMEM_ALLOC_BEGIN implementations (referenced by distmem_icm_gen.h)
 */

#define SAC_DISTMEM_ALLOC_BEGIN__DAO(var_NT, rc, dim, basetype)                          \
    {                                                                                    \
        if (SAC_DISTMEM_MIRROR_IS_DIST (var_NT)) {                                       \
            SAC_TR_DISTMEM_PRINT ("Allocating dsm memory for array: %s",                 \
                                  NT_STR (var_NT));                                      \
                                                                                         \
            SAC_DISTMEM_MIRROR_FIRST_ELEMS (var_NT)                                      \
              = SAC_DISTMEM_DET_MAX_ELEMS_PER_NODE (SAC_ND_A_MIRROR_SIZE (var_NT),       \
                                                    SAC_ND_A_MIRROR_SHAPE (var_NT, 0));  \
                                                                                         \
            SAC_DISTMEM_ALLOC__DESC_AND_DATA (var_NT, dim, basetype)                     \
        } else {                                                                         \
            SAC_ND_ALLOC__DESC_AND_DATA (var_NT, dim, basetype)                          \
        }                                                                                \
        SAC_ND_INIT__RC (var_NT, rc)

/*
 * SAC_ND_FREE__DATA implementations (referenced by distmem_icm_gen.h)
 * Does not actually free memory yet.
 */

#define SAC_DISTMEM_FREE__DATA__AKS_NHD(var_NT, freefun)                                 \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_DISTMEM_FREE(var_NT, freefun)                                                \
    {                                                                                    \
        SAC_DISTMEM_FREE__DATA (var_NT, freefun)                                         \
        SAC_ND_FREE__DESC (var_NT)                                                       \
    }

#define SAC_DISTMEM_DEC_RC_FREE__C99(var_NT, rc, freefun)                                \
    {                                                                                    \
        SAC_TR_REF_PRINT (                                                               \
          ("ND_DEC_RC_FREE( %s, %d, %s)", NT_STR (var_NT), rc, #freefun))                \
        if ((SAC_ND_A_RC__C99 (var_NT) -= rc) == 0) {                                    \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
            SAC_DISTMEM_FREE (var_NT, freefun)                                           \
        } else {                                                                         \
            SAC_TR_REF_PRINT_RC (var_NT)                                                 \
        }                                                                                \
    }

#define SAC_DISTMEM_DEC_RC_FREE__DEFAULT(var_NT, rc, freefun)                            \
    if (SAC_DISTMEM_MIRROR_IS_DIST (var_NT) == TRUE) {                                   \
        SAC_DISTMEM_DEC_RC_FREE__C99 (var_NT, rc, freefun)                               \
    } else {                                                                             \
        SAC_ND_DEC_RC_FREE__C99 (var_NT, rc, freefun)                                    \
    }

#endif /* defined(_SAC_DISTMEM_ICM_H_) */
