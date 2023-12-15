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

#define DBUG_PREFIX "ICC"
#include "debug.h"

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
#include "free.h"
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
    node *wlguardids;
    node *generator;
    node *withops;
    node *cexprs;
};

/**
  A template entry in the template info structure
 */
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WLGUARDIDS(n) ((n)->wlguardids)
#define INFO_GENERATOR(n) ((n)->generator)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_CEXPRS(n) ((n)->cexprs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_POSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WLGUARDIDS (result) = NULL;
    INFO_GENERATOR (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_CEXPRS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 * @fn node *EmitAfterguards( node **lhs, node **assigns, node *cids
 *                                       node **vardecs)
 *
 * @brief Given an N_ids chain and an N_id chain of guards, this function
 *        appends an afterguard assignment to assigns for each N_ids node
 *        using the cids. The lhs and argument is consumed.
 *
 * @param lhs      lhs N_ids to use as template
 * @param assigns  assign to use as SSA_ASSIGN for new avis nodes
 * @param cids     N_id chain of guards
 * @param vardecs  resulting vardecs
 *
 * @return new N_ids chain
 ******************************************************************************/
static node *
EmitAfterguards (node **lhs, node **assigns, node *cids, node **vardecs)
{
    node *result;
    node *avis;

    DBUG_ENTER ();

    if (*lhs != NULL) {
        result = EmitAfterguards (&IDS_NEXT (*lhs), assigns, cids, vardecs);

        DBUG_ASSERT (IDS_NEXT (*lhs) == NULL, "N_ids has not been consumed!");

        DBUG_PRINT (" ...emitting afterguard");

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (IDS_AVIS (*lhs))));
        *vardecs = TBmakeVardec (avis, *vardecs);

        *assigns
          = TBmakeAssign (TBmakeLet (*lhs, TBmakePrf (F_afterguard,
                                                      TBmakeExprs (TBmakeId (avis),
                                                                   DUPdoDupTree (cids)))),
                          *assigns);

        /* swap SSAASSIGNS */
        AVIS_SSAASSIGN (avis) = AVIS_SSAASSIGN (IDS_AVIS (*lhs));
        AVIS_SSAASSIGN (IDS_AVIS (*lhs)) = *assigns;

        /* ids has been consumed */
        *lhs = NULL;

        result = TBmakeIds (avis, result);
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
ArgEncodingToTypeConstraint (prf fun, unsigned int argno, ntype *scalartype)
{
    ntype *result = NULL;

    DBUG_ENTER ();

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
        DBUG_UNREACHABLE ("unknown arg encoding found!");
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *EmitConstraint( node *ids, node *constraint)
 *
 * @brief Emits the given constraint and appends the corresponding
 *        guard expression, if any, to the Exprs chain ids.
 *
 * @param ids        chain of N_exprs node
 * @param constraint the constraint (N_ap or N_prf)
 *
 * @return the amended N_exprs chain
 ******************************************************************************/
static node *
EmitConstraint (node *ids, node *constraint)
{
    node *avis;

    DBUG_ENTER ();

#ifndef DBUG_OFF
    if (NODE_TYPE (constraint) == N_prf) {
        DBUG_PRINT ("...emitting %s", PRF_NAME (PRF_PRF (constraint)));
    } else {
        DBUG_PRINT ("...emitting fun constraint");
    }
#endif

    avis = IDCaddFunConstraint (constraint);

    if (avis != NULL) {
        ids = TBmakeExprs (TBmakeId (avis), ids);
    }

    DBUG_RETURN (ids);
}

static node *
EmitTypeConstraint (node *ids, node *arg, ntype *constraint)
{
    node *cavis;

    DBUG_ENTER ();

    if (NODE_TYPE (arg) == N_id) {
        cavis = IDCaddTypeConstraint (constraint, ID_AVIS (arg));

        if (cavis != NULL) {
            ids = TBmakeExprs (TBmakeId (cavis), ids);
        }
    }

    DBUG_RETURN (ids);
}

static node *
ICCnone (node *ids, node *args)
{
    DBUG_ENTER ();

    DBUG_RETURN (ids);
}

static node *
ICCsameShape (node *ids, node *args)
{
    DBUG_ENTER ();

    ids = EmitConstraint (ids, TBmakePrf (F_same_shape_AxA, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCreshape (node *ids, node *args)
{
    DBUG_ENTER ();

    ids = EmitConstraint (ids,
                          TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR (args))));
    ids = EmitConstraint (ids,
                          TBmakePrf (F_prod_matches_prod_shape_VxA, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCsel (node *ids, node *args)
{
    DBUG_ENTER ();

    ids = EmitConstraint (ids, TBmakePrf (F_shape_matches_dim_VxA, DUPdoDupTree (args)));
    ids = EmitConstraint (ids,
                          TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR (args))));
    ids = EmitConstraint (ids, TBmakePrf (F_val_lt_shape_VxA, DUPdoDupTree (args)));

    DBUG_RETURN (ids);
}

static node *
ICCsimd_sel (node *ids, node *args)
{
    DBUG_ENTER ();
    /* Throw away the first arg, and do the same as for a standard
     * selection.  The first argument should be just a number which
     * might be checked elsewhere.
     */
    args = EXPRS_NEXT (args);
    DBUG_RETURN (ICCsel (ids, args));
}

static node *
ICCprfModarray (node *ids, node *args)
{
    DBUG_ENTER ();

    ids = EmitConstraint (ids, TCmakePrf2 (F_shape_matches_dim_VxA,
                                           DUPdoDupTree (EXPRS_EXPR2 (args)),
                                           DUPdoDupTree (EXPRS_EXPR (args))));
    ids = EmitConstraint (ids, TCmakePrf1 (F_non_neg_val_V,
                                           DUPdoDupTree (EXPRS_EXPR2 (args))));
    ids = EmitConstraint (ids, TCmakePrf2 (F_val_lt_shape_VxA,
                                           DUPdoDupTree (EXPRS_EXPR2 (args)),
                                           DUPdoDupTree (EXPRS_EXPR (args))));

    DBUG_RETURN (ids);
}

static node *
ICCvalMatchLen (node *ids, node *args)
{
    DBUG_ENTER ();

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
 * @brief Performs a traversal of the fundef chain and inserts INFO_VARDECS into
 * the fundefs vardec chain. We do not need to traverse fundefs that are marked
 * as guard functions, since conformity checks have already been inserted into
 * these functions by type pattern analysis and resolution.
 *
 *****************************************************************************/
node *
ICCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("traversing %s", FUNDEF_NAME (arg_node));

        arg_node = IDCinitialize (arg_node, FALSE);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDECS (arg_node) =
                TCappendVardec (FUNDEF_VARDECS (arg_node),
                                INFO_VARDECS (arg_info));
            INFO_VARDECS (arg_info) = NULL;
        }

        arg_node = IDCinsertConstraints (arg_node, FALSE);
        arg_node = IDCfinalize (arg_node, FALSE);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCassign(node *arg_node, info *arg_info)
 *
 * @brief Inserts INFO_POSTASSIGN after the current assign but does not
 *        traverse the inserted assigns.
 *
 *****************************************************************************/
node *
ICCassign (node *arg_node, info *arg_info)
{
    node *postassigns;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /*
     * we have to stack the postassigns here, as we do not want
     * to emit domain checks on the domain checks we have just
     * created!
     */
    postassigns = INFO_POSTASSIGNS (arg_info);
    INFO_POSTASSIGNS (arg_info) = NULL;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (postassigns != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (postassigns, ASSIGN_NEXT (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICClet(node *arg_node, info *arg_info)
 *
 * @brief Sets INFO_LHS before traversing the LET_EXPR and inserts
 *        the (potentially) modified INFO_LHS back into the tree.
 *
 *****************************************************************************/
node *
ICClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCprf(node *arg_node, info *arg_info)
 *
 * @brief Emits type and prf constraints for the given prf.
 *
 *****************************************************************************/
node *
ICCprf (node *arg_node, info *arg_info)
{
    node *cids = NULL;
    node *args;
    unsigned int arg_cnt;
    ntype *constraint_type, *scalartype;

    DBUG_ENTER ();

    args = PRF_ARGS (arg_node);
    arg_cnt = 0;

    DBUG_PRINT ("Traversing prf %s...", PRF_NAME (PRF_PRF (arg_node)));

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
                DBUG_PRINT (" ...emitting type constraint");

                cids = EmitTypeConstraint (cids, EXPRS_EXPR (args), constraint_type);

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
        INFO_LHS (arg_info)
          = EmitAfterguards (&INFO_LHS (arg_info), &INFO_POSTASSIGNS (arg_info), cids,
                             &INFO_VARDECS (arg_info));

        cids = FREEdoFreeTree (cids);
    }

    DBUG_PRINT ("Done prf %s...", PRF_NAME (PRF_PRF (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCblock(node *arg_node, info *arg_info)
 *
 * @brief stacks the INFO_POSTASSIGNS and INFO_LHS chains
 *
 *****************************************************************************/
node *
ICCblock (node *arg_node, info *arg_info)
{
    node *postassigns;
    node *lhs;

    DBUG_ENTER ();

    postassigns = INFO_POSTASSIGNS (arg_info);
    INFO_POSTASSIGNS (arg_info) = NULL;
    lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_POSTASSIGNS (arg_info) = postassigns;
    INFO_LHS (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCwith(node *arg_node, info *arg_info)
 *
 * @brief Starts multiple traversals of the withloop.
 *
 *****************************************************************************/
node *
ICCwith (node *arg_node, info *arg_info)
{
    node *guardids;

    DBUG_ENTER ();

    guardids = INFO_WLGUARDIDS (arg_info);
    INFO_WLGUARDIDS (arg_info) = NULL;

    /*
     * generate constraints for parts/generators/cexprs, including those
     * that depend on withops
     */
    if (WITH_PART (arg_node) != NULL) {
        INFO_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_WITHOPS (arg_info) = NULL;
    }

    /*
     * now the self contained constraints in withops
     */
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    /*
     * now go on with the code
     */
    if (WITH_CODE (arg_node) != NULL) {
        INFO_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_WITHOPS (arg_info) = NULL;
    }

    if (INFO_WLGUARDIDS (arg_info) != NULL) {
        INFO_LHS (arg_info)
          = EmitAfterguards (&INFO_LHS (arg_info), &INFO_POSTASSIGNS (arg_info),
                             INFO_WLGUARDIDS (arg_info), &INFO_VARDECS (arg_info));

        INFO_WLGUARDIDS (arg_info) = FREEdoFreeTree (INFO_WLGUARDIDS (arg_info));
    }

    INFO_WLGUARDIDS (arg_info) = guardids;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCgenerator(node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for the generator, Starts a traversal of
 *        INFO_WITHOPS with INFO_GENERATOR set to the current node to
 *        insert generator/operator constraints.
 *
 *****************************************************************************/
node *
ICCgenerator (node *arg_node, info *arg_info)
{
    ntype *constraint_type;

    DBUG_ENTER ();

    /*
     * ensure all are int[.]
     */
    constraint_type = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));

    INFO_WLGUARDIDS (arg_info)
      = EmitTypeConstraint (INFO_WLGUARDIDS (arg_info), GENERATOR_BOUND1 (arg_node),
                            constraint_type);

    INFO_WLGUARDIDS (arg_info)
      = EmitTypeConstraint (INFO_WLGUARDIDS (arg_info), GENERATOR_BOUND2 (arg_node),
                            constraint_type);

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        INFO_WLGUARDIDS (arg_info)
          = EmitTypeConstraint (INFO_WLGUARDIDS (arg_info), GENERATOR_WIDTH (arg_node),
                                constraint_type);
    }

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_WLGUARDIDS (arg_info)
          = EmitTypeConstraint (INFO_WLGUARDIDS (arg_info), GENERATOR_STEP (arg_node),
                                constraint_type);
    }

    constraint_type = TYfreeType (constraint_type);

    /*
     * ensure bounds are non negative
     */

    INFO_WLGUARDIDS (arg_info)
      = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                        TCmakePrf1 (F_non_neg_val_V,
                                    DUPdoDupTree (GENERATOR_BOUND1 (arg_node))));

    INFO_WLGUARDIDS (arg_info)
      = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                        TCmakePrf1 (F_non_neg_val_V,
                                    DUPdoDupTree (GENERATOR_BOUND2 (arg_node))));

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf1 (F_non_neg_val_V,
                                        DUPdoDupTree (GENERATOR_WIDTH (arg_node))));
    }

    if (GENERATOR_STEP (arg_node) != NULL) {
        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf1 (F_non_neg_val_V,
                                        DUPdoDupTree (GENERATOR_STEP (arg_node))));
    }

    /*
     * now generate generate/withop constraints
     */
    if (INFO_WITHOPS (arg_info) != NULL) {
        INFO_GENERATOR (arg_info) = arg_node;
        INFO_WITHOPS (arg_info) = TRAVdo (INFO_WITHOPS (arg_info), arg_info);
        INFO_GENERATOR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCcode(node *arg_node, info *arg_info)
 *
 * @brief Traverses the code and inserts constraints for the cexprs by
 *        traversing INFO_WITHOPS with INFO_CEXPRS set.
 *
 *****************************************************************************/
node *
ICCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /*
     * emit withop/cexprs constraints
     */
    if (INFO_WITHOPS (arg_info) != NULL) {
        INFO_CEXPRS (arg_info) = CODE_CEXPRS (arg_node);
        INFO_WITHOPS (arg_info) = TRAVdo (INFO_WITHOPS (arg_info), arg_info);
        DBUG_ASSERT (INFO_CEXPRS (arg_info) == NULL,
                     "not all cexprs handled by withops!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCgenarray(node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for genarray. For different modes see ICCcode
 *        and ICCgenerator
 *
 *****************************************************************************/
node *
ICCgenarray (node *arg_node, info *arg_info)
{
    ntype *constraint_type;

    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        /*
         * emit generator-dependent constraints
         */
        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf2 (F_same_shape_AxA,
                                        DUPdoDupTree (
                                          GENERATOR_BOUND1 (INFO_GENERATOR (arg_info))),
                                        DUPdoDupTree (GENARRAY_SHAPE (arg_node))));

        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf2 (F_val_le_val_VxV,
                                        DUPdoDupTree (
                                          GENERATOR_BOUND1 (INFO_GENERATOR (arg_info))),
                                        DUPdoDupTree (GENARRAY_SHAPE (arg_node))));

        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf2 (F_same_shape_AxA,
                                        DUPdoDupTree (
                                          GENERATOR_BOUND2 (INFO_GENERATOR (arg_info))),
                                        DUPdoDupTree (GENARRAY_SHAPE (arg_node))));

        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf2 (F_val_le_val_VxV,
                                        DUPdoDupTree (
                                          GENERATOR_BOUND2 (INFO_GENERATOR (arg_info))),
                                        DUPdoDupTree (GENARRAY_SHAPE (arg_node))));
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        /*
         * emit cexpr constraints iff we have a default value
         */
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            DBUG_PRINT ("...emitting F_same_shape_AxA CEXPR-constraint");

            /*
             * we simply ignore the returned avis, as we do not propagate the
             * boolean guard out of the withloop.
             */
            IDCaddFunConstraint (
              TCmakePrf2 (F_same_shape_AxA,
                          DUPdoDupTree (EXPRS_EXPR (INFO_CEXPRS (arg_info))),
                          DUPdoDupTree (GENARRAY_DEFAULT (arg_node))));
        }

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        /*
         * emit withop-local constraints
         */
        constraint_type = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));

        INFO_WLGUARDIDS (arg_info)
          = EmitTypeConstraint (INFO_WLGUARDIDS (arg_info), GENARRAY_SHAPE (arg_node),
                                constraint_type);
        constraint_type = TYfreeType (constraint_type);

        INFO_WLGUARDIDS (arg_info)
          = EmitConstraint (INFO_WLGUARDIDS (arg_info),
                            TCmakePrf1 (F_non_neg_val_V,
                                        DUPdoDupTree (GENARRAY_SHAPE (arg_node))));
    }

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCmodarray(node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for modarray. For different modes see ICCcode
 *        and ICCgenerator
 *
 *****************************************************************************/
node *
ICCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        /*
         * emit generator dependent constraints
         */
        /* TODO */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        /*
         * emit cexpr constraints
         */
        /* TODO */

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        /*
         * emit withop-local constraints
         */
    }

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCfold(node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for fold. For different modes see ICCcode
 *        and ICCgenerator
 *
 *****************************************************************************/
node *
ICCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        /*
         * emit generator dependent constraints
         */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        /*
         * emit cexpr constraints
         */

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        /*
         * emit withop-local constraints
         */
    }

    FOLD_NEXT (arg_node) = TRAVopt(FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ICCpropagate(node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for propagate. For different modes see ICCcode
 *        and ICCgenerator
 *
 *****************************************************************************/
node *
ICCpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        /*
         * emit generator dependent constraints
         */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        /*
         * emit cexpr constraints
         */

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        /*
         * emit withop-local constraints
         */
    }

    PROPAGATE_NEXT (arg_node) = TRAVopt(PROPAGATE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Conformity Checks -->
 *****************************************************************************/

#undef DBUG_PREFIX
