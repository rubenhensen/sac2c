/*
 *
 * $Log$
 * Revision 3.15  2001/03/22 19:19:16  dkr
 * include of tree.h eliminated
 *
 * Revision 3.14  2001/03/20 19:04:53  dkr
 * wlcomp-pragma functions SchedulingWL(), SchedulingSegs() replaced by
 * Scheduling()
 *
 * Revision 3.13  2001/03/20 16:02:55  ben
 * wlcomp-pragma functions SchedulingWL, SchedulingSegs added
 *
 * Revision 3.12  2001/02/06 01:44:47  dkr
 * signature of MakeWLgrid() and MakeWLgridVar() modified
 *
 * Revision 3.11  2001/01/29 18:33:45  dkr
 * some superfluous attributes of N_WLsegVar removed
 *
 * Revision 3.10  2001/01/25 12:06:13  dkr
 * ExtractAplPragmaAp() modified.
 * bug in ExtractNaiveCompPragmaAp() fixed.
 *
 * Revision 3.9  2001/01/24 23:35:22  dkr
 * signature of MakeWLgridVar, MakeWLgrid, MakeWLseg, MakeWLsegVar
 * modified
 *
 * Revision 3.8  2001/01/10 14:27:50  dkr
 * function MakeWLsegX used
 *
 * Revision 3.7  2001/01/09 16:17:23  dkr
 * All() and Cubes() use function AllStridesAreConstant() now
 *
 * Revision 3.6  2001/01/08 16:12:22  dkr
 * support for naive compilation of with-loops added
 *
 * Revision 3.5  2001/01/08 13:40:09  dkr
 * functions ExtractAplPragma... moved from wltransform.c to
 * wlpragma_funs.c
 *
 * Revision 3.4  2001/01/08 12:07:26  dkr
 * no changes done
 *
 * Revision 3.3  2000/12/06 10:57:33  dkr
 * nothing changed
 *
 * Revision 3.2  2000/11/29 13:00:30  dkr
 * no warnings ...
 *
 * Revision 3.1  2000/11/20 18:01:28  sacbase
 * new release made
 *
 * [ eliminated ]
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "free.h"
#include "DupTree.h"
#include "resource.h"
#include "wltransform.h"
#include "scheduling.h"
#include "wlpragma_funs.h"

/******************************************************************************
 ******************************************************************************
 **
 **  Functions for naive-compilation pragma
 **/

/******************************************************************************
 *
 * Function:
 *   bool ExtractNaivePragmaAp( bool *do_naive_comp,
 *                              node *expr, int line);
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
ExtractNaiveCompPragmaAp (bool *do_naive_comp, node *exprs, int line)
{
    node *ap;

    DBUG_ENTER ("ExtractNaiveCompPragmaAp");

    if (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "Illegal wlcomp pragma!");
        ap = EXPRS_EXPR (exprs);
        DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), ("Illegal wlcomp pragma!"));

        if (!strcmp (AP_NAME (ap), "Naive")) {
            if (AP_ARGS (ap) != NULL) {
                ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                              "Naive(): Parameters found"));
            }
            (*do_naive_comp) = TRUE;

            exprs = ExtractNaiveCompPragmaAp (do_naive_comp, FreeNode (exprs), line);
        } else {
            EXPRS_NEXT (exprs)
              = ExtractNaiveCompPragmaAp (do_naive_comp, EXPRS_NEXT (exprs), line);
        }
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * Function:
 *   bool ExtractNaivePragma( node *pragma, int line);
 *
 * Description:
 *
 *
 ******************************************************************************/

bool
ExtractNaiveCompPragma (node *pragma, int line)
{
    bool do_naive_comp = FALSE;

    DBUG_ENTER ("ExtractNaiveCompPragma");

    if (pragma != NULL) {
        PRAGMA_WLCOMP_APS (pragma)
          = ExtractNaiveCompPragmaAp (&do_naive_comp, PRAGMA_WLCOMP_APS (pragma), line);
    }

    DBUG_RETURN (do_naive_comp);
}

/******************************************************************************
 ******************************************************************************
 **
 **  Functions for array-placement pragmas
 **/

/******************************************************************************
 *
 * Function:
 *   node *ExtractAplPragmaAp(node *exprs, node *pragma)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
ExtractAplPragmaAp (node *exprs, node *pragma, int line)
{
    node *ap;
    int size;

    DBUG_ENTER ("ExtractAplPragmaAp");

    if (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "Illegal wlcomp pragma.");
        ap = EXPRS_EXPR (exprs);
        DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), "Illegal wlcomp pragma.");
        if (0 == strcmp (AP_NAME (ap), "APL")) {
            if ((AP_ARGS (ap) == NULL) || (NODE_TYPE (AP_ARG1 (ap)) != N_id)
                || (EXPRS_NEXT (AP_ARGS (ap)) == NULL)
                || (NODE_TYPE (AP_ARG2 (ap)) != N_num)
                || (EXPRS_NEXT (EXPRS_NEXT (AP_ARGS (ap))) == NULL)
                || (NODE_TYPE (AP_ARG3 (ap)) != N_num)) {
                ERROR (line, ("Illegal wlcomp-pragma entry APL found"));
            } else {
                switch (NUM_VAL (AP_ARG3 (ap))) {
                case 1:
                    size = 1024 * config.cache1_size;
                    break;
                case 2:
                    size = 1024 * config.cache2_size;
                    break;
                case 3:
                    size = 1024 * config.cache3_size;
                    break;
                default:
                    size = 0;
                }
                if (size > 0) {
                    NUM_VAL (AP_ARG3 (ap)) = size;
                    PRAGMA_APL (pragma) = ap;
                } else {
                    FreeTree (ap);
                }
            }
            EXPRS_EXPR (exprs) = NULL;

            exprs = ExtractAplPragmaAp (FreeNode (exprs), pragma, line);
        } else {
            EXPRS_NEXT (exprs) = ExtractAplPragmaAp (EXPRS_NEXT (exprs), pragma, line);
        }
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *ExtractAplPragma(node *pragma, int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
ExtractAplPragma (node *pragma, int line)
{
    node *res;

    DBUG_ENTER ("ExtractAplPragma");

    if (pragma != NULL) {
        PRAGMA_WLCOMP_APS (pragma)
          = ExtractAplPragmaAp (PRAGMA_WLCOMP_APS (pragma), pragma, line);
        if (PRAGMA_APL (pragma) != NULL) {
            res = MakePragma ();
            PRAGMA_APL (res) = PRAGMA_APL (pragma);
            PRAGMA_APL (pragma) = NULL;
        } else {
            res = NULL;
        }
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 ******************************************************************************
 **
 **  Here the funs for wlcomp-pragmas are defined.
 **
 **  Each wlcomp-pragma-fun has the signature
 **     node *WLCOMP_Fun( node *segs, node *parms, node *cubes, int dims, int line)
 **  and returns a N_WLseg(Var)-chain with annotated temporary attributes
 **  (BV, UBV, SCHEDULING, ...).
 **
 **  The meaning of the parameters:
 **    - In 'segs' the current segmentation is given. This segmentation is the
 **      basis for the return value. It can be freed and build up again (like in
 **      'Cubes') or can be modified (see 'Bv').
 **    - 'parms' contains an N_exprs-chain with additional parameters for the
 **      function. (E.g. concrete blocking-vectors, ...).
 **    - 'cubes' contains the set of cubes of the current with-loop. This can be
 **      used to build up a new segmentation.
 **      (CAUTION: Do not forget to use DupTree/DupNode. Parts of 'cubes' must
 **      not be shared!!)
 **    - 'dims' gives the number of dimensions of the current with-loop.
 **/

/******************************************************************************
 *
 * Function:
 *   node *IntersectStridesArray( node *strides,
 *                                node *aelems1, node *aelems2,
 *                                int line)
 *
 * Description:
 *   returns the intersection of the N_WLstride-chain 'strides' with
 *    the index vector space ['alemes1', 'aelems2'].
 *    (-> 'aelems1' is the upper, 'aelems2' the lower bound).
 *
 ******************************************************************************/

static node *
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
                    grid2_b1 = grid2_b2 = width; /* dummy value */
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
                                        DupTree (nextdim), new_grids, code);
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
 * Function:
 *   node *Array2Bv( node *array, int *bv, int dims, int line)
 *
 * Description:
 *   converts an N_array node into a blocking vector (int *).
 *
 ******************************************************************************/

static int *
Array2Bv (node *array, int *bv, int dims, int line)
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
 * Function:
 *   node *WLCOMP_All( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   choose the hole array as the only segment;
 *   init blocking vectors ('NoBlocking')
 *
 * caution:
 *   'segs' can contain N_WLstriVar- as well as N_WLgridVar-nodes!!
 *
 ******************************************************************************/

node *
WLCOMP_All (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_All");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "All(): Too many parameters found"));
    }

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    segs = MakeWLsegX (dims, DupTree (cubes), NULL);
    segs = WLCOMP_NoBlocking (segs, parms, cubes, dims, line);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Cubes( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   choose every cube as a segment;
 *   init blocking vectors ('NoBlocking')
 *
 ******************************************************************************/

node *
WLCOMP_Cubes (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *new_seg;
    node *last_seg = NULL;

    DBUG_ENTER ("WLCOMP_Cubes");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Cubes(): Too many parameters found"));
    }

    if (segs != NULL) {
        segs = FreeTree (segs);
    }

    DBUG_ASSERT ((cubes != NULL), "no cubes found!");

    while (cubes != NULL) {
        /*
         * build new segment
         */
        new_seg = MakeWLsegX (dims, DupNode (cubes), NULL);

        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            WLSEGX_NEXT (last_seg) = new_seg;
        }
        last_seg = new_seg;

        cubes = WLSTRIDEX_NEXT (cubes);
    }

    segs = WLCOMP_NoBlocking (segs, parms, cubes, dims, line);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_ConstSegs( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   Defines a new set of segments in reliance on the given extra parameters
 *    'parms'. Each segment is described by two N_array nodes containing
 *    the start, stop index vector, respectively.
 *
 ******************************************************************************/

node *
WLCOMP_ConstSegs (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *new_cubes, *new_seg;
    node *last_seg = NULL;

    DBUG_ENTER ("WLCOMP_ConstSegs");

    if (NODE_TYPE (cubes) == N_WLstride) {
        if (segs != NULL) {
            segs = FreeTree (segs);
        }

        if (parms == NULL) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "ConstSegs(): No arguments found"));
        }

        do {
            DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                         "illegal parameter of wlcomp-pragma found!");

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
                new_seg = MakeWLsegX (dims, new_cubes, NULL);

                if (segs == NULL) {
                    segs = new_seg;
                } else {
                    WLSEGX_NEXT (last_seg) = new_seg;
                }
                last_seg = new_seg;
            }

            parms = EXPRS_NEXT (EXPRS_NEXT (parms));
        } while (parms != NULL);

        segs = WLCOMP_NoBlocking (segs, parms, cubes, dims, line);
    } else {
        WARN (line, ("wlcomp-pragma function ConstSeg() ignored"
                     " because generator is not constant"));
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_NoBlocking( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   sets all blocking vectors and the unrolling-blocking vector to
 *    (1, ..., 1)  ->  no blocking is performed.
 *
 ******************************************************************************/

node *
WLCOMP_NoBlocking (node *segs, node *parms, node *cubes, int dims, int line)
{
    int b, d;
    node *seg = segs;

    DBUG_ENTER ("WLCOMP_NoBlocking");

    if (parms != NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "NoBlocking(): Too many parameters found"));
    }

    while (seg != NULL) {
        /*
         * set ubv
         */
        if (NODE_TYPE (seg) == N_WLseg) {
            if (WLSEG_UBV (seg) == NULL) {
                WLSEG_UBV (seg) = (int *)MALLOC (sizeof (int) * dims);
            }
            for (d = 0; d < dims; d++) {
                (WLSEG_UBV (seg))[d] = 1;
            }

            /*
             * set bv[]
             */
            WLSEG_BLOCKS (seg) = 3; /* three blocking levels */
            for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                if (WLSEG_BV (seg, b) == NULL) {
                    WLSEG_BV (seg, b) = (int *)MALLOC (sizeof (int) * dims);
                }
                for (d = 0; d < dims; d++) {
                    (WLSEG_BV (seg, b))[d] = 1;
                }
            }
        }

        seg = WLSEGX_NEXT (seg);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Bv( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   Changes the blocking-vector for one blocking level (ubv respectively).
 *   The level number is given as first element in 'parms' (N_num);
 *    the rest of 'parms' contains the blocking-vectors for different
 *    segments (N_array).
 *   If ('level' == 'WLSEGX_BLOCKS( seg)'), the ubv is changed.
 *   If 'parms' contains less elements than 'segs', the last bv in
 *    'parms' is mapped to all remaining 'segs'.
 *   If 'parms' contains more elements than 'segs', the tail of 'parms'
 *    is ignored.
 *
 ******************************************************************************/

node *
WLCOMP_Bv (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *seg = segs;
    int level;

    DBUG_ENTER ("WLCOMP_Bv");

    if (parms == NULL) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Bv(): No parameters found"));
    }

    DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                 "illegal parameter of wlcomp-pragma found!");

    if (NODE_TYPE (EXPRS_EXPR (parms)) != N_num) {
        ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                      "Bv(): First argument is not an 'int'"));
    }

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if ((parms != NULL) && (seg != NULL)) {
        while (seg != NULL) {
            if (NODE_TYPE (seg) == N_WLseg) {
                if (NODE_TYPE (EXPRS_EXPR (parms)) != N_array) {
                    ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                                  "Bv(): Blocking-vector is not an array"));
                }

                if (level >= 0) {
                    DBUG_ASSERT ((level < WLSEG_BLOCKS (seg)),
                                 "illegal blocking level found!");
                    WLSEG_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims, line);
                } else {
                    WLSEG_UBV (seg)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims, line);
                }
            } else {
                WARN (line, ("Blocking on variable segments not supported"));
            }

            seg = WLSEG_NEXT (seg);
            if (EXPRS_NEXT (parms) != NULL) {
                parms = EXPRS_NEXT (parms);
            }
        }
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL0( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   uses 'WLCOMP_Bv' to change the blocking-vectors in level 0.
 *
 ******************************************************************************/

node *
WLCOMP_BvL0 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL0");

    parms = MakeExprs (MakeNum (0), parms);
    segs = WLCOMP_Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL1( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   uses 'Bv' to change the blocking-vectors in level 1.
 *
 ******************************************************************************/

node *
WLCOMP_BvL1 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL1");

    parms = MakeExprs (MakeNum (1), parms);
    segs = WLCOMP_Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL2( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   uses 'WLCOMP_Bv' to change the blocking-vectors in level 2.
 *
 ******************************************************************************/

node *
WLCOMP_BvL2 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL2");

    parms = MakeExprs (MakeNum (2), parms);
    segs = WLCOMP_Bv (segs, parms, cubes, dims, line);
    parms = FreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Ubv( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *   uses 'WLCOMP_Bv' to change the unrolling-blocking-vectors.
 *
 ******************************************************************************/

node *
WLCOMP_Ubv (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_Ubv");

    if (segs != NULL) {
        parms = MakeExprs (MakeNum (-1), parms);
        segs = WLCOMP_Bv (segs, parms, cubes, dims, line);
        parms = FreeNode (parms);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Scheduling( node *segs, node *parms, node *cubes, int dims, int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
WLCOMP_Scheduling (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *arg;
    node *seg = segs;

    DBUG_ENTER ("WLCOMP_Scheduling");

    while (seg != NULL) {
        if (parms == NULL) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "Scheduling(): Missing Parameter"));
        }

        DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                     "illegal parameter of wlcomp-pragma found!");

        arg = EXPRS_EXPR (parms);
        if (NODE_TYPE (arg) != N_ap) {
            ABORT (line, ("Illegal argument in wlcomp-pragma found; "
                          "Scheduling(): Argument is not an application"));
        }

        /*
         * set SCHEDULING
         */
        WLSEGX_SCHEDULING (seg) = SCHMakeSchedulingByPragma (arg, line);

        seg = WLSEGX_NEXT (seg);
        if (EXPRS_NEXT (parms) != NULL) {
            parms = EXPRS_NEXT (parms);
        }
    }

    DBUG_RETURN (segs);
}
