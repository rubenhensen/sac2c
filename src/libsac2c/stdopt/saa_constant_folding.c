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

    DBUG_ENTER ("SAACF_ids");
    node *res = NULL;

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
 * @result new arg_node if shape() operation could be replaced by
 *         the AVIS_SHAPE of the argument,
 *         else NULL
 *
 ********************************************************************/

node *
SAACFprf_shape (node *arg_node, info *arg_info)
{
    node *shp = NULL;
    node *res = NULL;

    DBUG_ENTER ("SAACFprf_shape");
    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SAACF_shape_ expected N_id node");
    shp = AVIS_SHAPE (ID_AVIS (PRF_ARG1 (arg_node)));
    if (NULL != shp) {
        res = DUPdoDupTree (shp);
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
    if (PM (PMsaashape (&shp, &arg1,
                        PMsaashape (&shp, &arg2,
                                    PMvar (&arg1, PMvar (&arg2, PMprf (F_same_shape_AxA,
                                                                       arg_node))))))) {
        /* See if saa shapes match */

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
 *   b', c', pred = prf_shape_matches_dim_VxS(B,C)
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
 *****************************************************************************/

node *
SAACFprf_shape_matches_dim_VxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
#ifdef CRUD
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *shp = NULL;
#endif // CRUD

    DBUG_ENTER ("SAACFshape_matches_dim_VxA");
#ifdef CRUD

    FIXME
      - write this.if (PM (PMsaashape (&shp, &arg1,
                                       PMsaashape (&shp, &arg2,
                                                   PMvar (&arg1,
                                                          PMvar (&arg2,
                                                                 PMprf (F_same_shape_AxA,
                                                                        arg_node)))))))
    {
        /* See if saa shapes match */

        res = TBmakeExprs (DUPdoDupTree (arg1),
                           TBmakeExprs (DUPdoDupTree (arg2),
                                        TBmakeExprs (TBmakeBool (TRUE), NULL)));
    }
#endif // CRUD
    DBUG_RETURN (res);
}
