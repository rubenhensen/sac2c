/*
 *
 * $Log$
 * Revision 1.44  1997/11/02 14:30:05  dkr
 * does not include memory.h anymore (redundant)
 *
 * Revision 1.43  1997/10/28 12:35:09  srs
 * renamed macro MALLOC to RT_MALLOC (runtime malloc)
 *
 * Revision 1.42  1997/10/10 13:42:19  srs
 * counter for mem allocation
 *
 * Revision 1.41  1997/10/09 14:00:07  srs
 * new ICMs for prfs min and max
 *
 * Revision 1.40  1997/09/14 13:19:33  dkr
 * eliminated an error in comments
 *
 * Revision 1.39  1997/08/29 12:37:12  sbs
 * output of prf's if prf-trace is on for
 * BINOP, PSI and MODARRAY inserted;
 * trace output for DEC_RC_FREE changed; now the decremented RC
 * will be printed!
 *
 * Revision 1.38  1997/05/29 13:41:44  sbs
 * ND_IDX_MODARRAY... added
 *
 * Revision 1.37  1997/05/02  09:33:06  cg
 * New ICMs ND_DECL_ARRAY and ND_KD_DECL_ARRAY for arrays with unknown
 * shape and dimension as well as arrays with known dimension but unknown shape.
 *
 * Revision 1.36  1997/04/24  10:06:45  cg
 * non-icm macros moved from icm2c.h to libsac.h
 *
 * Revision 1.35  1996/02/29  14:51:24  sbs
 * implementation for ND_IDX_PSI_A debugged;
 * accepts now constants as source parameter as well
 *
 * Revision 1.34  1996/02/21  15:10:05  cg
 * typedefs and defines moved to libsac.h
 *
 * Revision 1.33  1996/02/05  09:21:48  sbs
 * RuntimError => Runtime_Error
 *
 * Revision 1.32  1996/01/25  15:03:22  cg
 * renamed some icm macros
 * fixed bugs in trace output and extended it to hidden values
 *
 * Revision 1.31  1996/01/21  18:06:27  cg
 * bug fixed in PRINTREF
 *
 * Revision 1.30  1996/01/21  14:52:31  cg
 * bug fixed in creating arrays of type void*
 *
 * Revision 1.29  1996/01/21  14:11:49  cg
 * added new icms for refcounting external implicit types
 *
 * Revision 1.28  1995/12/18  16:27:39  cg
 * Some new macros for implicit types,
 * some array macros have new names.
 *
 * Revision 1.27  1995/12/04  16:16:01  hw
 * added ICM ND_2F, ND_2I & ND_2D
 *
 * Revision 1.26  1995/09/06  16:14:11  hw
 * added macro ND_IDX_PSI_A
 * renamed macro ND_IDX_PSI to ND_IDX_PSI_S
 *
 * Revision 1.25  1995/07/13  16:48:44  hw
 * N_icm ND_KS_DEC_IMPORT_IN_ARRAY &  ND_KS_DEC_IMPORT_OUT_ARRAY added
 * ( used in icm2c.c for compilation of declarations of imported
 *   functions)
 *
 * Revision 1.24  1995/07/04  09:26:11  hw
 * macros ND_I2D_A, ND_F2D_A, ND_D2I_A & ND_D2F_A inserted
 *
 * Revision 1.23  1995/06/30  12:19:57  hw
 * macros  ND_F2I_A & ND_I2F_A inserted
 *
 * Revision 1.22  1995/06/26  16:55:57  sbs
 * ND_KS_USE_GENVAR_OFFSET inserted
 *
 * Revision 1.21  1995/06/26  12:02:30  hw
 * macro N_IDX_PSI inserted
 *
 * Revision 1.20  1995/06/09  15:35:48  hw
 * macro OUT_OF_BOUND inserted
 * #include <stdio.h> inserted
 *
 * Revision 1.19  1995/06/08  17:49:28  hw
 *  N_icm ND_TYPEDEF_ARRAY inserted
 *
 * Revision 1.18  1995/05/24  16:56:05  sbs
 * now, the memory for the refcnts is freed as well :->>
 *
 * Revision 1.17  1995/05/24  13:58:31  sbs
 * ND_KS_DECL_ARRAY_ARG inserted
 *
 * Revision 1.16  1995/05/19  13:41:41  hw
 * added macro COMP_NOT_DO_LABEL for compilation with a c-compiler
 *  if -DCOMP_NOT_DO_LABEL will be given as argumnet to a c-compiler
 *   ND_LABEL and ND_GOTO will be expanded to nothing
 *
 * Revision 1.15  1995/05/11  10:27:08  hw
 * - added ND_GOTO & ND_LABEL for compilation of do-loop
 * - address of array  will be printed too, if TRACE_REF is defined
 *
 * Revision 1.14  1995/05/04  11:42:34  sbs
 * TRACE_REF inserted
 *
 * Revision 1.13  1995/04/27  12:57:05  sbs
 * *** empty log message ***
 *
 * Revision 1.12  1995/04/18  14:44:57  sbs
 * ND_ALLOC_ARRAY modified; initial rc parameter
 *
 * Revision 1.11  1995/04/18  14:09:33  sbs
 * args of BINOP_AxS_A exchanged
 *
 * Revision 1.10  1995/04/12  15:14:00  sbs
 * cat & rot inserted.
 *
 * Revision 1.9  1995/04/12  07:02:53  sbs
 * separation of IN and OUT arrays added
 *
 * Revision 1.8  1995/04/07  09:29:39  sbs
 * ND_KS_ASSIGN_ARRAY( name, res) inserted
 *
 * Revision 1.7  1995/04/06  14:23:25  sbs
 * some bugs fixed
 *
 * Revision 1.6  1995/04/05  15:35:39  sbs
 * malloc.h included
 *
 * Revision 1.5  1995/04/03  13:58:57  sbs
 * first "complete" version
 *
 * Revision 1.4  1995/03/31  13:57:34  sbs
 * ND_CREATE_CONST_ARRAY,ND_KS_ARG_ARRAY & ND_KS_RET_ARRAY inserted
 *
 * Revision 1.3  1995/03/10  17:24:37  sbs
 * New macros develloped; psi, take & drop integrated
 *
 * Revision 1.2  1995/03/07  18:19:10  sbs
 * PSI-macros done.
 *
 * Revision 1.1  1995/03/03  14:43:46  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_icm2c_h

#define _sac_icm2c_h

#include <malloc.h>
/*
#include <memory.h>
*/

#include "libsac.h"

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
    free (ND_A_RCP (name));                                                              \
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
    free (ND_A_FIELD (name));                                                            \
    free (ND_A_RCP (name));                                                              \
    PRINT_TRACEHEADER_ALL (("ND_FREE_ARRAY(%s)", #name));                                \
    PRINT_ARRAY_FREE (name);                                                             \
    DEC_ARRAY_MEMCNT (ND_A_SIZE (name));                                                 \
    PRINT_ARRAY_MEM (name);

#define ND_NO_RC_FREE_ARRAY(name)                                                        \
    free (ND_A_FIELD (name));                                                            \
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
        free (ND_A_RCP (old));                                                           \
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
        free (ND_A_RCP (old));                                                           \
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

/*
 * Macros for primitve arithmetic operations:
 * ==========================================
 *
 * ND_BINOP_SxA_A( op, s, a2, res)        : realizes res=(s op a2)
 * ND_BINOP_AxS_A( op, s, a2, res)        : realizes res=(a2 op s)
 * ND_BINOP_AxA_A( op, a1, a2, res)       : realizes res=(a1 op a2)
 *
 */

#define ND_BINOP_AxA_A(op, a1, a2, res)                                                  \
    PRINT_PRF (("ND_BINOP_AxA_A( %s, %s, %s, %s)\n", #op, #a1, #a2, #res));              \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a1)[__i] op ND_A_FIELD (a2)[__i];        \
    };

#define ND_BINOP_AxS_A(op, a2, s, res)                                                   \
    PRINT_PRF (("ND_BINOP_AxS_A( %s, %s, %s, %s)\n", #op, #a2, #s, #res));               \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a2)[__i] op s;                           \
    };

#define ND_BINOP_SxA_A(op, s, a2, res)                                                   \
    PRINT_PRF (("ND_BINOP_SxA_A( %s, %s, %s, %s)\n", #op, #s, #a2, #res));               \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = s op ND_A_FIELD (a2)[__i];                           \
    };

/*
 * Macros used for primitive function idx_psi:
 * ===========================================
 */

#define ND_IDX_PSI_S(s, a, res)                                                          \
    PRINT_PRF (("ND_IDX_PSI_S( %s, %s, %s)\n", #s, #a, #res));                           \
    res = ND_A_FIELD (a)[s];

#define ND_IDX_PSI_A(s, a, res)                                                          \
    PRINT_PRF (("ND_IDX_PSI_A( %s, %s, %s)\n", #s, #a, #res));                           \
    {                                                                                    \
        int __i, __s = s;                                                                \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__s++];                               \
    };

/*
 * Macros used for primitive function idx_modarray:
 * ================================================
 */

#define ND_IDX_MODARRAY_AxVxA_CHECK_REUSE(line, basetype, res, a, s, val)                \
    PRINT_PRF (("ND_IDX_MODARRAY_AxVxA_CHECK_REUSE( %s, %s, %s, %s, %s, %s)\n", #line,   \
                #basetype, #res, #a, #s, #val));                                         \
    ND_CHECK_REUSE_ARRAY (a, res)                                                        \
    {                                                                                    \
        int __i;                                                                         \
        ND_ALLOC_ARRAY (basetype, res, 0);                                               \
        for (__i = 0; __i < s; __i++)                                                    \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
        for (i = i + ND_A_SIZE (val); __i < ND_A_SIZE (res); __i++)                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
    }                                                                                    \
    {                                                                                    \
        int __i, __s;                                                                    \
        for (__s = 0, __i = s; __s < ND_A_SIZE (val); __i++, __s++)                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (val)[__s];                               \
    }

#define ND_IDX_MODARRAY_AxVxA(line, basetype, res, a, s, val)                            \
    PRINT_PRF (("ND_IDX_MODARRAY_AxVxA( %s, %s, %s, %s, %s, %s)\n", #line, #basetype,    \
                #res, #a, #s, #val));                                                    \
    {                                                                                    \
        int __i, __s;                                                                    \
        ND_ALLOC_ARRAY (basetype, res, 0);                                               \
        for (__i = 0; __i < s; __i++)                                                    \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
        for (__s = 0; __s < ND_A_SIZE (val); __i++, __s++)                               \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (val)[__s];                               \
        for (; __i < ND_A_SIZE (res); __i++)                                             \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
    }

#define ND_IDX_MODARRAY_AxVxS_CHECK_REUSE(line, basetype, res, a, s, val)                \
    PRINT_PRF (("ND_IDX_MODARRAY_AxVxS_CHECK_REUSE( %s, %s, %s, %s, %s, %s)\n", #line,   \
                #basetype, #res, #a, #s, #val));                                         \
    ND_CHECK_REUSE_ARRAY (a, res)                                                        \
    {                                                                                    \
        int __i;                                                                         \
        ND_ALLOC_ARRAY (basetype, res, 0);                                               \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
    }                                                                                    \
    ND_A_FIELD (res)[s] = val;

#define ND_IDX_MODARRAY_AxVxS(line, basetype, res, a, s, val)                            \
    PRINT_PRF (("ND_IDX_MODARRAY_AxVxS( %s, %s, %s, %s, %s, %s)\n", #line, #basetype,    \
                #res, #a, #s, #val));                                                    \
    {                                                                                    \
        int __i;                                                                         \
        ND_ALLOC_ARRAY (basetype, res, 0);                                               \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[__i];                                 \
    }                                                                                    \
    ND_A_FIELD (res)[s] = val;

#define ND_KS_USE_GENVAR_OFFSET(offsetvar, res) offsetvar = res##__destptr;

/*
 * Macros used for compilation of do-loop:
 * =======================================
 */

#define ND_GOTO(label) goto label;
#define ND_LABEL(label)                                                                  \
    label:

/*
 * Macro for typedefs of arrays:
 * =============================
 */

#define ND_TYPEDEF_ARRAY(basetype, name) typedef basetype name;

/*
 * Macro for primitive function itof & ftoi:
 * ===================================
 */

#define ND_F2I_A(a1, res)                                                                \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a1)[__i];                                \
    }

#define ND_I2F_A(a1, res) ND_F2I_A (a1, res)
#define ND_I2D_A(a1, res) ND_F2I_A (a1, res)
#define ND_F2D_A(a1, res) ND_F2I_A (a1, res)
#define ND_D2I_A(a1, res) ND_F2I_A (a1, res)
#define ND_D2F_A(a1, res) ND_F2I_A (a1, res)

/*
 * Macro for primitive function itof & ftoi:
 * =========================================
 */

#define ND_2I_A(a1, res)                                                                 \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a1)[__i];                                \
    }

#define ND_2F_A(a1, res) ND_2I_A (a1, res)

#define ND_2D_A(a1, res) ND_2I_A (a1, res)

/*
 * Macros for primitive functions min and max
 * ==========================================
 */

#define ND_MIN(a1, a2) a1 < a2 ? a1 : a2
#define ND_MAX(a1, a2) a1 > a2 ? a1 : a2

/*
 * and now some macros that don't belong to N_icm
 * ==============================================
 */

#define OUT_OF_BOUND(line, prf, size, idx)                                               \
    {                                                                                    \
        __SAC__Runtime_Error ("%d: access in function %s is out"                         \
                              " of range (size: %d, index:%d)",                          \
                              line, prf, size, idx);                                     \
    }

#endif /* _sac_icm2c_h */
