/*
 *
 * $Log$
 * Revision 1.26  1995/09/06 16:14:11  hw
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
 * Macros for declaring arrays:
 * ============================
 *
 * ND_DECL_ARRAY : declares an array
 */

#define ND_DECL_ARRAY(type, name)                                                        \
    type *name;                                                                          \
    int *__##name##_rc;                                                                  \
    int __##name##_sz;                                                                   \
    int __##name##_d;                                                                    \
    int *__##name##_s;

/*
 * Macros for array access:
 * ========================
 *
 * ND_A_FIELD : for accessing elements of the array
 * ND_A_RC    : accesses the refcnt
 * ND_A_RCP   : accesses the pointer to the refcnt
 * ND_A_SIZE  : accesses the size of the unrolling (in elements)
 * ND_A_DIM   : accesses the dimension of the array
 * ND_A_SHAPE : accesses one shape component of an array
 * ND_KS_DEC_IN_ARRAY	: macro for prototyping array as "in" parameters
 * ND_KS_DEC_OUT_ARRAY	: macro for prototyping array as "out" parameters
 * ND_KS_AP_IN_ARRAY	: macro for giving an array as argument
 * ND_KS_AP_OUT_ARRAY   : macro for getting an array as result
 * ND_KS_RET_OUT_ARRAY  : macro for returning an array
 * ND_KS_DEC_IMPORT_IN_ARRAY  : macro for prototyping array as "in" parameters
 *                              (imported functions only )
 * ND_KS_DEC_IMPORT_OUT_ARRAY : macro for prototyping array as "out" parameters
 *                              (imported functions only )
 *
 */

#define ND_A_FIELD(name) name
#define ND_A_RC(name) *__##name##_rc
#define ND_A_RCP(name) __##name##_rc
#define ND_A_SIZE(name) __##name##_sz
#define ND_A_DIM(name) __##name##_d
#define ND_KD_A_SHAPE(name, dim) __##name##_s##dim
#define ND_A_SHAPE(name, dim) __##name##_s[dim]

#define ND_KS_DEC_IN_ARRAY(type, name) type *##name, int *__##name##_rc
#define ND_KS_DEC_OUT_ARRAY(type, name) type **##name##__p, int **__##name##_rc__p
#define ND_KS_DEC_IMPORT_IN_ARRAY(type) type *, int *
#define ND_KS_DEC_IMPORT_OUT_ARRAY(type) type **, int **
#define ND_KS_AP_IN_ARRAY(name) name, __##name##_rc
#define ND_KS_AP_OUT_ARRAY(name) &name, &__##name##_rc
#define ND_KS_RET_OUT_ARRAY(name)                                                        \
    *##name##__p = name;                                                                 \
    *__##name##_rc__p = __##name##_rc

/*
 * Macros for initializing an array:
 * =================================
 *
 * ND_ALLOC_ARRAY : allocates memory needed
 *
 * ND_REUSE       : reuses old for new
 * ND_CHECK_REUSE : tries to reuse old for new
 *
 * ND_SET_RC      : sets the refcnt
 * ND_INC_RC      : increments the refcnt
 * ND_DEC_RC      : decrements the refcnt
 * ND_DEC_RC_FREE : decrements the refcnt
 *                  AND frees the array if refcnt becomes zero!
 * ND_SET_SIZE    : sets the size of the unrolling in elements
 *
 * ND_SET_DIM     : sets the dimension of the array
 *
 * ND_SET_SHAPE        : sets one shape component of an array
 *
 * ND_KS_ASSIGN_ARRAY  : copy pointer(s) for "res=name;"
 *
 */

#define ND_REUSE(old, new)                                                               \
    {                                                                                    \
        new = old;                                                                       \
        __##new##_rc = __##old##_rc;                                                     \
    }
#define ND_CHECK_REUSE(old, new)                                                         \
    if (ND_A_RC (old) == 1)                                                              \
    ND_REUSE (old, new) else

/*----------------------------------------------------------------------------*/
#if (defined(TRACE_MEM) || defined(TRACE_REF))
#define PRINT_TRACE_BUFFER fprintf (stderr, "%-40s -> ", __trace_buffer)
#define PRINT_TRACE_INDENT fprintf (stderr, "%-40s    ", " ")

#endif TRACE_MEM || TRACE_REF
/*----------------------------------------------------------------------------*/
#ifdef TRACE_REF
#define PRINTREF(name) fprintf (stderr, "refcnt of %s: %d\n", #name, ND_A_RC (name))

#define ND_SET_RC(name, num)                                                             \
    sprintf (__trace_buffer, "ND_SET_RC(%s, %d)", #name, num);                           \
    PRINT_TRACE_BUFFER;                                                                  \
    ND_A_RC (name) = num;                                                                \
    PRINTREF (name);
#define ND_INC_RC(name, num)                                                             \
    sprintf (__trace_buffer, "ND_INC_RC(%s, %d)", #name, num);                           \
    PRINT_TRACE_BUFFER;                                                                  \
    ND_A_RC (name) += num;                                                               \
    PRINTREF (name);
#define ND_DEC_RC(name, num)                                                             \
    sprintf (__trace_buffer, "ND_DEC_RC(%s, %d)", #name, num);                           \
    PRINT_TRACE_BUFFER;                                                                  \
    ND_A_RC (name) -= num;                                                               \
    PRINTREF (name);

#else /* TRACE_REF */

#define ND_SET_RC(name, num) ND_A_RC (name) = num;
#define ND_INC_RC(name, num) ND_A_RC (name) += num;
#define ND_DEC_RC(name, num) ND_A_RC (name) -= num;
#endif /* TRACE_REF */

/*----------------------------------------------------------------------------*/
#ifdef TRACE_MEM
#define PRINTMEM(name)                                                                   \
    fprintf (stderr, "adr: %p, size: %d elements\n", ND_A_FIELD (name),                  \
             ND_A_SIZE (name));                                                          \
    PRINT_TRACE_INDENT;                                                                  \
    fprintf (stderr, "total # of elements: %d\n", __trace_mem_cnt)
#endif /* TRACE_MEM */

/*----------------------------------------------------------------------------*/
#if (defined(TRACE_MEM) && !defined(TRACE_REF))

#define ND_ALLOC_ARRAY(type, name, rc)                                                   \
    {                                                                                    \
        sprintf (__trace_buffer, "ND_ALLOC_ARRAY(%s, %s, %d)", #type, #name, rc);        \
        PRINT_TRACE_BUFFER;                                                              \
        ND_A_FIELD (name) = (type *)malloc (sizeof (type) * ND_A_SIZE (name));           \
        ND_A_RCP (name) = (int *)malloc (sizeof (int));                                  \
        ND_A_RC (name) = rc;                                                             \
        __trace_mem_cnt += ND_A_SIZE (name);                                             \
        PRINTMEM (name);                                                                 \
    }
#define ND_DEC_RC_FREE(name, num)                                                        \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        sprintf (__trace_buffer, "ND_DEC_RC_FREE(%s, %d)", #name, num);                  \
        PRINT_TRACE_BUFFER;                                                              \
        free (ND_A_FIELD (name));                                                        \
        free (ND_A_RCP (name));                                                          \
        __trace_mem_cnt -= ND_A_SIZE (name);                                             \
        PRINTMEM (name);                                                                 \
    }

#endif /* TRACE_MEM && !TRACE_REF */

/*----------------------------------------------------------------------------*/
#if (defined(TRACE_MEM) && defined(TRACE_REF))

#define ND_ALLOC_ARRAY(type, name, rc)                                                   \
    {                                                                                    \
        sprintf (__trace_buffer, "ND_ALLOC_ARRAY(%s, %s, %d)", #type, #name, rc);        \
        PRINT_TRACE_BUFFER;                                                              \
        ND_A_FIELD (name) = (type *)malloc (sizeof (type) * ND_A_SIZE (name));           \
        ND_A_RCP (name) = (int *)malloc (sizeof (int));                                  \
        ND_A_RC (name) = rc;                                                             \
        PRINTREF (name);                                                                 \
        PRINT_TRACE_INDENT;                                                              \
        PRINTMEM (name);                                                                 \
    }

#define ND_DEC_RC_FREE(name, num)                                                        \
    sprintf (__trace_buffer, "ND_DEC_RC_FREE(%s, %d)", #name, num);                      \
    PRINT_TRACE_BUFFER;                                                                  \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        fprintf (stderr, "freeing %s\n", #name);                                         \
        PRINT_TRACE_INDENT;                                                              \
        PRINTMEM (name);                                                                 \
        free (ND_A_FIELD (name));                                                        \
        free (ND_A_RCP (name));                                                          \
    } else                                                                               \
        PRINTREF (name);

#endif /* TRACE_MEM && TRACE_REF */

/*----------------------------------------------------------------------------*/
#if (!defined(TRACE_MEM) && defined(TRACE_REF))

#define ND_ALLOC_ARRAY(type, name, rc)                                                   \
    {                                                                                    \
        sprintf (__trace_buffer, "ND_ALLOC_ARRAY(%s, %s, %d)", #type, #name, rc);        \
        PRINT_TRACE_BUFFER;                                                              \
        ND_A_FIELD (name) = (type *)malloc (sizeof (type) * ND_A_SIZE (name));           \
        ND_A_RCP (name) = (int *)malloc (sizeof (int));                                  \
        ND_A_RC (name) = rc;                                                             \
        PRINTREF (name);                                                                 \
    }

#define ND_DEC_RC_FREE(name, num)                                                        \
    sprintf (__trace_buffer, "ND_DEC_RC_FREE(%s, %d)", #name, num);                      \
    PRINT_TRACE_BUFFER;                                                                  \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        fprintf (stderr, "freeing %s\n", #name);                                         \
        free (ND_A_FIELD (name));                                                        \
        free (ND_A_RCP (name));                                                          \
    } else                                                                               \
        PRINTREF (name);

#endif /* !TRACE_MEM && TRACE_REF */
/*----------------------------------------------------------------------------*/
#if (!defined(TRACE_MEM) && !defined(TRACE_REF))

#define ND_ALLOC_ARRAY(type, name, rc)                                                   \
    {                                                                                    \
        ND_A_FIELD (name) = (type *)malloc (sizeof (type) * ND_A_SIZE (name));           \
        ND_A_RCP (name) = (int *)malloc (sizeof (int));                                  \
        ND_A_RC (name) = rc;                                                             \
    }
#define ND_DEC_RC_FREE(name, num)                                                        \
    if ((ND_A_RC (name) -= num) == 0) {                                                  \
        free (ND_A_FIELD (name));                                                        \
        free (ND_A_RCP (name));                                                          \
    }

#endif /* !TRACE_MEM && !TRACE_REF */
/*----------------------------------------------------------------------------*/

#define ND_SET_SIZE(name, num) ND_A_SIZE (name) = num;

#define ND_SET_DIM(name, num) ND_A_DIM (name) = num;

#define ND_SET_SHAPE(name, dim, s) ND_A_SHAPE (name, dim) = s;

#define ND_KS_ASSIGN_ARRAY(name, res)                                                    \
    ND_A_RCP (res) = ND_A_RCP (name);                                                    \
    ND_A_FIELD (res) = ND_A_FIELD (name);

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
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a1)[__i] op ND_A_FIELD (a2)[__i];        \
    };

#define ND_BINOP_AxS_A(op, a2, s, res)                                                   \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a2)[__i] op s;                           \
    };

#define ND_BINOP_SxA_A(op, s, a2, res)                                                   \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = s op ND_A_FIELD (a2)[__i];                           \
    };

/*
 * Macros used for primitive function idx_psi:
 * ===========================================
 */
#define ND_IDX_PSI_S(s, a, res) res = ND_A_FIELD (a)[s];
#define ND_IDX_PSI_A(s, a, res)                                                          \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a)[s++];                                 \
    };

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
#define ND_TYPEDEF_ARRAY(type1, type2) typedef type1 *type2;

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
/* and now some macros that don't belong to N_icm
 */
#define OUT_OF_BOUND(line, prf, size, idx)                                               \
    {                                                                                    \
        fprintf (stderr,                                                                 \
                 "runtime error in line %d: access in function %s is out"                \
                 " of range (size: %d, index:%d)\n",                                     \
                 line, prf, size, idx);                                                  \
        exit (1);                                                                        \
    }

#define true 1
#define false 0

typedef int bool;
#include <stdio.h>

#endif /* _sac_icm2c_h */
