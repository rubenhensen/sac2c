/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup icc Insert Conformity Checks
 *
 * Module description goes here.
 *
 * @ingroup icc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file insert_conformity_checks.c
 *
 * Prefix: ICC
 *
 *****************************************************************************/
#include "insert_conformity_checks.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "new_typecheck.h"
#include "globals.h"
#include "memory.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "insert_domain_constraints.h"

typedef node *(*iccfun_p) (node *, node *);

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *postassigns;
    node *vardecs;
    node *lhs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_POSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *ICCdoInsertConformityChecks( node *syntax_tree)
 *
 *****************************************************************************/
node *
ICCdoInsertConformityChecks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ICCdoInsertConformityChecks");

    info = MakeInfo ();

    TRAVpush (TR_icc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *GenerateIdsAndPrependArgs( node *lhs, node *assign, node **cids
 *                                      node **vardecs)
 *
 * @brief Given an N_ids chain, we generate a new N_ids chain of
 *        fresh variables of same length and prepend corresponding
 *        N_id nodes the the cids N_exprs chain.
 *
 * @param lhs      lhs N_ids to use as template
 * @param assign   assign to use as SSA_ASSIGN for new avis nodes
 * @param *cids    N_id chain to prepend to
 * @param *vardecs resulting vardecs
 *
 * @return new N_ids chain
 ******************************************************************************/
static node *
GenerateIdsAndPrependArgs (node *lhs, node *assign, node **cids, node **vardecs)
{
    node *result;
    node *avis;

    DBUG_ENTER ("GenerateIdsAndPrependArgs");

    if (lhs != NULL) {
        result = GenerateIdsAndPrependArgs (IDS_NEXT (lhs), assign, cids, vardecs);

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (IDS_AVIS (lhs))));
        AVIS_SSAASSIGN (avis) = assign;

        result = TBmakeIds (avis, result);
        *cids = TBmakeExprs (TBmakeId (avis), *cids);
        *vardecs = TBmakeVardec (avis, *vardecs);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *ArgEncodingToTypeConstraint( prf fun, int argno,
 *                                         ntype *scalartype)
 * @brief Returns a type constraint for the given prf argument and scalartype.
 *
 * @param fun        prf
 * @param argno      no of argument
 * @param scalartype the scalartype for the constraint
 *
 * @return constraint type or NULL if none needed
 ******************************************************************************/
static ntype *
ArgEncodingToTypeConstraint (prf fun, int argno, ntype *scalartype)
{
    ntype *result = NULL;

    DBUG_ENTER ("ArgEncodingToTypeConstraint");

    switch (PRF_ARGENCODING (fun, argno)) {
    case PA_S:
        result = TYmakeAKS (TYcopyType (scalartype), SHmakeShape (0));
        break;
    case PA_V:
        result = TYmakeAKD (TYcopyType (scalartype), 1, SHmakeShape (0));
        break;
    case PA_A:
    case PA_x:
        /* nothing to be done here */
        break;
    default:
        DBUG_ASSERT (0, "unknown arg encoding found!");
    }

    DBUG_RETURN (result);
}

static node *
EmitConstraint (node *ids, node *constraint)
{
    node *avis;

    DBUG_ENTER ("EmitConstraint");

#ifndef DBUG_OFF
    if (NODE_TYPE (constraint) == N_prf) {
        DBUG_PRINT ("ICC", ("...emitting %s", PRF_NAME (PRF_PRF (constraint))));
    } else {
        DBUG_PRINT ("ICC", ("...emitting fun constraint"));
    }
#endif

    avis = IDCaddFunConstraint (constraint);

    if (avis != NULL) {
        ids = TBmakeExprs (TBmakeId (avis), ids);
    }

    DBUG_RETURN (ids);
}

static node *
ICCnone (node *ids, node *args)
{
    DBUG_ENTER ("ICCnone");

    DBUG_RETURN (ids);
}

static node *
ICCsameShape (node *ids, node *args)
{
    DBUG_ENTER ("ICCsameShape");

    ids = EmitConstraint (ids, TBmakePrf (F_same_shape_VxV, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCreshape (node *ids, node *args)
{
    DBUG_ENTER ("ICCreshape");

    ids = EmitConstraint (ids,
                          TBmakePrf (F_prod_matches_prod_shape_VxA, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCsel (node *ids, node *args)
{
    DBUG_ENTER ("ICCsel");

    ids = EmitConstraint (ids, TBmakePrf (F_shape_matches_dim_VxA, DUPdoDupTree (args)));
    ids = EmitConstraint (ids,
                          TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR (args))));
    ids = EmitConstraint (ids, TBmakePrf (F_val_matches_shape_VxA, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCmodarray (node *ids, node *args)
{
    DBUG_ENTER ("ICCmodarray");

    ids = EmitConstraint (ids, TCmakePrf2 (F_shape_matches_dim_VxA,
                                           DUPdoDupTree (EXPRS_EXPR2 (args)),
                                           DUPdoDupTree (EXPRS_EXPR (args))));
    ids = EmitConstraint (ids, TCmakePrf1 (F_non_neg_val_V,
                                           DUPdoDupTree (EXPRS_EXPR2 (args))));
    ids = EmitConstraint (ids, TCmakePrf2 (F_val_matches_shape_VxA,
                                           DUPdoDupTree (EXPRS_EXPR2 (args)),
                                           DUPdoDupTree (EXPRS_EXPR (args))));

    DBUG_RETURN (ids);
}

static node *
ICCvalMatchLen (node *ids, node *args)
{
    DBUG_ENTER ("ICCvalMatchLen");

    DBUG_RETURN (ids);
}

static iccfun_p iccfuns[] = {
#define PRFicc_fun(icc_fun) icc_fun
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ICCfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
ICCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ICCfundef");

    DBUG_PRINT ("ICC", ("traversing %s:", CTIitemName (arg_node)));

    if (FUNDEF_BODY (arg_node) != NULL) {
        arg_node = IDCinitialize (arg_node, FALSE);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDEC (arg_node)
              = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));

            INFO_VARDECS (arg_info) = NULL;
        }

        arg_node = IDCinsertConstraints (arg_node, FALSE);

        arg_node = IDCfinalize (arg_node, FALSE);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ICCassign (node *arg_node, info *arg_info)
{
    node *postassigns;

    DBUG_ENTER ("ICCassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * we have to stack the postassigns here, as we do not want
     * to emit domain checks on the domain checks we have just
     * created!
     */
    postassigns = INFO_POSTASSIGNS (arg_info);
    INFO_POSTASSIGNS (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (postassigns != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (postassigns, ASSIGN_NEXT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICClet(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
ICClet (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ("ICClet");

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCprf(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
ICCprf (node *arg_node, info *arg_info)
{
    node *cids = NULL;
    node *cavis;
    node *args;
    node *newlhs = NULL;
    node *assign;
    int arg_cnt;
    ntype *constraint_type, *scalartype;

    DBUG_ENTER ("ICCprf");

    args = PRF_ARGS (arg_node);
    arg_cnt = 0;

    DBUG_PRINT ("ICC", ("Beackering prf %s...", PRF_NAME (PRF_PRF (arg_node))));

    while (args != NULL) {
        /*
         * we only act on N_id arguments with
         * proper array types (i.e. non-bottoms)
         */
        if ((NODE_TYPE (EXPRS_EXPR (args)) == N_id)
            && TYisArray (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))))) {
            scalartype = TYgetScalar (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (args))));

            constraint_type
              = ArgEncodingToTypeConstraint (PRF_PRF (arg_node), arg_cnt, scalartype);
            if (constraint_type != NULL) {
                DBUG_PRINT ("ICC", (" ...emitting type constraint"));

                cavis
                  = IDCaddTypeConstraint (constraint_type, ID_AVIS (EXPRS_EXPR (args)));
                if (cavis != NULL) {
                    cids = TBmakeExprs (TBmakeId (cavis), cids);
                }

                constraint_type = TYfreeType (constraint_type);
            }
        }

        args = EXPRS_NEXT (args);
        arg_cnt++;
    }

    /*
     * now handle non-type constraints
     */
    if (iccfuns[PRF_PRF (arg_node)] != NULL) {
        cids = iccfuns[PRF_PRF (arg_node)](cids, PRF_ARGS (arg_node));
    }

    /*
     * if we have collected any constraints, we emit an afterguard
     */
    if (cids != NULL) {
        DBUG_PRINT ("ICC", (" ...emitting afterguard"));
        assign = TBmakeAssign (NULL, NULL);

        newlhs = GenerateIdsAndPrependArgs (INFO_LHS (arg_info), assign, &cids,
                                            &INFO_VARDECS (arg_info));

        ASSIGN_INSTR (assign)
          = TBmakeLet (INFO_LHS (arg_info), TBmakePrf (F_afterguard, cids));
        ASSIGN_NEXT (assign) = INFO_POSTASSIGNS (arg_info);
        INFO_POSTASSIGNS (arg_info) = assign;

        INFO_LHS (arg_info) = newlhs;
    }

    DBUG_PRINT ("ICC", ("Done prf %s...", PRF_NAME (PRF_PRF (arg_node))));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Conformity Checks -->
 *****************************************************************************/
