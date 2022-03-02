#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "WLTRA"
#include "debug.h"

#include "free.h"
#include "DupTree.h"
#include "resource.h"
#include "wltransform.h"
#include "scheduling.h"
#include "wlpragma_funs.h"
#include "ctinfo.h"
#include "globals.h"
#include "math_utils.h"
#include "memory.h"
#include "vector.h"
#include "str.h"

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
ExtractNaiveCompPragmaAp (bool *do_naive_comp, node *exprs, size_t line)
{
    node *ap;

    DBUG_ENTER ();

    if (exprs != NULL) {
        DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "Illegal wlcomp pragma!");
        ap = EXPRS_EXPR (exprs);
        DBUG_ASSERT (NODE_TYPE (ap) == N_spap, "Illegal wlcomp pragma!");

        if (STReq (SPAP_NAME (ap), "Naive")) {
            if (SPAP_ARGS (ap) != NULL) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
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
ExtractNaiveCompPragma (node *pragma, size_t line)
{
    bool do_naive_comp = FALSE;

    DBUG_ENTER ();

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
ExtractAplPragmaAp (node *exprs, node *pragma, size_t line)
{
    node *ap;
    int size;

    DBUG_ENTER ();

    if (exprs != NULL) {
        DBUG_ASSERT (NODE_TYPE (exprs) == N_exprs, "Illegal wlcomp pragma.");
        ap = EXPRS_EXPR (exprs);
        DBUG_ASSERT (NODE_TYPE (ap) == N_spap, "Illegal wlcomp pragma.");
        if (STReq (SPAP_NAME (ap), "APL")) {
            if ((SPAP_EXPRS1 (ap) == NULL) || (NODE_TYPE (SPAP_ARG1 (ap)) != N_id)
                || (SPAP_EXPRS2 (ap) == NULL) || (NODE_TYPE (SPAP_ARG2 (ap)) != N_num)
                || (SPAP_EXPRS3 (ap) == NULL) || (NODE_TYPE (SPAP_ARG3 (ap)) != N_num)) {
                CTIerror (LINE_TO_LOC (line), "Illegal wlcomp-pragma entry APL found");
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
ExtractAplPragma (node *pragma, size_t line)
{
    node *res;

    DBUG_ENTER ();

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
IntersectStridesArray (node *strides, node *aelems1, node *aelems2, size_t line)
{
    node *isect, *nextdim, *code, *new_grids, *grids;
    int bound1, bound2, step, width, offset, grid1_b1, grid1_b2, grid2_b1, grid2_b2;
    bool empty = FALSE; /* is the intersection empty in the current dim? */

    DBUG_ENTER ();

    isect = NULL;
    if (strides != NULL) {

        DBUG_ASSERT (NODE_TYPE (strides) == N_wlstride, "no stride found");
        DBUG_ASSERT (!WLSTRIDE_ISDYNAMIC (strides), "dynamic stride found");

        if ((aelems1 == NULL) || (aelems2 == NULL)) {
            CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): Argument has wrong dimension");
        }
        if ((NODE_TYPE (EXPRS_EXPR (aelems1)) != N_num)
            || (NODE_TYPE (EXPRS_EXPR (aelems2)) != N_num)) {
            CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): Argument is not an 'int'-array");
        }

        /* compute outline of intersection in current dim */
        bound1
          = MATHmax (NUM_VAL (WLSTRIDE_BOUND1 (strides)), NUM_VAL (EXPRS_EXPR (aelems1)));
        bound2
          = MATHmin (NUM_VAL (WLSTRIDE_BOUND2 (strides)), NUM_VAL (EXPRS_EXPR (aelems2)));

        width = bound2 - bound1;
        step = MATHmin (NUM_VAL (WLSTRIDE_STEP (strides)), width);

        /* compute grids */
        if (width > 0) {
            isect = TBmakeWlstride (WLSTRIDE_LEVEL (strides), WLSTRIDE_DIM (strides),
                                    TBmakeNum (bound1), TBmakeNum (bound2),
                                    TBmakeNum (step), NULL, NULL);

            WLSTRIDE_DOUNROLL (isect) = WLSTRIDE_DOUNROLL (strides);

            new_grids = NULL;
            grids = WLSTRIDE_CONTENTS (strides);
            do {
                /* compute offset for current grid */
                offset = WLTRAgridOffset (bound1, NUM_VAL (WLSTRIDE_BOUND1 (strides)),
                                          NUM_VAL (WLSTRIDE_STEP (strides)),
                                          NUM_VAL (WLGRID_BOUND2 (grids)));

                if (offset <= NUM_VAL (WLGRID_BOUND1 (grids))) {
                    /* grid is still in one pice :) */

                    grid1_b1 = NUM_VAL (WLGRID_BOUND1 (grids)) - offset;
                    grid1_b2 = NUM_VAL (WLGRID_BOUND2 (grids)) - offset;
                    grid2_b1 = grid2_b2 = width; /* dummy value */
                } else {
                    /* the grid is split into two parts :( */

                    /* first part: */
                    grid1_b1 = 0;
                    grid1_b2 = NUM_VAL (WLGRID_BOUND2 (grids)) - offset;
                    /* second part: */
                    grid2_b1 = NUM_VAL (WLGRID_BOUND1 (grids))
                               - (offset - NUM_VAL (WLSTRIDE_STEP (strides)));
                    grid2_b2 = NUM_VAL (WLSTRIDE_STEP (strides));
                }

                nextdim = code = NULL;
                if (grid1_b1 < width) {
                    grid1_b2 = MATHmin (grid1_b2, width);

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
                          = TBmakeWlgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids), code,
                                          TBmakeNum (grid1_b1), TBmakeNum (grid1_b2),
                                          nextdim, new_grids);

                        WLGRID_DOUNROLL (new_grids) = WLGRID_DOUNROLL (grids);
                        CODE_INC_USED (code);
                    }
                }
                if (grid2_b1 < width) {
                    DBUG_ASSERT (grid1_b1 < width, "wrong grid bounds");
                    grid2_b2 = MATHmin (grid2_b2, width);

                    if (!empty) {
                        new_grids
                          = TBmakeWlgrid (WLGRID_LEVEL (grids), WLGRID_DIM (grids), code,
                                          TBmakeNum (grid2_b1), TBmakeNum (grid2_b2),
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
                DBUG_ASSERT (new_grids == NULL, "cubes not consistent");
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
 *   node *Array2BV( node *array, int dims, char *fun_name, int line)
 *
 * Description:
 *   transforms the given N_array node into a proper integer vector to be
 *   used as blocking vector.
 *
 ******************************************************************************/

static node *
Array2Bv (node *array, int dims, char *fun_name, size_t line)
{
    int d;
    node *result, *tmp;

    DBUG_ENTER ();

    tmp = ARRAY_AELEMS (array);
    for (d = 0; d < dims; d++) {
        if (tmp == NULL) {
            CTIabort (LINE_TO_LOC (line),
                          "Illegal argument in wlcomp-pragma found;"
                          " %s(): Blocking vector has wrong dimension",
                          fun_name);
        }
        if (NODE_TYPE (EXPRS_EXPR (tmp)) != N_num) {
            CTIabort (LINE_TO_LOC (line),
                          "Illegal argument in wlcomp-pragma found;"
                          " %s(): Blocking vector is not an 'int'-array",
                          fun_name);
        }
        tmp = EXPRS_NEXT (tmp);
    }
    if (tmp != NULL) {
        CTIabort (LINE_TO_LOC (line),
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): Blocking vector has wrong dimension",
                      fun_name);
    }

    result = TCmakeIntVector (DUPdoDupTree (ARRAY_AELEMS (array)));

    DBUG_RETURN (result);
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
StoreBv (node *segs, node *parms, node *cubes, int dims, char *fun_name, size_t line)
{
    node *seg = segs;
    node *abv;
    int level;

    DBUG_ENTER ();

    if (parms == NULL) {
        CTIabort (LINE_TO_LOC (line),
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): No parameters found",
                      fun_name);
    }

    DBUG_ASSERT (NODE_TYPE (parms) == N_exprs,
                 "illegal parameter of wlcomp-pragma found!");

    if (NODE_TYPE (EXPRS_EXPR (parms)) != N_num) {
        CTIabort (LINE_TO_LOC (line),
                      "Illegal argument in wlcomp-pragma found;"
                      " %s(): First argument is not an 'int'",
                      fun_name);
    }

    level = NUM_VAL (EXPRS_EXPR (parms));
    parms = EXPRS_NEXT (parms);

    if ((parms != NULL) && (seg != NULL)) {
        while (seg != NULL) {
            if (WLSEG_ISDYNAMIC (seg)) {
                CTIwarnLine (line,
                             "wlcomp-pragma function %s() ignored"
                             " because generator is not constant",
                             fun_name);
            } else {
                if (NODE_TYPE (EXPRS_EXPR (parms)) != N_array) {
                    CTIabort (LINE_TO_LOC (line),
                                  "Illegal argument in wlcomp-pragma found;"
                                  " %s(): Blocking-vector is not an array",
                                  fun_name);
                }

                if (level >= 0) {
                    unsigned int level_u = (unsigned int)level;
                    DBUG_ASSERT (level_u < WLSEG_BLOCKS (seg),
                                 "illegal blocking level found!");

                    abv = TCgetNthExprs (level_u, WLSEG_BV (seg));
                    EXPRS_EXPR (abv) = FREEdoFreeTree (EXPRS_EXPR (abv));

                    EXPRS_EXPR (abv)
                      = Array2Bv (EXPRS_EXPR (parms), dims, fun_name, line);
                } else {
                    WLSEG_UBV (seg) = FREEdoFreeTree (WLSEG_UBV (seg));
                    WLSEG_UBV (seg) = Array2Bv (EXPRS_EXPR (parms), dims, fun_name, line);
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
 *   choose the whole array as the only segment;
 *   init blocking vectors ('NoBlocking')
 *
 * caution:
 *   'segs' can contain dynamic N_wlstride- as well as dynamic N_wlgrid-nodes!!
 *
 ******************************************************************************/

node *
WLCOMP_All (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    DBUG_ENTER ();

    if (parms != NULL) {
        CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                            " All(): Too many parameters found");
    }

    if (segs != NULL) {
        segs = FREEdoFreeTree (segs);
    }

    segs = TBmakeWlseg (dims, DUPdoDupTree (cubes), NULL);
    WLSEG_ISDYNAMIC (segs)
      = !WLTRAallStridesAreConstant (WLSEG_CONTENTS (segs), TRUE, TRUE);
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
WLCOMP_Cubes (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    node *new_seg;
    node *last_seg = NULL;

    DBUG_ENTER ();

    if (parms != NULL) {
        CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                            " Cubes(): Too many parameters found");
    }

    if (segs != NULL) {
        segs = FREEdoFreeTree (segs);
    }

    DBUG_ASSERT (cubes != NULL, "no cubes found!");

    while (cubes != NULL) {
        /*
         * build new segment
         */
        new_seg = TBmakeWlseg (dims, DUPdoDupNode (cubes), NULL);
        WLSEG_ISDYNAMIC (new_seg)
          = !WLTRAallStridesAreConstant (WLSEG_CONTENTS (new_seg), TRUE, TRUE);

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
WLCOMP_ConstSegs (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    node *new_cubes, *new_seg;
    node *last_seg = NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (cubes) != N_wlstride) {
        CTIwarnLine (line, "wlcomp-pragma function ConstSeg() ignored"
                           " because generator is not constant");
    } else {
        if (segs != NULL) {
            segs = FREEdoFreeTree (segs);
        }

        if (parms == NULL) {
            CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                " ConstSegs(): No arguments found");
        }

        do {
            DBUG_ASSERT (NODE_TYPE (parms) == N_exprs,
                         "illegal parameter of wlcomp-pragma found!");

            if (EXPRS_NEXT (parms) == NULL) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " ConstSegs(): Upper bound not found");
            }
            if ((NODE_TYPE (EXPRS_EXPR1 (parms)) != N_array)
                || (NODE_TYPE (EXPRS_EXPR2 (parms)) != N_array)) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " ConstSegs(): Argument is not an array");
            }

            new_cubes = IntersectStridesArray (cubes, ARRAY_AELEMS (EXPRS_EXPR1 (parms)),
                                               ARRAY_AELEMS (EXPRS_EXPR2 (parms)), line);

            if (new_cubes != NULL) {
                new_seg = TBmakeWlseg (dims, new_cubes, NULL);
                WLSEG_ISDYNAMIC (new_seg)
                  = !WLTRAallStridesAreConstant (WLSEG_CONTENTS (new_seg), TRUE, TRUE);

                if (segs == NULL) {
                    segs = new_seg;
                } else {
                    WLSEG_NEXT (last_seg) = new_seg;
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
WLCOMP_NoBlocking (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    unsigned int b;
    node *seg = segs;

    DBUG_ENTER ();

    if (parms != NULL) {
        CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                            " NoBlocking(): Too many parameters found");
    }

    while (seg != NULL) {
        /*
         * set ubv
         */
        if (!WLSEG_ISDYNAMIC (seg)) {
            WLSEG_UBV (seg) = TCcreateIntVector (WLSEG_DIMS (seg), 1, 0);

            /*
             * set bv[]
             */
            WLSEG_BLOCKS (seg) = 3; /* three blocking levels */
            for (b = 0; b < WLSEG_BLOCKS (seg); b++) {
                WLSEG_BV (seg) = TBmakeExprs (TCcreateIntVector (WLSEG_DIMS (seg), 1, 0),
                                              WLSEG_BV (seg));
            }
        }

        seg = WLSEG_NEXT (seg);
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
WLCOMP_BvL0 (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    DBUG_ENTER ();

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
WLCOMP_BvL1 (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    DBUG_ENTER ();

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
WLCOMP_BvL2 (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    DBUG_ENTER ();

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
WLCOMP_Ubv (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    DBUG_ENTER ();

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
WLCOMP_Scheduling (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    node *arg;
    node *seg = segs;
    char *strategy;
    char *name;

    DBUG_ENTER ();

    if (global.mtmode == MT_none) {
        CTIwarnLine (line, "wlcomp-pragma function Scheduling() ignored"
                           " because multi-threading is inactive");
    } else {
        while (seg != NULL) {
            if (parms == NULL) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " Scheduling(): Missing Parameter");
            }

            DBUG_ASSERT (NODE_TYPE (parms) == N_exprs,
                         "illegal parameter of wlcomp-pragma found!");

            arg = EXPRS_EXPR (parms);
            if (NODE_TYPE (arg) != N_spap) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " Scheduling(): Argument is not an application");
            }

            /*
             * set SCHEDULING
             */
            if (WLSEG_SCHEDULING (seg) != NULL) {
                WLSEG_SCHEDULING (seg) = SCHremoveScheduling (WLSEG_SCHEDULING (seg));
            }
            WLSEG_SCHEDULING (seg) = SCHmakeSchedulingByPragma (arg, line);

            /* check dimension of task selector */
            name = SPID_NAME (SPAP_ID (arg));
            if (STReq (name, "Self")) {

                /* check if Self scheduler argument is valid */
                strategy = SPID_NAME (SPAP_ARG1 (arg));
                if (!STReq (strategy, "FirstStatic") && !STReq (strategy, "FirstDynamic")
                    && !STReq (strategy, "FirstAutomatic")) {
                    CTIerror (LINE_TO_LOC (line),
                              "Scheduler Self needs one of the following strategies"
                              " for his first task: FirstStatic, FirstDynamic,"
                              " FirstAutomatic");
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
 *   node *WLCOMP_Tasksel( node *segs, node *parms, node *cubes, int dims,
 *                            int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
WLCOMP_Tasksel (node *segs, node *parms, node *cubes, int dims, size_t line)
{
    node *arg;
    node *seg = segs;
    int dim;
    char *name;

    DBUG_ENTER ();

    if (global.mtmode == MT_none) {
        CTIwarnLine (line, "wlcomp-pragma function Tasksel() ignored"
                           " because multi-threading is inactive");
    } else {
        while (seg != NULL) {
            if (parms == NULL) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " Tasksel(): Missing Parameter");
            }

            DBUG_ASSERT (NODE_TYPE (parms) == N_exprs,
                         "illegal parameter of wlcomp-pragma found!");

            arg = EXPRS_EXPR (parms);
            if (NODE_TYPE (arg) != N_spap) {
                CTIabort (LINE_TO_LOC (line), "Illegal argument in wlcomp-pragma found;"
                                    " Tasksel(): Argument is not an application");
            }

            /*
             * set TaskSel
             */

            if (WLSEG_TASKSEL (seg) != NULL) {
                WLSEG_TASKSEL (seg) = SCHremoveTasksel (WLSEG_TASKSEL (seg));
            }

            WLSEG_TASKSEL (seg) = SCHmakeTaskselByPragma (arg, line);

            /* check dimension of task selector */
            name = SPID_NAME (SPAP_ID (arg));
            if (STReq (name, "Even") || STReq (name, "Factoring")) {
                dim = NUM_VAL (SPAP_ARG1 (arg));

                // negative numbers don't get parsed apparently, so not checking for that
                if (dim >= dims) {
                    CTIerror (LINE_TO_LOC (line),
                              "Task Distribution Dimension should be between 0 and"
                              " the dimension of the withloop");
                }

                /* check tasks_per_thread too? */
            }

            seg = WLSEG_NEXT (seg);

            if (EXPRS_NEXT (parms) != NULL) {
                parms = EXPRS_NEXT (parms);
            }
        }
    }

    DBUG_RETURN (segs);
}

#undef DBUG_PREFIX
