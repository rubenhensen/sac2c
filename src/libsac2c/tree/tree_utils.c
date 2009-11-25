/**
 *
 * $Id$
 *
 * This file contains several utility functions for inspecting or manipulating
 * ASTs or their elements.
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "types.h"
#include "new_types.h"
#include "pattern_match.h"
#include "dbug.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"

/**
 *
 * Functions for dealing with With-Loops:
 *
 * Terminology:
 *
 *   lb: Lower bound (GENERATOR_BOUND1) of a With-Loop generator.
 *
 *   ub: Upper bound (GENERATOR_BOUND2) of a With-Loop generator.
 *
 *   Zero-trip generator:  A generator for which 0 == product( ub - lb)
 *    NB: we name these generators zero-trip rather than empty here
 *        as we want them not to be confused with the one-trip generators
 *        that have empty bounds: ( [:int] <= iv < [:int] ).
 *        For those we have product( ub - lb) = product( []) == 1 !!!
 *
 *    IMPORTANT: we make use of the condition all( lb <= ub) here!!!
 *
 */

/** <!--********************************************************************-->
 *
 * @fn bool TULSisZeroTripGenerator( node *lb, node *ub, node *width)
 *
 * @brief checks for the following 5 criteria:
 *
 *    1) (  a <= iv <  a)    where a::int[n]  with n>0  !
 *    2) ( lb <= iv < ub)    where lb::int[n]{vec1}, ub::int[n]{vec2}
 *                                 and all( vec1 >= vec2)
 *                                 and n>0!
 *    3) ( [l1, ..., ln] <= iv < [u1, ..., un])
 *                           where n > 0
 *                           and exists k>=0 so that:
 *                                 lk and uk are the same variable
 *                                 or lk >= uk
 *    4) ( lb <= iv <= ub width a)
 *                           where a::int[n]{vec} and vec contains a 0
 *    5) ( lb <= iv <= ub width [v1,...,vn])
 *                           where exists i such that vi == 0
 *   In ase any of these is true, it returns TRUE.
 *
 *****************************************************************************/

bool
TULSisZeroTripGenerator (node *lb, node *ub, node *width)
{
    bool res = FALSE;
    pattern *pat1, *pat2, *pat3, *pat4;
    node *a, *x;
    constant *c = NULL;
    int n = 0, i, lk;

    DBUG_ENTER ("TULSisZeroTripGenerator");

    pat1 = PMmulti (2, PMvar (1, PMAgetNode (&a), 0), PMvar (1, PMAisVar (&a), 0));
    pat2 = PMmulti (2, PMconst (1, PMAgetVal (&c)), PMconst (1, PMAanyLeVal (&c)));
    pat3 = PMretryAny (&i, &n, 1,
                       PMmulti (2,
                                PMarray (1, PMAgetLen (&n), 3, PMskipN (&i, 0),
                                         PMvar (1, PMAgetNode (&x), 0), PMskip (0)),
                                PMarray (1, PMAhasLen (&n), 3, PMskipN (&i, 0),
                                         PMvar (1, PMAisVar (&x), 0), PMskip (0))));
    pat4 = PMretryAny (&i, &n, 1,
                       PMmulti (2,
                                PMarray (1, PMAgetLen (&n), 3, PMskipN (&i, 0),
                                         PMint (1, PMAgetIVal (&lk), 0), PMskip (0)),
                                PMarray (1, PMAhasLen (&n), 3, PMskipN (&i, 0),
                                         PMint (1, PMAleIVal (&lk), 0), PMskip (0))));

    DBUG_PRINT ("TULS", ("checking criteria 1, 2, and 3:"));
    if (PMmatchFlat (pat1, PMmultiExprs (2, lb, ub)) && (TUshapeKnown (ID_NTYPE (lb)))
        && (TYgetDim (ID_NTYPE (lb)) == 1)
        && (SHgetExtent (TYgetShape (ID_NTYPE (lb)), 0) > 0)) {
        /**
         * criteria 1) (  a <= iv <  a)    where a::int[n]  with n>0  !
         */
        DBUG_PRINT ("TULS", ("criterion 1 met!"));
        res = TRUE;
    } else if (PMmatchFlat (pat2, PMmultiExprs (2, lb, ub))
               && (SHgetExtent (COgetShape (c), 0) > 0)) {
        /**
         * criteria 2) ( lb <= iv < ub)    where lb::int[n]{vec1},
         *                                       ub::int[n]{vec2},
         *                                       vec1 >= vec2
         *                                       n > 0!
         */
        DBUG_PRINT ("TULS", ("criterion 2 met!"));
        res = TRUE;
    } else if ((PMmatchFlat (pat3, PMmultiExprs (2, lb, ub))
                || PMmatchFlat (pat4, PMmultiExprs (2, lb, ub)))
               && (n > 0)) {
        /**
         * criteria 3) ( [l1, ..., ln] <= iv < [u1, ..., un])
         *             where n > 0
         *             and exists k>=0 so that:
         *                   lk and uk are the same variable
         *                   || lk >= uk
         */
        DBUG_PRINT ("TULS", ("criterion 3 met!"));
        res = TRUE;
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    if (c != NULL) {
        c = COfreeConstant (c);
    }

    if (width != NULL) {
        pattern *pat1, *pat2;
        constant *c = NULL;
        int i, l, zero = 0;

        pat1 = PMconst (1, PMAgetVal (&c));
        pat2 = PMarray (1, PMAgetLen (&l), 1,
                        PMretryAny (&i, &l, 3, PMskipN (&i, 0),
                                    PMint (1, PMAisIVal (&zero)), PMskip (0)));
        DBUG_PRINT ("TULS", ("checking criteria 4 and 5:"));
        if (PMmatchFlat (pat1, width) && COisZero (c, FALSE)) {
            /**
             * criteria 4) ( lb <= iv <= ub width a)
             *              where a::int[n]{vec} and vec contains a 0
             */
            DBUG_PRINT ("TULS", ("criterion 4 met!"));
            res = TRUE;
        } else if (PMmatchFlat (pat2, width)) {
            /**
             * criteria 5) ( lb <= iv <= ub width [v1,...,vn])
             *             where exists i such that vi == 0
             */
            DBUG_PRINT ("TULS", ("criterion 5 met!"));
            res = TRUE;
        }
        pat1 = PMfree (pat1);
        pat2 = PMfree (pat2);
        if (c != NULL) {
            c = COfreeConstant (c);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static bool checkStepWidth( node *generator)
 *
 * @brief Predicate for determining if step (and/or width) is one of:
 *     NULL
 *     step matches width
 *     all 1's
 *
 * @result: TRUE if predicate is satisfied.
 *
 *****************************************************************************/
static bool
checkStepWidth (node *generator)
{
    bool z;
    constant *sw = NULL;
    pattern *pat;

    DBUG_ENTER ("checkStepWidth");
    pat = PMconst (1, PMAgetVal (&sw));
    z = (NULL == GENERATOR_STEP (generator))
        || (GENERATOR_STEP (generator) == GENERATOR_WIDTH (generator))
        || (PMmatchFlat (pat, GENERATOR_STEP (generator)) && COisOne (sw, TRUE));
    sw = (NULL != sw) ? COfreeConstant (sw) : sw;

    z = z
        && ((NULL == GENERATOR_WIDTH (generator))
            || (PMmatchFlat (pat, GENERATOR_WIDTH (generator)) && COisOne (sw, TRUE)));
    sw = (NULL != sw) ? COfreeConstant (sw) : sw;

    PMfree (pat);
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool TULSisFullGenerator( node *generator, node *operator)
 *
 * @brief
 *
 *    Predicate for determining if generator covers index set of
 *    WL result.
 *
 *    Specifically, it checks for the following criteria:
 *
 *    in case of fold, propagate: always considered full!
 *        NB: is only correct when the generator is the ONLY
 *            non-default generator!!
 *            This is the caller's responsbility to check.
 *
 *    in case of  genarray( shp, def) :
 *        ( lb <= iv < shp)    where lb::int[n]{0,...,0}
 *
 *    in case of modarray( a) :
 *        ( lb <= iv < shp)    where lb::int[n]{0,...,0}
 *                                   and a::<xyz>[s1, ..., sm]
 *                                   and shp::int[n]{s1, ..., sn}
 *                                   and sn<=sm (for the case of
 *                                       non-scalar cells) - not checked.
 *        or
 *
 *        ( lb <= iv < shp)    where lb::int[n]{0,...,0}
 *                                   and AVIS_SHAPE( a) = shp
 *
 *    genarray/ modarray: both these cases may have step and width expressions
 *    provided that these satisfy one of the following conditions:
 *       step a          where a::int[n]{1,...,1}
 *       step a width a
 *
 *   In case either of these is true, it returns TRUE.
 *
 *****************************************************************************/

bool
TULSisFullGenerator (node *generator, node *operator)
{
    bool z;
    bool z2 = FALSE;
    constant *lb = NULL;
    constant *ub = NULL;
    constant *arr = NULL;
    constant *shpgen = NULL;
    constant *shpa = NULL;
    constant *shp = NULL;

    pattern *patlb;
    pattern *patub;
    pattern *patarr;

    DBUG_ENTER ("TULSisFullGenerator");

    patlb = PMconst (1, PMAgetVal (&lb));
    patub = PMconst (1, PMAgetVal (&ub));
    patarr = PMconst (1, PMAgetVal (&arr));

    switch (NODE_TYPE (operator)) {

    case N_spfold:
    case N_break:
        z = FALSE;
        DBUG_ASSERT (FALSE, "Should not exist here.");
        break;

    case N_fold:
    case N_propagate:
        z = TRUE;
        break;

    case N_genarray:
        z = PMmatchFlat (patlb, GENERATOR_BOUND1 (generator)) && COisZero (lb, TRUE)
            && (GENERATOR_BOUND2 (generator) == GENARRAY_SHAPE (operator))
            && checkStepWidth (generator);
        break;

    case N_modarray:
        z = PMmatchFlat (patlb, GENERATOR_BOUND1 (generator)) && COisZero (lb, TRUE)
            && checkStepWidth (generator);

        if (PMmatchFlat (patub, GENERATOR_BOUND2 (generator))
            && PMmatchFlat (patarr, MODARRAY_ARRAY (operator))) {
            shpgen = COshape (ub);
            shpa = COshape (arr);
            shp = COtake (shpgen, shpa);
            z2 = COcompareConstants (shp, shpgen);
            COfreeConstant (shpgen);
            COfreeConstant (shpa);
            COfreeConstant (shp);
            COfreeConstant (ub);
            COfreeConstant (arr);
        }

        z = z
            && (z2
                || (GENERATOR_BOUND2 (generator)
                    == AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (operator)))));
        break;

    default:
        z = FALSE;
    }

    PMfree (patlb);
    PMfree (patub);
    PMfree (patarr);
    lb = (NULL != lb) ? COfreeConstant (lb) : NULL;

    DBUG_RETURN (z);
}
