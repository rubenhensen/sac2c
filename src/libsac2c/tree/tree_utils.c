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

#define DBUG_PREFIX "TULS"
#include "debug.h"

#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "free.h"
#include "compare_tree.h"
#include "traverse.h"

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

    DBUG_ENTER ();

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

    DBUG_PRINT ("checking criteria 1, 2, and 3:");
    if (PMmatchFlat (pat1, PMmultiExprs (2, lb, ub)) && (TUshapeKnown (ID_NTYPE (lb)))
        && (TYgetDim (ID_NTYPE (lb)) == 1)
        && (SHgetExtent (TYgetShape (ID_NTYPE (lb)), 0) > 0)) {
        /**
         * criteria 1) (  a <= iv <  a)    where a::int[n]  with n>0  !
         */
        DBUG_PRINT ("criterion 1 met!");
        res = TRUE;
    } else if (PMmatchFlat (pat2, PMmultiExprs (2, lb, ub))
               && (SHgetExtent (COgetShape (c), 0) > 0)) {
        /**
         * criteria 2) ( lb <= iv < ub)    where lb::int[n]{vec1},
         *                                       ub::int[n]{vec2},
         *                                       vec1 >= vec2
         *                                       n > 0!
         */
        DBUG_PRINT ("criterion 2 met!");
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
        DBUG_PRINT ("criterion 3 met!");
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
        DBUG_PRINT ("checking criteria 4 and 5:");
        if (PMmatchFlat (pat1, width) && COisZero (c, FALSE)) {
            /**
             * criteria 4) ( lb <= iv <= ub width a)
             *              where a::int[n]{vec} and vec contains a 0
             */
            DBUG_PRINT ("criterion 4 met!");
            res = TRUE;
        } else if (PMmatchFlat (pat2, width)) {
            /**
             * criteria 5) ( lb <= iv <= ub width [v1,...,vn])
             *             where exists i such that vi == 0
             */
            DBUG_PRINT ("criterion 5 met!");
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

    DBUG_ENTER ();
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
 * @fn bool checkBoundShape( node *arg1, node *arg2)
 * Predicate for arg1 matching arg2
 *
 *****************************************************************************/
static bool
checkBoundShape (node *arg1, node *arg2)
{
    pattern *pat1;
    pattern *pat2;
    node *node_ptr = NULL;
    bool res;

    DBUG_ENTER ();

    pat1 = PMany (1, PMAgetNodeOrAvis (&node_ptr), 0);
    pat2 = PMany (1, PMAisNodeOrAvis (&node_ptr), 0);
    res = PMmatchFlatSkipExtremaAndGuards (pat1, arg1)
          && PMmatchFlatSkipExtremaAndGuards (pat2, arg2);

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
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
    int shpgen;
    constant *lb = NULL;
    node *ub = NULL;
    node *arr = NULL;
    node *shp;
    node *modarrshp;

    pattern *patlb;
    pattern *patub;
    pattern *patarr;

    DBUG_ENTER ();

    patlb = PMconst (1, PMAgetVal (&lb));
    patub = PMarray (1, PMAgetNode (&ub), 0);
    patarr = PMarray (1, PMAgetNode (&arr), 0);

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
        z = PMmatchFlatSkipGuards (patlb, GENERATOR_BOUND1 (generator))
            && COisZero (lb, TRUE)
            && checkBoundShape (GENERATOR_BOUND2 (generator), GENARRAY_SHAPE (operator))
            && checkStepWidth (generator);
        break;

    case N_modarray:
        z = PMmatchFlatSkipGuards (patlb, GENERATOR_BOUND1 (generator))
            && COisZero (lb, TRUE) && checkStepWidth (generator);

        if (PMmatchFlatSkipGuards (patub, GENERATOR_BOUND2 (generator))
            && PMmatchFlatSkipGuards (patarr, MODARRAY_ARRAY (operator))) {

            /* Check for matching frame shapes */
            shpgen = SHgetUnrLen (ARRAY_FRAMESHAPE (ub));
            if (0 == shpgen) {
                z2 = TRUE;
            } else {
                shp = TCtakeDropExprs (shpgen, 0, ARRAY_AELEMS (arr));
                z2 = (CMPT_EQ == CMPTdoCompareTree (shp, ARRAY_AELEMS (ub)));
                FREEdoFreeTree (shp);
            }
        }

        DBUG_ASSERT (N_id == NODE_TYPE (GENERATOR_BOUND2 (generator)),
                     "TULSisFullGenerator wants N_id GENERATOR_BOUND");

        modarrshp = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (operator)));
        DBUG_ASSERT ((NULL == modarrshp) || (N_id == NODE_TYPE (modarrshp)),
                     "TULSisFullGenerator AVIS_SHAPE not flattened");

        z = z && (z2 || (GENERATOR_BOUND2 (generator) == modarrshp));
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

/** <!--********************************************************************-->
 *
 * @fn node TUremoveUnusedCodes(node *codes)
 *
 *   @brief removes all unused N_codes recursively
 *
 *   @param  node *codes : N_code chain
 *   @return node *      : modified N_code chain
 ******************************************************************************/
node *
TUremoveUnusedCodes (node *codes)
{
    DBUG_ENTER ();
    DBUG_ASSERT (codes != NULL, "no codes available!");
    DBUG_ASSERT (NODE_TYPE (codes) == N_code, "type of codes is not N_code!");

    if (CODE_NEXT (codes) != NULL)
        CODE_NEXT (codes) = TUremoveUnusedCodes (CODE_NEXT (codes));

    if (CODE_USED (codes) == 0)
        codes = FREEdoFreeNode (codes);

    DBUG_RETURN (codes);
}

/** <!--********************************************************************-->
 *
 * @fn node *TUmakeIntVec(...)
 *
 * @brief Create one-element integer vector, i.
 *        Return N_avis pointer for same.
 *
 * @param: i: the value of vector.
 *         preassigns: pointer to pointer of preassigns chain.
 *         vardecs:    pointer to pointer of vardecs chain.
 *
 * @result: N_avis node for resulting vector..
 *
 *****************************************************************************/
node *
TUmakeIntVec (int i, node **preassign, node **vardec)
{
    node *selarravis;

    DBUG_ENTER ();
    selarravis = TBmakeAvis (TRAVtmpVar (),
                             TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
    *vardec = TBmakeVardec (selarravis, *vardec);
    *preassign
      = TBmakeAssign (TBmakeLet (TBmakeIds (selarravis, NULL),
                                 TCmakeIntVector (TBmakeExprs (TBmakeNum (i), NULL))),
                      *preassign);
    AVIS_SSAASSIGN (selarravis) = *preassign;

    DBUG_RETURN (selarravis);
}

/** <!--********************************************************************-->
 *
 * @fn node *TUscalarizeVector(...)
 *
 * @brief  Scalarize vector arg_node.
 *         arg_node must be an AKS/AKV vector.
 *         We produce the following:
 *
 *         i0 = [0];
 *         s0 = _sel_VxA_( i0, arg_node);
 *         i1 = [1];
 *         s1 = _sel_VxA_( i1, arg_node);
 *         ...
 *         iz = [n-1];
 *         sz = _sel_VxA_( iz, arg_node);
 *         z = [ s0, s1, ..., sz];
 *
 * @param: arg_node: The N_avis of the vector to be scalarized.
 *         preassigns: pointer to pointer of preassigns chain.
 *         vardecs:    pointer to pointer of vardecs chain.
 *
 * @result: N_avis node for z.
 *
 *****************************************************************************/
node *
TUscalarizeVector (node *arg_node, node **preassigns, node **vardecs)
{
    node *selarravis;
    node *zavis;
    node *asgn;
    ntype *restyp;
    simpletype scalartype;
    node *z = NULL;
    int lim;
    int i;

    DBUG_ENTER ();

    restyp = AVIS_TYPE (arg_node);
    scalartype = TYgetSimpleType (TYgetScalar (restyp));
    DBUG_ASSERT (TYisAKV (restyp) || TYisAKS (restyp), "Expected AKS or AKD restyp");
    DBUG_ASSERT (N_avis == NODE_TYPE (arg_node), "Expected N_avis arg_node");
    lim = SHgetUnrLen (TYgetShape (restyp));

    /* Build i0, s0, i1,s1...,iz,sz */
    for (i = 0; i < lim; i++) {
        selarravis = TUmakeIntVec (i, preassigns, vardecs); /*  i0 = [i] */
        zavis = TBmakeAvis (TRAVtmpVarName ("ausv"),
                            TYmakeAKS (TYmakeSimpleType (scalartype), SHcreateShape (0)));
        *vardecs = TBmakeVardec (zavis, *vardecs);
        asgn = TBmakeAssign (TBmakeLet (TBmakeIds (zavis, NULL),
                                        TCmakePrf2 (F_sel_VxA, TBmakeId (selarravis),
                                                    TBmakeId (arg_node))),
                             NULL);
        *preassigns = TCappendAssign (*preassigns, asgn);
        AVIS_SSAASSIGN (zavis) = asgn;
        z = TCappendExprs (z, TBmakeExprs (TBmakeId (zavis), NULL));
    }

    /* Build N_array result, z */
    zavis = TBmakeAvis (TRAVtmpVarName ("ausv"), TYcopyType (restyp));
    *vardecs = TBmakeVardec (zavis, *vardecs);
    asgn
      = TBmakeAssign (TBmakeLet (TBmakeIds (zavis, NULL),
                                 TCmakeVector (TYmakeAKS (TYmakeSimpleType (scalartype),
                                                          SHcreateShape (0)),
                                               z)),
                      NULL);
    *preassigns = TCappendAssign (*preassigns, asgn);
    AVIS_SSAASSIGN (zavis) = asgn;

    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *TUmoveAssign(node *avis, node *preassigns)
 *
 *   @brief Find N_assign for avis in preassigns chain,
 *          and move it to the end of the preassigns chain.
 *
 *   @param  node *avis: An N_avis for the LHS we want to move.
 *           node *preassigns: A non-NULL N_assign chain that
 *                             contains an assign for N_avis
 *   @return node *      : modified N_assign chain
 *
 *   NB. We only look at the first N_ids element in the LHS.
 *       This is pure laziness on my part.
 *
 ******************************************************************************/
node *
TUmoveAssign (node *avis, node *preassigns)
{
    node *z;
    node *pred = NULL;
    node *ournode;

    DBUG_ENTER ();

    z = preassigns;

    /* Locate avis in assign chain */
    while ((NULL != preassigns) && (AVIS_SSAASSIGN (avis) != preassigns)) {
        pred = preassigns;
        preassigns = ASSIGN_NEXT (preassigns);
    }
    DBUG_ASSERT (NULL != preassigns, "Did not find ournode in preassigns chain");
    ournode = preassigns;

    if (preassigns == z) {
        z = ASSIGN_NEXT (preassigns); /* head-of-chain ournode removed from chain */
    } else {
        /* non-head of chain ournode removed*/
        ASSIGN_NEXT (pred) = ASSIGN_NEXT (ournode);
    }

    ASSIGN_NEXT (ournode) = NULL; /* ournode has no successor */

    /* Find end-of-chain */
    preassigns = pred; /* NULL if ournode was head-of-chain */
    while (NULL != preassigns) {
        pred = preassigns;
        preassigns = ASSIGN_NEXT (preassigns);
    }

    if (NULL != pred) {
        ASSIGN_NEXT (pred) = ournode; /* append ournode assign to end of chain */
    } else {
        z = ournode;
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
