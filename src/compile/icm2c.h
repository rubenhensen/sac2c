/*
 *
 * $Log$
 * Revision 1.2  1995/03/07 18:19:10  sbs
 * PSI-macros done.
 *
 * Revision 1.1  1995/03/03  14:43:46  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_icm2c_h

#define _sac_icm2c_h

/*
 * Macros for declaring arrays:
 * ============================
 *
 * ND_DECL_ARRAY : declares an array
 */

#define ND_DECL_ARRAY(type, name)                                                        \
    type *name;                                                                          \
    int __##name##_rc;                                                                   \
    int __##name##_sz;                                                                   \
    int __##name##_d;                                                                    \
    int *__##name##_s

/*
 * Macros for array access:
 * ========================
 *
 * ND_A_FIELD : for accessing elements of the array
 * ND_A_RC    : accesses the refcnt
 * ND_A_SIZE  : accesses the size of the unrolling (in elements)
 * ND_A_DIM   : accesses the dimension of the array
 * ND_A_SHAPE : accesses one shape component of an array
 *
 */

#define ND_A_FIELD(name) name
#define ND_A_RC(name) __##name##_rc
#define ND_A_SIZE(name) __##name##_sz
#define ND_A_DIM(name) __##name##_d
#define ND_KD_A_SHAPE(name, dim) __##name##_s##dim
#define ND_A_SHAPE(name, dim) __##name##_s[dim]

/*
 * Macros for initializing an array:
 * =================================
 *
 * ND_ALLOC_ARRAY : allocates memory needed
 *
 * ND_SET_RC      : sets the refcnt
 * ND_INC_RC      : increments the refcnt
 * ND_CHECK_RC    : inspects the refcnt
 * ND_DEC_RC      : decrements the refcnt
 * ND_DEC_RC_FREE : decrements the refcnt
 *                  AND frees the array if refcnt becomes zero!
 * ND_CHECK_SIZE  : inspects the size of the unrolling in elements
 * ND_SET_SIZE    : sets the size of the unrolling in elements
 *
 * ND_CHECK_DIM   : inspects the dimension of the array
 * ND_SET_DIM     : sets the dimension of the array
 *
 */

#define ND_ALLOC_ARRAY(type, name)                                                       \
    name = (type *)malloc (sizeof (type) * ND_CHECK_SIZE (name))

#define ND_SET_RC(name, num) ND_A_RC (name) = num
#define ND_INC_RC(name, num) ND_A_RC (name) += num
#define ND_DEC_RC(name, num) ND_A_RC (name) -= num
#define ND_DEC_RC_FREE(name, num)                                                        \
    if ((ND_A_RC (name) -= num) == 0)                                                    \
        free (ND_A_FIELD (name));

#define ND_SET_SIZE(name, num) ND_A_SIZE (name) = num

#define ND_SET_DIME(name, num) ND_A_DIM (name) = num

#define ND_SET_SHAPE(name, dim, s) ND_A_SHAPE (name, dim) = s

/*
 * Macros for primitve arithmetic operations:
 * ==========================================
 *
 * ND_BINOP_NEW         : allocates new array for the result
 * ND_BINOP_CHECK_BOTH  : tries to reuse (refcnt==1) one of the arg's arrays
 * ND_BINOP_CHECK_LEFT  : tries to reuse (refcnt==1) the "left" argument
 * ND_BINOP_CHECK_RIGHT : tries to reuse (refcnt==1) the "right" argument
 * ND_BINOP_LEFT        : uses "left" argument for the result
 * ND_BINOP_RIGHT       : uses "right" argument for the result
 *
 */

#define BINOP_LV __i
#define BINOP_FOR(re, a1, a2)                                                            \
    for (BINOP_LV = 0; BINOP_LV < ND_A_SIZE (res); BINOP_LV++)                           \
    ND_A_FIELD (res)[BINOP_LV] = ND_A_FIELD (a1)[BINOP_LV] op ND_A_FIELD (a2)[BINOP_LV]

/*----------------------------------------------------------------------------*/

#define ND_BINOP_NEW(op, type, a1, a2, res)                                              \
    ND_ALLOC_ARRAY (type, res);                                                          \
    {                                                                                    \
        int __i;                                                                         \
        BINOP_FOR (res, a1, a2);                                                         \
    }

#define ND_BINOP_CHECK_LEFT(op, type, a1, a2, res)                                       \
    {                                                                                    \
        int __i;                                                                         \
        if (ND_A_RC (a1) == 1)                                                           \
            BINOP_FOR (a1, a1, a2);                                                      \
        else {                                                                           \
            ND_ALLOC_ARRAY (type, res);                                                  \
            BINOP_FOR (res, a1, a2);                                                     \
        }                                                                                \
    }

#define ND_BINOP_CHECK_RIGHT(op, type, a1, a2, res)                                      \
    {                                                                                    \
        int __i;                                                                         \
        if (ND_A_RC (a2) == 1)                                                           \
            BINOP_FOR (a2, a1, a2);                                                      \
        else {                                                                           \
            ND_ALLOC_ARRAY (type, res);                                                  \
            BINOP_FOR (res, a1, a2);                                                     \
        }                                                                                \
    }

#define ND_BINOP_LEFT(op, type, a1, a2)                                                  \
    {                                                                                    \
        int __i;                                                                         \
        BINOP_FOR (a1, a1, a2);                                                          \
    }

#define ND_BINOP_RIGHT(op, type, a1, a2)                                                 \
    {                                                                                    \
        int __i;                                                                         \
        BINOP_FOR (a2, a1, a2);                                                          \
    }

/*
 * Macros for PSI-primitves:
 * =========================
 *
 * ND_PSI_VxA_S    :
 * ND_PSI_VxA_A    :
 * ND_TAKE_VxA_A_1 :
 *
 */

#define ND_PSI_VxA_S(name, offset) name[offset]

#define ND_PSI_VxA_A(type, a1, res, offset)                                              \
    ND_ALLOC_ARRAY (type, res);                                                          \
    {                                                                                    \
        type *__src, *__dest;                                                            \
                                                                                         \
        __src = a1 + offset;                                                             \
        __dest = res;                                                                    \
        for (__i = 0; __i < ND_CHECK_SIZE (res); __i++)                                  \
            *__dest++ = *__src++;                                                        \
    }

#define ND_TAKE_VxA_A_1(type, a1, res, v0) ND_ALLOC_ARRAY (type, res);
{
    type *__src, *__dest;

    __src = a1;
    __dest = res;
    for (__i0 = 0; __i0 < v0; __i0++) {
        __dest++ = *__src++;
    };
}

#define ND_TAKE_VxA_A_2(type, a1, res, v0, v1)                                           \
    ND_ALLOC_ARRAY (type, res);                                                          \
    {                                                                                    \
        type *__src, *__dest;                                                            \
        int __i0, __i1;                                                                  \
                                                                                         \
        __src = a1;                                                                      \
        __dest = res;                                                                    \
        for (__i0 = 0; __i0 < v0; __i0++) {                                              \
            for (__i1 = 0; __i1 < v1; __i1++) {                                          \
                __dest++ = *__src++;                                                     \
            };                                                                           \
            __src += __##name##_s1 - v1;                                                 \
        };                                                                               \
    }

#endif /* _sac_icm2c_h */

#endif /* _sac_icm2c_h */
