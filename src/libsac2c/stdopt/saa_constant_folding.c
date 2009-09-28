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
 *   This module handles identities on reshape, replacing expressions of the form:
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

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "compare_tree.h"
#include "str.h"
#include "new_types.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "ctinfo.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SAACF_ids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SAACF_ids (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACF_ids");

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
 *   by
 *      arr
 *   when SAA information can show that:
 *      shp <==> shape(arr)
 *
 * returns:
 *   new result node or NULL if no replacement is possible.
 *
 *****************************************************************************/
node *
SAACFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_reshape");

    /* Check if arg1 matches shape of arg2 */
    if (((NULL != PRF_ARG1 (arg_node)) && (NULL != PRF_ARG2 (arg_node))
         && (N_id == NODE_TYPE (PRF_ARG1 (arg_node)))
         && (NULL != AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))))
         && (CMPT_EQ
             == CMPTdoCompareTree (PRF_ARG1 (arg_node),
                                   AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))))))
        ||
        /* or if rhs of assign of arg1 matches shape of arg2 */
        ((NULL != AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node))))
         && (CMPT_EQ
             == CMPTdoCompareTree (ASSIGN_RHS (
                                     AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)))),
                                   AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))))))) {
        DBUG_PRINT ("CF", ("idempotent _reshape_ eliminated"));
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    }
    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
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

    DBUG_ENTER ("SAACFprf_dim");
    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SAACF_dim_ expected N_id node");

    dim = AVIS_DIM (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != dim) {
        DBUG_PRINT ("CF", ("_dim_A replaced by AVIS_DIM"));
        res = DUPdoDupTree (dim);
    }
    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
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

    DBUG_ENTER ("SAACFprf_shape");

    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SAACF_shape_ expected N_id node");
    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != shp) {
        /* Case 1 */
        DBUG_PRINT ("CF", ("_shape_A replaced by AVIS_SHAPE"));
        res = DUPdoDupTree (shp);
    } else {
        /* Case 2 */
        pat1 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&arg1), 0));
        pat2 = PMprf (1, PMAisPrf (F_saabind), 1, PMvar (1, PMAgetNode (&dm), 0), 1,
                      PMvar (1, PMAgetNode (&shp), 0), PMskip (0));

        if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
            rhs = AVIS_SSAASSIGN (ID_AVIS (arg1));
            if (NULL != rhs) {
                rhs = LET_EXPR (ASSIGN_INSTR (rhs));
                if (PMmatchFlatSkipExtrema (pat2, rhs)) {
                    res = shp;
                    DBUG_PRINT ("CF", ("_shape_A(_saabnd(dim,shp,val)) replaced by shp"));
                }
            }
        }
        pat1 = PMfree (pat1);
        pat2 = PMfree (pat2);
    }

    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
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
    int shape_elem;

    DBUG_ENTER ("SAACFprf_idx_shape_sel");

    /* If idx is a constant, try the replacement */
    argconst = COaST2Constant (PRF_ARG1 (arg_node));
    if (NULL != argconst) {
        shape_elem = ((int *)COgetDataVec (argconst))[0];
        argconst = COfreeConstant (argconst);

        shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));
        if ((shp != NULL) && (NODE_TYPE (shp) == N_array)) {
            shpel = TCgetNthExprsExpr (shape_elem, ARRAY_AELEMS (shp));
            res = DUPdoDupTree (shpel);
            DBUG_PRINT ("CF", ("idx_shape_sel replaced by N_array element"));
        }
    }
    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
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
    node *arg2 = NULL;
    node *arg1 = NULL;
    pattern *patarg1;
    pattern *patarg2;

    DBUG_ENTER ("SAACFprf_take_SxV");

    patarg1
      = PMprf (1, PMAisPrf (F_take_SxV), 2, PMvar (1, PMAgetNode (&arg1), 0), PMskip (0));

    patarg2 = PMarray (1, PMAgetNode (&arg2), 1, PMskip (0));

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));

    if ((NULL != shp) && PMmatchFlatSkipExtrema (patarg1, arg_node)
        && PMmatchFlatSkipExtrema (patarg2, shp)) {
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
        DBUG_PRINT ("CF", ("Take replaced by PRF_ARG2"));
    }

    patarg1 = PMfree (patarg1);
    patarg2 = PMfree (patarg2);

    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
 *
 * @fn node *SAACFprf_drop_SxV(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on drop primitive
 *
 *         1. _drop_SxV_( _idx_shape_sel( 0, V), V);
 *            If PRF_ARG1 matches AVIS_SHAPE( PRF_ARG2( arg_node))[0]:
 *            return an empty vector of the type of PRF_ARG2.
 *            Although this resembles the take() case just above,
 *            typechecker can't do this much analysis.
 *
 *         2. _drop_SxV_( N , V);
 *            If [N] == AVIS_SHAPE( PRF_ARG2( arg_node)),
 *            treat as case 1.
 *
 *         3. _drop_SxV_( 0, V);
 *            If PRF_ARG1 is 0:
 *            return PRF_ARG2 as the result.
 *
 * @param arg_node
 *
 * @result New arg_node if folding happens.
 *
 ********************************************************************/

node *
SAACFprf_drop_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    node *N;
    node *V;
    node *shpV;
    constant *con = NULL;

    DBUG_ENTER ("SAACFprf_drop_SxV");

    pat1 = PMprf (1, PMAisPrf (F_drop_SxV), 2,
                  PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMvar (0, 0),
                         PMvar (1, PMAgetNode (&V), 0)),
                  PMvar (1, PMAisVar (&V), 0));

    pat2
      = PMprf (1, PMAisPrf (F_drop_SxV), 2, PMconst (1, PMAgetVal (&con), 0), PMskip (0));

    pat3 = PMprf (1, PMAisPrf (F_drop_SxV), 2, PMvar (1, PMAgetNode (&N), 0),
                  PMvar (1, PMAgetSaaShape (&shpV), 0));

    pat4 = PMarray (0, 1, PMvar (1, PMAisVar (&N), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)
        || (PMmatchFlatSkipExtrema (pat3, arg_node)
            && PMmatchFlatSkipExtrema (pat4, shpV))) {
        /* Case 1 and Case 2: conjure up empty vector. */
        res = TBmakeArray (TYmakeAKS (TYcopyType (
                                        TYgetScalar (ID_NTYPE (PRF_ARG2 (arg_node)))),
                                      SHcreateShape (0)),
                           SHcreateShape (1, 0), NULL);
        DBUG_PRINT ("CF", ("drop(shape(V), V)  replaced by empty vector"));

    } else if (PMmatchFlatSkipExtrema (pat2, arg_node) && (COisZero (con, TRUE))) {
        /* Case 3 */
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
        con = COfreeConstant (con);
        DBUG_PRINT ("CF", ("drop(0, V) replaced by V"));
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/**<!--*************************************************************-->
 *
 * @fn node *SAACFprf_non_neg_val_V(node *arg_node, info *arg_info)
 *
 * @brief: performs saa constant-folding on F_non_neg_val_V primitive
 *         We start with:
 *           v', p = F_non_neg_val( v);
 *
 *         If AVIS_MINVAL( v) exists and is constant, we compare it to
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

    DBUG_ENTER ("SAACFprf_non_neg_val_V");

    minv = AVIS_MINVAL (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != minv) {
        con = COaST2Constant (minv);
        if ((NULL != con) && COisNonNeg (con, TRUE)) {
            DBUG_PRINT ("CF", ("non_neg_val_V guard removed"));
            con = COfreeConstant (con);
            res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                               TBmakeExprs (TBmakeBool (TRUE), NULL));
        }
    }
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
    node *arg2 = NULL;
    node *shp = NULL;

    DBUG_ENTER ("SAACFprf_same_shape_AxA");

    if (PMO (PMOsaashape (&shp, &arg2,
                          PMOsaashape (&shp, &arg1,
                                       PMOvar (&arg2,
                                               PMOvar (&arg1, PMOprf (F_same_shape_AxA,
                                                                      arg_node))))))) {
        /* See if saa shapes match */

        DBUG_PRINT ("CF", ("same_shape_AxA guard removed"));
        res = TBmakeExprs (DUPdoDupTree (arg1),
                           TBmakeExprs (DUPdoDupTree (arg2),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }
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

    DBUG_ENTER ("SAACFshape_matches_dim_VxA");

#ifdef FIXME
    probably handled by symbolic CF already.

      node *dim;
    node *shp;

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node)));
    dim = AVIS_DIM (ID_AVIS (PRF_ARG2 (arg_node)));
    if ((NULL != shp) && (NULL != dim)) {

        DBUG_PRINT ("CF", ("shape_matches_dim guard removed"));
        res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                           TBmakeExprs (DUPdoDupTree (PRF_ARG2 (arg_node)),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }

#endif // FIXME

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
 *           2. AVIS_MAXVAL( arg1) AKV
 *         with AVIS_SHAPE( arr) AKV in both cases.
 *
 * #@return: possibly updated arg_node
 *
 *****************************************************************************/

node *
SAACFprf_val_lt_shape_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *val;
    constant *shp;
    node *maxv;
    bool z = FALSE;

    DBUG_ENTER ("SAACFprf_val_lt_shape_VxA");

    if (NULL != AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)))) {
        shp = COaST2Constant (AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node))));
        if (NULL != shp) {
            val = COaST2Constant (PRF_ARG1 (arg_node)); /* Case 1 */
            if ((NULL != val) && COlt (val, shp)) {
                z = TRUE;
                val = COfreeConstant (val);
            } else {
                maxv = AVIS_MAXVAL (ID_AVIS (PRF_ARG1 (arg_node)));
                if (NULL != maxv) {
                    val = COaST2Constant (maxv); /* Case 2 */
                    if ((NULL != val) && COlt (val, shp)) {
                        z = TRUE;
                        val = COfreeConstant (val);
                    }
                }
            }
            if (z) {
                DBUG_PRINT ("CF", ("val_lt_shape_VxA guard removed"));
                res = TBmakeExprs (DUPdoDupTree (PRF_ARG1 (arg_node)),
                                   TBmakeExprs (TBmakeBool (TRUE), NULL));
            }

            shp = COfreeConstant (shp);
        }
    }

    DBUG_RETURN (res);
}
