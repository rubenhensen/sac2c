/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:40:30  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_icm_rc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *   This file contains all those ICMs that deal with refcounted data
 *   objects in any way
 *
 *****************************************************************************/

#ifndef SAC_ICM_RC_H

#define SAC_ICM_RC_H

/*
 *  README: The difference between 'type' and 'basetype'
 *
 *  'type' and 'basetype' both specify a string containing a type.
 *
 *  'basetype' is used in all icms dedicated to arrays and simply means
 *  the base type of the array, e.g. 'int'.
 *
 *  'type' is used in all icms not specific to arrays and must
 *  specify the actual type, e.g. 'int *' for an integer array,
 *  'int' for an integer, 'void*' for a hidden, etc.
 *
 */

/*
 * Macros for array access:
 * ========================
 *
 * ND_A_FIELD : for accessing elements of the array
 * ND_A_SIZE  : accesses the size of the unrolling (in elements)
 * ND_A_DIM   : accesses the dimension of the array
 * ND_A_SHAPE : accesses one shape component of an array
 *
 */

#define ND_A_FIELD(name) name
#define ND_A_SIZE(name) __##name##_sz
#define ND_A_DIM(name) __##name##_d
#define ND_KD_A_SHAPE(name, dim) __##name##_s##dim
#define ND_A_SHAPE(name, dim) __##name##_s[dim]

/*
 * Macros for refcount access:
 * ===========================
 *
 * ND_A_RC(name)   : accesses the refcnt
 * ND_A_RCP(name)  : accesses the pointer to the refcnt
 *
 */

#define ND_A_RC(name) *__##name##_rc
#define ND_A_RCP(name) __##name##_rc

/*
 * Macros for declaring refcounted data:
 * =====================================
 *
 * ND_KD_DECL_ARRAY(basetype, name, dim)
 *   declares an array with dimension but without shape information
 *
 * ND_DECL_ARRAY(basetype, name)
 *   declares an array without dimension and shape information
 *
 * ND_DECL_RC(type, name)
 *   declares a refcounted variable in general
 *
 */

#define ND_DECL_RC(type, name)                                                           \
    type name;                                                                           \
    int ND_A_RC (name);

#define ND_KD_DECL_ARRAY(basetype, name, dim)                                            \
    ND_DECL_RC (basetype *, name)                                                        \
    int __##name##_sz;                                                                   \
    int __##name##_d = dim;                                                              \
    int *__##name##_s;

#define ND_DECL_ARRAY(basetype, name)                                                    \
    ND_DECL_RC (basetype *, name)                                                        \
    int __##name##_sz;                                                                   \
    int __##name##_d;                                                                    \
    int *__##name##_s;

/*
 * Macros for removing refcounted data :
 * =====================================
 *
 * ND_FREE_HIDDEN(name, freefun)
 *   frees hidden data
 *
 * ND_NO_RC_FREE_HIDDEN(name, freefun)
 *   frees hidden data to whom a refcount is not yet assigned.
 *
 * ND_FREE_ARRAY(name)
 *   removes an array
 *
 * ND_NO_RC_FREE_ARRAY(name)
 *   removes an array to whom a refcount is not yet assigned
 *
 */

#define ND_FREE_HIDDEN(name, freefun)                                                    \
    freefun (name);                                                                      \
    RT_FREE (ND_A_RCP (name));                                                           \
    PRINT_TRACEHEADER_ALL (("ND_FREE_HIDDEN(%s, %s)", #name, #freefun));                 \
    PRINT_HIDDEN_FREE (name);                                                            \
    DEC_HIDDEN_MEMCNT (1);                                                               \
    PRINT_HIDDEN_MEM (name);

#define ND_NO_RC_FREE_HIDDEN(name, freefun)                                              \
    freefun (name);                                                                      \
    PRINT_TRACEHEADER_ALL (("ND_NO_RC_FREE_HIDDEN(%s, %s)", #name, #freefun));           \
    PRINT_HIDDEN_FREE (name);                                                            \
    PRINT_HIDDEN_MEM (name);

#define ND_FREE_ARRAY(name)                                                              \
    RT_FREE (ND_A_FIELD (name));                                                         \
    RT_FREE (ND_A_RCP (name));                                                           \
    PRINT_TRACEHEADER_ALL (("ND_FREE_ARRAY(%s)", #name));                                \
    PRINT_ARRAY_FREE (name);                                                             \
    DEC_ARRAY_MEMCNT (ND_A_SIZE (name));                                                 \
    PRINT_ARRAY_MEM (name);

#define ND_NO_RC_FREE_ARRAY(name)                                                        \
    RT_FREE (ND_A_FIELD (name));                                                         \
    PRINT_TRACEHEADER_ALL (("ND_NO_RC_FREE_ARRAY(%s)", #name));                          \
    PRINT_ARRAY_FREE (name);                                                             \
    DEC_ARRAY_MEMCNT (ND_A_SIZE (name));                                                 \
    PRINT_ARRAY_MEM (name);

/*
 * Macros for assigning refcounted data :
 * ====================================
 *
 * ND_ASSIGN_HIDDEN(old, new)
 *  copies the pointer to a hidden (including refcount)
 *
 * ND_NO_RC_ASSIGN_HIDDEN(old, new)
 *  copies the pointer to a hidden (without refcount)
 *
 * ND_COPY_HIDDEN(old, new, copyfun)
 *  copies hidden data using given function
 *
 * ND_KS_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (including refcount)
 *
 * ND_KS_NO_RC_ASSIGN_ARRAY(name, res)
 *   copies pointer to array field (without refcount)
 *
 * ND_KS_COPY_ARRAY(old, new, basetypesize)
 *   copies the array, doesn't care about refcount
 *
 */

#define ND_ASSIGN_HIDDEN(old, new)                                                       \
    new = old;                                                                           \
    ND_A_RCP (new) = ND_A_RCP (old);

#define ND_NO_RC_ASSIGN_HIDDEN(old, new) new = old;

#define ND_COPY_HIDDEN(old, new, copyfun)                                                \
    new = copyfun (old);                                                                 \
    PRINT_TRACEHEADER_ALL (("ND_COPY_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));        \
    PRINT_REF (old);                                                                     \
    PRINT_HIDDEN_MEM (new);

#define ND_KS_ASSIGN_ARRAY(name, res)                                                    \
    ND_A_RCP (res) = ND_A_RCP (name);                                                    \
    ND_A_FIELD (res) = ND_A_FIELD (name);

#define ND_KS_NO_RC_ASSIGN_ARRAY(name, res) ND_A_FIELD (res) = ND_A_FIELD (name);

#define ND_KS_COPY_ARRAY(old, new, basetypesize)                                         \
    ND_A_FIELD (new) = RT_MALLOC (basetypesize * ND_A_SIZE (old));                       \
    memcpy (ND_A_FIELD (new), ND_A_FIELD (old), basetypesize *ND_A_SIZE (old));          \
    PRINT_TRACEHEADER_ALL (("ND_COPY_ARRAY(%s, %s)", #old, #new));                       \
    PRINT_REF (old);                                                                     \
    INC_ARRAY_MEMCNT (ND_A_SIZE (old));                                                  \
    PRINT_ARRAY_MEM (new);

/*
 * Macros for creating refcounted data:
 * ====================================
 *
 * ND_ALLOC_ARRAY(basetype, name, rc)
 *   allocates memory needed
 *
 * ND_SET_SIZE( name, num)
 *   sets the size of the unrolling in elements
 *
 * ND_SET_DIM( name, num)
 *   sets the dimension of the array
 *
 * ND_SET_SHAPE( name, dim, s)
 *   sets one shape component of an array
 *
 * ND_ALLOC_RC(name)
 *   allocates memory for refcount (no initialization)
 *
 * ND_CREATE_CONST_ARRAY_C( name, str)
 *   creates a constant character array (string)
 *   Also see ND_CREATE_CONST_ARRAY_S for the creation of scalar arrays.
 */

#define ND_ALLOC_RC(name)                                                                \
    ND_A_RCP (name) = (int *)RT_MALLOC (sizeof (int));                                   \
    INC_HIDDEN_MEMCNT (1);

#define ND_ALLOC_ARRAY(basetype, name, rc)                                               \
    {                                                                                    \
        PRINT_TRACEHEADER_ALL (("ND_ALLOC_ARRAY(%s, %s, %d)", #basetype, #name, rc));    \
        ND_A_FIELD (name)                                                                \
          = (basetype *)RT_MALLOC (sizeof (basetype) * ND_A_SIZE (name));                \
        ND_A_RCP (name) = (int *)RT_MALLOC (sizeof (int));                               \
        ND_A_RC (name) = rc;                                                             \
        INC_ARRAY_MEMCNT (ND_A_SIZE (name));                                             \
        PRINT_REF (name);                                                                \
        PRINT_ARRAY_MEM (name);                                                          \
    }

#define ND_SET_SIZE(name, num) ND_A_SIZE (name) = num;

#define ND_SET_DIM(name, num) ND_A_DIM (name) = num;

#define ND_SET_SHAPE(name, dim, s) ND_A_SHAPE (name, dim) = s;

#define ND_CREATE_CONST_ARRAY_C(name, str) _SAC_String2Array (name, str);

/*
 * Macros for reference counting :
 * ===============================
 *
 * ND_SET_RC(name, num)
 *   sets the refcnt
 *
 * ND_INC_RC(name, num)
 *   increments the refcnt
 *
 * ND_DEC_RC(name, num)
 *   decrements the refcnt
 *
 * ND_DEC_RC_FREE_ARRAY(name, num)
 *   decrements the refcnt AND frees the array if refcnt becomes zero!
 *
 * ND_DEC_RC_FREE_HIDDEN(name, num, freefun)
 *   decrements the refcnt AND frees the hidden object if refcnt becomes zero!
 *
 * ND_CHECK_REUSE_ARRAY(old, new)
 *   tries to reuse old array for new
 *
 * ND_CHECK_REUSE_HIDDEN(old, new, copyfun)
 *   tries to reuse old hidden data for new, copies if impossible
 *
 * ND_KS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)
 *   assigns old to new if refcount is zero and copies the array otherwise
 *   A new refcount is allocated if necessary
 *
 * ND_MAKE_UNIQUE_HIDDEN(old, new, copyfun)
 *   assigns old to new if refcount is zero and copies the hidden otherwise
 *   A new refcount is allocated if necessary
 *
 * ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(old, new, basetypesize)
 *   assigns old to new if refcount is zero and copies the array otherwise
 *   No new refcount is allocated, the old one is freed when not copying
 *
 * ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)
 *   assigns old to new if refcount is zero and copies the hidden otherwise
 *   No new refcount is allocated, the old one is freed when not copying
 *
 *
 */

#define ND_SET_RC(name, num)                                                             \
    PRINT_TRACEHEADER_REF (("ND_SET_RC(%s, %d)", #name, num));                           \
    ND_A_RC (name) = num;                                                                \
    PRINT_REF (name);

#define ND_INC_RC(name, num)                                                             \
    PRINT_TRACEHEADER_REF (("ND_INC_RC(%s, %d)", #name, num));                           \
    ND_A_RC (name) += num;                                                               \
    PRINT_REF (name);

#define ND_DEC_RC(name, num)                                                             \
    PRINT_TRACEHEADER_REF (("ND_DEC_RC(%s, %d)", #name, num));                           \
    ND_A_RC (name) -= num;                                                               \
    PRINT_REF (name);

#define ND_DEC_RC_FREE_ARRAY(name, num)                                                  \
    PRINT_TRACEHEADER_REF (("ND_DEC_RC_FREE(%s, %d)", #name, num));                      \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        PRINT_REF (name);                                                                \
        ND_FREE_ARRAY (name);                                                            \
    } else                                                                               \
        PRINT_REF (name);

#define ND_DEC_RC_FREE_HIDDEN(name, num, freefun)                                        \
    PRINT_TRACEHEADER_REF (("ND_DEC_RC_FREE(%s, %d)", #name, num));                      \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        PRINT_REF (name);                                                                \
        ND_FREE_HIDDEN (name, freefun);                                                  \
    } else                                                                               \
        PRINT_REF (name);

#define ND_CHECK_REUSE_ARRAY(old, new)                                                   \
    if (ND_A_RC (old) == 1) {                                                            \
        ND_KS_ASSIGN_ARRAY (old, new);                                                   \
    } else

#if 0
#define ND_CHECK_REUSE_HIDDEN(old, new, copyfun)                                         \
    new = ND_A_RC (old) == 1 ? old : copyfun (old);
#endif

#define ND_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                                         \
    PRINT_TRACEHEADER_REF (("ND_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun)); \
    PRINT_REF (old);                                                                     \
    if (ND_A_RC (old) == 1) {                                                            \
        ND_ASSIGN_HIDDEN (old, new);                                                     \
    } else {                                                                             \
        ND_ALLOC_RC (new);                                                               \
        ND_COPY_HIDDEN (old, new, copyfun);                                              \
        ND_DEC_RC (old, 1);                                                              \
    }

#define ND_KS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                                  \
    PRINT_TRACEHEADER_REF (                                                              \
      ("ND_KS_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));                \
    PRINT_REF (old);                                                                     \
    if ((ND_A_RC (old)) == 1) {                                                          \
        ND_KS_ASSIGN_ARRAY (old, new);                                                   \
    } else {                                                                             \
        ND_KS_COPY_ARRAY (old, new, basetypesize);                                       \
        ND_ALLOC_RC (new);                                                               \
        ND_DEC_RC (old, 1);                                                              \
    }

#define ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                                   \
    PRINT_TRACEHEADER_REF (                                                              \
      ("ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));                \
    PRINT_REF (old);                                                                     \
    if (ND_A_RC (old) == 1) {                                                            \
        ND_NO_RC_ASSIGN_HIDDEN (old, new);                                               \
        RT_FREE (ND_A_RCP (old));                                                        \
    } else {                                                                             \
        ND_COPY_HIDDEN (old, new, copyfun);                                              \
        ND_DEC_RC (old, 1);                                                              \
    }

#define ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                            \
    PRINT_TRACEHEADER_REF (                                                              \
      ("ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));          \
    PRINT_REF (old);                                                                     \
    if ((ND_A_RC (old)) == 1) {                                                          \
        ND_KS_NO_RC_ASSIGN_ARRAY (old, new);                                             \
        RT_FREE (ND_A_RCP (old));                                                        \
    } else {                                                                             \
        ND_KS_COPY_ARRAY (old, new, basetypesize);                                       \
        ND_DEC_RC (old, 1);                                                              \
    }

/*
 * Macros for passing refcounted data to functions :
 * =================================================
 *
 *
 * ND_KS_DEC_IN_RC( type, name)
 *   macro for prototyping refcounted data as "in" parameter
 *
 * ND_KS_DEC_OUT_RC( type, name)
 *   macro for prototyping refcounted data as "out" parameter
 *
 * ND_KS_DEC_INOUT_RC( type, name)
 *   macro for prototyping refcounted data as "inout" parameter
 *
 * ND_KS_DEC_IMPORT_IN_RC( type)
 *   macro for prototyping refcounted data as "in" parameter
 *   (imported functions only )
 *
 * ND_KS_DEC_IMPORT_OUT_RC( type)
 *   macro for prototyping refcounted data as "out" parameter
 *   (imported functions only )
 *
 * ND_KS_DEC_IMPORT_INOUT_RC( type)
 *   macro for prototyping refcounted data as "inout" parameter
 *   (imported functions only )
 *
 * ND_KS_AP_IN_RC( name)
 *   macro for giving refcounted data as argument
 *
 * ND_KS_AP_OUT_RC( name)
 *   macro for getting refcounted data as result
 *
 * ND_KS_AP_INOUT_RC( name)
 *   macro for giving refcounted data as "inout" argument
 *
 * ND_KS_RET_OUT_RC( name)
 *   macro for returning refcounted data
 *
 * ND_KS_RET_INOUT_RC( name)
 *   macro for returning "inout" refcounted data
 *
 */

#define ND_KS_DEC_IN_RC(type, name) type name, int *__##name##_rc

#define ND_KS_DEC_OUT_RC(type, name) type *##name##__p, int **__##name##_rc__p

#define ND_KS_DEC_INOUT_RC(type, name) type *##name##__p, int **__##name##_rc__p

#define ND_KS_DEC_IMPORT_IN_RC(type) type, int *

#define ND_KS_DEC_IMPORT_OUT_RC(type) type *, int **

#define ND_KS_DEC_IMPORT_INOUT_RC(type) type *, int **

#define ND_KS_AP_IN_RC(name) name, __##name##_rc

#define ND_KS_AP_OUT_RC(name) &name, &__##name##_rc

#define ND_KS_AP_INOUT_RC(name) &name, &__##name##_rc

#define ND_KS_RET_OUT_RC(name)                                                           \
    *##name##__p = name;                                                                 \
    *__##name##_rc__p = __##name##_rc

#define ND_KS_RET_INOUT_RC(name)                                                         \
    *##name##__p = name;                                                                 \
    *__##name##_rc__p = __##name##_rc

#define ND_DECL_INOUT_PARAM(type, name) type name = *##name##__p;

#define ND_DECL_INOUT_PARAM_RC(type, name)                                               \
    type name = *##name##__p;                                                            \
    int *__##name##_rc = *__##name##_rc__p;

#endif /* SAC_ICM_RC_H */
