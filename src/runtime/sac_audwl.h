
/*
 *
 * $Log$
 * Revision 1.1  2004/12/18 15:37:25  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_audwl.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *   This macros are used for compilation of the with-loop in case not even
 *   the dimensionality could be inferred, i.e., AUD.
 *
 *****************************************************************************/

#ifndef _SAC_AUDWL_H
#define _SAC_AUDWL_H

/*****************************************************************************/

#define SAC_AUD_WL_BEGIN(idx_vec_NT, res_NT)                                             \
    {                                                                                    \
        int SAC_in_gen;                                                                  \
        int SAC_max_off = SAC_ND_A_SIZE (res_NT);                                        \
        int SAC_off_inc = 1;                                                             \
        int SAC_max_d = SAC_ND_A_SIZE (idx_vec_NT) - 1;                                  \
        int SAC_d = SAC_ND_A_DIM (res_NT) - 1;                                           \
        int SAC_WL_OFFSET (res_NT) = 0;                                                  \
                                                                                         \
        while (SAC_d > SAC_max_d) {                                                      \
            SAC_off_inc *= SAC_ND_A_SHAPE (res_NT, SAC_d);                               \
            SAC_d--;                                                                     \
        }                                                                                \
                                                                                         \
        while (SAC_d >= 0) {                                                             \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                                        \
            SAC_d--;                                                                     \
        }                                                                                \
                                                                                         \
        while (SAC_WL_OFFSET (res_NT) < SAC_max_off) {

#define SAC_AUD_WL_LU_GEN(lower_NT, idx_vec_NT, upper_NT)                                \
    SAC_d = SAC_max_d;                                                                   \
                                                                                         \
    SAC_in_gen = 1;                                                                      \
    while (SAC_d >= 0) {                                                                 \
        SAC_in_gen                                                                       \
          = SAC_in_gen                                                                   \
            && (SAC_ND_READ (lower_NT, SAC_d) <= SAC_ND_READ (idx_vec_NT, SAC_d))        \
            && (SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d));        \
        SAC_d--;                                                                         \
    }                                                                                    \
    {                                                                                    \
        int i;                                                                           \
        if (SAC_in_gen)                                                                  \
            printf ("body at: ");                                                        \
        else                                                                             \
            printf ("default at: ");                                                     \
        for (i = 0; i < SAC_ND_A_SIZE (idx_vec_NT); i++)                                 \
            printf (" %d", SAC_ND_READ (idx_vec_NT, i));                                 \
        printf ("\n");                                                                   \
    }

#define SAC_AUD_WL_LUS_GEN(lower_NT, idx_vec_NT, upper_NT, step_NT)                      \
    SAC_d = SAC_max_d;                                                                   \
                                                                                         \
    SAC_in_gen = 1;                                                                      \
    while (SAC_d >= 0) {                                                                 \
        SAC_in_gen                                                                       \
          = SAC_in_gen                                                                   \
            && (SAC_ND_READ (lower_NT, SAC_d) <= SAC_ND_READ (idx_vec_NT, SAC_d))        \
            && (SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d))         \
            && (((SAC_ND_READ (idx_vec_NT, SAC_d) - SAC_ND_READ (lower_NT, SAC_d))       \
                 % SAC_ND_READ (step_NT, SAC_d))                                         \
                == 0);                                                                   \
        SAC_d--;                                                                         \
    }                                                                                    \
    {                                                                                    \
        int i;                                                                           \
        if (SAC_in_gen)                                                                  \
            printf ("body at: ");                                                        \
        else                                                                             \
            printf ("default at: ");                                                     \
        for (i = 0; i < SAC_ND_A_SIZE (idx_vec_NT); i++)                                 \
            printf (" %d", SAC_ND_READ (idx_vec_NT, i));                                 \
        printf ("\n");                                                                   \
    }

#define SAC_AUD_WL_LUSW_GEN(lower_NT, idx_vec_NT, upper_NT, step_NT, width_NT)           \
    SAC_d = SAC_max_d;                                                                   \
                                                                                         \
    SAC_in_gen = 1;                                                                      \
    while (SAC_d >= 0) {                                                                 \
        SAC_in_gen                                                                       \
          = SAC_in_gen                                                                   \
            && (SAC_ND_READ (lower_NT, SAC_d) <= SAC_ND_READ (idx_vec_NT, SAC_d))        \
            && (SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d))         \
            && (((SAC_ND_READ (idx_vec_NT, SAC_d) - SAC_ND_READ (lower_NT, SAC_d))       \
                 % SAC_ND_READ (step_NT, SAC_d))                                         \
                < SAC_ND_READ (width_NT, SAC_d));                                        \
        SAC_d--;                                                                         \
    }                                                                                    \
    {                                                                                    \
        int i;                                                                           \
        if (SAC_in_gen)                                                                  \
            printf ("body at: ");                                                        \
        else                                                                             \
            printf ("default at: ");                                                     \
        for (i = 0; i < SAC_ND_A_SIZE (idx_vec_NT); i++)                                 \
            printf (" %d", SAC_ND_READ (idx_vec_NT, i));                                 \
        printf ("\n");                                                                   \
    }

#define SAC_AUD_WL_COND_BODY() if (SAC_in_gen) {

#define SAC_AUD_WL_COND_DEFAULT()                                                        \
    }                                                                                    \
    else                                                                                 \
    {

#define SAC_AUD_WL_COND_END() }

#define SAC_AUD_WL_END(idx_vec_NT, res_NT)                                               \
    SAC_d = SAC_max_d;                                                                   \
    SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;              \
    while ((SAC_d > 0)                                                                   \
           && (SAC_ND_READ (idx_vec_NT, SAC_d) == SAC_ND_A_SHAPE (res_NT, SAC_d))) {     \
        SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                                            \
        SAC_d--;                                                                         \
        SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;          \
    }                                                                                    \
    SAC_WL_OFFSET (res_NT) += SAC_off_inc;                                               \
    }                                                                                    \
    }

/*****************************************************************************/

#endif /* _SAC_AUDWL_H */
