/*****************************************************************************
 *
 * file:   nested.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h
 *
 *   It is the major header file of the implementation of the nested arrays
 *
 *****************************************************************************/

#ifndef _SAC_NESTED_H_
#define _SAC_NESTED_H_

#define SAC_ND_DECL_NESTED__DATA__SCL(var_NT, basetype, decoration)                      \
    SAC_ND_DECL_NESTED__DATA__SCL_##decoration (var_NT, basetype, decoration)

#define SAC_ND_DECL_NESTED__DATA__SCL_(var_NT, basetype, decoration)                     \
    decoration void *SAC_ND_A_FIELD (var_NT)[2] = {NULL, NULL};

#define SAC_ND_DECL_NESTED__DATA__DEFAULT(var_NT, basetype, decoration)                  \
    SAC_ND_DECL_NESTED__DATA__DEFAULT_##decoration (var_NT, basetype, decoration)

#define SAC_ND_DECL_NESTED__DATA__DEFAULT_(var_NT, basetype, decoration)                 \
    decoration void **SAC_ND_A_FIELD (var_NT) = NULL;

#define SAC_ND_PARAM_NESTED_out__NODESC(var_NT, basetype)                                \
    void *(*SAC_NAMEP (SAC_ND_A_FIELD (var_NT)))[2]

#define SAC_ND_PARAM_NESTED_in__NODESC(var_NT, basetype) void *SAC_ND_A_FIELD (var_NT)[2]

#define SAC_ND_RET_NESTED_out__NODESC(retvar_NT, var_NT)                                 \
    {                                                                                    \
        (*SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT))))[0]    \
          = SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT))[0];                          \
        (*SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT))))[1]    \
          = SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT))[1];                          \
    }

#define SAC_ND_RET_NESTED_out__DESC(retvar_NT, var_NT)                                   \
    {                                                                                    \
        (*SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_FIELD (retvar_NT))))       \
          = SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT));                             \
        *SAC_ND_GETVAR_INOUT (retvar_NT, SAC_NAMEP (SAC_ND_A_DESC_NAME (retvar_NT)))     \
          = SAC_ND_A_DESC (var_NT);                                                      \
    }

#define SAC_ND_A_DESC_NESTED__SCL(var_NT) SAC_ND_A_FIELD (var_NT)[0]

#define SAC_ND_A_DATA_NESTED__SCL(var_NT) SAC_ND_A_FIELD (var_NT)[1]

#define SAC_ND_ALLOC__DESC_AND_DATA_NESTED__AKS(var_NT, dim, basetype)                   \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_HM_MALLOC_FIXED_SIZE_WITH_DESC (SAC_ND_A_FIELD (var_NT),                     \
                                            SAC_ND_A_DESC (var_NT),                      \
                                            SAC_ND_A_MIRROR_SIZE (var_NT) * 2            \
                                              * sizeof (*SAC_ND_A_FIELD (var_NT)),       \
                                            SAC_ND_A_MIRROR_DIM (var_NT), void *,        \
                                            SAC_ND_DESC_BASETYPE (var_NT))               \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#define SAC_ND_WRITE_COPY__HNS(to_NT, to_pos, expr, copyfun)                             \
    {                                                                                    \
        SAC_ND_A_FIELD (to_NT)[to_pos * 2] = expr[0];                                    \
        SAC_ND_A_FIELD (to_NT)[to_pos * 2 + 1] = expr[1];                                \
    }

/*
 * Probably not going to work since we need a var name etc.
 */
#define SAC_ND_FREE__DATA__SCL_HNS(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_ND_DEC_RC (var_NT, 1)                                                        \
    }

#define SAC_ND_PARAM_NESTED_in__DESC(var_NT, basetype)                                   \
    void **SAC_ND_A_FIELD (var_NT), void *SAC_ND_A_DESC_NAME (var_NT)

#define SAC_ND_PARAM_NESTED_out__DESC(var_NT, basetype)                                  \
    void ***SAC_NAMEP (SAC_ND_A_FIELD (var_NT)),                                         \
      void **SAC_NAMEP (SAC_ND_A_DESC_NAME (var_NT))

#define SAC_ND_ARG_NESTED_out__DESC(var_NT, basetype)                                    \
    &SAC_ND_A_FIELD (var_NT), (void **)&SAC_ND_A_DESC_NAME (var_NT)

#define SAC_ND_WRITE_READ_COPY__NESTED(to_NT, to_pos, from_NT, from_pos, copyfun)        \
    {                                                                                    \
        SAC_ND_A_FIELD (to_NT)[to_pos * 2] = SAC_ND_A_FIELD (from_NT)[from_pos * 2];     \
        SAC_ND_A_FIELD (to_NT)                                                           \
        [to_pos * 2 + 1] = SAC_ND_A_FIELD (from_NT)[from_pos * 2 + 1];                   \
    }

#define SAC_ND_FREE__DATA__AKS_HNS(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_HM_FREE_FIXED_SIZE (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)),         \
                                SAC_ND_A_SIZE (var_NT) * 1                               \
                                  * sizeof (*SAC_ND_A_FIELD (var_NT)))                   \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_ND_FREE__DATA__AKD_HNS(var_NT, freefun)                                      \
    {                                                                                    \
        SAC_TR_MEM_PRINT (("ND_FREE__DATA( %s, %s) at addr: %p", NT_STR (var_NT),        \
                           #freefun, SAC_ND_A_FIELD (var_NT)))                           \
        SAC_HM_FREE (SAC_ND_GETVAR (var_NT, SAC_ND_A_FIELD (var_NT)))                    \
        SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_UNREGISTER_ARRAY (var_NT)                                                 \
    }

#define SAC_ND_ALLOC_END__NO_DAO__NESTED(var_NT, rc, dim, basetype)                      \
    SAC_ND_ALLOC__DATA_BASETYPE__NESTED (var_NT, basetype)                               \
    }

#define SAC_ND_ALLOC__DATA_BASETYPE__NESTED__AKD_AUD(var_NT, basetype)                   \
    {                                                                                    \
        SAC_HM_MALLOC (SAC_ND_A_FIELD (var_NT),                                          \
                       SAC_ND_A_SIZE (var_NT) * 2 * sizeof (*SAC_ND_A_FIELD (var_NT)),   \
                       void *)                                                           \
        SAC_TR_MEM_PRINT (                                                               \
          ("ND_ALLOC__DATA( %s) at addr: %p", NT_STR (var_NT), SAC_ND_A_FIELD (var_NT))) \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (var_NT))                                 \
        SAC_CS_REGISTER_ARRAY (var_NT)                                                   \
    }

#endif /* _SAC_NESTED_H_ */
