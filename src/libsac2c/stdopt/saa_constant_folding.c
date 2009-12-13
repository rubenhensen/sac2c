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
 * by knowledge of AVIS_DIM, AVIS_SHAPE, AVIS_MINVAL, and AVIS_MAXVAL.
 *
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
#include "free.h"
#include "symbolic_constant_simplification.h"

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
    pattern *pat;
    node *narray;
    int shape_elem;

    DBUG_ENTER ("SAACFprf_idx_shape_sel");

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
                res = DUPdoDupTree (shpel);
                DBUG_PRINT ("CF", ("idx_shape_sel replaced by N_array element"));
            }
            PMfree (pat);
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
    node *arg1 = NULL;
    pattern *patarg1;
    pattern *patarg2;
    node *arg2shp;
    node *shpel;

    DBUG_ENTER ("SAACFprf_take_SxV");

    patarg1
      = PMprf (1, PMAisPrf (F_take_SxV), 2, PMvar (1, PMAgetNode (&arg1), 0), PMskip (0));

    patarg2 = PMarray (1, PMAgetNode (&arg2shp), 1, PMskip (0));

    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG2 (arg_node)));

    if ((NULL != shp) && PMmatchFlatSkipExtrema (patarg1, arg_node)
        && PMmatchFlatSkipExtrema (patarg2, shp)) {
        shpel = EXPRS_EXPR (ARRAY_AELEMS (arg2shp));
        if ((shpel == arg1)
            || ((N_id == NODE_TYPE (shpel)) && (N_id == NODE_TYPE (arg1))
                && (ID_AVIS (arg1) == ID_AVIS (shpel)))) {
            res = DUPdoDupTree (PRF_ARG2 (arg_node));
            DBUG_PRINT ("CF", ("Take replaced by PRF_ARG2"));
        }
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
 *          If AVIS_MAXVAL(V) <  S,  return:  genarray( shape(V), TRUE);
 *
 * If that fails, try the other extrema, via a recursive call:
 *
 *          If AVIS_MINVAL(S) >= V,  return:  genarray( shape(V), TRUE);
 *
 * If that fails, try the converse cases, as above:
 *
 *          If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), FALSE);
 * and
 *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), FALSE);
 *
 * TODO:
 *  With a bit of work, we could check both the TRUE and
 *  FALSE cases, and merge them if they cover the argument.
 *  I have no idea if this is worthwhile in practice.
 *
 *****************************************************************************/

typedef enum { REL_lt, REL_le, REL_ge, REL_gt } relationalfns;

static constant *(*relfn[]) (constant *, constant *) = {COlt, COle, COge, COgt};

/** <!--********************************************************************-->
 *
 * @fn static node *saarelat( node *prfarg1, node *prfarg2, info *arg_info,
 *                         node* ( *fn)( node *, node *),
 *                         bool minmax,
 *                         node *prfargres,
 *                         bool tf,
 *                         bool recur);
 *
 * @brief: Generic function for performing CF on VxS relationals
 *         and their extrema. For example, if we are doing:
 *
 *           z = _gt_VxS_( V, S);
 *
 *         We compute all( AVIS_MINVAL( V) > S).
 *         If that is true, we generate a vector of TRUE for z.
 *
 * @params: prfarg1: PRF_ARG1 or PRF_ARG2. This argument is
 *                   checked for extrema.
 * @params: prfarg2: PRF_ARG2 or PRF_ARG1. This argument is not
 *                   checked for extrema.
 * @params: arg_info: as usual.
 *
 * @params: fn is the relationalfn enum to be used for comparison against zero,
 *          e.g., REL_gt in the above example.
 *
 * @params: minmax: A boolean, used to select AVIS_MINVAL or AVIS_MAXVAL.
 *
 * @params: prfargres: the PRF_ARG to be used to determine the result
 *          shape.
 *
 * @params: tf: Boolean result value if comparison is TRUE.
 *
 * @params: recur: Recur once with recur FALSE, if recur is TRUE
 *          res still NULL.
 *
 * @result: NULL, or a Boolean of the same shape as prfargres, of
 *          all TRUE or FALSE.
 *
 *****************************************************************************/
static node *
saarelat (node *prfarg1, node *prfarg2, info *arg_info, int fna, int fnb, bool minmax,
          node *prfargres, bool tf, bool recur)
{
    node *res = NULL;
    pattern *pat;
    constant *arg1c = NULL;
    constant *arg1cp = NULL;
    constant *arg2c = NULL;
    constant *b;
    constant *adj;
    simpletype tp;
    node *arg1ex;

    DBUG_ENTER ("saarelat");

#define RELMIN FALSE
#define RELMAX TRUE

    tp = GetBasetypeOfExpr (prfarg1);
    adj = minmax ? COmakeOne (tp, SHmakeShape (0)) : COmakeZero (tp, SHmakeShape (0));

    if (N_id == NODE_TYPE (prfarg1)) {

        pat = PMconst (1, PMAgetVal (&arg1c));
        arg1ex
          = minmax ? AVIS_MAXVAL (ID_AVIS (prfarg1)) : AVIS_MINVAL (ID_AVIS (prfarg1));
        if (NULL != arg1ex) {
            DBUG_ASSERT (NULL != AVIS_SSAASSIGN (arg1ex), "AVIS_SSAASSIGN missing!");
            arg1ex = LET_IDS (ASSIGN_INSTR (AVIS_SSAASSIGN (arg1ex)));
            arg1ex = TBmakeExprs (TBmakeId (IDS_AVIS (arg1ex)), NULL);
        }
        arg2c = COaST2Constant (prfarg2);

        if ((PMmatchFlatSkipExtrema (pat, arg1ex)) && (NULL != arg2c)) {
            arg1cp = COsub (arg1c, adj); /* Correct AVIS_MAXVAL */
            b = ((relfn[fna])) (arg1cp, arg2c);
            if (COisTrue (b, TRUE)) {
                res = tf ? MakeTrue (prfargres) : MakeFalse (prfargres);
                DBUG_PRINT ("CF", ("saarelat replacing RHS by constant"));
            }
            b = COfreeConstant (b);
        }

        arg1c = (NULL != arg1c) ? COfreeConstant (arg1c) : arg1c;
        arg1cp = (NULL != arg1cp) ? COfreeConstant (arg1cp) : arg1cp;
        arg2c = (NULL != arg2c) ? COfreeConstant (arg2c) : arg2c;
        if (NULL != arg1ex) {
            FREEdoFreeTree (arg1ex);
        }
        pat = PMfree (pat);

        /* If no joy, try again to catch case where y has extrema.
         * E.g., if we are doing _lt_VxS_( V, S)
         *
         *          If AVIS_MAXVAL(V) <  S,  return:  genarray( shape(V), TRUE);
         *
         * If that fails, try this:
         *
         *          If AVIS_MINVAL(S) >= V,  return:  genarray( shape(V), TRUE);
         *
         */
        if ((NULL == res) && recur) {
            res = saarelat (prfarg2, prfarg1, arg_info, fnb, fnb, (!minmax), prfargres,
                            tf, FALSE);
        }

        /*
         * If that fails, try the FALSE case, as above:
         *
         *          If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), FALSE);
         */
        if ((NULL == res) && recur) {
            res = saarelat (prfarg1, prfarg2, arg_info, fnb, fnb, (!minmax), prfargres,
                            (!tf), FALSE);
        }

        /*
         * If that fails, try the other FALSE case, as above:
         *
         *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), FALSE);
         */
        if ((NULL == res) && recur) {
            res = saarelat (prfarg2, prfarg1, arg_info, fna, fnb, minmax, prfargres,
                            (!tf), FALSE);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_SxS_( x, y)
 *
 *          If AVIS_MAXVAL(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_lt_SxS");
    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_lt, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_SxV( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_SxV_( x, y)
 *
 *          If AVIS_MAXVAL(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_lt_SxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_lt, REL_ge,
                    RELMAX, PRF_ARG2 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_VxS( node *arg_node, info *arg_info)
 *
 * @brief: for _lt_VxS_( x, y)
 *
 *          If AVIS_MAXVAL(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_lt_VxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_lt, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_lt_VxV( node *arg_node, info *arg_info)

 * @brief: for _lt_VxS_( x, y)
 *
 *          If AVIS_MAXVAL(x) <  y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >= y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_lt_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_lt_VxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_lt, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _le_SxS_( x, y)
 *
 *          If AVIS_MAXVAL(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_le_SxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_le, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_SxV( node *arg_node, info *arg_info)
 *
 * @brief: for _le_SxV_( x, y)
 *
 *          If AVIS_MAXVAL(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_le_SxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_le, REL_ge,
                    RELMAX, PRF_ARG2 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_VxS( node *arg_node, info *arg_info)
 *
 * @brief: for _le_VxS_( x, y)
 *
 *          If AVIS_MAXVAL(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_le_VxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_le, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_le_VxV( node *arg_node, info *arg_info)
 *
 * @brief: for _le_VxV_( x, y):
 *
 *          If AVIS_MAXVAL(x) <= y,  return:  genarray( shape(x), TRUE);
 *          If AVIS_MINVAL(y) >= x,  return:  genarray( shape(x), TRUE);
 *
 *          If AVIS_MINVAL(x) >  y, return:  genarray( shape(x), FALSE);
 *          If AVIS_MAXVAL(y) <  x, return:  genarray( shape(x), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_le_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_le_VxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_le, REL_ge,
                    RELMAX, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_SxS( node *arg_node, info *arg_info)
 *
 * @brief: for _ge_SxS_( V, S):
 *
 *          If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) < S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) > V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_ge_SxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_ge, REL_le,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_SxV( node *arg_node, info *arg_info)
 *
 * @brief:  If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <  S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >  V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_ge_SxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_ge, REL_le,
                    RELMIN, PRF_ARG2 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_VxS( node *arg_node, info *arg_info)
 *
 * @brief:  If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <  S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >  V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_ge_VxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_ge, REL_le,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_ge_VxV( node *arg_node, info *arg_info)
 *
 * @brief: For _ge_VxV_( V, S),
 *
 *          If AVIS_MINVAL(V) >= S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <= V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <  S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >  V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_ge_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_ge_VxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_ge, REL_le,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_SxS( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxS_( V, S),
 *
 *          If AVIS_MINVAL(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_SxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_gt_SxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_gt, REL_lt,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_SxV( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxV_( V, S),
 *
 *          If AVIS_MINVAL(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_gt_SxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_gt, REL_lt,
                    RELMIN, PRF_ARG2 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_VxS( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxV_( V, S),
 *
 *          If AVIS_MINVAL(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_VxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_gt_VxS");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_gt, REL_lt,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *SAACFprf_gt_VxV( node *arg_node, info *arg_info)
 *
 * @brief: For _gt_SxV_( V, S),
 *
 *          If AVIS_MINVAL(V) >  S, return:  genarray( shape(V), TRUE);
 *          If AVIS_MAXVAL(S) <  V, return:  genarray( shape(V), TRUE);
 *
 *          If AVIS_MAXVAL(V) <= S, return:  genarray( shape(V), FALSE);
 *          If AVIS_MINVAL(S) >= V, return:  genarray( shape(V), FALSE);
 *
 *****************************************************************************/
node *
SAACFprf_gt_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_gt_VxV");

    res = saarelat (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info, REL_gt, REL_lt,
                    RELMIN, PRF_ARG1 (arg_node), TRUE, TRUE);

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

    DBUG_ENTER ("SAACFprf_saabind");

    pat = PMprf (1, PMAisPrf (F_saabind), 3, PMvar (1, PMAgetNode (&dim), 0),
                 PMvar (1, PMAgetNode (&shp), 0), PMvar (1, PMAgetNode (&val), 0));

    if (PMmatchFlat (pat, arg_node)) {
        arg3rhs = AVIS_SSAASSIGN (ID_AVIS (val));
        if ((NULL != arg3rhs)
            && (N_ap != NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg3rhs))))) {
            DBUG_PRINT ("CF", ("_saabind_() replaced by assignment"));
            /*
             * This little bit of code that does the actual work is
             * crippled, until we decide what to do about _saabind_()
             * ops where PRF_ARG3 is a formal argument to this function.
             *
            res = DUPdoDupNode( val);
            */
        }
    }
    PMfree (pat);

    DBUG_RETURN (res);
}
