/*
 *
 * $Log$
 * Revision 1.2  1998/06/03 14:59:05  cg
 * generation of new identifier names as extensions of old ones
 * by macros made compatible with new renaming scheme
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_std.h
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

#ifndef SAC_STD_H

#define SAC_STD_H

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
 * ICMs for array access:
 * ========================
 *
 * ND_A_FIELD : for accessing elements of the array
 * ND_A_SIZE  : accesses the size of the unrolling (in elements)
 * ND_A_DIM   : accesses the dimension of the array
 * ND_A_SHAPE : accesses one shape component of an array
 *
 */

#define SAC_ND_A_FIELD(name) name
#define SAC_ND_A_SIZE(name) name##__sz
#define SAC_ND_A_DIM(name) name##__d
#define SAC_ND_KD_A_SHAPE(name, dim) name##__s##dim
#define SAC_ND_A_SHAPE(name, dim) name##__s[dim]

/*
 * ICMs for refcount access:
 * ===========================
 *
 * ND_A_RC(name)   : accesses the refcnt
 * ND_A_RCP(name)  : accesses the pointer to the refcnt
 *
 */

#define SAC_ND_A_RC(name) *name##__rc
#define SAC_ND_A_RCP(name) name##__rc

/*
 * ICMs for declaring refcounted data:
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

#define SAC_ND_DECL_RC(type, name)                                                       \
    type name;                                                                           \
    int SAC_ND_A_RC (name);

#define SAC_ND_KD_DECL_ARRAY(basetype, name, dim)                                        \
    SAC_ND_DECL_RC (basetype *, name)                                                    \
    int name##__sz;                                                                      \
    int name##__d = dim;                                                                 \
    int *name##__s;

#define SAC_ND_DECL_ARRAY(basetype, name)                                                \
    SAC_ND_DECL_RC (basetype *, name)                                                    \
    int name##__sz;                                                                      \
    int name##__d;                                                                       \
    int *name##__s;

/*
 * ICMs for removing refcounted data :
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

#define SAC_ND_FREE_HIDDEN(name, freefun)                                                \
    freefun (name);                                                                      \
    SAC_FREE (SAC_ND_A_RCP (name));                                                      \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_FREE_HIDDEN(%s, %s)", #name, #freefun));          \
    SAC_TR_PRINT_HIDDEN_FREE (name);                                                     \
    SAC_TR_DEC_HIDDEN_MEMCNT (1);                                                        \
    SAC_TR_PRINT_HIDDEN_MEM (name);

#define SAC_ND_NO_RC_FREE_HIDDEN(name, freefun)                                          \
    freefun (name);                                                                      \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_NO_RC_FREE_HIDDEN(%s, %s)", #name, #freefun));    \
    SAC_TR_PRINT_HIDDEN_FREE (name);                                                     \
    SAC_TR_PRINT_HIDDEN_MEM (name);

#define SAC_ND_FREE_ARRAY(name)                                                          \
    SAC_FREE (SAC_ND_A_FIELD (name));                                                    \
    SAC_FREE (SAC_ND_A_RCP (name));                                                      \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_FREE_ARRAY(%s)", #name));                         \
    SAC_TR_PRINT_ARRAY_FREE (name);                                                      \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_TR_PRINT_ARRAY_MEM (name);

#define SAC_ND_NO_RC_FREE_ARRAY(name)                                                    \
    SAC_FREE (SAC_ND_A_FIELD (name));                                                    \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_NO_RC_FREE_ARRAY(%s)", #name));                   \
    SAC_TR_PRINT_ARRAY_FREE (name);                                                      \
    SAC_TR_DEC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                      \
    SAC_TR_PRINT_ARRAY_MEM (name);

/*
 * ICMs for assigning refcounted data :
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

#define SAC_ND_ASSIGN_HIDDEN(old, new)                                                   \
    new = old;                                                                           \
    SAC_ND_A_RCP (new) = SAC_ND_A_RCP (old);

#define SAC_ND_NO_RC_ASSIGN_HIDDEN(old, new) new = old;

#define SAC_ND_COPY_HIDDEN(old, new, copyfun)                                            \
    new = copyfun (old);                                                                 \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_COPY_HIDDEN(%s, %s, %s)", #old, #new, #copyfun)); \
    SAC_TR_PRINT_REF (old);                                                              \
    SAC_TR_PRINT_HIDDEN_MEM (new);

#define SAC_ND_KS_ASSIGN_ARRAY(name, res)                                                \
    SAC_ND_A_RCP (res) = SAC_ND_A_RCP (name);                                            \
    SAC_ND_A_FIELD (res) = SAC_ND_A_FIELD (name);

#define SAC_ND_KS_NO_RC_ASSIGN_ARRAY(name, res)                                          \
    SAC_ND_A_FIELD (res) = SAC_ND_A_FIELD (name);

#define SAC_ND_KS_COPY_ARRAY(old, new, basetypesize)                                     \
    SAC_ND_A_FIELD (new) = SAC_MALLOC (basetypesize * SAC_ND_A_SIZE (old));              \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < SAC_ND_A_SIZE (old); __i++)                                  \
            SAC_ND_A_FIELD (new)[__i] = SAC_ND_A_FIELD (old)[__i];                       \
    }                                                                                    \
    SAC_TR_PRINT_TRACEHEADER_ALL (("ND_KS_COPY_ARRAY(%s, %s)", #old, #new));             \
    SAC_TR_PRINT_REF (old);                                                              \
    SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (old));                                       \
    SAC_TR_PRINT_ARRAY_MEM (new);

/*
 * ICMs for creating refcounted data:
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

#define SAC_ND_ALLOC_RC(name)                                                            \
    SAC_ND_A_RCP (name) = (int *)SAC_MALLOC (sizeof (int));                              \
    SAC_TR_INC_HIDDEN_MEMCNT (1);

#define SAC_ND_ALLOC_ARRAY(basetype, name, rc)                                           \
    {                                                                                    \
        SAC_TR_PRINT_TRACEHEADER_ALL (                                                   \
          ("ND_ALLOC_ARRAY(%s, %s, %d)", #basetype, #name, rc));                         \
        SAC_ND_A_FIELD (name)                                                            \
          = (basetype *)SAC_MALLOC (sizeof (basetype) * SAC_ND_A_SIZE (name));           \
        SAC_ND_A_RCP (name) = (int *)SAC_MALLOC (sizeof (int));                          \
        SAC_ND_A_RC (name) = rc;                                                         \
        SAC_TR_INC_ARRAY_MEMCNT (SAC_ND_A_SIZE (name));                                  \
        SAC_TR_PRINT_REF (name);                                                         \
        SAC_TR_PRINT_ARRAY_MEM (name);                                                   \
    }

#define SAC_ND_SET_SIZE(name, num) SAC_ND_A_SIZE (name) = num;

#define SAC_ND_SET_DIM(name, num) SAC_ND_A_DIM (name) = num;

#define SAC_ND_SET_SHAPE(name, dim, s) SAC_ND_A_SHAPE (name, dim) = s;

#define SAC_ND_CREATE_CONST_ARRAY_C(name, str) SAC_String2Array (name, str);

/*
 * ICMs for reference counting :
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

#define SAC_ND_SET_RC(name, num)                                                         \
    SAC_TR_PRINT_TRACEHEADER_REF (("ND_SET_RC(%s, %d)", #name, num));                    \
    SAC_ND_A_RC (name) = num;                                                            \
    SAC_TR_PRINT_REF (name);

#define SAC_ND_INC_RC(name, num)                                                         \
    SAC_TR_PRINT_TRACEHEADER_REF (("ND_INC_RC(%s, %d)", #name, num));                    \
    SAC_ND_A_RC (name) += num;                                                           \
    SAC_TR_PRINT_REF (name);

#define SAC_ND_DEC_RC(name, num)                                                         \
    SAC_TR_PRINT_TRACEHEADER_REF (("ND_DEC_RC(%s, %d)", #name, num));                    \
    SAC_ND_A_RC (name) -= num;                                                           \
    SAC_TR_PRINT_REF (name);

#define SAC_ND_DEC_RC_FREE_ARRAY(name, num)                                              \
    SAC_TR_PRINT_TRACEHEADER_REF (("ND_DEC_RC_FREE(%s, %d)", #name, num));               \
    if ((SAC_ND_A_RC (name) -= num) == 0) {                                              \
        SAC_TR_PRINT_REF (name);                                                         \
        SAC_ND_FREE_ARRAY (name);                                                        \
    } else                                                                               \
        SAC_TR_PRINT_REF (name);

#define SAC_ND_DEC_RC_FREE_HIDDEN(name, num, freefun)                                    \
    SAC_TR_PRINT_TRACEHEADER_REF (("ND_DEC_RC_FREE(%s, %d)", #name, num));               \
    if ((SAC_ND_A_RC (name) -= num) == 0) {                                              \
        SAC_TR_PRINT_REF (name);                                                         \
        SAC_ND_FREE_HIDDEN (name, freefun);                                              \
    } else                                                                               \
        SAC_TR_PRINT_REF (name);

#define SAC_ND_CHECK_REUSE_ARRAY(old, new)                                               \
    if (SAC_ND_A_RC (old) == 1) {                                                        \
        SAC_ND_KS_ASSIGN_ARRAY (old, new);                                               \
    } else

#if 0
#define SAC_ND_CHECK_REUSE_HIDDEN(old, new, copyfun)                                     \
    new = SAC_ND_A_RC (old) == 1 ? old : copyfun (old);
#endif

#define SAC_ND_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                                     \
    SAC_TR_PRINT_TRACEHEADER_REF (                                                       \
      ("ND_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));                      \
    SAC_TR_PRINT_REF (old);                                                              \
    if (SAC_ND_A_RC (old) == 1) {                                                        \
        SAC_ND_ASSIGN_HIDDEN (old, new);                                                 \
    } else {                                                                             \
        SAC_ND_ALLOC_RC (new);                                                           \
        SAC_ND_COPY_HIDDEN (old, new, copyfun);                                          \
        SAC_ND_DEC_RC (old, 1);                                                          \
    }

#define SAC_ND_KS_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                              \
    SAC_TR_PRINT_TRACEHEADER_REF (                                                       \
      ("ND_KS_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));                \
    SAC_TR_PRINT_REF (old);                                                              \
    if ((SAC_ND_A_RC (old)) == 1) {                                                      \
        SAC_ND_KS_ASSIGN_ARRAY (old, new);                                               \
    } else {                                                                             \
        SAC_ND_KS_COPY_ARRAY (old, new, basetypesize);                                   \
        SAC_ND_ALLOC_RC (new);                                                           \
        SAC_ND_DEC_RC (old, 1);                                                          \
    }

#define SAC_ND_NO_RC_MAKE_UNIQUE_HIDDEN(old, new, copyfun)                               \
    SAC_TR_PRINT_TRACEHEADER_REF (                                                       \
      ("ND_NO_RC_MAKE_UNIQUE_HIDDEN(%s, %s, %s)", #old, #new, #copyfun));                \
    SAC_TR_PRINT_REF (old);                                                              \
    if (SAC_ND_A_RC (old) == 1) {                                                        \
        SAC_ND_NO_RC_ASSIGN_HIDDEN (old, new);                                           \
        SAC_FREE (SAC_ND_A_RCP (old));                                                   \
    } else {                                                                             \
        SAC_ND_COPY_HIDDEN (old, new, copyfun);                                          \
        SAC_ND_DEC_RC (old, 1);                                                          \
    }

#define SAC_ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(old, new, basetypesize)                        \
    SAC_TR_PRINT_TRACEHEADER_REF (                                                       \
      ("ND_KS_NO_RC_MAKE_UNIQUE_ARRAY(%s, %s, %d)", #old, #new, basetypesize));          \
    SAC_TR_PRINT_REF (old);                                                              \
    if ((SAC_ND_A_RC (old)) == 1) {                                                      \
        SAC_ND_KS_NO_RC_ASSIGN_ARRAY (old, new);                                         \
        SAC_FREE (SAC_ND_A_RCP (old));                                                   \
    } else {                                                                             \
        SAC_ND_KS_COPY_ARRAY (old, new, basetypesize);                                   \
        SAC_ND_DEC_RC (old, 1);                                                          \
    }

/*
 * ICMs for passing refcounted data to functions :
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

#define SAC_ND_KS_DEC_IN_RC(type, name) type name, int *name##__rc

#define SAC_ND_KS_DEC_OUT_RC(type, name) type *name##__p, int **name##__rc__p

#define SAC_ND_KS_DEC_INOUT_RC(type, name) type *name##__p, int **name##__rc__p

#define SAC_ND_KS_DEC_IMPORT_IN_RC(type) type, int *

#define SAC_ND_KS_DEC_IMPORT_OUT_RC(type) type *, int **

#define SAC_ND_KS_DEC_IMPORT_INOUT_RC(type) type *, int **

#define SAC_ND_KS_AP_IN_RC(name) name, name##__rc

#define SAC_ND_KS_AP_OUT_RC(name) &name, &name##__rc

#define SAC_ND_KS_AP_INOUT_RC(name) &name, &name##__rc

#define SAC_ND_KS_RET_OUT_RC(name)                                                       \
    *##name##__p = name;                                                                 \
    *name##__rc__p = name##__rc

#define SAC_ND_KS_RET_INOUT_RC(name)                                                     \
    *##name##__p = name;                                                                 \
    *name##__rc__p = name##__rc

#define SAC_ND_DECL_INOUT_PARAM(type, name) type name = *##name##__p;

#define SAC_ND_DECL_INOUT_PARAM_RC(type, name)                                           \
    type name = *##name##__p;                                                            \
    int *name##__rc = *name##__rc__p;

#endif /* SAC_STD_H */
