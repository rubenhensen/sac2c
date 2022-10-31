/**
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
#include "free_lhs_avis_sons.h"
#include "symbolic_constant_simplification.h"

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
 * @fn bool isZeroTripGeneratorComposition( node *lb, node *ub)
 *
 * @brief Predicate for determining if WL generator bounds lb and ub
 *        are zero-trip, when we need to analyze a function composition to do so.
 *
 *        lb and ub must be N_id or N_array nodes.
 *
 *        A typical case shows up in the wlcondoAPL.sac unit test,
 *        where one partition looks like this:
 *
 *          lb = _max_SxS_( x, N);
 *          ub = N;
 *          z = with {
 *            ( [ lb] <= iv < [ ub]) ...
 *            }
 *
 *        We have a zero-trip generator if lb >= ub. Plugging
 *        the arguments into the relational gives us:
 *
 *           _max_SxS_( x, N) >= N
 *
 *        which can be solved by SCS. Two other ways we could
 *        solve this in a cleaner, more general manner are:
 *
 *          - extend extrema past the point where GENWIDTH is
 *            introduced. Were that done, a trip through IVEXP and CF
 *            should provide extrema for the GENWIDTH value, which
 *            we could examine here.
 *
 *          - Introduce a new Boolean N_generator field, similar to GENWIDTH,
 *            but containing the desired relational: lb >= ub.
 *
 *
 * @result: TRUE if the relational above is satisfied for
 *          any generator element pairs.
 *
 *****************************************************************************/
static bool
isZeroTripGeneratorComposition (node *lb, node *ub)
{
    bool z = FALSE;
    pattern *patlb;
    pattern *patub;
    node *aelemslb = NULL;
    node *aelemsub = NULL;
    node *ellb;
    node *elub;
    constant *fslb = NULL;
    constant *fsub = NULL;
    int lenlb = 0;
    int lenub = 0;
    bool relres;

    DBUG_ENTER ();

    patlb = PMarray (1, PMAgetFS (&fslb), 1, PMskip (1, PMAgetNode (&aelemslb)));
    patub = PMarray (1, PMAgetFS (&fsub), 1, PMskip (1, PMAgetNode (&aelemsub)));
    PMmatchFlat (patlb, lb);
    PMmatchFlat (patub, ub);

    if ((NULL == aelemslb) && (N_array == NODE_TYPE (lb))) {
        aelemslb = ARRAY_AELEMS (lb);
        lenlb = SHgetUnrLen (ARRAY_FRAMESHAPE (lb));
    }
    if (NULL != fslb) {
        lenlb = SHgetUnrLen (COgetShape (fslb));
    }

    if ((NULL == aelemsub) && (N_array == NODE_TYPE (ub))) {
        aelemsub = ARRAY_AELEMS (ub);
        lenub = SHgetUnrLen (ARRAY_FRAMESHAPE (ub));
    }
    if (NULL != fsub) {
        lenub = SHgetUnrLen (COgetShape (fsub));
    }

    // shapes must match, but eschew [:int]
    if ((0 != lenlb) && (0 != lenub) && (lenlb == lenub)) {
        while ((!z) && (NULL != aelemslb)) {
            ellb = EXPRS_EXPR (aelemslb);
            elub = EXPRS_EXPR (aelemsub);
            if ((N_id == NODE_TYPE (ellb)) && // We may have non-flattened bounds!
                (N_id == NODE_TYPE (elub))
                && SCScanOptGEOnDyadicFn (ellb, elub, &relres)) {
                z = relres;
            }
            aelemslb = EXPRS_NEXT (aelemslb);
            aelemsub = EXPRS_NEXT (aelemsub);
        }
    }

    patlb = PMfree (patlb);
    patub = PMfree (patub);
    fslb = (NULL != fslb) ? COfreeConstant (fslb) : NULL;
    fsub = (NULL != fsub) ? COfreeConstant (fsub) : NULL;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool TULSisZeroTripGenerator( node *lb, node *ub, node *width)
 *
 * @brief checks for the following 5 criteria:
 *
 *    1) (  a <= iv <  a)    where a::int[n]  with n>0  !
 *    2) ( lb <= iv < ub)    where lb::int[n]{vec1}, ub::int[n]{vec2}
 *                                 and any( vec1 >= vec2)
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
 *
 *    6) See isZeroTripGeneratorComposition, above.
 *
 *   If any of these is true, it returns TRUE.
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

    c = (NULL != c) ? COfreeConstant (c) : NULL;

    if (width != NULL) {
        pattern *pat1, *pat2;
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
        c = (NULL != c) ? COfreeConstant (c) : NULL;
    }

    if (!res) {
        res = isZeroTripGeneratorComposition (lb, ub);
        if (res)
            DBUG_PRINT ("criterion 6 met!");
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
 * @fn bool TULSisValuesMatch( node *arg1, node *arg2)
 *
 * Predicate to determine if two nodes have the same algebraic value.
 *
 * Nodes may be anything that can appear in a GENERATOR_BOUND node.
 * I.e.: N_num, N_id, or  N_array, in any combination.
 * NB. N_avis nodes do NOT work.
 *
 * We can safely skip guards and extrema, because this is a predicate
 * only.
 *
 * NB: If you were planning to use this function to pick one arg
 * over the other (say, for something like VP), do NOT do so,
 * or we will have unguarded values propagating past their guards,
 * and other unpleasantness.
 *
 *****************************************************************************/
bool
TULSisValuesMatch (node *arg1, node *arg2)
{
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    pattern *pat5;
    pattern *pat6;
    bool res = FALSE;
    node *elem = NULL;
    constant *con = NULL;
    node *aelems1 = NULL;
    node *aelems2 = NULL;
    constant *fs1 = NULL;
    constant *fs2 = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis != NODE_TYPE (arg1), "arg1 not expected to be N_avis");
    DBUG_ASSERT (N_avis != NODE_TYPE (arg2), "arg2 not expected to be N_avis");
    pat1 = PMvar (1, PMAgetNode (&elem), 0);
    pat2 = PMvar (1, PMAisVar (&elem), 0);

    pat3 = PMconst (1, PMAgetVal (&con), 0);
    pat4 = PMconst (1, PMAisVal (&con), 0);

    pat5 = PMarray (1, PMAgetFS (&fs1), 1, PMskip (1, PMAgetNode (&aelems1)));
    pat6 = PMarray (1, PMAgetFS (&fs2), 1, PMskip (1, PMAgetNode (&aelems2)));

    /* Try N_ids first */
    res = PMmatchFlatSkipExtremaAndGuards (pat1, arg1)
          && PMmatchFlatSkipExtremaAndGuards (pat2, arg2);

    /* And then constants */

    res = res
          || (PMmatchFlatSkipExtremaAndGuards (pat3, arg1)
              && PMmatchFlatSkipExtremaAndGuards (pat4, arg2));
    con = (NULL != con) ? COfreeConstant (con) : NULL;

    /* And, failing that, N_arrays of same shape and all same values */
    if ((!res)
        && (PMmatchFlatSkipExtremaAndGuards (pat5, arg1)
            && PMmatchFlatSkipExtremaAndGuards (pat6, arg2))
        && (SHcompareShapes (COgetShape (fs1), COgetShape (fs2)))) {
        res = TRUE;

        while (res && (NULL != aelems1) && (NULL != aelems2)) {
            res = res && TULSisValuesMatch (EXPRS_EXPR (aelems1), EXPRS_EXPR (aelems2));
            aelems1 = EXPRS_NEXT (aelems1);
            aelems2 = EXPRS_NEXT (aelems2);
        }
        fs1 = (NULL != fs1) ? COfreeConstant (fs1) : NULL;
        fs2 = (NULL != fs2) ? COfreeConstant (fs2) : NULL;
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    pat5 = PMfree (pat5);
    pat6 = PMfree (pat6);

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
TULSisFullGenerator (node *generator, node *op)
{
    bool z = FALSE;
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

    switch (NODE_TYPE (op)) {

    case N_spfold:
    case N_break:
        z = FALSE;
        DBUG_UNREACHABLE ("Should not exist here.");
        break;

    case N_fold:
    case N_propagate:
        z = TRUE;
        break;

    case N_genarray:
        z = PMmatchFlatSkipGuards (patlb, GENERATOR_BOUND1 (generator))
            && COisZero (lb, TRUE)
            && checkBoundShape (GENERATOR_BOUND2 (generator), GENARRAY_SHAPE (op))
            && checkStepWidth (generator);
        break;

    case N_modarray:
        if (PMmatchFlatSkipGuards (patlb, GENERATOR_BOUND1 (generator))
            && COisZero (lb, TRUE) && checkStepWidth (generator)
            && PMmatchFlatSkipGuards (patub, GENERATOR_BOUND2 (generator))
            && PMmatchFlatSkipGuards (patarr, MODARRAY_ARRAY (op))) {

            /* Check for matching frame shapes, or empty array shape */
            shpgen = SHgetUnrLen (ARRAY_FRAMESHAPE (ub));
            if ((0 == shpgen) || (0 == ARRAY_AELEMS (arr))) {
                z2 = TRUE;
            } else {
                shp = TCtakeDropExprs (shpgen, 0, ARRAY_AELEMS (arr));
                z2 = (CMPT_EQ == CMPTdoCompareTree (shp, ARRAY_AELEMS (ub)));
                FREEdoFreeTree (shp);
            }
            DBUG_ASSERT (N_id == NODE_TYPE (GENERATOR_BOUND2 (generator)),
                         "Expected N_id GENERATOR_BOUND");

            modarrshp = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (op)));
            DBUG_ASSERT ((NULL == modarrshp) || (N_id == NODE_TYPE (modarrshp)),
                         "AVIS_SHAPE not flattened");
            z = (z2 || (GENERATOR_BOUND2 (generator) == modarrshp));
        }
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

/** <!--********************************************************************--> *
 * @fn void  TUclearSsaAssign( node *arg_node)
 *
 * @brief Clear AVIS_SSAASSIGN nodes associated with arg_node
 *        N_ids entry/entries.
 *
 * @param: arg_node: N_assign node, which we believe points to an N_let
 *
 *
 *****************************************************************************/
void
TUclearSsaAssign (node *arg_node)
{
    node *ids;

    DBUG_ENTER ();

    DBUG_ASSERT (N_let == NODE_TYPE (ASSIGN_STMT (arg_node)), "Expected N_let");
    ids = LET_IDS (ASSIGN_STMT (arg_node));

    while (NULL != ids) {
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = NULL;
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void  TUsetSsaAssign( node *arg_node)
 *
 * @brief Set AVIS_SSAASSIGN nodes associated with arg_node
 *        N_ids entry/entries.
 *
 * @param: arg_node: N_assign node, which we believe points to an N_let
 *
 *****************************************************************************/
void
TUsetSsaAssign (node *arg_node)
{
    node *ids;

    DBUG_ENTER ();

    DBUG_ASSERT (N_assign == NODE_TYPE (arg_node), "Expected N_assign");
    DBUG_ASSERT (N_let == NODE_TYPE (ASSIGN_STMT (arg_node)), "Expected N_let");
    ids = LET_IDS (ASSIGN_STMT (arg_node));

    while (NULL != ids) {
        AVIS_SSAASSIGN (IDS_AVIS (ids)) = arg_node;
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN ();
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

/** <!--********************************************************************-->
 *
 * @fn int TULSsearchAssignChainForAssign( node *chn, node *assgn)
 *
 * @brief Search the N_assign chain chn for N_assign assgn.
 *        Return the 0-origin index of its location in the chain,
 *        if it exists, and otherwise -1.
 *
 *        This could have been written as a set-membership predicate,
 *        but I thought this version might come in handy for something.
 *
 *
 * @param  chn: An N_assign chain
 * @param  assgn: An N_assign node
 *
 * @return As above. If assgn's SSA_ASSIGN is NULL, return -1.
 *         This situation will arise if assgn is a WITH_ID or the
 *         formal parameter of a defined function.
 *
 ******************************************************************************/
int
TULSsearchAssignChainForAssign (node *chn, node *assgn)
{
    int z = -1;
    int cnt = 0;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (chn) == N_assign, "Expected N_assign chn");

    while ((-1 == z) && (NULL != chn)) {
        if (assgn != chn) {
            cnt++;
            chn = ASSIGN_NEXT (chn);
        } else {
            z = cnt;
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool TULSisInPrfFamily( prf fun, prf funfam)
 * @fn prf TULSgetPrfFamilyName( prf fun)
 *
 * @brief Predicate for determining if an N_prf is in the same function
 *        family as funfam.
 *
 * @param fun: the function we are curious about, e.g, F_add_SxS, F_add_SxV,
 *            F_add_VxS, F_add_VxV, F_sub_SxS.
 *
 * @param funfam: the prototypical function that represents the function.
 *                In most cases, this is the SxS version of the function.
 *                E.g., for addition, it is F_add_SxS.
 *
 * @return TRUE if fun is a member of funfam; else FALSE. For example, all four
 *         addition functions above are members of F_add_SxS, but F_sub_SxS
 *         is not.
 *
 ******************************************************************************/
bool
TULSisInPrfFamily (prf fun, prf funfam)
{
    bool z;

    DBUG_ENTER ();

    z = (TULSgetPrfFamilyName (fun) == funfam);

    DBUG_RETURN (z);
}

prf
TULSgetPrfFamilyName (prf fun)
{ // Return prototypical family name for function.
    // E.g., F_sub_SxS for F_sub_VxS
    // If not part of a family, return fun.

    prf z;

    DBUG_ENTER ();

#define FNFAM(fn)                                                                        \
    case F_##fn##_SxS:                                                                   \
    case F_##fn##_SxV:                                                                   \
    case F_##fn##_VxS:                                                                   \
    case F_##fn##_VxV:                                                                   \
        z = F_##fn##_SxS;                                                                \
        break;

    switch (fun) {
    default:
        z = fun;
        break;

        // Dyadic scalar functions, which are all I care about just now...
        FNFAM (add)
        FNFAM (sub)
        FNFAM (mul)
        FNFAM (div)

        FNFAM (min)
        FNFAM (max)

        FNFAM (lt)
        FNFAM (le)
        FNFAM (eq)
        FNFAM (ge)
        FNFAM (gt)
        FNFAM (neq)

        FNFAM (and)
        FNFAM (or)

        FNFAM (mod)
        FNFAM (aplmod)
    }

    DBUG_RETURN (z);
}

/** <!--*******************************************************************-->
 *
 * @fn bool TUisPrfGuard(node *arg_node)
 *
 * @brief If arg_node is an N_prf and is a guard with PRF_ARG1 as
 *        its primary result, return TRUE; else FALSE.
 * @param
 * @return
 *
 *****************************************************************************/
bool
TUisPrfGuard (node *arg_node)
{
    bool z;

    DBUG_ENTER ();
    z = (N_prf == NODE_TYPE (arg_node));
    if (z) {
        switch (PRF_PRF (arg_node)) {
        default:
            z = FALSE;
            break;
        case F_guard:
        case F_noteminval:
        case F_notemaxval:
        case F_noteintersect:
        case F_non_neg_val_V:
        case F_non_neg_val_S:
        case F_val_lt_shape_VxA:
        case F_val_lt_val_SxS:
        case F_val_le_val_SxS:
        case F_val_le_val_VxV:
        case F_shape_matches_dim_VxA:
            z = TRUE;
            break;
        }
    }

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *TUnode2Avis( node *arg_node)
 *
 * @brief Find N_avis node for arg_node
 *
 * @param An N_avis, N_id, N_num, or N_ids node
 *
 * @return the associated N_avis node, or NULL if this is an N_num
 *         or if arg_node is NULL
 *
 ******************************************************************************/
node *
TUnode2Avis (node *arg_node)
{
    node *avis = NULL;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        switch (NODE_TYPE (arg_node)) {
        case N_id:
            avis = ID_AVIS (arg_node);
            break;

        case N_ids:
            avis = IDS_AVIS (arg_node);
            break;

        case N_avis:
            avis = arg_node;
            break;

        case N_num:
        case N_bool:
            break;

        default:
            DBUG_ASSERT (NULL != avis, "Expected N_id, N_avis, or N_ids node");
        }
    }

    DBUG_RETURN (avis);
}

#undef DBUG_PREFIX
