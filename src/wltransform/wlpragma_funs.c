/* $Id$ */

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
#include "ctinfo.h"

#include <string.h>

/******************************************************************************
 ******************************************************************************
 **
 **  Functions for naive-compilation pragma
 **/

/******************************************************************************
 *
 * Function:
 *   bool ExtractNaivePragmaAp( bool *do_naive_comp, node *expr, int line);
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
        DBUG_ASSERT ((NODE_TYPE (ap) == N_spap), ("Illegal wlcomp pragma!"));

        if (!strcmp (SPAP_NAME (ap), "Naive")) {
            if (SPAP_ARGS (ap) != NULL) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " Naive(): Parameters found");
            }
            (*do_naive_comp) = TRUE;

            exprs
              = ExtractNaiveCompPragmaAp (do_naive_comp, FREEdoFreeNode (exprs), line);
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
 *   node *ExtractAplPragmaAp(node *exprs, node *pragma, int line)
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
        DBUG_ASSERT ((NODE_TYPE (ap) == N_spap), "Illegal wlcomp pragma.");
        if (0 == strcmp (SPAP_NAME (ap), "APL")) {
            if ((SPAP_EXPRS1 (ap) == NULL) || (NODE_TYPE (SPAP_ARG1 (ap)) != N_id)
                || (SPAP_EXPRS2 (ap) == NULL) || (NODE_TYPE (SPAP_ARG2 (ap)) != N_num)
                || (SPAP_EXPRS3 (ap) == NULL) || (NODE_TYPE (SPAP_ARG3 (ap)) != N_num)) {
                CTIerrorLine (line, "Illegal wlcomp-pragma entry APL found");
            } else {
                switch (NUM_VAL (SPAP_ARG3 (ap))) {
                case 1:
                    size = 1024 * global.config.cache1_size;
                    break;
                case 2:
                    size = 1024 * global.config.cache2_size;
                    break;
                case 3:
                    size = 1024 * global.config.cache3_size;
                    break;
                default:
                    size = 0;
                }
                if (size > 0) {
                    NUM_VAL (SPAP_ARG3 (ap)) = size;
                    PRAGMA_APL (pragma) = ap;
                } else {
                    FREEdoFreeTree (ap);
                }
            }
            EXPRS_EXPR (exprs) = NULL;

            exprs = ExtractAplPragmaAp (FREEdoFreeNode (exprs), pragma, line);
        } else {
            EXPRS_NEXT (exprs) = ExtractAplPragmaAp (EXPRS_NEXT (exprs), pragma, line);
        }
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *ExtractAplPragma( node *pragma, int line)
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
            res = TBmakePragma ();
            /* TODO check arguments of TBmakePragma - former call was MakePragma();*/
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
 **  All wlcomp-pragma-funs have the signature
 **    node *WLCOMP_Fun( node *segs, node *parms, node *cubes, int dims,
 **                      int line)
 **  and return a N_wlseg(Var)-chain with annotated temporary attributes
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
 *   returns the intersection of the N_wlstride-chain 'strides' with
 *    the index vector space ['alemes1', 'aelems2'].
 *    (-> 'aelems1' is the upper, 'aelems2' the lower bound).
 *
 ******************************************************************************/

static node *
IntersectStridesArray (node *strides, node *aelems1, node *aelems2, int line)
{
    node *isect, *nextdim, *code, *new_grids, *grids;
    int bound1, bound2, step, width, offset, grid1_b1, grid1_b2, grid2_b1, grid2_b2;
    bool empty = FALSE; /* is the intersection empty in the current dim? */

    DBUG_ENTER ("IntersectStridesArray");

    isect = NULL;
    if (strides != NULL) {

        DBUG_ASSERT ((NODE_TYPE (strides) == N_wlstride), "no constant stride found");

        if ((aelems1 == NULL) || (aelems2 == NULL)) {
            CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): Argument has wrong dimension");
        }
        if ((NODE_TYPE (EXPRS_EXPR (aelems1)) != N_num)
            || (NODE_TYPE (EXPRS_EXPR (aelems2)) != N_num)) {
            CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): Argument is not an 'int'-array");
        }

        /* compute outline of intersection in current dim */
        bound1 = MAX (WLSTRIDE_BOUND1 (strides), NUM_VAL (EXPRS_EXPR (aelems1)));
        bound2 = MIN (WLSTRIDE_BOUND2 (strides), NUM_VAL (EXPRS_EXPR (aelems2)));

        width = bound2 - bound1;
        step = MIN (WLSTRIDE_STEP (strides), width);

        /* compute grids */
        if (width > 0) {
            isect = TBmakeWlstride (WLSTRIDE_LEVEL (strides), WLSTRIDE_DIM (strides),
                                    bound1, bound2, step, NULL, NULL);

            WLSTRIDE_DOUNROLL (isect) = WLSTRIDE_DOUNROLL (strides);

            new_grids = NULL;
            grids = WLSTRIDE_CONTENTS (strides);
            do {
                /* compute offset for current grid */
                offset = WLTRAgridOffset (bound1, WLSTRIDE_BOUND1 (strides),
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
                            empty = TRUE;
                        }
                    } else {
                        code = WLGRID_CODE (grids);
                    }

                    if (!empty) {
                        new_grids
                          = TBmakeWlgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids),
                                          grid1_b1, grid1_b2, code, nextdim, new_grids);

                        WLGRID_DOUNROLL (new_grids) = WLGRID_DOUNROLL (grids);
                        CODE_INC_USED (code);
                    }
                }
                if (grid2_b1 < width) {
                    DBUG_ASSERT ((grid1_b1 < width), "wrong grid bounds");
                    grid2_b2 = MIN (grid2_b2, width);

                    if (!empty) {
                        new_grids
                          = TBmakeWlgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids),
                                          grid2_b1, grid2_b2, code,
                                          DUPdoDupTree (nextdim), new_grids);

                        WLGRID_DOUNROLL (new_grids) = WLGRID_DOUNROLL (grids);
                        CODE_INC_USED (code);
                    }
                }

                grids = WLGRID_NEXT (grids);
            } while ((grids != NULL) && (!empty));

            if (!empty) {
                /* sorted insertion of new grids */
                WLSTRIDE_CONTENTS (isect)
                  = WLTRAinsertWlNodes (WLSTRIDE_CONTENTS (isect), new_grids);
            } else {
                /* next dim is empty -> erase current dim */
                DBUG_ASSERT ((new_grids == NULL), "cubes not consistent");
                isect = FREEdoFreeTree (isect);
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
 *   node *Array2Bv( node *array, int *bv, int dims,
 *                   char *fun_name, int line)
 *
 * Description:
 *   converts an N_array node into a blocking vector (int *).
 *
 ******************************************************************************/

static int *
Array2Bv (node *array, int *bv, int dims, char *fun_name, int line)
{
    int d;

    DBUG_ENTER ("Array2Bv");

    array = ARRAY_AELEMS (array);
    for (d = 0; d < dims; d++) {
        if (array == NULL) {
            CTIabortLine (line,
                          "Illegal argument in wlcomp-pragma found;"
                          " %s(): Blocking vector has wrong dimension",
                          fun_name);
        }
        if (NODE_TYPE (EXPRS_EXPR (array)) != N_num) {
            CTIabortLine (line,
                          "Illegal argument in wlcomp-pragma found;"
                          " %s(): Blocking vector is not an 'int'-array",
                          fun_name);
        }
        bv[d] = NUM_VAL (EXPRS_EXPR (array));
        array = EXPRS_NEXT (array);
    }
    if (array != NULL) {
        CTIabortLine (line,
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): Blocking vector has wrong dimension",
                      fun_name);
    }

    DBUG_RETURN (bv);
}

/******************************************************************************
 *
 * Function:
 *   node *StoreBv( node *segs, node *parms, node *cubes, int dims,
 *                  char *fun_name, int line)
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

static node *
StoreBv (node *segs, node *parms, node *cubes, int dims, char *fun_name, int line)
{
    node *seg = segs;
    int level;

    DBUG_ENTER ("StoreBv");

    if (parms == NULL) {
        CTIabortLine (line,
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): No parameters found",
                      fun_name);
    }

    DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                 "illegal parameter of wlcomp-pragma found!");

    if (NODE_TYPE (EXPRS_EXPR (parms)) != N_num) {
        CTIabortLine (line,
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): First argument is not an 'int'",
                      fun_name);
    }

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if ((parms != NULL) && (seg != NULL)) {
        while (seg != NULL) {
            if (NODE_TYPE (seg) != N_wlseg) {
                CTIwarnLine (line,
                             "wlcomp-pragma function %s() ignored"
                             " because generator is not constant",
                             fun_name);
            } else {
                if (NODE_TYPE (EXPRS_EXPR (parms)) != N_array) {
                    CTIabortLine (line,
                                  "Illegal argument in wlcomp-pragma found;"
                                  " %s(): Blocking-vector is not an array",
                                  fun_name);
                }

                if (level >= 0) {
                    DBUG_ASSERT ((level < WLSEG_BLOCKS (seg)),
                                 "illegal blocking level found!");

                    WLSEG_BV (seg, level)
                      = Array2Bv (EXPRS_EXPR (parms), WLSEG_BV (seg, level), dims,
                                  fun_name, line);
                } else {
                    WLSEG_UBV (seg) = Array2Bv (EXPRS_EXPR (parms), WLSEG_UBV (seg), dims,
                                                fun_name, line);
                }
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
 *   node *WLCOMP_All( node *segs, node *parms, node *cubes, int dims,
 *                     int line)
 *
 * Description:
 *   choose the hole array as the only segment;
 *   init blocking vectors ('NoBlocking')
 *
 * caution:
 *   'segs' can contain N_wlstriVar- as well as N_wlgridVar-nodes!!
 *
 ******************************************************************************/

node *
WLCOMP_All (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_All");

    if (parms != NULL) {
        CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                            " All(): Too many parameters found");
    }

    if (segs != NULL) {
        segs = FREEdoFreeTree (segs);
    }

    segs = TCmakeWlSegX (dims, DUPdoDupTree (cubes), NULL);
    segs = WLCOMP_NoBlocking (segs, parms, cubes, dims, line);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Cubes( node *segs, node *parms, node *cubes, int dims,
 *                       int line)
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
        CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                            " Cubes(): Too many parameters found");
    }

    if (segs != NULL) {
        segs = FREEdoFreeTree (segs);
    }

    DBUG_ASSERT ((cubes != NULL), "no cubes found!");

    while (cubes != NULL) {
        /*
         * build new segment
         */
        new_seg = TCmakeWlSegX (dims, DUPdoDupNode (cubes), NULL);

        /*
         * append 'new_seg' at 'segs'
         */
        if (segs == NULL) {
            segs = new_seg;
        } else {
            L_WLSEGX_NEXT (last_seg, new_seg);
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
 *   node *WLCOMP_ConstSegs( node *segs, node *parms, node *cubes, int dims,
 *                           int line)
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

    if (NODE_TYPE (cubes) != N_wlstride) {
        CTIwarnLine (line, "wlcomp-pragma function ConstSeg() ignored"
                           " because generator is not constant");
    } else {
        if (segs != NULL) {
            segs = FREEdoFreeTree (segs);
        }

        if (parms == NULL) {
            CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): No arguments found");
        }

        do {
            DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                         "illegal parameter of wlcomp-pragma found!");

            if (EXPRS_NEXT (parms) == NULL) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " ConstSegs(): Upper bound not found");
            }
            if ((NODE_TYPE (EXPRS_EXPR1 (parms)) != N_array)
                || (NODE_TYPE (EXPRS_EXPR2 (parms)) != N_array)) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " ConstSegs(): Argument is not an array");
            }

            new_cubes = IntersectStridesArray (cubes, ARRAY_AELEMS (EXPRS_EXPR1 (parms)),
                                               ARRAY_AELEMS (EXPRS_EXPR2 (parms)), line);

            if (new_cubes != NULL) {
                new_seg = TCmakeWlSegX (dims, new_cubes, NULL);

                if (segs == NULL) {
                    segs = new_seg;
                } else {
                    L_WLSEGX_NEXT (last_seg, new_seg);
                }
                last_seg = new_seg;
            }

            parms = EXPRS_EXPRS3 (parms);
        } while (parms != NULL);

        segs = WLCOMP_NoBlocking (segs, parms, cubes, dims, line);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_NoBlocking( node *segs, node *parms, node *cubes, int dims,
 *                            int line)
 *
 * Description:
 *   sets all blocking vectors and the unrolling-blocking vector to
 *    (1, ..., 1)  ->  no blocking is performed.
 *
 ******************************************************************************/

node *
WLCOMP_NoBlocking (node *segs, node *parms, node *cubes, int dims, int line)
{
    int b;
    node *seg = segs;

    DBUG_ENTER ("WLCOMP_NoBlocking");

    if (parms != NULL) {
        CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                            " NoBlocking(): Too many parameters found");
    }

    while (seg != NULL) {
        /*
         * set ubv
         */
        if (NODE_TYPE (seg) == N_wlseg) {
            MALLOC_INIT_VECT (WLSEG_UBV (seg), WLSEGX_DIMS (seg), int, 1);

            /*
             * set bv[]
             */
            WLSEG_BLOCKS (seg) = 3; /* three blocking levels */
            for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                MALLOC_INIT_VECT (WLSEG_BV (seg, b), WLSEGX_DIMS (seg), int, 1);
            }
        }

        seg = WLSEGX_NEXT (seg);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL0( node *segs, node *parms, node *cubes, int dims,
 *                      int line)
 *
 * Description:
 *   uses 'StoreBv()' to change the blocking-vectors in level 0.
 *
 ******************************************************************************/

node *
WLCOMP_BvL0 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL0");

    parms = TBmakeExprs (TBmakeNum (0), parms);
    segs = StoreBv (segs, parms, cubes, dims, "BvL0", line);
    parms = FREEdoFreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL1( node *segs, node *parms, node *cubes, int dims,
 *                      int line)
 *
 * Description:
 *   uses 'StoreBv()' to change the blocking-vectors in level 1.
 *
 ******************************************************************************/

node *
WLCOMP_BvL1 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL1");

    parms = TBmakeExprs (TBmakeNum (1), parms);
    segs = StoreBv (segs, parms, cubes, dims, "BvL1", line);
    parms = FREEdoFreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_BvL2( node *segs, node *parms, node *cubes, int dims,
 *                      int line)
 *
 * Description:
 *   uses 'StoreBv()' to change the blocking-vectors in level 2.
 *
 ******************************************************************************/

node *
WLCOMP_BvL2 (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_BvL2");

    parms = TBmakeExprs (TBmakeNum (2), parms);
    segs = StoreBv (segs, parms, cubes, dims, "BvL2", line);
    parms = FREEdoFreeNode (parms);

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Ubv( node *segs, node *parms, node *cubes, int dims,
 *                     int line)
 *
 * Description:
 *   uses 'StoreBv()' to change the unrolling-blocking-vectors.
 *
 ******************************************************************************/

node *
WLCOMP_Ubv (node *segs, node *parms, node *cubes, int dims, int line)
{
    DBUG_ENTER ("WLCOMP_Ubv");

    if (segs != NULL) {
        parms = TBmakeExprs (TBmakeNum (-1), parms);
        segs = StoreBv (segs, parms, cubes, dims, "Ubv", line);
        parms = FREEdoFreeNode (parms);
    }

    DBUG_RETURN (segs);
}

/******************************************************************************
 *
 * Function:
 *   node *WLCOMP_Scheduling( node *segs, node *parms, node *cubes, int dims,
 *                            int line)
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

    if (global.mtmode == MT_none) {
        CTIwarnLine (line, "wlcomp-pragma function Scheduling() ignored"
                           " because multi-threading is inactive");
    } else {
        while (seg != NULL) {
            if (parms == NULL) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " Scheduling(): Missing Parameter");
            }

            DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                         "illegal parameter of wlcomp-pragma found!");

            arg = EXPRS_EXPR (parms);
            if (NODE_TYPE (arg) != N_spap) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " Scheduling(): Argument is not an application");
            }

            /*
             * set SCHEDULING
             */
            if (WLSEGX_SCHEDULING (seg) != NULL) {
                L_WLSEGX_SCHEDULING (seg, SCHremoveScheduling (WLSEGX_SCHEDULING (seg)));
            }
            L_WLSEGX_SCHEDULING (seg, SCHmakeSchedulingByPragma (arg, line));

            seg = WLSEGX_NEXT (seg);
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
 *   node *WLCOMP_Tasksel( node *segs, node *parms, node *cubes, int dims,
 *                            int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
WLCOMP_Tasksel (node *segs, node *parms, node *cubes, int dims, int line)
{
    node *arg;
    node *seg = segs;

    DBUG_ENTER ("WLCOMP_Tasksel");

    if (global.mtmode == MT_none) {
        CTIwarnLine (line, "wlcomp-pragma function Tasksel() ignored"
                           " because multi-threading is inactive");
    } else {
        while (seg != NULL) {
            if (parms == NULL) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " Tasksel(): Missing Parameter");
            }

            DBUG_ASSERT ((NODE_TYPE (parms) == N_exprs),
                         "illegal parameter of wlcomp-pragma found!");

            arg = EXPRS_EXPR (parms);
            if (NODE_TYPE (arg) != N_spap) {
                CTIabortLine (line, "Illegal argument in wlcomp-pragma found;"
                                    " Tasksel(): Argument is not an application");
            }

            /*
             * set TaskSel
             */

            if (WLSEGX_TASKSEL (seg) != NULL) {
                L_WLSEGX_TASKSEL (seg, SCHremoveTasksel (WLSEGX_TASKSEL (seg)));
            }

            L_WLSEGX_TASKSEL (seg, SCHmakeTaskselByPragma (arg, line));

            seg = WLSEGX_NEXT (seg);

            if (EXPRS_NEXT (parms) != NULL) {
                parms = EXPRS_NEXT (parms);
            }
        }
    }

    DBUG_RETURN (segs);
}
