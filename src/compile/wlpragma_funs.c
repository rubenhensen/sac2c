
#include "types.h"
#include "tree_basic.h"
#include "free.h"
#include "DupTree.h"

#include "wlpragma_funs.h"

#include "dbug.h"

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
    node *array1;
    int d;

    DBUG_ENTER ("Array2Bv");

    array1 = ARRAY_AELEMS (EXPRS_EXPR (array));
    for (d = 0; d < dims; d++) {
        DBUG_ASSERT ((array1 != NULL), "Bv(): bv-arg has wrong dim");
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (array1)) == N_num),
                     "Bv(): bv-arg not an int-array");
        bv[d] = NUM_VAL (EXPRS_EXPR (array1));
        array1 = EXPRS_NEXT (array1);
    }
    DBUG_ASSERT ((array1 == NULL), "Bv(): bv-arg has wrong dim");

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

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    segs = MakeWLseg (dims, DupTree (cubes, NULL), NULL);
    WLSEG_SV (segs) = CalcSV (segs, WLSEG_SV (segs));

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

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    while (cubes != NULL) {
        /*
         * build new segment
         */
        new_seg = MakeWLseg (dims, DupNode (cubes), NULL);
        new_seg = CalcSV (new_seg, WLSEG_SV (new_seg));
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

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (parms)) == N_num),
                 "Bv(): first argument not an int");

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if ((parms != NULL) && (level <= WLSEG_BLOCKS (seg))) {

        while ((seg != NULL) && (EXPRS_NEXT (parms) != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (parms)) == N_array),
                         "Bv(): bv-arg not an array");

            if (level < WLSEG_BLOCKS (seg)) {
                WLSEG_BV (seg, level) = Array2Bv (parms, WLSEG_BV (seg, level), dims);
            } else {
                WLSEG_UBV (seg) = Array2Bv (parms, WLSEG_UBV (seg), dims);
            }

            seg = WLSTRIDE_NEXT (seg);
            parms = EXPRS_NEXT (parms);
        }

        while (seg != NULL) {

            if (level < WLSEG_BLOCKS (seg)) {
                WLSEG_BV (seg, level) = Array2Bv (parms, WLSEG_BV (seg, level), dims);
            } else {
                WLSEG_UBV (seg) = Array2Bv (parms, WLSEG_UBV (seg), dims);
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

    parms = MakeExprs (MakeNum (WLSEG_BLOCKS (segs)), parms);
    segs = Bv (segs, parms, cubes, dims);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}
