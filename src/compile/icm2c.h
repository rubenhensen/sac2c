/*
 *
 * $Log$
 * Revision 1.12  1995/04/18 14:44:57  sbs
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

#define ND_ALLOC_ARRAY(type, name, rc)                                                   \
    {                                                                                    \
        ND_A_FIELD (name) = (type *)malloc (sizeof (type) * ND_A_SIZE (name));           \
        ND_A_RCP (name) = (int *)malloc (sizeof (int));                                  \
        ND_A_RC (name) = rc;                                                             \
    }

#define ND_REUSE(old, new)                                                               \
    {                                                                                    \
        new = old;                                                                       \
        __##new##_rc = __##old##_rc;                                                     \
    }
#define ND_CHECK_REUSE(old, new)                                                         \
    if (ND_A_RC (old) == 1)                                                              \
    ND_REUSE (old, new) else

#define ND_SET_RC(name, num) ND_A_RC (name) = num;
#define ND_INC_RC(name, num) ND_A_RC (name) += num;
#define ND_DEC_RC(name, num) ND_A_RC (name) -= num;
#define ND_DEC_RC_FREE(name, num)                                                        \
    if ((ND_A_RC (name) -= num) == 0)                                                    \
        free (ND_A_FIELD (name));

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

#endif /* _sac_icm2c_h */
