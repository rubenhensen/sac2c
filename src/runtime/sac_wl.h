/*
 *
 * $Log$
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
 * Revision 2.4  2000/03/10 10:34:53  dkr
 * minor modifications in ICM definitions done
 *
 * Revision 2.3  1999/07/21 09:42:53  sbs
 * SAC_ND_READ_ARRAY changed into SAC_ND_WRITE_ARRAY in def of
 * SAC_WL_GRID_SET_IDX!
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

/*

 * for each loop-prolog-ICM two different versions exist:
 * ..LOOP0_BEGIN is used for the outermost node of each dimension (..LEVEL == 0)
 * ..LOOP_BEGIN is used for all inner nodes of each dimension (..LEVEL > 0)
 *
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
 * BEGIN: if (BLOCK_LEVEL == 0)
 *
 *******
 *
 * SAC_WL_VAR( start, idx_sca) contain always the first index position of the
 * current block and is needed by the ..STRIDE_.._BEGIN.., ..UBLOCK_LOOP_BEGIN..
 * and ..BLOCK_LOOP_BEGIN.. (hierarchical blocking!) macros.
 *
 * SAC_WL_VAR( stop, idx_sca) contain always the last index position of the
 * current block and is needed by the ..GRIDVAR_LOOP_BEGIN..,
 * ..STRIDE_LOOP_BEGIN.., ..UBLOCK_LOOP_BEGIN.. and ..BLOCK_LOOP_BEGIN..
 * (hierarchical blocking!) macros.
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in this macro only!
 */

#define SAC_WL_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        for (idx_sca = bnd1; idx_sca < bnd2;) {                                          \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (stop, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (start, idx_sca) + step, bnd2);

#define SAC_WL_MT_BLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca)                                             \
          = SAC_ND_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                            \
        for (idx_sca = SAC_ND_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (stop, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (start, idx_sca) + step,                          \
                            SAC_WL_VAR (block_stop, idx_sca));

/*
 * BEGIN: if (BLOCK_LEVEL > 0)
 *
 *******
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in this macro only!
 */

#define SAC_WL_BLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                 \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca) = SAC_WL_VAR (stop, idx_sca);               \
        for (idx_sca = SAC_WL_VAR (start, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;                                   \
            int SAC_WL_VAR (stop, idx_sca)                                               \
              = SAC_ND_MIN (SAC_WL_VAR (start, idx_sca) + step,                          \
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
 * BEGIN: if (UBLOCK_LEVEL == 0)
 *
 *******
 *
 * SAC_WL_VAR( start, idx_sca) contains always the first index position of the
 * current unrolling-block and is needed by the ..STRIDE_.._BEGIN.. macros.
 *
 * remark:
 *   SAC_WL_VAR( stop, idx_sca) is *not* set because this value is not needed
 *   in the loop body. Note that all contained strides are unrolled, i.e.
 *   the ...STRIDE_UNROLL... instead of the ..STRIDE_LOOP.. macros are used!
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in this macro only!
 */

#define SAC_WL_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)               \
    {                                                                                    \
        for (idx_sca = bnd1; idx_sca < bnd2;) {                                          \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;

#define SAC_WL_MT_UBLOCK_LOOP0_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)            \
    {                                                                                    \
        int SAC_WL_VAR (block_stop, idx_sca)                                             \
          = SAC_ND_MIN (bnd2, SAC_WL_MT_SCHEDULE_STOP (dim));                            \
        for (idx_sca = SAC_ND_MAX (bnd1, SAC_WL_MT_SCHEDULE_START (dim));                \
             idx_sca < SAC_WL_VAR (block_stop, idx_sca);) {                              \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;

/*
 * BEGIN: if (UBLOCK_LEVEL > 0)
 */

#define SAC_WL_UBLOCK_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        for (idx_sca = SAC_WL_VAR (start, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (stop, idx_sca);) {                                    \
            int SAC_WL_VAR (start, idx_sca) = idx_sca;

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
 * BEGIN: if (STRIDE_LEVEL == 0), (STRIDE_UNROLLING == FALSE)
 *
 *******
 *
 * SAC_WL_VAR( stop, idx_sca) contain always the last index position of the
 * current block and is needed by the ..GRIDVAR_LOOP_BEGIN.. macros.
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

#define SAC_WL_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)                \
    {                                                                                    \
        for (idx_sca = SAC_WL_VAR (start, idx_sca) + bnd1;                               \
             idx_sca < SAC_WL_VAR (stop, idx_sca);) {

#define SAC_WL_MT_STRIDE_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)             \
    SAC_WL_STRIDE_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2, step)

/*
 * BEGIN: if (STRIDE_UNROLLING == TRUE)
 */

#define SAC_WL_STRIDE_UNROLL_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2, step)              \
    idx_sca = SAC_WL_VAR (start, idx_sca) + bnd1;

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

/***
 *** grid-loop (constant step)
 ***/

/*
 * CAUTION: This macro executes unconditioned the whole grid.
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
 * BEGIN: if (GRID_UNROLLING == FALSE)
 *
 *******
 *
 * remark:
 *   SAC_WL_VAR( grid_stop, idx_sca) is used locally in this macro only!
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
 *** grid-loop (variable step)
 ***/

/*
 * BEGIN
 *
 *******
 *
 * remark:
 *   SAC_WL_VAR( block_stop, idx_sca) is used locally in this macro only!
 */

#define SAC_WL_GRIDVAR_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                     \
    {                                                                                    \
        int SAC_WL_VAR (grid_stop, idx_sca)                                              \
          = SAC_ND_MIN (idx_sca + (bnd2 - bnd1), SAC_WL_VAR (stop, idx_sca));            \
        for (; idx_sca < SAC_WL_VAR (grid_stop, idx_sca); idx_sca++) {

#define SAC_WL_MT_GRIDVAR_LOOP_BEGIN(dim, idx_vec, idx_sca, bnd1, bnd2)                  \
    SAC_WL_GRIDVAR_LOOP_BEGIN (dim, idx_vec, idx_sca, bnd1, bnd2)

/*
 * END
 */

#define SAC_WL_GRIDVAR_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                       \
    }                                                                                    \
    }

#define SAC_WL_MT_GRIDVAR_LOOP_END(dim, idx_vec, idx_sca, bnd1, bnd2)                    \
    SAC_WL_GRIDVAR_LOOP_END (dim, idx_vec, idx_sca, bnd1, bnd2)

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

#endif /* _sac_wl_h */
