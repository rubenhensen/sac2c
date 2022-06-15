/** <!--********************************************************************-->
 *
 * @defgroup SAACF SAA-based Constant Folding
 *
 *   Module saa_constant_folding.c implements constant folding using
 *   saa-derived information.
 *
 *   This driver module performs AST traversal, invoking four
 *   distinct sets of constant-folding functions, via function tables:
 *
 *      SAACF uses function table prf_SAACF.
 *
 *   This module handles identities on reshape, replacing expressions of
 *   the form:
 *
 *      z = reshape( shp, arr)
 *
 *   with:
 *
 *      z = arr;
 *
 *   when it can prove that:
 *
 *     shp <==> shape( arr)
 *
 * This and other such optimizations in this file are driven
 * by knowledge of AVIS_DIM, AVIS_SHAPE, AVIS_MIN, and AVIS_MAX.
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file saa_constant_folding.c
 *
 * Prefix: SAA_CF
 *
 *****************************************************************************/
#include "saa_constant_folding.h"

#define DBUG_PREFIX "SAACF"
#include "debug.h"

#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "compare_tree.h"
#include "str.h"
#include "new_types.h"
#include "tree_utils.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "ctinfo.h"
#include "pattern_match.h"
#include "free.h"
#include "symbolic_constant_simplification.h"
#include "constants.h"
#include "constant_folding_info.h"

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 *
 * function: constant *SAACFchaseMinMax(node *arg_node, bool minmax)
 *
 * description: Chase AVIS_MIN( AVIS_MIN( AVIS_MIN( arg_node)))
 *              until we find a constant (or don't).
 *
 * params: minmax = AVIS_MIN: SAACFCHASEMIN = FALSE
 *                  AVIS_MAX: SAACFCHASEMAX = TRUE
 *
 * returns: constant or NULL.
 *
 * NB. If the argument is already a constant, and we are looking for
 *     AVIS_MAX, we have to add 1 to the result!
 *
 *****************************************************************************/
constant *
SAACFchaseMinMax (node *arg_node, bool minmax)
{
    DBUG_ENTER ();

    constant *z = NULL;
    constant *cone = NULL;
    constant *consum = NULL;
    node *extr;
    pattern *pat;

    if (NULL != arg_node) {
        pat = PMconst (1, PMAgetVal (&z));
        if ((N_id == NODE_TYPE (arg_node)) && (PMmatchFlatSkipExtrema (pat, arg_node))) {
            if (SAACFCHASEMAX == minmax) { // arg_node is already constant
                cone = COmakeConstantFromInt (1);
                consum = COadd (cone, z, NULL);
                cone = COfreeConstant (cone);
                z = COfreeConstant (z);
                z = consum;
            }
        } else {
            extr = minmax ? AVIS_MAX (ID_AVIS (arg_node)) : AVIS_MIN (ID_AVIS (arg_node));
            z = SAACFchaseMinMax (extr, minmax);
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACF_ids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACF_ids (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

#if SAACFIDS
    /*
     * This has been commented out since when throwing away a WL on the RHS,
     * we MUST ensure that the references to shape variables defined in that
     * WL are removed.
     * I see some solutions:
     * 1) Move shape attributes to IDS node (probably more DT like)
     * 2) Traverse WL to remove all SV references first
     */

    node *dim, *shape;

    dim = AVIS_DIM (IDS_AVIS (arg_node));
    shape = AVIS_SHAPE (IDS_AVIS (arg_node));
    else if ((dim != NULL) && (shape != NULL))
    {
        if ((NODE_TYPE (dim) == N_num) && (NUM_VAL (dim) == 1)
            && (NODE_TYPE (shape) == N_array)
            && (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (shape))) == N_num)
            && (NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (shape))) == 0)) {
            ntype *ty = TYmakeAKS (TYcopyType (TYgetScalar (IDS_NTYPE (arg_node))),
                                   SHmakeShape (0));

            INFO_PREASSIGN (arg_info) = TBmakeAssign (TBmakeLet (DUPdoDupTree (arg_node),
                                                                 TCmakeVector (ty, NULL)),
                                                      INFO_PREASSIGN (arg_info));
        }
    }
#endif
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SAACFprf_reshape(node *arg_node, info *arg_info)
 *
 * description:
 *   Replace the expression:
 *      reshape( shp, arr)
 *   by arr, when SAA information can show that the reshape is idempotent,
 *   i.e.:
 *
 *      shp <==> shape(arr)
 *
 *   We have to skip past guards on both arguments.
 *
 * returns:
 *   new result node or NULL if no replacement is possible.
 *
 *****************************************************************************/
node *
SAACFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *ashp = NULL;
    node *ashpp = NULL;
    node *arg1p = NULL;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0), PMskip (0));

    pat2 = PMany (1, PMAgetNode (&ashpp), 0);
    pat3 = PMany (1, PMAgetNode (&arg1p), 0);

    if ((PMmatchFlat (pat1, arg_node)) && (NULL != AVIS_SHAPE (ID_AVIS (arg2)))) {
        ashp = AVIS_SHAPE (ID_AVIS (arg2));
        /* SkipGuards required for unit test SAACFprf_reshapeAKD.sac */
        if ((PMmatchFlatSkipGuards (pat2, ashp)) && (PMmatchFlatSkipGuards (pat3, arg1))
            && (CMPT_EQ == CMPTdoCompareTree (ashpp, arg1p))) {
            DBUG_PRINT ("idempotent _reshape_ eliminated");
            res = DUPdoDupNode (arg2);
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_dim(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on dim primitive
 *         If SAA information has deduced the rank of the
 *         argument, that computation replaces the dim() operation.
 *
 * @param arg_node
 *
 * @result new arg_node if dim() operation could be removed
 *         else NULL
 *
 ********************************************************************/
node *
SAACFprf_dim (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *dim = NULL;

    DBUG_ENTER ();
    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)), "expected N_id node");

    dim = AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != dim) {
        DBUG_PRINT ("_dim_A replaced by AVIS_DIM");
        res = DUPdoDupNode (dim);
    }
    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_shape(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on shape primitive
 *
 * @param arg_node
 *
 * @result 1. new arg_node if shape() operation could be replaced by
 *            the AVIS_SHAPE of the argument,
 *
 *         2. new arg_node if we have:
 *                 x = _saabind_(dimx, shapex, valx);
 *                 z = _shape_A_(x);
 *              in which case we return:
 *                 z = shapex;
 *
 *         3.  else NULL
 *
 ********************************************************************/
node *
SAACFprf_shape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *shp = NULL;
    node *arg1 = NULL;
    node *dm = NULL;
    node *rhs;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ();

    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)), "expected N_id node");
    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != shp) {
        /* Case 1 */
        DBUG_PRINT ("_shape_A replaced by AVIS_SHAPE");
        res = DUPdoDupNode (shp);
    } else {
        /* Case 2 */
        pat1 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&arg1), 0));
        pat2 = PMprf (1, PMAisPrf (F_saabind), 1, PMvar (1, PMAgetNode (&dm), 0), 1,
                      PMvar (1, PMAgetNode (&shp), 0), PMskip (0));

        if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
            rhs = AVIS_SSAASSIGN (ID_AVIS (arg1));
            if (NULL != rhs) {
                rhs = LET_EXPR (ASSIGN_STMT (rhs));
                if (PMmatchFlatSkipExtrema (pat2, rhs)) {
                    res = DUPdoDupNode (shp);
                    DBUG_PRINT ("_shape_A(_saabnd(dim,shp,val)) replaced by %s",
                                AVIS_NAME (ID_AVIS (shp)));
                }
            }
        }
        pat1 = PMfree (pat1);
        pat2 = PMfree (pat2);
    }

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_shape_sel(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on shape primitive
 *
 * @param arg_node
 *
 * @result new arg_node if idx_shape_sel( idx, X) operation has a constant
 *         idx, and shape(X) is being kept as an N_array. If so, the
 *         operation is replaced by a reference to the appropriate element
 *         of the N_array.
 *         else NULL
 *
 ********************************************************************/

node *
SAACFprf_idx_shape_sel (node *arg_node, info *arg_info)
{
    node *shp = NULL;
    node *res = NULL;
    node *shpel;
    constant *argconst;
    pattern *pat;
    node *narray;
    int shape_elem;

    DBUG_ENTER ();

    /* If idx is a constant, try the replacement */
    argconst = COaST2Constant (PRF_ARG1 (arg_node));
    if (NULL != argconst) {
        shape_elem = ((int *)COgetDataVec (argconst))[0];
        argconst = COfreeConstant (argconst);

        shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));
        if (shp != NULL) {
            pat = PMarray (1, PMAgetNode (&narray), 1, PMskip (0));
            if (PMmatchFlatSkipExtrema (pat, shp)) {
                shpel = TCgetNthExprsExpr (shape_elem, ARRAY_AELEMS (narray));
                res = DUPdoDupNode (shpel);
                DBUG_PRINT ("idx_shape_sel replaced by N_array element");
            }
            pat = PMfree (pat);
        }
    }
    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_take_SxV(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on take primitive
 *
 *         1. Idempotent call:
 *            If PRF_ARG1 matches AVIS_SHAPE( PRF_ARG2( arg_node))[0]:
 *            return PRF_ARG2 as the result.
 *            This means that AVIS_SHAPE must be, eventually, an N_array
 *            node.
 *
 *         2. If PRF_ARG1 is 0:
 *            Typechecker should get this one, because the result is AKV,
 *            and PRF_ARG1 is constant.
 *
 * @param arg_node
 *
 * @result New arg_node if folding happens.
 *
 ********************************************************************/

node *
SAACFprf_take_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *shp;
    node *arg1 = NULL;
    pattern *patarg1;
    pattern *patarg2;
    pattern *pat3;
    pattern *pat4;
    node *arg2shp;
    node *shpel = NULL;
    node *arg1el = NULL;

    DBUG_ENTER ();

    patarg1
      = PMprf (1, PMAisPrf (F_take_SxV), 2, PMvar (1, PMAgetNode (&arg1), 0), PMskip (0));

    patarg2 = PMarray (1, PMAgetNode (&arg2shp), 1, PMskip (0));
    pat3 = PMany (1, PMAgetNode (&shpel), 0);
    pat4 = PMany (1, PMAgetNode (&arg1el), 0);

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));

    if ((NULL != shp) && PMmatchFlatSkipExtrema (patarg1, arg_node)
        && PMmatchFlatSkipExtrema (patarg2, shp) &&
        /* Skip past any guards on the shape vector element */
        PMmatchFlatSkipGuards (pat3, EXPRS_EXPR (ARRAY_AELEMS (arg2shp))) &&
        /* Ditto on PRF_ARG1 */
        PMmatchFlatSkipGuards (pat4, arg1)) {
        if ((shpel == arg1el)
            || ((N_id == NODE_TYPE (shpel)) && (N_id == NODE_TYPE (arg1el))
                && (ID_AVIS (arg1el) == ID_AVIS (shpel)))) {
            res = DUPdoDupNode (PRF_ARG2 (arg_node));
            DBUG_PRINT ("Take replaced by PRF_ARG2");
        }
    }

    patarg1 = PMfree (patarg1);
    patarg2 = PMfree (patarg2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_drop_SxV(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on drop primitive
 *
 *   Case 1. _drop_SxV_( _idx_shape_sel( 0, V), V);
 *           If PRF_ARG1 matches AVIS_SHAPE( PRF_ARG2( arg_node))[0]:
 *           return an empty vector of the type of PRF_ARG2.
 *           Although this resembles the take() case just above,
 *           typechecker can't do this much analysis.
 *
 *   Case 2. _drop_SxV_( N , V);
 *           If [N] == AVIS_SHAPE( PRF_ARG2( arg_node)),
 *           treat as case 1.
 *
 *   Case 3. _drop_SxV_( 0, V);
 *           If PRF_ARG1 is 0:
 *           return PRF_ARG2 as the result.
 *
 * @param arg_node
 *
 * @result New arg_node if folding happens.
 *
 * NB. SCSprf_drop2.sac requires skipping guards.
 *
 ********************************************************************/

node *
SAACFprf_drop_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat2a;
    pattern *pat2b;
    pattern *pat3;
    pattern *pat4;
    node *N = NULL;
    node *V = NULL;
    node *shpV = NULL;
    node *narr = NULL;
    constant *con = NULL;

    DBUG_ENTER ();

    /* Case 1*/
    pat1 = PMprf (1, PMAisPrf (F_drop_SxV), 2,
                  PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMvar (0, 0),
                         PMvar (1, PMAgetNode (&V), 0)),
                  PMvar (1, PMAisVar (&V), 0));

    /* Case 2 */
    pat2 = PMprf (1, PMAisPrf (F_drop_SxV), 2, PMvar (1, PMAgetNode (&N), 0),
                  PMvar (1, PMAgetSaaShape (&shpV), 0));
    /* Case 2 - chase AVIS_SHAPE back to N_array */
    pat2a = PMarray (0, 1, PMvar (1, PMAgetNode (&narr), 0));
    pat2b = PMany (1, PMAgetNode (&shpV), 0);

    /* Case 3 */
    pat3
      = PMprf (1, PMAisPrf (F_drop_SxV), 2, PMconst (1, PMAgetVal (&con), 0), PMskip (0));
    pat4 = PMany (1, PMAisNode (&shpV), 0);

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        /* Case 1: conjure up empty vector. */
        res = TBmakeArray (TYmakeAKS (TYcopyType (
                                        TYgetScalar (ID_NTYPE (PRF_ARG2 (arg_node)))),
                                      SHcreateShape (0)),
                           SHcreateShape (1, 0), NULL);
        DBUG_PRINT (" Case 1: drop(shape(V), V) replaced by empty vector");
    }

    if ((NULL == res) && (PMmatchFlatSkipExtremaAndGuards (pat2, arg_node))
        && (PMmatchFlatSkipExtremaAndGuards (pat2a, shpV))
        && (PMmatchFlatSkipExtremaAndGuards (pat2b, narr))
        && (PMmatchFlatSkipExtremaAndGuards (pat4, N))) {

        /* Case 2: conjure up empty vector. */
        res = TBmakeArray (TYmakeAKS (TYcopyType (
                                        TYgetScalar (ID_NTYPE (PRF_ARG2 (arg_node)))),
                                      SHcreateShape (0)),
                           SHcreateShape (1, 0), NULL);
        DBUG_PRINT ("Case 2: drop(shape(V), V)  replaced by empty vector");

    } else if (PMmatchFlatSkipExtrema (pat3, arg_node) && (COisZero (con, TRUE))) {
        DBUG_ASSERT (NULL == res, "Oopsie");
        /* Case 3 */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
        con = COfreeConstant (con);
        DBUG_PRINT ("drop(0, V) replaced by V");
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat2a = PMfree (pat2a);
    pat2b = PMfree (pat2b);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_non_neg_val_V(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on F_non_neg_val_V primitive
 *         We start with:
 *           v', p = F_non_neg_val( v);
 *
 *         If AVIS_MIN( v) exists and is constant, we compare it to
 *         zero. If v turns out to be >= 0, we eliminate the guard,
 *         and return:
 *
 *           v, TRUE;
 *
 * @param arg_node
 *
 * @result new arg_node if v is non_negative.
 *
 ********************************************************************/

node *
SAACFprf_non_neg_val_V (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *minv;
    constant *con;

    DBUG_ENTER ();

    minv = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
    con = SAACFchaseMinMax (minv, SAACFCHASEMIN);
    if ((NULL != con) && COisNonNeg (con, TRUE)) {
        DBUG_PRINT ("non_neg_val_V guard removed");
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
    }
    con = (NULL != con) ? COfreeConstant (con) : con;

    DBUG_RETURN (res);
}

/** <!--*************************************************************-->
 *
 * @fn node *SAACFprf_non_neg_val_S(node *arg_node, info *arg_info)
 *
 * @description: Identical to _V version, essentially.
 *
 ********************************************************************/

node *
SAACFprf_non_neg_val_S (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *minv;
    constant *con;

    DBUG_ENTER ();

    minv = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
    con = SAACFchaseMinMax (minv, SAACFCHASEMIN);
    if ((NULL != con) && COisNonNeg (con, TRUE)) {
        DBUG_PRINT ("non_neg_val_S guard removed");
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
    }
    con = (NULL != con) ? COfreeConstant (con) : con;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_same_shape_AxA( node *arg_node, info *arg_info)
 *
 * If arguments B and C are same shape, replace:
 *   b', c', pred = prf_same_shape_AxA_(B,C)
 * by:
 *   b', c', pred = B,C,TRUE;
 *
 * CFassign will turn this into:
 *   b' = b;
 *   c' = c;
 *   pred = TRUE;
 *
 * This code catches the SAA case.
 *
 *
 *****************************************************************************/

node *
SAACFprf_same_shape_AxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_same_shape_AxA), 2, PMvar (1, PMAgetNode (&arg1), 0),
                 PMvar (1, PMAisNode (&arg1), 0));
    if (PMmatchFlat (pat, arg_node)) {
        /* See if saa shapes match */

        DBUG_PRINT ("same_shape_AxA guard removed");
        res = TBmakeExprs (DUPdoDupNode (arg1),
                           TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_shape_matches_dim_VxA( node *arg_node, info *arg_info)
 *
 * If shape(B) matches dim(C), replace:
 *   b', c', pred = _shape_matches_dim_VxA(B,C)
 *
 * by:
 *
 *   b', c', pred = B,C,TRUE;
 *
 * CFassign will turn this into:
 *   b' = b;
 *   c' = c;
 *   pred = TRUE;
 *
 * This code catches two cases:
 *  AVIS_DIM constant ==  AVIS_SHAPE constant
 * and
 *  AVIS_DIM varb == AVIS_SHAPE varb
 *
 *
 *****************************************************************************/

node *
SAACFprf_shape_matches_dim_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

#ifdef FIXME
    probably handled by symbolic CF already.

      node *dim;
    node *shp;

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node)));
    dim = AVIS_DIM (ID_AVIS (PRF_ARG2 (arg_node)));
    if ((NULL != shp) && (NULL != dim)) {
        and broken, because there is code missing here...

          DBUG_PRINT ("shape_matches_dim guard removed");
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (DUPdoDupNode (PRF_ARG2 (arg_node)),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }

#endif // FIXME

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_val_lt_shape_SxA( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACFprf_val_lt_shape_SxA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_val_lt_shape( node *arg_node, info *arg_info)
 *
 * @brief: We start with:
 *
 *           idx', p = _val_lt_shape_VxA_( idx, arr);
 *
 *         If idx < shape(arr), return:
 *
 *           idx', p = idx, TRUE;
 *
 * @notes: The two cases we handle here are:
 *           1. arg1 AKV
 *           2. AVIS_MAX( arg1) AKV
 *         with AVIS_SHAPE( arr) AKV in both cases.
 *
 * @return: possibly updated arg_node
 *
 *****************************************************************************/

node *
SAACFprf_val_lt_shape_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *val;
    constant *val2;
    constant *shp;
    node *maxv;
    bool z = FALSE;

    DBUG_ENTER ();

    if (NULL != AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)))) {
        shp = COaST2Constant (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))));
        if (NULL != shp) {
            val = COaST2Constant (PRF_ARG1 (arg_node)); /* Case 1 */
            if ((NULL != val) && COlt (val, shp, NULL)) {
                z = TRUE;
            } else {
                maxv = AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)));
                val2 = SAACFchaseMinMax (maxv, SAACFCHASEMAX);
                if ((NULL != val2) && COle (val2, shp, NULL)) { /* Case 2 */
                    /* COle because maxv is 1 greater than its true value */
                    z = TRUE;
                }
                val2 = (NULL != val2) ? COfreeConstant (val2) : val2;
            }
            if (z) {
                DBUG_PRINT ("val_lt_shape_VxA guard removed");
                res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                   TBmakeExprs (TBmakeBool (TRUE), NULL));
            }

            val = (NULL != val) ? COfreeConstant (val) : val;
            shp = COfreeConstant (shp);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_val_lt_shape_SxS node *arg_node, info *arg_info)
 *
 * @brief: We start with:
 *
 *           iv', p = _val_lt_val_SxS_( iv, lim);
 *
 *           if AVIS_MAX( iv) == lim, then replace the op by:
 *
 *           iv', p = iv, TRUE;
 *
 * @notes: The two cases we handle here are:
 *
 * @return: possibly updated arg_node
 *
 *****************************************************************************/

node *
SAACFprf_val_lt_val_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *maxv;
    node *maxv2 = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ();

    pat1 = PMany (1, PMAgetNode (&maxv2), 0);

    pat2 = PMprf (1, PMAisPrf (F_val_lt_val_SxS), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0));
    pat3 = PMany (1, PMAisNode (&maxv2), 0);

    /* Chase maxval back to its origin */
    maxv = AVIS_MAX (ID_AVIS (PRF_ARG1 (arg_node)));
    if ((NULL != maxv) && (PMmatchFlatSkipGuards (pat1, maxv))
        && (PMmatchFlat (pat2, arg_node)) && (PMmatchFlatSkipGuards (pat3, arg2))) {

        DBUG_PRINT ("val_lt_val_SxS guard removed");
        res = TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                           TBmakeExprs (TBmakeBool (TRUE), NULL));
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool isEqExtrema(node *arg_node1, node *arg_node2)
 *
 * @brief: For a relational primitive on SxS or VxV:
 *         Attempt an exact compare between
 *         arg_node1 and arg_node2. [One of them is supposed
 *         to be an AVIS_MIN or AVIS_MAX.
 *
 *         We should extend this via ISMOP to handle SxV and VxS, at
 *         least when the non-scalar argument is an N_array,
 *         potentially of S.
 *
 * @result: TRUE if the arguments match. Do NOT make assumptions
 *          about values if result is false.
 *
 *****************************************************************************/
static bool
isEqExtrema (node *arg_node1, node *arg_node2)
{
    bool res = FALSE;
    pattern *pat;

    DBUG_ENTER ();

    if ((NULL != arg_node1) && (NULL != arg_node2)
        && (TUeqShapes (AVIS_TYPE (ID_AVIS (arg_node1)),
                        AVIS_TYPE (ID_AVIS (arg_node2))))) {
        pat = PMvar (1, PMAisVar (&arg_node1), 0);
        res = PMmatchFlatSkipExtrema (pat, arg_node2);
        DBUG_PRINT ("comparing %s and %s gave %d", AVIS_NAME (ID_AVIS (arg_node1)),
                    AVIS_NAME (ID_AVIS (arg_node2)), res);
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * Code below here provides extrema-based CF for relationals
 *
 * The basic principle is that if the appropriate extremum of
 * one argument is constant, and the other argument is constant,
 * we MAY be able to provide an answer to the function.
 *
 * E.g., if we are doing _lt_VxS( V, S):
 *
 *          If AVIS_MAX(V) <  S,  return:  genarray( shape(V), TRUE);
 *
 * If that fails, try the other extrema, via a recursive call:
 *
 *          If AVIS_MIN(S) >= V,  return:  genarray( shape(V), TRUE);
 *
 * If that fails, try the converse cases, as above:
 *
 *          If AVIS_MIN(V) >= S, return:  genarray( shape(V), FALSE);
 * and
 *          If AVIS_MAX(S) <  V, return:  genarray( shape(V), FALSE);
 *
 *  Enhancement: Consider a nested WL in which the
 *  lower bound of an inner WL is the index vector of the outer:
 *    z = with ... ( [0] <= iv1 < [N1]) ...
 *            with... ( iv1 <= iv2 < N2)....
 *
 *  AVIS_MIN( iv2) is iv1, but iv1 is not constant. However,
 *  AVIS_MIN( AVIS_MIN( iv2)) IS constant. So, for relationals,
 *  we search the extrema chains seeking a constant.
 *
 *****************************************************************************/

// This table must match the relationalfns typedef in constant_folding_info.h
// I.e., NULL, F_lt_SxS, F_le_SxS, F_ge_SxS, F_gt_SxS.
static constant *(*relfn[]) (constant *, constant *, constant *)
  = {NULL, COlt, COle, COge, COgt};

/** <!--********************************************************************-->
 *
 * @fn node *SAACFonRelationalswithExtrema( ...)
 *
 * @brief: Generic function for performing CF on relationals
 *         and their extrema. For example, if we are doing:
 *
 *           z = _gt_VxS_( V, S);
 *
 *         We compute all( AVIS_MIN( V) > S).
 *         If that is true, we generate a vector of TRUE for z.
 *
 * @params: prfarg1: PRF_ARG1 or PRF_ARG2. This argument is
 *                   checked for extrema.
 * @params: prfarg2: PRF_ARG2 or PRF_ARG1. This argument is not
 *                   checked for extrema.
 * @params: arg_info: as usual.
 *
 * @params: fun is the relational fn enum to be used for comparison
 *          against zero, e.g., REL_gt in the above example.
 *
 * @result: NULL, or a Boolean ast constant of the same shape as prfargres, of
 *          all TRUE or FALSE.
 *
 *****************************************************************************/
static int
GetFunNum (prf fun)
{ // Map function name to an integer for indexing
    int z;

    DBUG_ENTER ();
    switch (fun) {
    default:
        z = 0;
        DBUG_UNREACHABLE ("fun confusion");
        break;
    case F_lt_SxS:
        z = 1;
        break;
    case F_le_SxS:
        z = 2;
        break;
    case F_ge_SxS:
        z = 3;
        break;
    case F_gt_SxS:
        z = 4;
        break;
    }

    DBUG_RETURN (z);
}

static node *
relatHelper (node *prfarg1, node *prfarg2, info *arg_info, prf fun, bool minmax, bool tf,
             bool swapped)
{
    node *res = NULL;
    constant *arg1c = NULL;
    constant *arg1cp = NULL;
    constant *arg2c = NULL;
    constant *b;
    constant *adj;
    simpletype tp;
    node *arg1ex;
    node *prfargres;
    bool mat = FALSE;

    DBUG_ENTER ();

    tp = SCSgetBasetypeOfExpr (prfarg1);

    // This block handles the direct cases:
    //
    //  AVIS_MAX( x) <  y --> TRUE
    //  AVIS_MIN( x) >  y --> TRUE
    //  AVIS_MAX( x) <= y --> TRUE
    //  AVIS_MIN( x) >= y --> TRUE
    if (N_id == NODE_TYPE (prfarg1)) {
        arg1ex = minmax ? AVIS_MAX (ID_AVIS (prfarg1)) : AVIS_MIN (ID_AVIS (prfarg1));
        // Try exact match on extremum N_id:
        // If we get one, fake up arguments to the relfn call, below.
        if (isEqExtrema (arg1ex, prfarg2)) {
            arg1c = COmakeOne (tp, SHmakeShape (0));
            arg2c = COmakeOne (tp, SHmakeShape (0));
        } else { // Otherwise, try to match constants
            arg1c = SAACFchaseMinMax (arg1ex, minmax);
            arg2c = COaST2Constant (prfarg2);
        }

        if (((NULL != arg1c)) && (NULL != arg2c)) {
            adj = minmax ? COmakeOne (tp, SHmakeShape (0))
                         : COmakeZero (tp, SHmakeShape (0));
            arg1cp = COsub (arg1c, adj, NULL); /* Denormalize AVIS_MAX*/
            adj = COfreeConstant (adj);
            b = ((relfn[GetFunNum (fun)])) (arg1cp, arg2c, NULL);
            mat = COisTrue (b, TRUE);
            b = COfreeConstant (b);
        }

        arg1c = (NULL != arg1c) ? COfreeConstant (arg1c) : arg1c;
        arg1cp = (NULL != arg1cp) ? COfreeConstant (arg1cp) : arg1cp;
        arg2c = (NULL != arg2c) ? COfreeConstant (arg2c) : arg2c;
        if (mat) {
            prfargres = (TUisVector (AVIS_TYPE (ID_AVIS (prfarg1)))) ? prfarg1 : prfarg2;
            res = tf ? SCSmakeTrue (prfargres) : SCSmakeFalse (prfargres);
            DBUG_PRINT ("replacing RHS by tob( %d)", tf);
        }
    }

    DBUG_RETURN (res);
}

node *
SAACFonRelationalsWithExtrema (node *prfarg1, node *prfarg2, info *arg_info, prf fun)
{
    node *res = NULL;
    prf fna;
    prf fnb;
    prf fnc;
    prf fnd;
    int funnuma;
    bool minmax;

    // These tables are used to obtain the complementary relational fns for relfn.
    // Make sure that they correspond to the REL_xxx definitions.
    //                     NULL,       COlt,     COle,     COge,     COgt};
    prf complementfnb[] = {F_unknown, F_gt_SxS, F_ge_SxS, F_le_SxS, F_lt_SxS};
    prf complementfnc[] = {F_unknown, F_ge_SxS, F_gt_SxS, F_lt_SxS, F_le_SxS};
    prf complementfnd[] = {F_unknown, F_le_SxS, F_lt_SxS, F_gt_SxS, F_ge_SxS};
    bool minmaxtab[]
      = {FALSE, SAACFCHASEMAX, SAACFCHASEMAX, SAACFCHASEMIN, SAACFCHASEMIN};

    DBUG_ENTER ();

    fna = TULSgetPrfFamilyName (fun);
    switch (fna) { // sheep vs. goats
    case F_lt_SxS:
    case F_le_SxS:
    case F_ge_SxS:
    case F_gt_SxS: // sheep

        funnuma = GetFunNum (fna);
        fnb = complementfnb[funnuma];
        fnc = complementfnc[funnuma];
        fnd = complementfnd[funnuma];
        minmax = minmaxtab[funnuma];

        // This block handles the direct cases:
        //
        // x <  y --> TRUE, if AVIS_MAX( x) <  y
        // x <= y --> TRUE, if AVIS_MAX( x) <= y
        // x >= y --> TRUE, if AVIS_MIN( x) >= y
        // x >  y --> TRUE, if AVIS_MIN( x) >  y
        res = relatHelper (prfarg1, prfarg2, arg_info, fna, minmax, TRUE, FALSE);

        // This block handles the case of extrema on y, via argument
        // and function swapping. This is done because our relatHelper only
        // deals with extrema on x.
        //
        // x <  y --> TRUE, if AVIS_MIN( y) >  x
        // x <= y --> TRUE, if AVIS_MIN( y) >= x
        // x >= y --> TRUE, if AVIS_MAX( y) <= x
        // x >  y --> TRUE, if AVIS_MAX( y) <  x

        if (NULL == res) {
            res = relatHelper (prfarg2, prfarg1, arg_info, fnb, (!minmax), TRUE, TRUE);
        }

        // This block handles the direct complementary case:
        //
        // x <  y --> FALSE, if AVIS_MIN( x) >= y
        // x <= y --> FALSE, if AVIS_MIN( x) >  y
        // x >= y --> FALSE, if AVIS_MAX( x) <  y
        // x >  y --> FALSE, if AVIS_MAX( x) <= y

        if (NULL == res) {
            res = relatHelper (prfarg1, prfarg2, arg_info, fnc, (!minmax), FALSE, FALSE);
        }

        // This block handles the swapped complementary case, with
        //
        // x <  y --> FALSE, if AVIS_MAX( y) <= x;
        // x <= y --> FALSE, if AVIS_MAX( y) <  x;
        // x >= y --> FALSE, if AVIS_MIN( y) >  x;
        // x >  y --> FALSE, if AVIS_MIN( y) >= x;
        //
        if (NULL == res) {
            res = relatHelper (prfarg2, prfarg1, arg_info, fnd, minmax, FALSE, TRUE);
        }

        break;

    default:
        break; // goats
    }

    // Temporary code to aid in measuring POGO performance on relationals
    if (!global.optimize.dorelcf) {
        res = (NULL != res) ? FREEdoFreeNode (res) : res;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_SxS_( x, y)
 *
 *          If AVIS_MAX(x) <  y, return:  genarray( shape(x), TRUE);
 *          If x < AVIS_MIN(y),  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >= y, return:  genarray( shape(x), FALSE);
 *          If x >= AVIS_MAX(y),  return:  genarray( shape(x), FALSE);
 *
 * CompareEqExtrema handles this case for non-constant extrema:
 *
 *          If AVIS_MAX(x) == y, return:  genarray( shape(x), TRUE);
 *          Since AVIS_MAX is normalized, x<y.
 *
 *          If AVIS_MIN(x) == y, then x>=y. Return FALSE.
 *          If x == AVIS_MAX(y), then x>y.  Return FALSE.
 *
 *
 *****************************************************************************/
node *
SAACFprf_lt_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_SxV( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_SxV_( x, y)
 *
 *          If AVIS_MAX(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MIN(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAX(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_VxS( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_VxS_( x, y)
 *
 *          If AVIS_MAX(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MIN(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAX(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACFprf_lt_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_lt_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _le_SxS_( x, y)
 *
 *          If AVIS_MAX(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MIN(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAX(y) <  x, return:  genarray( shape(x), FALSE);
 *
 * CompareEqExtrema handles this case for non-constant extrema:
 *
 *          If x == AVIS_MIN(y), return:  genarray( shape(x), TRUE);
 *          If AVIS_MAX(x) == y, return:  genarray( shape(x), TRUE);
 *          If x == AVIS_MAX(y), return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_SxV( node *arg_node, info *arg_info)
 *
 * @brief: for _le_SxV_( x, y)
 *
 *          If AVIS_MAX(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MIN(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAX(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_VxS( node *arg_node, info *arg_info)
 *
 * @brief: for _le_VxS_( x, y)
 *
 *          If AVIS_MAX(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MIN(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MIN(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAX(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACFprf_le_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_le_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * All the _eq_ and _neq_ cases
 *
 *****************************************************************************/
node *
SAACFprf_eq_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

node *
SAACFprf_eq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_eq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

node *
SAACFprf_eq_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_eq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

node *
SAACFprf_eq_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_eq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

node *
SAACFprf_neq_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

node *
SAACFprf_neq_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_neq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

node *
SAACFprf_neq_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_neq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

node *
SAACFprf_neq_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_neq_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _ge_SxS_( V, S):
 *
 *          If AVIS_MIN(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) < S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) > V, return:  genarray( shape(V), FALSE);
 *
 * CompareEqExtrema handles this case for non-constant extrema:
 *
 *          If x == AVIS_MAX(y), return:  genarray( shape(x), TRUE);
 *          Since AVIS_MAX is normalized, x>y, so x>=y is satisfied.
 *
 *          If AVIS_MIN(x) == y, return:  genarray( shape(x), TRUE);
 *          If AVIS_MAX(x) == y, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_SxV( node *arg_node, info *arg_info)
 *
 * @brief:  If AVIS_MIN(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) <  S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) >  V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_VxS( node *arg_node, info *arg_info)
 *
 * @brief:  If AVIS_MIN(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) <  S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) >  V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACFprf_ge_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_ge_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_SxS( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxS_( V, S),
 *
 *  saarelat handles these cases for constant extrema:
 *
 *          If AVIS_MIN(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) >= V, return:  genarray( shape(V), FALSE);
 *
 * CompareEqExtrema handles these cases for non-constant extrema:
 *
 *          If V == AVIS_MAX(S), return:  genarray( shape(V), TRUE);
 *          Since AVIS_MAX is normalized, V>S.
 *
 *          If V == AVIS_MIN(S), return:  genarray( shape(V), FALSE);
 *          If AVIS_MAX(V) == S, return:  genarray( shape(V), FALSE);
 *
 *
 *
 *****************************************************************************/
node *
SAACFprf_gt_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_SxV( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxV_( V, S),
 *
 *          If AVIS_MIN(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_VxS( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxV_( V, S),
 *
 *          If AVIS_MIN(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAX(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAX(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MIN(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFonRelationalsWithExtrema (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node),
                                         arg_info, PRF_PRF (arg_node));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_VxV( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACFprf_gt_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = SAACFprf_gt_SxS (arg_node, arg_info);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_saabind( node *arg_node, info *arg_info)
 *
 * @brief:  If PRF_ARG3 does not derive from an N_ap,
 *          replace the _saabind_( dim, shp, val) by val.
 *
 *****************************************************************************/
node *
SAACFprf_saabind (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *dim;
    node *shp;
    node *val;
    node *arg3rhs;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_saabind), 3, PMvar (1, PMAgetNode (&dim), 0),
                 PMvar (1, PMAgetNode (&shp), 0), PMvar (1, PMAgetNode (&val), 0));

    if (PMmatchFlat (pat, arg_node)) {
        arg3rhs = AVIS_SSAASSIGN (ID_AVIS (val));
        if ((NULL != arg3rhs) && (N_ap != NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg3rhs))))
            && (N_arg != NODE_TYPE (AVIS_DECL (ID_AVIS (val))))) {

            DBUG_PRINT ("_saabind_() replaced by %s", AVIS_NAME (ID_AVIS (val)));
            res = DUPdoDupNode (val);
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
