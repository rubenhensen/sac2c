/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:39:50  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_icm_prf.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef SAC_ICM_PRF_H

#define SAC_ICM_PRF_H

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
 * Macros for primitive type conversion functions on arrays
 */

#define ND_CONV_A(a1, res)                                                               \
    {                                                                                    \
        int __i;                                                                         \
        for (__i = 0; __i < ND_A_SIZE (res); __i++)                                      \
            ND_A_FIELD (res)[__i] = ND_A_FIELD (a1)[__i];                                \
    }

#define ND_I2F_A(a1, res) ND_CONV_A (a1, res)
#define ND_I2D_A(a1, res) ND_CONV_A (a1, res)
#define ND_F2D_A(a1, res) ND_CONV_A (a1, res)
#define ND_F2I_A(a1, res) ND_CONV_A (a1, res)
#define ND_D2I_A(a1, res) ND_CONV_A (a1, res)
#define ND_D2F_A(a1, res) ND_CONV_A (a1, res)

#define ND_2I_A(a1, res) ND_CONV_A (a1, res)
#define ND_2F_A(a1, res) ND_CONV_A (a1, res)
#define ND_2D_A(a1, res) ND_CONV_A (a1, res)

/*
 * Macros for primitive functions min, max, and abs
 */

#define ND_MIN(a1, a2) ((a1) < (a2) ? (a1) : (a2))
#define ND_MAX(a1, a2) ((a1) > (a2) ? (a1) : (a2))
#define ND_ABS(a) ((a) < 0 ? (-(a)) : (a))

#endif /* SAC_ICM_PRF_H */
