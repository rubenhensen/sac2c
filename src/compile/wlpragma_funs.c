/*
 *
 * $Log$
 * Revision 1.15  1998/08/11 00:26:21  dkr
 * support for N_WLsegVar added
 *
 * Revision 1.14  1998/08/10 14:49:49  dkr
 * fixed a bug in ComputeIndexMinMax
 *
 * Revision 1.13  1998/06/09 16:47:06  dkr
 * IDX_MIN, IDX_MAX now segment-specific
 *
 * Revision 1.12  1998/05/25 13:14:14  dkr
 * ASSERTs about wrong arguments in wlcomp-pragmas are now ABORT-messages
 *
 * Revision 1.11  1998/05/24 00:42:26  dkr
 * changed some assert-messages
 *
 * Revision 1.10  1998/04/29 17:14:03  dkr
 * includes now wltransform.h instead of precompile.h
 *
 * Revision 1.9  1998/04/24 18:29:33  dkr
 * added comment
 *
 * Revision 1.8  1998/04/20 02:38:38  dkr
 * includes now tree.h
 *
 * Revision 1.7  1998/04/17 17:32:32  dkr
 * removed unused vars
 *
 * Revision 1.6  1998/04/17 15:44:19  dkr
 * fixed a few bugs:
 *   IntersectStridesArray() now uses InsertWLnodes to sort the strides/grids
 *   ConstArrays() now ignores empty segments
 *
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

#include "dbug.h"
#include "tree.h"
#include "free.h"
#include "DupTree.h"
#include "wltransform.h"
#include "wlpragma_funs.h"

#include <limits.h> /* INT_MAX */

/*
 * Here the funs for wlcomp-pragmas are defined.
 *
 * Each wlcomp-pragma-fun has the signature
 *    node *WlcompFun( node *segs, node *parms, node *cubes, int dims)
 * and returns a WL_seg-chain with annotated temporary attributes (BV, UBV, ...).
 *
 * The meaning of the parameters:
 *   - In 'segs' the current segmentation is given. This segmentation is the
 *     basis for the return value. It can be freed and build up again (like in
 *     'Cubes') or can be modified (see 'Bv').
 *   - 'parms' contains an N_exprs-chain with additional parameters for the
 *     function. (E.g. concrete blocking-vectors, ...).
 *   - 'cubes' contains the set of cubes of the current with-loop. This can be
 *     used to build up a new segmentation.
 *     (CAUTION: Do not forget to use DupTree/DupNode. Parts of 'cubes' must
 *     not be shared!!)
 *   - 'dims' gives the number of dimensions of the current with-loop.
 */

/******************************************************************************
 *
 * function:
 *   node *IntersectStridesArray( node *strides,
 *                                node *aelems1, node *aelems2,
 *                                int line)
 *
 * description:
 *   returns the intersection of the N_WLstride-chain 'strides' with
 *    the index vector space ['alemes1', 'aelems2'].
 *    (-> 'aelems1' is the upper, 'aelems2' the lower bound).
 *
 ******************************************************************************/

node *
IntersectStridesArray (node *strides, node *aelems1, node *aelems2, int line)
{
    node *isect, *nextdim, *code, *new_grids, *grids;
    int bound1, bound2, step, width, offset, grid1_b1, grid1_b2, grid2_b1, grid2_b2;
    int empty = 0; /* is the intersection empty in the current dim? */

    DBUG_ENTER ("IntersectStridesArray");

    isect = NULL;
    if (strides != NULL) {

        DBUG_ASSERT ((NODE_TYPE (strides) == N_WLstride), "no constant stride found");

        if ((aelems1 == NULL) || (aelems2 == NULL)) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "ConstSegs(): Argument has wrong dimension"));
        }
        if ((NODE_TYPE (EXPRS_EXPR (aelems1)) != N_num)
            || (NODE_TYPE (EXPRS_EXPR (aelems2)) != N_num)) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "ConstSegs(): Argument is not an 'int'-array"));
        }

        /* compute outline of intersection in current dim */
        bound1 = MAX (WLSTRIDE_BOUND1 (strides), NUM_VAL (EXPRS_EXPR (aelems1)));
        bound2 = MIN (WLSTRIDE_BOUND2 (strides), NUM_VAL (EXPRS_EXPR (aelems2)));

        width = bound2 - bound1;
        step = MIN (WLSTRIDE_STEP (strides), width);

        /* compute grids */
        if (width > 0) {
            isect
              = MakeWLstride (WLSTRIDE_LEVEL (strides), WLSTRIDE_DIM (strides), bound1,
                              bound2, step, WLSTRIDE_UNROLLING (strides), NULL, NULL);

            new_grids = NULL;
            grids = WLSTRIDE_CONTENTS (strides);
            do {
                /* compute offset for current grid */
                offset = GridOffset (bound1, WLSTRIDE_BOUND1 (strides),
                                     WLSTRIDE_STEP (strides), WLGRID_BOUND2 (grids));

                if (offset <= WLGRID_BOUND1 (grids)) {
                    /* grid is still in one pice :) */

                    grid1_b1 = WLGRID_BOUND1 (grids) - offset;
                    grid1_b2 = WLGRID_BOUND2 (grids) - offset;
                    grid2_b1 = width; /* dummy value */
                } else {
                    /* the grid is split into two parts :( */

                    /* first part: */
                    grid1_b1 = 0;
                    grid1_b2 = WLGRID_BOUND2 (grids) - offset;
                    /* second part: */
                    grid2_b1 = WLGRID_BOUND1 (grids) - (offset - WLSTRIDE_STEP (strides));
                    grid2_b2 = WLSTRIDE_STEP (strides);
                }

                nextdim = code = NULL;
                if (grid1_b1 < width) {
                    grid1_b2 = MIN (grid1_b2, width);

                    if (WLGRID_NEXTDIM (grids) != NULL) {
                        /* compute intersection of next dim */
                        nextdim = IntersectStridesArray (WLGRID_NEXTDIM (grids),
                                                         EXPRS_NEXT (aelems1),
                                                         EXPRS_NEXT (aelems2), line);
                        if (nextdim == NULL) {
                            /* next dim is empty */
                            empty = 1;
                        }
                    } else {
                        code = WLGRID_CODE (grids);
                    }

                    if (empty == 0) {
                        new_grids
                          = MakeWLgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids),
                                        grid1_b1, grid1_b2, WLGRID_UNROLLING (grids),
                                        nextdim, new_grids, code);
                    }
                }
                if (grid2_b1 < width) {
                    DBUG_ASSERT ((grid1_b1 < width), "wrong grid bounds");
                    grid2_b2 = MIN (grid2_b2, width);

                    if (empty == 0) {
                        new_grids
                          = MakeWLgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids),
                                        grid2_b1, grid2_b2, WLGRID_UNROLLING (grids),
                                        DupTree (nextdim, NULL), new_grids, code);
                    }
                }

                grids = WLGRID_NEXT (grids);
            } while ((grids != NULL) && (empty == 0));

            if (empty == 0) {
                /* sorted insertion of new grids */
                WLSTRIDE_CONTENTS (isect)
                  = InsertWLnodes (WLSTRIDE_CONTENTS (isect), new_grids);
            } else {
                /* next dim is empty -> erase current dim */
                DBUG_ASSERT ((new_grids == NULL), "cubes not consistent");
                isect = FreeTree (isect);
            }
        }

        /* compute intersection of next stride */
        if (isect == NULL) {
            isect
              = IntersectStridesArray (WLSTRIDE_NEXT (strides), aelems1, aelems2, line);
        } else {
            WLSTRIDE_NEXT (isect)
              = IntersectStridesArray (WLSTRIDE_NEXT (strides), aelems1, aelems2, line);
        }
    }

    DBUG_RETURN (isect);
}

/******************************************************************************
 *
 * function:
 *   node *Array2Bv( node *array, long *bv, int dims, int line)
 *
 * description:
 *   converts an N_array node into a blocking vector (long *).
 *
 ******************************************************************************/

long *
Array2Bv (node *array, long *bv, int dims, int line)
{
    int d;

    DBUG_ENTER ("Array2Bv");

    array = ARRAY_AELEMS (array);
    for (d = 0; d < dims; d++) {
        if (array == NULL) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "Bv(): Blocking vector has wrong dimension"));
        }
        if (NODE_TYPE (EXPRS_EXPR (array)) != N_num) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "Bv(): Blocking vector is not an 'int'-array"));
        }
        bv[d] = NUM_VAL (EXPRS_EXPR (array));
        array = EXPRS_NEXT (array);
    }
    if (array != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Bv(): Blocking vector has wrong dimension"));
    }

    DBUG_RETURN (bv);
}

/******************************************************************************
 *
 * function:
 *   long *CalcSV( node *stride, long *sv)
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

/******************************************************************************
 *
 * function:
 *   void ComputeIndexMinMax( int *idx_min, int *idx_max,
 *                            int dims, node *strides)
 *
 * description:
 *   Computes the minimum and maximum of the index-vector found in 'strides'.
 *
 * remark:
 *   if 'strides' is a N_stride-node, this must be a sequence of cubes.
 *   if 'strides' is a N_striVar-node, this must be a fully optimized
 *     nested stride/grid-tree.
 *   (see wltransform.c)
 *
 ******************************************************************************/

void
ComputeIndexMinMax (int *idx_min, int *idx_max, int dims, node *strides)
{
    node *stride;
    int min, max, d;

    DBUG_ENTER ("ComputeIndexMinMax");

    switch (NODE_TYPE (strides)) {

    case N_WLstride:

        /*
         * initialize 'idx_min', 'idx_max'.
         */
        for (d = 0; d < dims; d++) {
            idx_min[d] = INT_MAX;
            idx_max[d] = 0;
        }

        /*
         * we must visit every dim in every stride.
         */
        while (strides != NULL) {
            stride = strides;
            for (d = 0; d < dims; d++) {
                DBUG_ASSERT ((stride != NULL), "no stride found");

                min = WLSTRIDE_BOUND1 (stride);
                max = WLSTRIDE_BOUND2 (stride);
                stride = WLGRID_NEXTDIM (WLSTRIDE_CONTENTS (stride));

                if (min < idx_min[d]) {
                    idx_min[d] = min;
                }
                if (max > idx_max[d]) {
                    idx_max[d] = max;
                }
            }
            strides = WLSTRIDE_NEXT (strides);
        }
        break;

    case N_WLstriVar:

        /*
         * we have not a cube, but a fully optimized stride/grid-tree.
         *  -> we need to traverse the first stride only.
         */
        for (d = 0; d < dims; d++) {
            DBUG_ASSERT ((strides != NULL), "no stride found");

            if (NODE_TYPE (strides) == N_WLstride) {
                idx_min[d] = WLSTRIDE_BOUND1 (strides);
            } else { /* N_WLstriVar */
                if (NODE_TYPE (WLSTRIVAR_BOUND1 (strides)) == N_num) {
                    idx_min[d] = NUM_VAL (WLSTRIVAR_BOUND1 (strides));
                } else {
                    idx_min[d] = 0;
                }
            }

            /*
             * we will find the maximum in the last stride of current dim
             */
            stride = strides;
            while (WLSTRIVAR_NEXT (stride) != NULL) {
                stride = WLSTRIVAR_NEXT (stride);
            }

            if (NODE_TYPE (stride) == N_WLstride) {
                idx_max[d] = WLSTRIDE_BOUND2 (stride);
            } else { /* N_WLstriVar */
                if (NODE_TYPE (WLSTRIVAR_BOUND2 (stride)) == N_num) {
                    idx_max[d] = NUM_VAL (WLSTRIVAR_BOUND2 (stride));
                } else {
                    idx_max[d] = INT_MAX;
                }
            }

            strides = (NODE_TYPE (WLSTRIVAR_CONTENTS (strides)) == N_WLgridVar)
                        ? WLGRIDVAR_NEXTDIM (WLSTRIVAR_CONTENTS (strides))
                        : WLGRID_NEXTDIM (WLSTRIVAR_CONTENTS (strides));
        }
        break;

    default:

        DBUG_ASSERT ((0), "wrong node type found");
    }

    DBUG_VOID_RETURN;
}

/***************************************
 *
 * extern functions
 */

/******************************************************************************
 *
 * function:
 *   node *All( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   choose the hole array as the only segment;
 *   init blocking vectors ('NoBlocking')
 *
 * caution:
 *   in 'segs' are possibly N_WLstriVar- and N_WLgridVar-nodes!!
 *
 ******************************************************************************/

node *
All (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("All");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "All(): Parameters found"));
    }

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    if (NODE_TYPE (cubes) == N_WLstride) {

        segs = MakeWLseg (dims, DupTree (cubes, NULL), NULL);

        /*
         * we compute the infimum and supremum of the index-vector
         *  for the current segment.
         */
        ComputeIndexMinMax (WLSEG_IDX_MIN (segs), WLSEG_IDX_MAX (segs), WLSEG_DIMS (segs),
                            WLSEG_CONTENTS (segs));

        WLSEG_SV (segs) = CalcSV (WLSEG_CONTENTS (segs), WLSEG_SV (segs));

    } else {

        segs = MakeWLsegVar (dims, DupTree (cubes, NULL), NULL);

        /*
         * we compute the infimum and supremum of the index-vector
         *  for the current segment.
         */
        ComputeIndexMinMax (WLSEGVAR_IDX_MIN (segs), WLSEGVAR_IDX_MAX (segs),
                            WLSEGVAR_DIMS (segs), WLSEGVAR_CONTENTS (segs));
    }

    segs = NoBlocking (segs, parms, cubes, dims, line);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Cubes( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   choose every cube as a segment;
 *   init blocking vectors ('NoBlocking')
 *
 ******************************************************************************/

node *
Cubes (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *new_seg, *last_seg;

    DBUG_ENTER ("Cubes");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Cubes(): Parameters found"));
    }

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    while (cubes != NULL) {
        /*
         * build new segment
         */
        if (NODE_TYPE (cubes) == N_WLstride) {

            new_seg = MakeWLseg (dims, DupNode (cubes), NULL);

            /*
             * we compute the infimum and supremum of the index-vector
             *  for the current segment.
             */
            ComputeIndexMinMax (WLSEG_IDX_MIN (new_seg), WLSEG_IDX_MAX (new_seg),
                                WLSEG_DIMS (new_seg), WLSEG_CONTENTS (new_seg));

            WLSEG_SV (new_seg) = CalcSV (WLSEG_CONTENTS (new_seg), WLSEG_SV (new_seg));

        } else {

            new_seg = MakeWLsegVar (dims, DupNode (cubes), NULL);

            /*
             * we compute the infimum and supremum of the index-vector
             *  for the current segment.
             */
            ComputeIndexMinMax (WLSEGVAR_IDX_MIN (new_seg), WLSEGVAR_IDX_MAX (new_seg),
                                WLSEGVAR_DIMS (new_seg), WLSEGVAR_CONTENTS (new_seg));
        }

        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            if (NODE_TYPE (last_seg) == N_WLseg) {
                WLSEG_NEXT (last_seg) = new_seg;
            } else {
                WLSEGVAR_NEXT (last_seg) = new_seg;
            }
        }
        last_seg = new_seg;

        cubes = WLSTRIDE_NEXT (cubes);
    }

    segs = NoBlocking (segs, parms, cubes, dims, line);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *ConstSegs( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   Defines a new set of segments in reliance on the given extra parameters
 *    'parms'. Each segment is described by two N_array nodes containing
 *    the start, stop index vector, respectively.
 *
 ******************************************************************************/

node *
ConstSegs (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *new_cubes, *new_seg, *last_seg;

    DBUG_ENTER ("ConstSegs");

    if (NODE_TYPE (cubes) == N_WLstride) {

        if (segs != NULL) {
            segs = FreeTree (segs);
        }

        while (parms != NULL) {
            if (EXPRS_NEXT (parms) == NULL) {
                ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                              "ConstSegs(): Upper bound not found"));
            }
            if ((NODE_TYPE (EXPRS_EXPR (parms)) != N_array)
                || (NODE_TYPE (EXPRS_EXPR (EXPRS_NEXT (parms))) != N_array)) {
                ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                              "ConstSegs(): Argument is not an array"));
            }

            new_cubes
              = IntersectStridesArray (cubes, ARRAY_AELEMS (EXPRS_EXPR (parms)),
                                       ARRAY_AELEMS (EXPRS_EXPR (EXPRS_NEXT (parms))),
                                       line);

            if (new_cubes != NULL) {
                new_seg = MakeWLseg (dims, new_cubes, NULL);

                /*
                 * we compute the infimum and supremum of the index-vector
                 *  for the current segment.
                 */
                ComputeIndexMinMax (WLSEG_IDX_MIN (new_seg), WLSEG_IDX_MAX (new_seg),
                                    WLSEG_DIMS (new_seg), WLSEG_CONTENTS (new_seg));

                if (segs == NULL) {
                    segs = new_seg;
                } else {
                    WLSEG_NEXT (last_seg) = new_seg;
                }
                last_seg = new_seg;
            }

            parms = EXPRS_NEXT (EXPRS_NEXT (parms));
        }

        segs = NoBlocking (segs, parms, cubes, dims, line);

    } else {

        WARN (line, ("wlcomp-pragma function ConstSeg() ignored"
                     " because generator is not constant."));
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *NoBlocking( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   sets all blocking vectors and the unrolling-blocking vector to
 *    (1, ..., 1)  ->  no blocking is performed.
 *
 ******************************************************************************/

node *
NoBlocking (node *segs, node *parms, node *cubes, int dims, int line)
{
    int b, d;
    node *seg = segs;

    DBUG_ENTER ("NoBlocking");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "NoBlocking(): Parameters found"));
    }

    while (seg != NULL) {

        if (NODE_TYPE (seg) == N_WLseg) {

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

        } else {

            /*
             * set ubv
             */
            for (d = 0; d < dims; d++) {
                (WLSEGVAR_UBV (seg))[d] = 1;
            }

            /*
             * set bv[]
             */
            for (b = 0; b < WLSEGVAR_BLOCKS (seg); b++) {
                for (d = 0; d < dims; d++) {
                    (WLSEGVAR_BV (seg, b))[d] = 1;
                }
            }

            seg = WLSEGVAR_NEXT (seg);
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Bv( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   Changes the blocking-vector for one blocking level (ubv respectively).
 *   The level number is given as first element in 'parms' (N_num);
 *    the rest of 'parms' contains the blocking-vectors for different
 *    segments (N_array).
 *   If ('level' == 'WLSEG_BLOCKS( seg)'), the ubv is changed.
 *   If 'parms' contains less elements than 'segs', the last bv in
 *    'parms' is mapped to all remaining 'segs'.
 *   If 'parms' contains more elements than 'segs', the tail of 'parms'
 *    is ignored.
 *
 ******************************************************************************/

node *
Bv (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *seg = segs;
    int level;

    DBUG_ENTER ("Bv");

    if (parms == NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Bv(): First parameter not found"));
    }
    if (NODE_TYPE (EXPRS_EXPR (parms)) != N_num) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Bv(): First argument is not an 'int'"));
    }

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if (NODE_TYPE (seg) == N_WLseg) {

        if ((parms != NULL) && (seg != NULL) && (level <= WLSEG_BLOCKS (seg))) {

            while ((seg != NULL) && (EXPRS_NEXT (parms) != NULL)) {
                if (NODE_TYPE (EXPRS_EXPR (parms)) != N_array) {
                    ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                                  "Bv(): Blocking-vector is not an array"));
                }

                if (level < WLSEG_BLOCKS (seg)) {
                    WLSEG_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims, line);
                } else {
                    WLSEG_UBV (seg)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims, line);
                }

                seg = WLSTRIDE_NEXT (seg);
                parms = EXPRS_NEXT (parms);
            }

            while (seg != NULL) {

                if (level < WLSEG_BLOCKS (seg)) {
                    WLSEG_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims, line);
                } else {
                    WLSEG_UBV (seg)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims, line);
                }

                seg = WLSTRIDE_NEXT (seg);
            }
        }

    } else {

        if ((parms != NULL) && (seg != NULL) && (level <= WLSEGVAR_BLOCKS (seg))) {

            while ((seg != NULL) && (EXPRS_NEXT (parms) != NULL)) {
                if (NODE_TYPE (EXPRS_EXPR (parms)) != N_array) {
                    ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                                  "Bv(): Blocking-vector is not an array"));
                }

                if (level < WLSEGVAR_BLOCKS (seg)) {
                    WLSEGVAR_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEGVAR_BV (seg, level), dims,
                                  line);
                } else {
                    WLSEGVAR_UBV (seg)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEGVAR_UBV (seg), dims, line);
                }

                seg = WLSTRIDE_NEXT (seg);
                parms = EXPRS_NEXT (parms);
            }

            while (seg != NULL) {

                if (level < WLSEGVAR_BLOCKS (seg)) {
                    WLSEGVAR_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEGVAR_BV (seg, level), dims,
                                  line);
                } else {
                    WLSEGVAR_UBV (seg)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEGVAR_UBV (seg), dims, line);
                }

                seg = WLSTRIDE_NEXT (seg);
            }
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL0( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 0.
 *
 ******************************************************************************/

node *
BvL0 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("BvL0");

    parms = MakeExprs (MakeNum (0), parms);
    segs = Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL1( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 1.
 *
 ******************************************************************************/

node *
BvL1 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("BvL1");

    parms = MakeExprs (MakeNum (1), parms);
    segs = Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *BvL2( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   uses 'Bv' to change the blocking-vectors in level 2.
 *
 ******************************************************************************/

node *
BvL2 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("BvL2");

    parms = MakeExprs (MakeNum (2), parms);
    segs = Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * function:
 *   node *Ubv( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * description:
 *   uses 'Bv' to change the unrolling-blocking-vectors.
 *
 ******************************************************************************/

node *
Ubv (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("Ubv");

    if (segs != NULL) {
        parms
          = MakeExprs (MakeNum ((NODE_TYPE (segs) == N_WLseg) ? WLSEG_BLOCKS (segs)
                                                              : WLSEGVAR_BLOCKS (segs)),
                       parms);
        segs = Bv (segs, parms, cubes, dims, line);
        parms = FreeNode (parms);
    }

    DBUG_RETURN (segs);
}
