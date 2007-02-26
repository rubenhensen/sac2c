
/*
 *
 * $Id$
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

/*******************************************************************************
 *
 * Setting DEBUG_AUD_GENERATOR_CODE has two effects:
 *  - a compiler warning about "printf" when compiling SaC programs :-)
 *  - printed output of the index vectors traversed
 */

#define DEBUG_AUD_GENERATOR_CODE 0

#if DEBUG_AUD_GENERATOR_CODE
#define SAC_AUD_WL_DEBUG_PRINT(idx_vec_NT)                                               \
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
#else
#define SAC_AUD_WL_DEBUG_PRINT(idx_vec_NT)
#endif

/*****************************************************************************/

#define SAC_AUD_WL_BEGIN(idx_vec_NT, offset_NT, res_NT)                                  \
    {                                                                                    \
        int SAC_in_gen;                                                                  \
        int SAC_max_off = SAC_ND_A_SIZE (res_NT);                                        \
        int SAC_off_inc = 1;                                                             \
        int SAC_max_d = SAC_ND_A_SIZE (idx_vec_NT) - 1;                                  \
        int SAC_d = SAC_ND_A_DIM (res_NT) - 1;                                           \
        int SAC_WL_SHAPE_FACTOR (res_NT, 0);                                             \
                                                                                         \
        SAC_ND_WRITE (offset_NT, 0) = 0;                                                 \
                                                                                         \
        while (SAC_d > SAC_max_d) {                                                      \
            SAC_off_inc *= SAC_ND_A_SHAPE (res_NT, SAC_d);                               \
            SAC_d--;                                                                     \
        }                                                                                \
        SAC_WL_SHAPE_FACTOR (res_NT, 0) = SAC_off_inc;                                   \
                                                                                         \
        while (SAC_d >= 0) {                                                             \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                                        \
            SAC_d--;                                                                     \
        }                                                                                \
                                                                                         \
        while (SAC_ND_READ (offset_NT, 0) < SAC_max_off) {

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
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

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
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

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
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

#define SAC_AUD_WL_COND_BODY() if (SAC_in_gen) {

#define SAC_AUD_WL_COND_DEFAULT()                                                        \
    }                                                                                    \
    else                                                                                 \
    {

#define SAC_AUD_WL_COND_END() }

#define SAC_AUD_WL_END(idx_vec_NT, offset_NT, res_NT)                                    \
    SAC_d = SAC_max_d;                                                                   \
    if (SAC_d >= 0) {                                                                    \
        SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;          \
        while ((SAC_d > 0)                                                               \
               && (SAC_ND_READ (idx_vec_NT, SAC_d) == SAC_ND_A_SHAPE (res_NT, SAC_d))) { \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                                        \
            SAC_d--;                                                                     \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;      \
        }                                                                                \
    }                                                                                    \
    SAC_ND_WRITE (offset_NT, 0) = SAC_ND_READ (offset_NT, 0) + SAC_off_inc;              \
    }                                                                                    \
    }

/*****************************************************************************/

#define SAC_AUD_WL_FOLD_BEGIN(idx_vec_NT, lower_NT, upper_NT)                            \
    {                                                                                    \
        int SAC_in_gen;                                                                  \
        int SAC_max_d = SAC_ND_A_SIZE (idx_vec_NT) - 1;                                  \
        int SAC_d = 0;                                                                   \
        int SAC_within_bounds = 1;                                                       \
                                                                                         \
        while (SAC_d <= SAC_max_d) {                                                     \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (lower_NT, SAC_d);            \
            SAC_within_bounds                                                            \
              = SAC_within_bounds                                                        \
                && (SAC_ND_READ (lower_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d));      \
            SAC_d++;                                                                     \
        }                                                                                \
        while (SAC_within_bounds) {

#define SAC_AUD_WL_FOLD_LU_GEN(lower_NT, idx_vec_NT, upper_NT)                           \
    SAC_in_gen = 1;                                                                      \
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

#define SAC_AUD_WL_FOLD_LUS_GEN(lower_NT, idx_vec_NT, upper_NT, step_NT)                 \
    SAC_d = SAC_max_d;                                                                   \
    SAC_in_gen = 1;                                                                      \
    while (SAC_d >= 0) {                                                                 \
        SAC_in_gen                                                                       \
          = SAC_in_gen                                                                   \
            && (((SAC_ND_READ (idx_vec_NT, SAC_d) - SAC_ND_READ (lower_NT, SAC_d))       \
                 % SAC_ND_READ (step_NT, SAC_d))                                         \
                == 0);                                                                   \
        SAC_d--;                                                                         \
    }                                                                                    \
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

#define SAC_AUD_WL_FOLD_LUSW_GEN(lower_NT, idx_vec_NT, upper_NT, step_NT, width_NT)      \
    SAC_d = SAC_max_d;                                                                   \
    SAC_in_gen = 1;                                                                      \
    while (SAC_d >= 0) {                                                                 \
        SAC_in_gen                                                                       \
          = SAC_in_gen                                                                   \
            && (((SAC_ND_READ (idx_vec_NT, SAC_d) - SAC_ND_READ (lower_NT, SAC_d))       \
                 % SAC_ND_READ (step_NT, SAC_d))                                         \
                < SAC_ND_READ (width_NT, SAC_d));                                        \
        SAC_d--;                                                                         \
    }                                                                                    \
    SAC_AUD_WL_DEBUG_PRINT (idx_vec_NT)

#define SAC_AUD_WL_FOLD_END(idx_vec_NT, lower_NT, upper_NT)                              \
    SAC_d = SAC_max_d;                                                                   \
    if (SAC_d >= 0) {                                                                    \
        SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;          \
        while ((SAC_d > 0)                                                               \
               && (SAC_ND_READ (idx_vec_NT, SAC_d) == SAC_ND_READ (upper_NT, SAC_d))) {  \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (lower_NT, SAC_d);            \
            SAC_d--;                                                                     \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;      \
        }                                                                                \
        SAC_within_bounds                                                                \
          = SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d);             \
    } else {                                                                             \
        SAC_within_bounds = 0;                                                           \
    }                                                                                    \
    }                                                                                    \
    }

/*****************************************************************************/

#endif /* _SAC_AUDWL_H */
