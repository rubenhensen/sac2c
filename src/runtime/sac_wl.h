/*
 *
 * $Log$
 * Revision 3.9  2001/01/24 23:40:39  dkr
 * ICMs SAC_WL_GRIDVAR_... renamed into SAC_WL_GRID_FIT_...
 *
 * Revision 3.8  2001/01/22 13:47:51  dkr
 * ICM SAC_WL_STRIDE_LAST_LOOP_BEGIN added
 *
 * Revision 3.7  2001/01/19 11:54:19  dkr
 * some with-loop ICMs renamed
 *
 * Revision 3.6  2001/01/17 17:41:52  dkr
 * some minor changes done
 *
 * Revision 3.5  2001/01/10 18:34:37  dkr
 * icm WL_GRID_SET_IDX renamed into WL_GRID_SET_IDXVEC
 *
 * Revision 3.4  2001/01/09 20:00:45  dkr
 * ICMs for naive compilation added
 *
 * Revision 3.3  2001/01/08 21:59:32  dkr
 * support for naive compilation of with-loops added (not finished yet)
 *
 * Revision 3.2  2001/01/08 16:35:33  dkr
 * comments modifed
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

#define SAC_WL_OFFSET(target) CAT0 (target, __off)

/*****************************************************************************/

/***
 *** these macros is used to generate names of aux-variables
 ***/

#define SAC_WL_VAR(type, idx_sca) CAT0 (CAT0 (CAT0 (SAC__, type), _), idx_sca)

#define SAC_WL_MT_SCHEDULE_START(dim) CAT0 (SAC__schedule_start, dim)

#define SAC_WL_MT_SCHEDULE_STOP(dim) CAT0 (SAC__schedule_stop, dim)

/*****************************************************************************/

/***
 *** blocking-loop
 ***/

/*
 * SAC_WL_VAR( first, idx_sca) contain always the first index of the current
 * block and is needed by the ..STRIDE_.._BEGIN.., ..UBLOCK_LOOP_BEGIN..
 * and ..BLOCK_LOOP_BEGIN.. (hierarchical blocking!) macros.
 *
 * SAC_WL_VAR( last, idx_sca) contain always the last index of the current
 * block and is needed by the ..STRIDE_LOOP_BEGIN.., ..UBLOCK_LOOP_BEGIN..
 * and ..BLOCK_LOOP_BEGIN.. (hierarchical blocking!) macros.
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in these macros only!
 */

/*
 * BEGIN: if (BLOCK_LEVEL == 0)
 */

#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        for (idx_sca = bnd1; idx_sca < bnd2;) {                                          \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (last, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (first, idx_sca) + step, bnd2);

#define SAC_WL_MT_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca)                                             \
          = SAC_ND_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                            \
        for (idx_sca = SAC_ND_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (last, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (first, idx_sca) + step,                          \
                            SAC_WL_VAR (block_stop, idx_sca));

/*
 * BEGIN: if (BLOCK_LEVEL > 0)
 */

#define SAC_WL_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                 \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca) = SAC_WL_VAR (last, idx_sca);               \
        for (idx_sca = SAC_WL_VAR (first, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (last, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (first, idx_sca) + step,                          \
                            SAC_WL_VAR (block_stop, idx_sca));

#define SAC_WL_MT_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)              \
    SAC_WL_BLOCK_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_BLOCK_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)                   \
    }                                                                                    \
    }

#define SAC_WL_MT_BLOCK_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    SAC_WL_BLOCK_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** unrolling-blocking-loop
 ***/

/*
 * SAC_WL_VAR( first, idx_sca) contains always the first index of the current
 * unrolling-block and is needed by the ..STRIDE_.._BEGIN.. macros.
 *
 * remark:
 *   SAC_WL_VAR( last, idx_sca) is *not* set because this value is not needed
 *   in the loop body. Note that all contained strides are unrolled, i.e.
 *   the ...STRIDE_UNROLL... instead of the ..STRIDE_LOOP.. macros are used!
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in these macros only!
 */

/*
 * BEGIN: if (UBLOCK_LEVEL == 0)
 */

#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)               \
    {                                                                                    \
        for (idx_sca = bnd1; idx_sca < bnd2;) {                                          \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;

#define SAC_WL_MT_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)            \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca)                                             \
          = SAC_ND_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                            \
        for (idx_sca = SAC_ND_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;

/*
 * BEGIN: if (UBLOCK_LEVEL > 0)
 */

#define SAC_WL_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        for (idx_sca = SAC_WL_VAR (first, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (last, idx_sca);) {                                    \
            int SAC_WL_VAR (first, idx_sca) = idx_sca;

#define SAC_WL_MT_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    SAC_WL_UBLOCK_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * END
 */

#define SAC_WL_UBLOCK_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)                  \
    }                                                                                    \
    }

#define SAC_WL_MT_UBLOCK_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)               \
    SAC_WL_UBLOCK_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*****************************************************************************/

/***
 *** stride-loop
 ***/

/*
 * SAC_WL_VAR( stop, idx_sca) contain always the upper bound of the current
 * stride and is needed by the ..GRID_FIT_LOOP_BEGIN.. macros.
 */

/*
 * BEGIN: if (STRIDE_LEVEL == 0), (STRIDE_UNROLLING == FALSE)
 */

#define SAC_WL_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)               \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_sca) = bnd2;                                           \
        for (idx_sca = bnd1; idx_sca < bnd2;) {

#define SAC_WL_MT_STRIDE_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)            \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_sca)                                                   \
          = SAC_ND_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                            \
        for (idx_sca = SAC_ND_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                \
             idx_sca < SAC_WL_VAR (stop, idx_sca);) {

/*
 * BEGIN: if (STRIDE_LEVEL > 0), (STRIDE_UNROLLING == FALSE)
 */

/*
 * the stride is *not* the last one in the current dimension
 */
#define SAC_WL_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_sca)                                                   \
          = SAC_ND_MIN (SAC_WL_VAR (first, idx_sca) + bnd2, SAC_WL_VAR (last, idx_sca)); \
        for (idx_sca = SAC_WL_VAR (first, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (stop, idx_sca);) {

#define SAC_WL_MT_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    SAC_WL_STRIDE_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * the stride is the last one in the current dimension
 */
#define SAC_WL_STRIDE_LAST_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)           \
    {                                                                                    \
        int SAC_WL_VAR (stop, idx_sca) = SAC_WL_VAR (last, idx_sca);                     \
        for (idx_sca = SAC_WL_VAR (first, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (stop, idx_sca);) {

#define SAC_WL_MT_STRIDE_LAST_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)        \
    SAC_WL_STRIDE_LAST_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * BEGIN: if (STRIDE_UNROLLING == TRUE)
 */

#define SAC_WL_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)              \
    idx_sca = SAC_WL_VAR (first, idx_sca) + bnd1;

#define SAC_WL_MT_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)           \
    SAC_WL_STRIDE_UNROLL_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * END: if (STRIDE_UNROLLING == FALSE)
 */

#define SAC_WL_STRIDE_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)                  \
    }                                                                                    \
    }

#define SAC_WL_MT_STRIDE_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)               \
    SAC_WL_STRIDE_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * END: if (STRIDE_UNROLLING == TRUE)
 */

#define SAC_WL_STRIDE_UNROLL_END(dim, idx_vec, idx_sca, bnd1, bnd2, step) /* empty */

#define SAC_WL_MT_STRIDE_UNROLL_END(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    SAC_WL_STRIDE_UNROLL_END (dim, idx_vec, idx_sca, bnd1, bnd2, step)

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
 *** grid-loop (fitted already)
 ***/

/*
 * remark:
 *   SAC_WL_VAR( grid_stop, idx_sca) is used locally in these macros only!
 */

/*
 * BEGIN: if (GRID_UNROLLING == FALSE)
 */

#define SAC_WL_GRID_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                        \
    {                                                                                    \
        int SAC_WL_VAR (grid_stop, idx_sca) = idx_sca + (bnd2 - bnd1);                   \
        for (; idx_sca < SAC_WL_VAR (grid_stop, idx_sca); idx_sca++) {

#define SAC_WL_MT_GRID_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                     \
    SAC_WL_GRID_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2)

/*
 * BEGIN: if (GRID_UNROLLING == TRUE)
 */

#define SAC_WL_GRID_UNROLL_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2) /* empty */

#define SAC_WL_MT_GRID_UNROLL_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                   \
    SAC_WL_GRID_UNROLL_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2)

/*
 * END: if (GRID_UNROLLING == FALSE)
 */

#define SAC_WL_GRID_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                          \
    }                                                                                    \
    }

#define SAC_WL_MT_GRID_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                       \
    SAC_WL_GRID_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2)

/*
 * END: if (GRID_UNROLLING == TRUE)
 */

#define SAC_WL_GRID_UNROLL_END(dim, idx_vec, idx_sca, bnd1, bnd2) idx_sca++;

#define SAC_WL_MT_GRID_UNROLL_END(dim, idx_vec, idx_sca, bnd1, bnd2)                     \
    SAC_WL_GRID_UNROLL_END (dim, idx_vec, idx_sca, bnd1, bnd2)

/*****************************************************************************/

/***
 *** grid-loop (not fitted yet)
 ***/

/*
 * remark:
 *   SAC_WL_VAR( grid_stop, idx_sca) is used locally in this macro only!
 */

/*
 * BEGIN
 */

#define SAC_WL_GRID_FIT_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                    \
    {                                                                                    \
        int SAC_WL_VAR (grid_stop, idx_sca)                                              \
          = SAC_ND_MIN (idx_sca + (bnd2 - bnd1), SAC_WL_VAR (stop, idx_sca));            \
        for (; idx_sca < SAC_WL_VAR (grid_stop, idx_sca); idx_sca++) {

#define SAC_WL_MT_GRID_FIT_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                 \
    SAC_WL_GRID_FIT_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2)

/*
 * END
 */

#define SAC_WL_GRID_FIT_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                      \
    }                                                                                    \
    }

#define SAC_WL_MT_GRID_FIT_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                   \
    SAC_WL_GRID_FIT_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2)

/*****************************************************************************/

/***
 *** grid-loop (empty body)
 ***/

#define SAC_WL_GRID_EMPTY_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                       \
    idx_sca += (bnd2 - bnd1);

#define SAC_WL_MT_GRID_EMPTY_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                    \
    SAC_WL_GRID_EMPTY_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2)

#define SAC_WL_GRID_EMPTY_END(dim, idx_vec, idx_sca, bnd1, bnd2) /* empty */

#define SAC_WL_MT_GRID_EMPTY_END(dim, idx_vec, idx_sca, bnd1, bnd2)                      \
    SAC_WL_GRID_EMPTY_END (dim, idx_vec, idx_sca, bnd1, bnd2)

/*****************************************************************************/

/***
 *** This macro is needed, if the index-vector is referenced somewhere in the
 *** with-loop-body.
 ***/

#define SAC_WL_SET_IDXVEC(dim, idx_vec, idx_sca, bnd1, bnd2)                             \
    SAC_ND_WRITE_ARRAY (idx_vec, dim) = idx_sca;

/*****************************************************************************/

#endif /* _sac_wl_h_ */
