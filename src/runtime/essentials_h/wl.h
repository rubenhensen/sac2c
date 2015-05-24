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

#ifndef _SAC_WL_H_
#define _SAC_WL_H_

/*
 * for each loop-prolog-ICM two different versions exist:
 * ..LOOP0_BEGIN is used for the outermost node of each dimension (..LEVEL == 0)
 * ..LOOP_BEGIN is used for all inner nodes of each dimension (..LEVEL > 0)
 */

/*****************************************************************************/

/***
 *** these macros is used to generate names of aux-variables
 ***/

#define SAC_WL_VAR(type, idx_scl_NT)                                                     \
    CAT0 (CAT0 (CAT0 (SAC_, type), _), NT_NAME (idx_scl_NT))

#define SAC_WL_SHAPE_FACTOR(res_NT, dim)                                                 \
    CAT0 (CAT0 (CAT0 (SAC_, NT_NAME (res_NT)), _shpfac), dim)

#define SAC_WL_MT_SCHEDULE_START(dim) CAT0 (SAC_schedule_start, dim)

#define SAC_WL_MT_SCHEDULE_STOP(dim) CAT0 (SAC_schedule_stop, dim)

#define SAC_WL_IS_DISTRIBUTED SAC_wl_is_distributed

#define SAC_WL_DIST_DIM0_SIZE SAC_wl_dist_dim0_size

#define SAC_WL_DIST_OFFS SAC_wl_dist_offs

#define SAC_WL_DIST_BYTES SAC_wl_dist_bytes

/*****************************************************************************/

/***
 *** blocking-loop (! noop)
 ***/

/*
 * SAC_WL_VAR( first, idx_scl_NT) contain always the first index of the current
 * block and is needed by the ..STRIDE_.._BEGIN.., ..UBLOCK_LOOP_BEGIN..
 * and ..BLOCK_LOOP_BEGIN.. (hierarchical blocking!) macros.
 *
 * SAC_WL_VAR( last, idx_scl_NT) contain always the last index of the current
 * block and is needed by the ..STRIDE_LOOP_BEGIN.., ..UBLOCK_LOOP_BEGIN..
 * and ..BLOCK_LOOP_BEGIN.. (hierarchical blocking!) macros.
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_scl_NT) is used locally in these macros only!
 */

/*
 * BEGIN: (BLOCK_LEVEL == 0)
 */

#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    {                                                                                    \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = bnd1; SAC_ND_READ (idx_scl_NT, 0) < bnd2;) { \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);            \
            int SAC_WL_VAR (last, idx_scl_NT)                                            \
              = SAC_MIN (SAC_WL_VAR (first, idx_scl_NT) + step, bnd2);

#define SAC_WL_MT_BLOCK_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_scl_NT)                                          \
          = SAC_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                               \
        for (SAC_ND_WRITE (idx_scl_NT, 0)                                                \
             = SAC_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                           \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (block_stop, idx_scl_NT);) {       \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);            \
            int SAC_WL_VAR (last, idx_scl_NT)                                            \
              = SAC_MIN (SAC_WL_VAR (first, idx_scl_NT) + step,                          \
                         SAC_WL_VAR (block_stop, idx_scl_NT));

/*
 * BEGIN: (BLOCK_LEVEL > 0)
 */

#define SAC_WL_BLOCK_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)           \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_scl_NT) = SAC_WL_VAR (last, idx_scl_NT);         \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = SAC_WL_VAR (first, idx_scl_NT) + bnd1;       \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (block_stop, idx_scl_NT);) {       \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);            \
            int SAC_WL_VAR (last, idx_scl_NT)                                            \
              = SAC_MIN (SAC_WL_VAR (first, idx_scl_NT) + step,                          \
                         SAC_WL_VAR (block_stop, idx_scl_NT));

#define SAC_WL_MT_BLOCK_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)        \
    SAC_WL_BLOCK_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_BLOCK_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)             \
    }                                                                                    \
    }

#define SAC_WL_MT_BLOCK_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    SAC_WL_BLOCK_LOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** blocking-loop (noop)
 ***/

/*
 * SAC_WL_VAR( diff, idx_scl_NT) contain the width of the noop region and
 * is needed by the SAC_WL_ADJUST_OFFSET macro.
 */

/*
 * BEGIN
 */

#define SAC_WL_BLOCK_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)           \
    {                                                                                    \
        int SAC_WL_VAR (diff, idx_scl_NT) = (bnd2 - bnd1);

#define SAC_WL_MT_BLOCK_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)        \
    SAC_WL_BLOCK_NOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_BLOCK_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step) }

#define SAC_WL_MT_BLOCK_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    SAC_WL_BLOCK_NOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** unrolling-blocking-loop (! noop)
 ***/

/*
 * SAC_WL_VAR( first, idx_scl_NT) contains always the first index of the current
 * unrolling-block and is needed by the ..STRIDE_.._BEGIN.. macros.
 *
 * remark:
 *   SAC_WL_VAR( last, idx_scl_NT) is *not* set because this value is not needed
 *   in the loop body. Note that all contained strides are unrolled, i.e.
 *   the ...STRIDE_UNROLL... instead of the ..STRIDE_LOOP.. macros are used!
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_scl_NT) is used locally in these macros only!
 */

/*
 * BEGIN: (UBLOCK_LEVEL == 0)
 */

#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    {                                                                                    \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = bnd1; SAC_ND_READ (idx_scl_NT, 0) < bnd2;) { \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);

#define SAC_WL_MT_UBLOCK_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)      \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_scl_NT)                                          \
          = SAC_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                               \
        for (SAC_ND_WRITE (idx_scl_NT, 0)                                                \
             = SAC_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                           \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (block_stop, idx_scl_NT);) {       \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);

/*
 * BEGIN: (UBLOCK_LEVEL > 0)
 */

#define SAC_WL_UBLOCK_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    {                                                                                    \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = SAC_WL_VAR (first, idx_scl_NT) + bnd1;       \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (last, idx_scl_NT);) {             \
            int SAC_WL_VAR (first, idx_scl_NT) = SAC_ND_READ (idx_scl_NT, 0);

#define SAC_WL_MT_UBLOCK_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    SAC_WL_UBLOCK_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_UBLOCK_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)            \
    }                                                                                    \
    }

#define SAC_WL_MT_UBLOCK_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    SAC_WL_UBLOCK_LOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** unrolling-blocking-loop (noop)
 ***/

/*
 * SAC_WL_VAR( diff, idx_scl_NT) contain the width of the noop region and
 * is needed by the SAC_WL_ADJUST_OFFSET macro.
 */

/*
 * BEGIN
 */

#define SAC_WL_UBLOCK_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    {                                                                                    \
        int SAC_WL_VAR (diff, idx_scl_NT) = (bnd2 - bnd1);

#define SAC_WL_MT_UBLOCK_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    SAC_WL_UBLOCK_NOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_UBLOCK_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step) }

#define SAC_WL_MT_UBLOCK_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    SAC_WL_UBLOCK_NOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** stride-loop (! noop)
 ***/

/*
 * SAC_WL_VAR( stop, idx_scl_NT) contain always the upper bound of the current
 * stride and is needed by the ..GRID_FIT_..._BEGIN.. macros.
 */

/*
 * BEGIN: (STRIDE_LEVEL == 0) && (STRIDE_UNROLLING == FALSE)
 */

#define SAC_WL_STRIDE_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_scl_NT) = bnd2;                                        \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = bnd1; SAC_ND_READ (idx_scl_NT, 0) < bnd2;) {

#define SAC_WL_MT_STRIDE_LOOP0_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)      \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_scl_NT)                                                \
          = SAC_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                               \
        for (SAC_ND_WRITE (idx_scl_NT, 0)                                                \
             = SAC_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                           \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (stop, idx_scl_NT);) {

/*
 * BEGIN: (STRIDE_LEVEL > 0) && (STRIDE_UNROLLING == FALSE)
 */

/*
 * the stride is *not* the last one in the current dimension
 */
#define SAC_WL_STRIDE_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_scl_NT)                                                \
          = SAC_MIN (SAC_WL_VAR (first, idx_scl_NT) + bnd2,                              \
                     SAC_WL_VAR (last, idx_scl_NT));                                     \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = SAC_WL_VAR (first, idx_scl_NT) + bnd1;       \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (stop, idx_scl_NT);) {

#define SAC_WL_MT_STRIDE_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    SAC_WL_STRIDE_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * the stride is the last one in the current dimension
 */
#define SAC_WL_STRIDE_LAST_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)     \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_scl_NT) = SAC_WL_VAR (last, idx_scl_NT);               \
        for (SAC_ND_WRITE (idx_scl_NT, 0) = SAC_WL_VAR (first, idx_scl_NT) + bnd1;       \
             SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (stop, idx_scl_NT);) {

#define SAC_WL_MT_STRIDE_LAST_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)  \
    SAC_WL_STRIDE_LAST_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * BEGIN: (STRIDE_UNROLLING == TRUE)
 */

#define SAC_WL_STRIDE_UNROLL_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)        \
    SAC_ND_WRITE (idx_scl_NT, 0) = SAC_WL_VAR (first, idx_scl_NT) + bnd1;

#define SAC_WL_MT_STRIDE_UNROLL_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)     \
    SAC_WL_STRIDE_UNROLL_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END: (STRIDE_UNROLLING == FALSE)
 */

#define SAC_WL_STRIDE_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)            \
    }                                                                                    \
    }

#define SAC_WL_MT_STRIDE_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    SAC_WL_STRIDE_LOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END: (STRIDE_UNROLLING == TRUE)
 */

#define SAC_WL_STRIDE_UNROLL_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    /* empty */

#define SAC_WL_MT_STRIDE_UNROLL_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    SAC_WL_STRIDE_UNROLL_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** stride-loop (noop)
 ***/

/*
 * SAC_WL_VAR( diff, idx_scl_NT) contain the width of the noop region and
 * is needed by the SAC_WL_ADJUST_OFFSET macro.
 */

/*
 * BEGIN
 */

#define SAC_WL_STRIDE_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)          \
    {                                                                                    \
        int SAC_WL_VAR (diff, idx_scl_NT) = (bnd2 - bnd1);

#define SAC_WL_MT_STRIDE_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)       \
    SAC_WL_STRIDE_NOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_STRIDE_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step) }

#define SAC_WL_MT_STRIDE_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)         \
    SAC_WL_STRIDE_NOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2, step)

/*****************************************************************************/

/*
 * CAUTION: The SAC_WL_GRID_... macros execute unconditionly the whole grid.
 *          Therefore we must take care that the parent stride meets the
 *          condition
 *                step | (bnd2 - bnd1).
 *
 *          Examples:
 *
 *              0 -> 10 step 4           Here the step (4) is not a divisor of
 *                      0 -> 1: op0      the stride-width (10);
 *                      1 -> 3: op1      In the last loop-pass we must cut off
 *                      3 -> 4: op2      the grids (1->3), (3->4).
 *
 *            For constant parameters this is done statically in compiler-phase
 *            'wltransform: fit':
 *
 *              0 ->  8 step 4
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
 *              0 ->  b step 2
 *                      0 -> 1: op0
 *                      1 -> 2: op1,
 *
 *            or
 *
 *              0 -> 10 step s
 *                      0 -> 1: op0
 *                      1 -> s: op1,
 *
 *            we can use these macros only for (0->1) grids!!!!
 *            For all other grids we must add a runtime-check, whether the grid
 *            must be cut or not ('WL_GRID_FIT_LOOP_...').
 */

/*****************************************************************************/

/***
 *** grid-loop (! noop)
 ***/

/*
 * remark:
 *   SAC_WL_VAR( grid_stop, idx_scl_NT) is used locally in these macros only!
 */

/*
 * BEGIN: (GRID_UNROLLING == FALSE) && (fitted already)
 */

#define SAC_WL_GRID_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                  \
    {                                                                                    \
        int SAC_WL_VAR (grid_stop, idx_scl_NT)                                           \
          = SAC_ND_READ (idx_scl_NT, 0) + (bnd2 - bnd1);                                 \
        for (; SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (grid_stop, idx_scl_NT);         \
             SAC_ND_WRITE (idx_scl_NT, 0) = SAC_ND_READ (idx_scl_NT, 0) + 1) {

#define SAC_WL_MT_GRID_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)               \
    SAC_WL_GRID_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * BEGIN: (GRID_UNROLLING == TRUE) && (fitted already)
 */

#define SAC_WL_GRID_UNROLL_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2) /* empty */

#define SAC_WL_MT_GRID_UNROLL_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)             \
    SAC_WL_GRID_UNROLL_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * BEGIN: (not fitted yet)
 */

#define SAC_WL_GRID_FIT_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)              \
    {                                                                                    \
        int SAC_WL_VAR (grid_stop, idx_scl_NT)                                           \
          = SAC_MIN (SAC_ND_READ (idx_scl_NT, 0) + (bnd2 - bnd1),                        \
                     SAC_WL_VAR (stop, idx_scl_NT));                                     \
        for (; SAC_ND_READ (idx_scl_NT, 0) < SAC_WL_VAR (grid_stop, idx_scl_NT);         \
             SAC_ND_WRITE (idx_scl_NT, 0) = SAC_ND_READ (idx_scl_NT, 0) + 1) {

#define SAC_WL_MT_GRID_FIT_LOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)           \
    SAC_WL_GRID_FIT_LOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * END: (GRID_UNROLLING == FALSE) && (fitted already)
 */

#define SAC_WL_GRID_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                    \
    }                                                                                    \
    }

#define SAC_WL_MT_GRID_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                 \
    SAC_WL_GRID_LOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * END: (GRID_UNROLLING == TRUE) && (fitted already)
 */

#define SAC_WL_GRID_UNROLL_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                  \
    SAC_ND_WRITE (idx_scl_NT, 0) = SAC_ND_READ (idx_scl_NT, 0) + 1;

#define SAC_WL_MT_GRID_UNROLL_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)               \
    SAC_WL_GRID_UNROLL_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * END: (not fitted yet)
 */

#define SAC_WL_GRID_FIT_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                \
    }                                                                                    \
    }

#define SAC_WL_MT_GRID_FIT_LOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)             \
    SAC_WL_GRID_FIT_LOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*****************************************************************************/

/***
 *** grid-loop (noop)
 ***/

/*
 * SAC_WL_VAR( diff, idx_scl_NT) contain the width of the noop region and
 * is needed by the SAC_WL_ADJUST_OFFSET macro.
 */

/*
 * BEGIN: (fitted already)
 */

#define SAC_WL_GRID_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                  \
    {                                                                                    \
        int SAC_WL_VAR (diff, idx_scl_NT) = (bnd2 - bnd1);                               \
        SAC_ND_WRITE (idx_scl_NT, 0)                                                     \
          = SAC_ND_READ (idx_scl_NT, 0) + SAC_WL_VAR (diff, idx_scl_NT);

#define SAC_WL_MT_GRID_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)               \
    SAC_WL_GRID_NOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * BEGIN: (not fitted yet)
 */

#define SAC_WL_GRID_FIT_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)              \
    {                                                                                    \
        int SAC_WL_VAR (diff, idx_scl_NT)                                                \
          = SAC_MIN ((bnd2 - bnd1),                                                      \
                     SAC_WL_VAR (stop, idx_scl_NT) - SAC_ND_READ (idx_scl_NT, 0));       \
        SAC_ND_WRITE (idx_scl_NT, 0)                                                     \
          = SAC_ND_READ (idx_scl_NT, 0) + SAC_WL_VAR (diff, idx_scl_NT);

#define SAC_WL_MT_GRID_FIT_NOOP_BEGIN(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)           \
    SAC_WL_GRID_FIT_NOOP_BEGIN (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * END: (fitted already)
 */

#define SAC_WL_GRID_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2) }

#define SAC_WL_MT_GRID_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                 \
    SAC_WL_GRID_NOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*
 * END: (not fitted yet)
 */

#define SAC_WL_GRID_FIT_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2) }

#define SAC_WL_MT_GRID_FIT_NOOP_END(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)             \
    SAC_WL_GRID_FIT_NOOP_END (dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)

/*****************************************************************************/

/***
 *** This macro is needed, if the index-vector is referenced somewhere in the
 *** with-loop-body.
 ***/

#define SAC_WL_SET_IDXVEC(dim, idx_vec_NT, idx_scl_NT, bnd1, bnd2)                       \
    SAC_ND_WRITE_READ (idx_vec_NT, dim, idx_scl_NT, 0);

/*****************************************************************************/

/***
 *** This macro is needed, if the index-vector is referenced somewhere in the
 *** with-loop-body.
 ***/

#define SAC_WL_INC_OFFSET(off_NT, val_NT)                                                \
    SAC_ND_WRITE (off_NT, 0) = SAC_ND_READ (off_NT, 0) + SAC_ND_A_SIZE (val_NT);

/*****************************************************************************/

#endif /* _SAC_WL_H_ */
