/*
 *
 * $Log$
 * Revision 1.5  1998/04/17 02:15:15  dkr
 * ConstSegs() is now implemented
 *
 * Revision 1.4  1998/04/13 19:58:40  dkr
 * added ConstSegs (dummy)
 *
 * Revision 1.3  1998/04/13 18:10:59  dkr
 * rcs-header added
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "free.h"
#include "DupTree.h"
#include "precompile.h"

#include "wlpragma_funs.h"

#include "dbug.h"

/******************************************************************************
 *
 * function:
 *   node *IntersectStridesArray(node *strides,
 *                               node *aelems1, node *aelems2)
 *
 * description:
 *   returns the intersection of the N_WLstride-chain 'strides' with
 *    the index vector space ['alemes1', 'aelems2'].
 *    (-> 'aelems1' is the upper, 'aelems2' the lower bound).
 *
 ******************************************************************************/

node *
IntersectStridesArray (node *strides, node *aelems1, node *aelems2)
{
    node *stride, *last_stride, *grid, *grid2, *last_grid;
    int width, old_bound1, old_step, offset;
    int empty = 0;

    DBUG_ENTER ("IntersectStridesArray");

    last_stride = NULL;
    stride = strides;
    while (stride != NULL) {

        DBUG_ASSERT (((aelems1 != NULL) && (aelems2 != NULL)),
                     "ConstSegs(): arg has wrong dim");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (aelems1)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (aelems2)) == N_num)),
                     "ConstSegs(): array element not an int");

        /* intersect outline */
        old_bound1 = WLSTRIDE_BOUND1 (stride);
        WLSTRIDE_BOUND1 (stride) = MAX (old_bound1, NUM_VAL (EXPRS_EXPR (aelems1)));
        WLSTRIDE_BOUND2 (stride)
          = MIN (WLSTRIDE_BOUND2 (stride), NUM_VAL (EXPRS_EXPR (aelems2)));

        old_step = WLSTRIDE_STEP (stride);
        WLSTRIDE_STEP (stride) = MIN (old_step, width);

        /* adjust grids */
        width = WLSTRIDE_BOUND2 (stride) - WLSTRIDE_BOUND1 (stride);
        if (width > 0) {
            last_grid = NULL;
            grid = WLSTRIDE_CONTENTS (stride);
            do {
                /* compute offset */
                offset = GridOffset (WLSTRIDE_BOUND1 (stride), old_bound1, old_step,
                                     WLGRID_BOUND2 (grid));

                if (offset <= WLGRID_BOUND1 (grid)) {
                    /* grid is still in one pice :) */
                    grid2 = NULL;
                    WLGRID_BOUND1 (grid) -= offset;
                    WLGRID_BOUND2 (grid) -= offset;
                } else {
                    /* the grid is split into two parts :( */
                    grid2 = DupNode (grid);
                    /* first part: */
                    WLGRID_BOUND1 (grid) -= offset - old_step;
                    WLGRID_BOUND2 (grid) = old_step;
                    /* second part: duplicate old grid first */
                    WLGRID_BOUND1 (grid2) = 0;
                    WLGRID_BOUND2 (grid2) -= offset;
                }

                if (WLGRID_BOUND1 (grid) >= width) {
                    if (last_grid == NULL) {
                        grid = WLSTRIDE_CONTENTS (stride)
                          = FreeNode (WLSTRIDE_CONTENTS (stride));
                    } else {
                        grid = WLGRID_NEXT (last_grid)
                          = FreeNode (WLGRID_NEXT (last_grid));
                    }
                } else {
                    WLGRID_BOUND2 (grid) = MIN (WLGRID_BOUND2 (grid), width);

                    if (WLGRID_NEXTDIM (grid) != NULL) {
                        WLGRID_NEXTDIM (grid)
                          = IntersectStridesArray (WLGRID_NEXTDIM (grid),
                                                   EXPRS_NEXT (aelems1),
                                                   EXPRS_NEXT (aelems2));
                        if (WLGRID_NEXTDIM (grid) == NULL) {
                            empty = 1;
                        }
                    }

                    last_grid = grid;
                    grid = WLGRID_NEXT (grid);
                }

                if (grid2 != NULL) {
                    WLGRID_BOUND2 (grid2) = MIN (WLGRID_BOUND2 (grid2), width);

                    if (WLGRID_NEXTDIM (grid2) != NULL) {
                        WLGRID_NEXTDIM (grid2)
                          = IntersectStridesArray (WLGRID_NEXTDIM (grid2),
                                                   EXPRS_NEXT (aelems1),
                                                   EXPRS_NEXT (aelems2));
                        if (WLGRID_NEXTDIM (grid2) == NULL) {
                            empty = 1;
                        }
                    }

                    if (last_grid == NULL) {
                        WLGRID_NEXT (grid2) = WLSTRIDE_CONTENTS (stride);
                        WLSTRIDE_CONTENTS (stride) = grid2;
                    } else {
                        WLGRID_NEXT (grid2) = WLGRID_NEXT (last_grid);
                        WLGRID_NEXT (last_grid) = grid2;
                    }
                    last_grid = grid2;
                }
            } while ((empty == 0) && (grid != NULL));
        }

        if ((width < 1) || (empty == 1)) {
            /* stride is empty -> remove it */
            if (last_stride == NULL) {
                stride = strides = FreeNode (strides);
            } else {
                stride = WLSTRIDE_NEXT (last_stride)
                  = FreeNode (WLSTRIDE_NEXT (last_stride));
            }
        } else {
            last_stride = stride;
            stride = WLSTRIDE_NEXT (stride);
        }
    }

    DBUG_RETURN (strides);
}

/******************************************************************************
 *
 * function:
 *   node *Array2Bv(node *array, long *bv, int dims)
 *
 * description:
 *   converts an N_array node into a blocking vector (long *).
 *
 ******************************************************************************/

long *
Array2Bv (node *array, long *bv, int dims)
{
    int d;

    DBUG_ENTER ("Array2Bv");

    array = ARRAY_AELEMS (array);
    for (d = 0; d < dims; d++) {
        DBUG_ASSERT ((array != NULL), "Bv(): bv-arg has wrong dim");
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (array)) == N_num),
                     "Bv(): bv-arg not an int-array");
        bv[d] = NUM_VAL (EXPRS_EXPR (array));
        array = EXPRS_NEXT (array);
    }
    DBUG_ASSERT ((array == NULL), "Bv(): bv-arg has wrong dim");

    DBUG_RETURN (bv);
}

/******************************************************************************
 *
 * function:
 *   long *CalcSV(node *stride, long *sv)
 *
 * description:
 *   calculates the lcm of all grid-steps in N_WLstride-N_WLgrid-chain
 *    'stride'.
 *   CAUTION: sv must be initialized with (1, ..., 1)
 *
 ******************************************************************************/

long *
CalcSV (node *stride, long *sv)
{
    node *grid;

    DBUG_ENTER ("CalcSV");

    if (stride != NULL) {

        do {
            sv[WLSTRIDE_DIM (stride)]
              = lcm (sv[WLSTRIDE_DIM (stride)], WLSTRIDE_STEP (stride));

            grid = WLSTRIDE_CONTENTS (stride);
            DBUG_ASSERT ((grid != NULL), "no grid found");
            do {
                sv = CalcSV (WLGRID_NEXTDIM (grid), sv);
                grid = WLGRID_NEXT (grid);
            } while (grid != NULL);

            stride = WLSTRIDE_NEXT (stride);
        } while (stride != NULL);
    }

    DBUG_RETURN (sv);
}

/***************************************
 *
 * extern functions
 */

/******************************************************************************
 *
 * function:
 *   node *All(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   choose the hole array as the only segment;
 *   init blocking vectors ('NoBlocking')
 *
 ******************************************************************************/

node *
All (node *segs, node *parms, node *cubes, int dims)
{
    DBUG_ENTER ("All");

    DBUG_ASSERT ((parms == NULL), "All(): too many parms found");

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    segs = MakeWLseg (dims, DupTree (cubes, NULL), NULL);
    WLSEG_SV (segs) = CalcSV (WLSEG_CONTENTS (segs), WLSEG_SV (segs));

    segs = NoBlocking (segs, parms, cubes, dims);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Cubes(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   choose every cube as a segment;
 *   init blocking vectors ('NoBlocking')
 *
 ******************************************************************************/

node *
Cubes (node *segs, node *parms, node *cubes, int dims)
{
    node *new_seg, *last_seg;

    DBUG_ENTER ("Cubes");

    DBUG_ASSERT ((parms == NULL), "Cubes(): too many parms found");

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    while (cubes != NULL) {
        /*
         * build new segment
         */
        new_seg = MakeWLseg (dims, DupNode (cubes), NULL);
        WLSEG_SV (new_seg) = CalcSV (WLSEG_CONTENTS (new_seg), WLSEG_SV (new_seg));
        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            WLSEG_NEXT (last_seg) = new_seg;
        }
        last_seg = new_seg;

        cubes = WLSTRIDE_NEXT (cubes);
    }

    segs = NoBlocking (segs, parms, cubes, dims);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *ConstSegs(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
ConstSegs (node *segs, node *parms, node *cubes, int dims)
{
    node *new_cubes, *new_seg, *last_seg;

    DBUG_ENTER ("ConstSegs");

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    while (parms != NULL) {
        DBUG_ASSERT ((EXPRS_NEXT (parms) != NULL), "ConstSegs(): upper bound not found");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (parms)) == N_array)
                      && (NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (parms))) == N_array)),
                     "ConstSegs(): argument is not an array");

        new_cubes
          = IntersectStridesArray (DupTree (cubes, NULL),
                                   ARRAY_AELEMS (EXPRS_EXPR (parms)),
                                   ARRAY_AELEMS (EXPRS_EXPR (EXPRS_NEXT (parms))));

        new_seg = MakeWLseg (dims, new_cubes, NULL);

        if (segs == NULL) {
            segs = new_seg;
        } else {
            WLSEG_NEXT (last_seg) = new_seg;
        }
        last_seg = new_seg;

        parms = EXPRS_NEXT (EXPRS_NEXT (parms));
    }

    segs = NoBlocking (segs, parms, cubes, dims);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *NoBlocking(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   sets all blocking vectors and the unrolling-blocking vector to
 *    (1, ..., 1)  ->  no blocking is performed.
 *
 ******************************************************************************/

node *
NoBlocking (node *segs, node *parms, node *cubes, int dims)
{
    int b, d;
    node *seg = segs;

    DBUG_ENTER ("NoBlocking");

    DBUG_ASSERT ((parms == NULL), "NoBlocking(): too many parms found");

    while (seg != NULL) {

        /*
         * set ubv
         */
        for (d = 0; d < dims; d++) {
            (WLSEG_UBV (seg))[d] = 1;
        }

        /*
         * set bv[]
         */
        for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
            for (d = 0; d < dims; d++) {
                (WLSEG_BV (seg, b))[d] = 1;
            }
        }

        seg = WLSEG_NEXT (seg);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Bv(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   Changes the blocking-vector for one blocking level (ubv respectively).
 *   The level number is given as first element in 'parms' (N_num);
 *    the rest of 'parms' contains the blocking-vectors for different
 *    segments (N_array).
 *   If ('level' == 'WLSEG_BLOCKS(seg)'), the ubv is changed.
 *   If 'parms' contains less elements than 'segs', the last bv in
 *    'parms' is mapped to all remaining 'segs'.
 *   If 'parms' contains more elements than 'segs', the tail of 'parms'
 *    is ignored.
 *
 ******************************************************************************/

node *
Bv (node *segs, node *parms, node *cubes, int dims)
{
    node *seg = segs;
    int level;

    DBUG_ENTER ("Bv");

    DBUG_ASSERT ((parms != NULL), "Bv(): first parm not found");
    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (parms)) == N_num),
                 "Bv(): first argument not an int");

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if ((parms != NULL) && (seg != NULL) && (level <= WLSEG_BLOCKS (seg))) {

        while ((seg != NULL) && (EXPRS_NEXT (parms) != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (parms)) == N_array),
                         "Bv(): bv-arg not an array");

            if (level < WLSEG_BLOCKS (seg)) {
                WLSEG_BV (seg, level)
                  = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims);
            } else {
                WLSEG_UBV (seg) = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims);
            }

            seg = WLSTRIDE_NEXT (seg);
            parms = EXPRS_NEXT (parms);
        }

        while (seg != NULL) {

            if (level < WLSEG_BLOCKS (seg)) {
                WLSEG_BV (seg, level)
                  = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims);
            } else {
                WLSEG_UBV (seg) = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims);
            }

            seg = WLSTRIDE_NEXT (seg);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL0(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 0.
 *
 ******************************************************************************/

node *
BvL0 (node *segs, node *parms, node *cubes, int dims)
{
    DBUG_ENTER ("BvL0");

    parms = MakeExprs (MakeNum (0), parms);
    segs = Bv (segs, parms, cubes, dims);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL1(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 1.
 *
 ******************************************************************************/

node *
BvL1 (node *segs, node *parms, node *cubes, int dims)
{
    DBUG_ENTER ("BvL1");

    parms = MakeExprs (MakeNum (1), parms);
    segs = Bv (segs, parms, cubes, dims);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL2(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 2.
 *
 ******************************************************************************/

node *
BvL2 (node *segs, node *parms, node *cubes, int dims)
{
    DBUG_ENTER ("BvL2");

    parms = MakeExprs (MakeNum (2), parms);
    segs = Bv (segs, parms, cubes, dims);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Ubv(node *segs, node *parms, node *cubes, int dims)
 *
 * description:
 *   uses 'Bv' to change the unrolling-blocking-vectors.
 *
 ******************************************************************************/

node *
Ubv (node *segs, node *parms, node *cubes, int dims)
{
    DBUG_ENTER ("Ubv");

    if (segs != NULL) {
        parms = MakeExprs (MakeNum (WLSEG_BLOCKS (segs)), parms);
        segs = Bv (segs, parms, cubes, dims);
        parms = FreeNode (parms);
    }

    DBUG_RETURN (segs);
}
