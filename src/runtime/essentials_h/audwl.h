
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

#ifndef DEBUG_AUD_GENERATOR_CODE
#define DEBUG_AUD_GENERATOR_CODE 0
#endif

#if DEBUG_AUD_GENERATOR_CODE
#define SAC_AUD_WL_DEBUG_PRINT(idx_vec_NT)                                     \
    {                                                                          \
        int i;                                                                 \
        if (SAC_in_gen)                                                        \
            printf ("body at: ");                                              \
        else                                                                   \
            printf ("default at: ");                                           \
        for (i = 0; i < SAC_ND_A_SIZE (idx_vec_NT); i++)                       \
            printf (" %d", SAC_ND_READ (idx_vec_NT, i));                       \
        printf ("\n");                                                         \
    }
#else
#define SAC_AUD_WL_DEBUG_PRINT(idx_vec_NT)
#endif

/*****************************************************************************/

/*
 * all AUD WLs start with this one:
 */
#define SAC_AUD_WL_BEGIN(idx_vec_NT)                                           \
    {                                                                          \
        int SAC_d = 0;                                                         \
        int SAC_max_d = SAC_ND_A_SIZE (idx_vec_NT) - 1;                        \
        int SAC_within_bounds = 1;                                             \
        int SAC_in_gen;                                                        \

/*
 * WLs with modarray and genarray operators (but no folds!) require this:
 * we initialiaze the index vector with 0s and set SAC_within_bounds.
 */
#define SAC_AUD_WL_INIT_IDX_VEC_FULL(idx_vec_NT, res_NT)                       \
        while (SAC_d <= SAC_max_d) {                                           \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                              \
            SAC_d++;                                                           \
        }                                                                      \
        SAC_within_bounds = SAC_ND_A_SIZE (res_NT) > 0;

/*
 * WLs with fold operators (but no modarray/genarray operators!) or propagates
 * only require this:
 * we initialiaze the index vector with the lower bound
 * and set SAC_within_bounds.
 */
#define SAC_AUD_WL_INIT_IDX_VEC_LOWER(idx_vec_NT, lower_NT, upper_NT)          \
        while (SAC_d <= SAC_max_d) {                                           \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (lower_NT, SAC_d);  \
            SAC_within_bounds                                                  \
              = SAC_within_bounds                                              \
                && ( SAC_ND_READ (lower_NT, SAC_d)                             \
                     < SAC_ND_READ (upper_NT, SAC_d) );                        \
            SAC_d++;                                                           \
        }

/*
 * modarray and genarray operators require one of these each:
 */
#define SAC_AUD_WL_INIT_OFFSET(offset_NT, res_NT, num)                         \
    int SAC_off_inc ## num = 1;                                                \
    int SAC_WL_SHAPE_FACTOR (res_NT, 0);                                       \
                                                                               \
    SAC_d = SAC_ND_A_DIM (res_NT) - 1;                                         \
    SAC_ND_WRITE (offset_NT, 0) = 0;                                           \
                                                                               \
    while (SAC_d > SAC_max_d) {                                                \
        SAC_off_inc ## num *= SAC_ND_A_SHAPE (res_NT, SAC_d);                  \
        SAC_d--;                                                               \
    }                                                                          \
    SAC_WL_SHAPE_FACTOR (res_NT, 0) = SAC_off_inc ## num;

/*
 * all AUD WLs use this one:
 */
#define SAC_AUD_WL_START_ITERATE()                                             \
    while (SAC_within_bounds) {                                                \
    SAC_in_gen = 1;

/*
 * check needed for modarray and genarray WLs only:
 */
#define SAC_AUD_WL_CHECK_LU(idx_vec_NT, lower_NT, upper_NT)                    \
    SAC_d = SAC_max_d;                                                         \
    while (SAC_d >= 0) {                                                       \
        SAC_in_gen                                                             \
          = SAC_in_gen                                                         \
            && (SAC_ND_READ (lower_NT, SAC_d)                                  \
                <= SAC_ND_READ (idx_vec_NT, SAC_d))                            \
            && (SAC_ND_READ (idx_vec_NT, SAC_d)                                \
                < SAC_ND_READ (upper_NT, SAC_d));                              \
        SAC_d--;                                                               \
    }                                                                          \

#define SAC_AUD_WL_CHECK_STEP(idx_vec_NT, lower_NT, step_NT)                   \
    SAC_d = SAC_max_d;                                                         \
    while (SAC_d >= 0) {                                                       \
        SAC_in_gen                                                             \
          = SAC_in_gen                                                         \
            && ( ( ( SAC_ND_READ (idx_vec_NT, SAC_d)                           \
                     - SAC_ND_READ (lower_NT, SAC_d) )                         \
                    % SAC_ND_READ (step_NT, SAC_d) ) == 0 );                   \
        SAC_d--;                                                               \
    }
    
#define SAC_AUD_WL_CHECK_STEPWIDTH(idx_vec_NT, lower_NT, step_NT, width_NT)    \
    SAC_d = SAC_max_d;                                                         \
    while (SAC_d >= 0) {                                                       \
        SAC_in_gen                                                             \
          = SAC_in_gen                                                         \
            && ( ( ( SAC_ND_READ (idx_vec_NT, SAC_d)                           \
                     - SAC_ND_READ (lower_NT, SAC_d) )                         \
                    % SAC_ND_READ (step_NT, SAC_d) )                           \
                    < SAC_ND_READ (width_NT, SAC_d) );                         \
        SAC_d--;                                                               \
    }
    
/*
 * the two cases that all WLs have to distinguish:
 */
#define SAC_AUD_WL_COND_BODY(idx_vec_NT)                                       \
    SAC_AUD_WL_DEBUG_PRINT(idx_vec_NT)                                         \
    if (SAC_in_gen) {

#define SAC_AUD_WL_COND_DEFAULT()                                              \
    } else {

#define SAC_AUD_WL_COND_END()                                                  \
    }

/*
 * for each modarray/genarray WL, we need to adjust the offset variable
 * with their result-specific increment!
 */
#define SAC_AUD_WL_INC_OFFSET(offset_NT, num)                                  \
    SAC_ND_WRITE (offset_NT, 0) = SAC_ND_READ (offset_NT, 0)                   \
                                  + SAC_off_inc ## num;

/*
 * increment the index vector and set SAC_within_bounds in case we are dealing
 * with a modarray/genarray WL
 */
#define SAC_AUD_WL_INC_IDX_VEC_FULL(idx_vec_NT, res_NT)                        \
    SAC_d = SAC_max_d;                                                         \
    if (SAC_d >= 0) {                                                          \
        SAC_ND_WRITE (idx_vec_NT, SAC_d)                                       \
            = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;                             \
        while ((SAC_d > 0)                                                     \
               && (SAC_ND_READ (idx_vec_NT, SAC_d)                             \
                   == SAC_ND_A_SHAPE (res_NT, SAC_d))) {                       \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = 0;                              \
            SAC_d--;                                                           \
            SAC_ND_WRITE (idx_vec_NT, SAC_d)                                   \
                = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;                         \
        }                                                                      \
        SAC_within_bounds                                                      \
            = SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_A_SHAPE (res_NT, SAC_d);\
    } else {                                                                   \
        SAC_within_bounds = 0;                                                 \
    }

/*
 * increment the index vector and set SAC_within_bounds in case we are dealing
 * with a fold/propagate-only WL
 */
#define SAC_AUD_WL_INC_IDX_VEC_LOWER(idx_vec_NT, lower_NT, upper_NT)           \
    SAC_d = SAC_max_d;                                                         \
    if (SAC_d >= 0) {                                                          \
        SAC_ND_WRITE (idx_vec_NT, SAC_d)                                       \
            = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;                             \
        while ((SAC_d > 0)                                                     \
               && (SAC_ND_READ (idx_vec_NT, SAC_d)                             \
                   == SAC_ND_READ (upper_NT, SAC_d))) {                        \
            SAC_ND_WRITE (idx_vec_NT, SAC_d) = SAC_ND_READ (lower_NT, SAC_d);  \
            SAC_d--;                                                           \
            SAC_ND_WRITE (idx_vec_NT, SAC_d)                                   \
                = SAC_ND_READ (idx_vec_NT, SAC_d) + 1;                         \
        }                                                                      \
        SAC_within_bounds                                                      \
            = SAC_ND_READ (idx_vec_NT, SAC_d) < SAC_ND_READ (upper_NT, SAC_d); \
    } else {                                                                   \
        SAC_within_bounds = 0;                                                 \
    }

#define SAC_AUD_WL_END_ITERATE()                                               \
    } // while from SAC_AUD_WL_START_ITERATE

#define SAC_AUD_WL_END()                                                       \
    } // entire AUD_WL block

/*****************************************************************************/

#endif /* _SAC_AUDWL_H */
