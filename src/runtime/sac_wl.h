/*
 *
 * $Log$
 * Revision 1.12  1998/05/30 15:45:07  dkr
 * added some macros for unrolling
 *
 * Revision 1.11  1998/05/28 23:53:17  dkr
 * fixed a bug in WL_STRIDE_LOOP0_BEGIN
 *
 * Revision 1.10  1998/05/24 12:15:13  dkr
 * added optimized macros for non-MT (if SAC_DO_MULTITHREAD not defined)
 *
 * Revision 1.8  1998/05/16 19:48:15  dkr
 * added comments
 *
 * Revision 1.7  1998/05/16 16:38:59  dkr
 * WL_END is a h-icm now
 *
 * Revision 1.6  1998/05/15 23:55:40  dkr
 * added SAC_WL_GRIDVAR_LOOP_... macros
 * fixed some minor bugs
 *
 * Revision 1.5  1998/05/14 21:37:20  dkr
 * removed WL_NOOP
 *
 * Revision 1.4  1998/05/12 22:46:54  dkr
 * added SAC_WL_NOOP
 *
 * Revision 1.3  1998/05/08 00:50:25  dkr
 * added macro WL_GRID_SET_IDX
 *
 * Revision 1.2  1998/05/07 16:21:03  dkr
 * new name conventions
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 * Revision 1.5  1998/05/04 16:21:11  dkr
 * removed MT_SPMD_BLOCK macro (temporary used only)
 *
 * Revision 1.4  1998/05/04 15:36:06  dkr
 * WL_ASSIGN is now a C-ICM
 *
 * Revision 1.3  1998/05/04 09:35:37  dkr
 * fixed a bug in BLOCK_LOOP_BEGIN...
 *
 * Revision 1.2  1998/05/03 13:10:07  dkr
 * added macros
 *
 * Revision 1.1  1998/05/02 17:49:52  dkr
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_wl.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *   This macros are used for compilation of the with-loop
 *
 *****************************************************************************/

#ifndef _sac_wl_h

#define _sac_wl_h

/*****************************************************************************/

/***
 *** this macro is used to generate names of aux-variables
 ***/

#define SAC_VAR(type, idx_scalar) __##type##idx_scalar

/*****************************************************************************/

/***
 *** minimum, maximum of two values
 ***/

#define MIN(x, y) SAC_ND_MIN (x, y)

#define MAX(x, y) SAC_ND_MAX (x, y)

/*****************************************************************************/

/***
 *** blocking-loop
 ***/

/*
 * if (BLOCK_LEVEL == 0)
 */

#if SAC_DO_MULTITHREAD
#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        int SAC_VAR (block, idx_scalar) = MIN (bound2, SAC_VAR (stop0, idx_scalar));     \
        for (idx_scalar = MAX (bound1, SAC_VAR (start0, idx_scalar));                    \
             idx_scalar < SAC_VAR (block, idx_scalar);) {                                \
            int SAC_VAR (start, idx_scalar) = idx_scalar;                                \
            int SAC_VAR (stop, idx_scalar)                                               \
              = MIN (SAC_VAR (start, idx_scalar) + step, SAC_VAR (block, idx_scalar));
#else /* SAC_DO_MULTITHREAD */
#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = bound1; idx_scalar < bound2;) {                                \
            int SAC_VAR (start, idx_scalar) = idx_scalar;                                \
            int SAC_VAR (stop, idx_scalar)                                               \
              = MIN (SAC_VAR (start, idx_scalar) + step, bound2);
#endif /* SAC_DO_MULTITHREAD */

/*
 * if (BLOCK_LEVEL > 0)
 */

#define SAC_WL_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)          \
    {                                                                                    \
        int SAC_VAR (block, idx_scalar) = SAC_VAR (stop, idx_scalar);                    \
        for (idx_scalar = SAC_VAR (start, idx_scalar);                                   \
             idx_scalar < SAC_VAR (block, idx_scalar);) {                                \
            int SAC_VAR (start, idx_scalar) = idx_scalar;                                \
            int SAC_VAR (stop, idx_scalar)                                               \
              = MIN (SAC_VAR (start, idx_scalar) + step, SAC_VAR (block, idx_scalar));

#define SAC_WL_BLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)            \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** unrolling-blocking-loop
 ***/

/*
 * if (UBLOCK_LEVEL == 0)
 */

#if SAC_DO_MULTITHREAD
#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        int SAC_VAR (block, idx_scalar) = MIN (bound2, SAC_VAR (stop0, idx_scalar));     \
        for (idx_scalar = MAX (bound1, SAC_VAR (start0, idx_scalar));                    \
             idx_scalar < SAC_VAR (block, idx_scalar);) {                                \
            int SAC_VAR (start, idx_scalar) = idx_scalar;                                \
            int SAC_VAR (stop, idx_scalar)                                               \
              = MIN (SAC_VAR (start, idx_scalar) + step, SAC_VAR (block, idx_scalar));
#else /* SAC_DO_MULTITHREAD */
#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        for (idx_scalar = bound1; idx_scalar < bound2;) {                                \
            int SAC_VAR (start, idx_scalar) = idx_scalar;                                \
            int SAC_VAR (stop, idx_scalar)                                               \
              = MIN (SAC_VAR (start, idx_scalar) + step, bound2);
#endif /* SAC_DO_MULTITHREAD */

/*
 * if (UBLOCK_LEVEL > 0)
 */

#define SAC_WL_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = SAC_VAR (start, idx_scalar);                                   \
             idx_scalar < SAC_VAR (stop, idx_scalar);) {                                 \
            int SAC_VAR (start, idx_scalar) = idx_scalar;

#define SAC_WL_UBLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)           \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** stride-loop
 ***/

/*
 * if (STRIDE_LEVEL == 0), (STRIDE_UNROLLING == 0)
 */

#if SAC_DO_MULTITHREAD
#define SAC_WL_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        int SAC_VAR (start, idx_scalar) = MAX (bound1, SAC_VAR (start0, idx_scalar));    \
        int SAC_VAR (stop, idx_scalar) = MIN (bound2, SAC_VAR (stop0, idx_scalar));      \
        for (idx_scalar = SAC_VAR (start, idx_scalar);                                   \
             idx_scalar < SAC_VAR (stop, idx_scalar);) {
#else /* SAC_DO_MULTITHREAD */
#define SAC_WL_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        int SAC_VAR (stop, idx_scalar) = bound2;                                         \
        for (idx_scalar = bound1; idx_scalar < bound2;) {
#endif /* SAC_DO_MULTITHREAD */

/*
 * if (STRIDE_LEVEL > 0), (STRIDE_UNROLLING == 0)
 */

#define SAC_WL_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = SAC_VAR (start, idx_scalar);                                   \
             idx_scalar < SAC_VAR (stop, idx_scalar);) {

#define SAC_WL_STRIDE_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)           \
    }                                                                                    \
    }

/*
 * if (STRIDE_UNROLLING == 1)
 */

#define SAC_WL_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)       \
    idx_scalar = SAC_VAR (start, idx_scalar);

#define SAC_WL_STRIDE_UNROLL_END(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    /* empty */

/*****************************************************************************/

/***
 *** grid-loop (constant step)
 ***/

/*
 * CAUTION: This macro executes unconditionlly the whole grid.
 *          Therefore we must take care that the parent stride meets the condition
 *                step | (bound2 - bound1).
 *
 *          Examples:
 *
 *              0 -> 10 step 4               Here the step (4) is not a divisor of
 *                      0 -> 1: op0          the stride-width (10);
 *                      1 -> 3: op1          In the last loop-pass we must cut off
 *                      3 -> 4: op2          the grids (1->3), (3->4).
 *
 *            For constant parameters this is done statically in compiler-phase
 *            'wltransform: fit':
 *
 *              0 -> 8  step 4
 *                      0 -> 1: op0
 *                      1 -> 3: op1
 *                      3 -> 4: op2
 *              8 -> 10 step 2
 *                      0 -> 1: op0
 *                      1 -> 2: op1
 *
 *            Therefore we can use 'WL_GRID_LOOP_...' in this case.
 *
 *            But if some of the parameters are not constant, e.g.
 *
 *              0 -> b step 2
 *                     0 -> 1: op0
 *                     1 -> 2: op1,
 *
 *            we can use these macros only for (0->1) grids!!!!
 *            For all other grids we must add a runtime-check, whether the grid
 *            must be cut or not ('WL_GRIDVAR_LOOP_...').
 */

/*
 * if (GRID_UNROLLING == 0)
 */

#define SAC_WL_GRID_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)                 \
    {                                                                                    \
        int grid_##idx_scalar = idx_scalar + bound2 - bound1;                            \
        for (; idx_scalar < grid_##idx_scalar; idx_scalar++) {

#define SAC_WL_GRID_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)                   \
    }                                                                                    \
    }

/*
 * if (GRID_UNROLLING == 1)
 */

#define SAC_WL_GRID_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2) /* empty */

#define SAC_WL_GRID_UNROLL_END(dim, idx_vec, idx_scalar, bound1, bound2) idx_scalar++;

/*****************************************************************************/

/***
 *** grid-loop (variable step)
 ***/

#define SAC_WL_GRIDVAR_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)              \
    {                                                                                    \
        int grid_##idx_scalar                                                            \
          = MIN (idx_scalar + bound2 - bound1, SAC_VAR (stop, idx_scalar));              \
        for (; idx_scalar < grid_##idx_scalar; idx_scalar++) {

#define SAC_WL_GRIDVAR_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)                \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** This macro is needed, if the index-vector is referenced somewhere in the
 ***  with-loop-body.
 ***/

#define SAC_WL_GRID_SET_IDX(dim, idx_vec, idx_scalar, bound1, bound2)                    \
    SAC_ND_A_FIELD (idx_vec)[dim] = idx_scalar;

/*****************************************************************************/

/***
 *** end of with-loop
 ***/

#define SAC_WL_END() }

/*****************************************************************************/

#endif /* _sac_wl_h */
