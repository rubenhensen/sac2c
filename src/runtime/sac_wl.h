/*
 *
 * $Log$
 * Revision 2.3  1999/07/21 09:42:53  sbs
 * SAC_ND_READ_ARRAY changed into SAC_ND_WRITE_ARRAY in def of SAC_WL_GRID_SET_IDX!
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:44:02  sacbase
 * new release made
 *
 * Revision 1.18  1998/08/07 16:05:23  dkr
 * WL_...FOLD_BEGIN, WL_...FOLD_END are now C-ICMs
 *
 * Revision 1.17  1998/06/29 08:58:09  cg
 * added new arguments for new with-loop begin/end ICMs
 *
 * Revision 1.16  1998/06/24 10:35:53  dkr
 * WL_(NON)FOLD_BEGIN/END are now h-icms
 *
 * Revision 1.15  1998/06/19 18:50:38  dkr
 * added extra WL-ICMs for MT
 *
 * Revision 1.14  1998/06/19 18:30:01  dkr
 * removed WL_END
 * removed WL_START, WL_STOP
 * added SAC_WL_SCHEDULE_START, ..._STOP
 *
 * Revision 1.13  1998/06/15 10:35:17  dkr
 * added WL_START, WL_STOP
 *
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
 *** these macros is used to generate names of aux-variables
 ***/

#define SAC_WL_VAR(type, idx_scalar) SAC__##type##idx_scalar

#define SAC_WL_MT_SCHEDULE_START(dim) SAC__schedule_start##dim

#define SAC_WL_MT_SCHEDULE_STOP(dim) SAC__schedule_stop##dim

/*****************************************************************************/

/***
 *** minimum, maximum of two values
 ***/

#define SAC_WL_MIN(x, y) SAC_ND_MIN (x, y)

#define SAC_WL_MAX(x, y) SAC_ND_MAX (x, y)

/*****************************************************************************/

/***
 *** blocking-loop
 ***/

/*
 * BEGIN: if (BLOCK_LEVEL == 0)
 */

#define SAC_WL_MT_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)      \
    {                                                                                    \
        int SAC_WL_VAR (block, idx_scalar)                                               \
          = SAC_WL_MIN (bound2, SAC_WL_MT_SCHEDULE_STOP (dim));                          \
        for (idx_scalar = SAC_WL_MAX (bound1, SAC_WL_MT_SCHEDULE_START (dim));           \
             idx_scalar < SAC_WL_VAR (block, idx_scalar);) {                             \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step,                       \
                            SAC_WL_VAR (block, idx_scalar));

#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = bound1; idx_scalar < bound2;) {                                \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step, bound2);

/*
 * BEGIN: if (BLOCK_LEVEL > 0)
 */

#define SAC_WL_MT_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)       \
    {                                                                                    \
        int SAC_WL_VAR (block, idx_scalar) = SAC_WL_VAR (stop, idx_scalar);              \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (block, idx_scalar);) {                             \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step,                       \
                            SAC_WL_VAR (block, idx_scalar));

#define SAC_WL_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)          \
    {                                                                                    \
        int SAC_WL_VAR (block, idx_scalar) = SAC_WL_VAR (stop, idx_scalar);              \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (block, idx_scalar);) {                             \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step,                       \
                            SAC_WL_VAR (block, idx_scalar));

/*
 * END
 */

#define SAC_WL_MT_BLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    }                                                                                    \
    }

#define SAC_WL_BLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)            \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** unrolling-blocking-loop
 ***/

/*
 * BEGIN: if (UBLOCK_LEVEL == 0)
 */

#define SAC_WL_MT_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)     \
    {                                                                                    \
        int SAC_WL_VAR (block, idx_scalar)                                               \
          = SAC_WL_MIN (bound2, SAC_WL_MT_SCHEDULE_STOP (dim));                          \
        for (idx_scalar = SAC_WL_MAX (bound1, SAC_WL_MT_SCHEDULE_START (dim));           \
             idx_scalar < SAC_WL_VAR (block, idx_scalar);) {                             \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step,                       \
                            SAC_WL_VAR (block, idx_scalar));

#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        for (idx_scalar = bound1; idx_scalar < bound2;) {                                \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;                             \
            int SAC_WL_VAR (stop, idx_scalar)                                            \
              = SAC_WL_MIN (SAC_WL_VAR (start, idx_scalar) + step, bound2);

/*
 * BEGIN: if (UBLOCK_LEVEL > 0)
 */

#define SAC_WL_MT_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)      \
    {                                                                                    \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (stop, idx_scalar);) {                              \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;

#define SAC_WL_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (stop, idx_scalar);) {                              \
            int SAC_WL_VAR (start, idx_scalar) = idx_scalar;

/*
 * END
 */

#define SAC_WL_MT_UBLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    }                                                                                    \
    }

#define SAC_WL_UBLOCK_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)           \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** stride-loop
 ***/

/*
 * BEGIN: if (STRIDE_LEVEL == 0), (STRIDE_UNROLLING == 0)
 */

#define SAC_WL_MT_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)     \
    {                                                                                    \
        int SAC_WL_VAR (start, idx_scalar)                                               \
          = SAC_WL_MAX (bound1, SAC_WL_MT_SCHEDULE_START (dim));                         \
        int SAC_WL_VAR (stop, idx_scalar)                                                \
          = SAC_WL_MIN (bound2, SAC_WL_MT_SCHEDULE_STOP (dim));                          \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (stop, idx_scalar);) {

#define SAC_WL_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_scalar) = bound2;                                      \
        for (idx_scalar = bound1; idx_scalar < bound2;) {

/*
 * BEGIN: if (STRIDE_LEVEL > 0), (STRIDE_UNROLLING == 0)
 */

#define SAC_WL_MT_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)      \
    {                                                                                    \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (stop, idx_scalar);) {

#define SAC_WL_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)         \
    {                                                                                    \
        for (idx_scalar = SAC_WL_VAR (start, idx_scalar);                                \
             idx_scalar < SAC_WL_VAR (stop, idx_scalar);) {

/*
 * BEGIN: if (STRIDE_UNROLLING == 1)
 */

#define SAC_WL_MT_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)    \
    idx_scalar = SAC_WL_VAR (start, idx_scalar);

#define SAC_WL_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2, step)       \
    idx_scalar = SAC_WL_VAR (start, idx_scalar);

/*
 * END: if (STRIDE_UNROLLING == 0)
 */

#define SAC_WL_MT_STRIDE_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)        \
    }                                                                                    \
    }

#define SAC_WL_STRIDE_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2, step)           \
    }                                                                                    \
    }

/*
 * END: if (STRIDE_UNROLLING == 1)
 */

#define SAC_WL_MT_STRIDE_UNROLL_END(dim, idx_vec, idx_scalar, bound1, bound2, step)      \
    /* empty */

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
 * BEGIN: if (GRID_UNROLLING == 0)
 */

#define SAC_WL_MT_GRID_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)              \
    {                                                                                    \
        int SAC_WL_VAR (grid, idx_scalar) = idx_scalar + bound2 - bound1;                \
        for (; idx_scalar < SAC_WL_VAR (grid, idx_scalar); idx_scalar++) {

#define SAC_WL_GRID_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)                 \
    {                                                                                    \
        int SAC_WL_VAR (grid, idx_scalar) = idx_scalar + bound2 - bound1;                \
        for (; idx_scalar < SAC_WL_VAR (grid, idx_scalar); idx_scalar++) {

/*
 * BEGIN: if (GRID_UNROLLING == 1)
 */

#define SAC_WL_MT_GRID_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2) /* empty   \
                                                                               */

#define SAC_WL_GRID_UNROLL_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2) /* empty */

/*
 * END: if (GRID_UNROLLING == 0)
 */

#define SAC_WL_MT_GRID_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)                \
    }                                                                                    \
    }

#define SAC_WL_GRID_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)                   \
    }                                                                                    \
    }

/*
 * END: if (GRID_UNROLLING == 1)
 */

#define SAC_WL_MT_GRID_UNROLL_END(dim, idx_vec, idx_scalar, bound1, bound2) idx_scalar++;

#define SAC_WL_GRID_UNROLL_END(dim, idx_vec, idx_scalar, bound1, bound2) idx_scalar++;

/*****************************************************************************/

/***
 *** grid-loop (variable step)
 ***/

/*
 * BEGIN
 */

#define SAC_WL_MT_GRIDVAR_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)           \
    {                                                                                    \
        int SAC_WL_VAR (grid, idx_scalar)                                                \
          = SAC_WL_MIN (idx_scalar + bound2 - bound1, SAC_WL_VAR (stop, idx_scalar));    \
        for (; idx_scalar < SAC_WL_VAR (grid, idx_scalar); idx_scalar++) {

#define SAC_WL_GRIDVAR_LOOP_BEGIN(dim, idx_vec, idx_scalar, bound1, bound2)              \
    {                                                                                    \
        int SAC_WL_VAR (grid, idx_scalar)                                                \
          = SAC_WL_MIN (idx_scalar + bound2 - bound1, SAC_WL_VAR (stop, idx_scalar));    \
        for (; idx_scalar < SAC_WL_VAR (grid, idx_scalar); idx_scalar++) {

/*
 * END
 */

#define SAC_WL_MT_GRIDVAR_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)             \
    }                                                                                    \
    }

#define SAC_WL_GRIDVAR_LOOP_END(dim, idx_vec, idx_scalar, bound1, bound2)                \
    }                                                                                    \
    }

/*****************************************************************************/

/***
 *** This macro is needed, if the index-vector is referenced somewhere in the
 ***  with-loop-body.
 ***/

#define SAC_WL_GRID_SET_IDX(dim, idx_vec, idx_scalar, bound1, bound2)                    \
    SAC_ND_WRITE_ARRAY (idx_vec, dim) = idx_scalar;

/*****************************************************************************/

#endif /* _sac_wl_h */
