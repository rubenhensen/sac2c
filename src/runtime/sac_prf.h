/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:57  sacbase
 * new release made
 *
 * Revision 1.3  1998/07/10 08:09:21  cg
 * some bugs fixed, appropriate renaming of macros
 *
 * Revision 1.2  1998/06/05 07:49:45  cg
 * converted to new renaming conventions of local identifiers.
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_prf.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef SAC_PRF_H

#define SAC_PRF_H

/*
 * Macros for primitve arithmetic operations:
 * ==========================================
 *
 * ND_BINOP_SxA_A( op, s, a2, res)        : realizes res=(s op a2)
 * ND_BINOP_AxS_A( op, s, a2, res)        : realizes res=(a2 op s)
 * ND_BINOP_AxA_A( op, a1, a2, res)       : realizes res=(a1 op a2)
 *
 */

#define SAC_ND_BINOP_AxA_A(op, a1, a2, res)                                              \
    SAC_TR_PRF_PRINT (("ND_BINOP_AxA_A( %s, %s, %s, %s)\n", #op, #a1, #a2, #res));       \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++)                            \
            SAC_ND_A_FIELD (res)                                                         \
            [SAC_i] = SAC_ND_A_FIELD (a1)[SAC_i] op SAC_ND_A_FIELD (a2)[SAC_i];          \
    };

#define SAC_ND_BINOP_AxS_A(op, a2, s, res)                                               \
    SAC_TR_PRF_PRINT (("ND_BINOP_AxS_A( %s, %s, %s, %s)\n", #op, #a2, #s, #res));        \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++)                            \
            SAC_ND_A_FIELD (res)[SAC_i] = SAC_ND_A_FIELD (a2)[SAC_i] op s;               \
    };

#define SAC_ND_BINOP_SxA_A(op, s, a2, res)                                               \
    SAC_TR_PRF_PRINT (("ND_BINOP_SxA_A( %s, %s, %s, %s)\n", #op, #s, #a2, #res));        \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++)                            \
            SAC_ND_A_FIELD (res)[SAC_i] = s op SAC_ND_A_FIELD (a2)[SAC_i];               \
    };

/*
 * Macros for primitive type conversion functions on arrays
 */

#define SAC_ND_CONV_A(a1, res)                                                           \
    {                                                                                    \
        int SAC_i;                                                                       \
        for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE (res); SAC_i++)                            \
            SAC_ND_A_FIELD (res)[SAC_i] = SAC_ND_A_FIELD (a1)[SAC_i];                    \
    }

#define SAC_ND_I2F_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_I2D_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_F2D_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_F2I_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_D2I_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_D2F_A(a1, res) SAC_ND_CONV_A (a1, res)

#define SAC_ND_2I_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_2F_A(a1, res) SAC_ND_CONV_A (a1, res)
#define SAC_ND_2D_A(a1, res) SAC_ND_CONV_A (a1, res)

/*
 * Macros for primitive functions min, max, and abs
 */

#define SAC_ND_MIN(a1, a2) ((a1) < (a2) ? (a1) : (a2))
#define SAC_ND_MAX(a1, a2) ((a1) > (a2) ? (a1) : (a2))
#define SAC_ND_ABS(a) ((a) < 0 ? (-(a)) : (a))

#endif /* SAC_PRF_H */
