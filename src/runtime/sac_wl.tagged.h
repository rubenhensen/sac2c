/*
 *
 * $Log$
 * Revision 1.3  2003/09/19 12:07:45  dkr
 * code for old backend (no TAGGED_ARRAYS) removed
 *
 * Revision 1.2  2003/09/15 13:01:11  dkr
 * SAC_ND_MIN, SAC_ND_MAX replaced by SAC_MIN, SAC_MAX
 *
 * Revision 1.1  2003/03/14 15:40:56  dkr
 * Initial revision
 *
 * Revision 3.19  2003/03/14 13:22:37  dkr
 * all arguments of WL-ICMs are tagged now
 *
 * Revision 3.18  2003/03/14 11:40:26  dkr
 * support for sac2c flag -minarrayrep added
 *
 * Revision 3.17  2002/10/24 20:37:47  dkr
 * SAC_WL_SHAPE_FACTOR added
 *
 * Revision 3.16  2002/07/30 20:04:19  dkr
 * prefix for varnames is SAC_ instead of SAC__ now
 *
 * Revision 3.15  2002/07/12 17:17:10  dkr
 * some modifications for TAGGED_ARRAYS done
 *
 * Revision 3.13  2001/04/03 22:32:12  dkr
 * modification of SAC_WL_MT_BLOCK_LOOP0_BEGIN (revision 3.12) undone
 *
 * Revision 3.12  2001/03/16 15:16:46  dkr
 * fixed a bug in SAC_WL_MT_BLOCK_LOOP0_BEGIN
 *
 * Revision 3.11  2001/02/06 01:47:46  dkr
 * ..._EMPTY_... replaced by ..._NOOP_...
 *
 * Revision 3.10  2001/01/30 12:23:21  dkr
 * WL_..._GRID_FIT_EMPTY_... ICMs added
 *
 * Revision 3.9  2001/01/24 23:40:39  dkr
 * ICMs SAC_WL_GRIDVAR_... renamed into SAC_WL_GRID_FIT_...
 *
 * Revision 3.8  2001/01/22 13:47:51  dkr
 * ICM SAC_WL_STRIDE_LAST_LOOP_BEGIN added
 *
 * Revision 3.7  2001/01/19 11:54:19  dkr
 * some with-loop ICMs renamed
 *
 * Revision 3.5  2001/01/10 18:34:37  dkr
 * icm WL_GRID_SET_IDX renamed into WL_GRID_SET_IDXVEC
 *
 * Revision 3.1  2000/11/20 18:02:23  sacbase
 * new release made
 *
 * Revision 2.9  2000/08/24 11:16:16  dkr
 * macros cat? renamed to CAT?
 *
 * Revision 2.8  2000/08/18 14:06:04  dkr
 * ## replaced by cat0
 *
 * Revision 2.7  2000/07/06 17:23:07  dkr
 * SAC_WL_MIN replaced by SAC_ND_MIN
 * SAC_WL_MAX replaced by SAC_ND_MAX
 *
 * Revision 2.6  2000/07/06 17:00:15  dkr
 * some minor corrections done
 *
 * Revision 2.5  2000/07/06 16:38:38  dkr
 * macro SAC_WL_DEST added
 *
 * [...]
 *
 * Revision 1.1  1998/05/02 17:49:52  dkr
 * Initial revision
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

#ifndef _sac_wl_h_
#define _sac_wl_h_

/*
 * for each loop-prolog-ICM two different versions exist:
 * ..LOOP0_BEGIN is used for the outermost node of each dimension (..LEVEL == 0)
 * ..LOOP_BEGIN is used for all inner nodes of each dimension (..LEVEL > 0)
 */

/*****************************************************************************/

/***
 *** Definition the name of the destination variable of the with-loop.
 *** During the with-loop iteration this variable always points to the
 *** current array entry.
 ***/

#define SAC_WL_OFFSET(to_NT) CAT0 (NT_NAME (to_NT), __off)

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

#define SAC_WL_SET_IDXVEC(dim, idx_vec_NT_NT, idx_scl_NT, bnd1, bnd2)                    \
    SAC_ND_WRITE (idx_vec_NT_NT, dim) = SAC_ND_READ (idx_scl_NT, 0);

/*****************************************************************************/

#endif /* _sac_wl_h_ */
