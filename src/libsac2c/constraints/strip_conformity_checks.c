/** <!--********************************************************************-->
 *
 * @defgroup scc Strip Conformity Checks
 *
 * Removes all conformity checks from the dataflow graph so that they
 * can be safely removed by dead code removal.
 *
 * This phase also removes extrema and dataflow guards, as they should never
 * reach anywhere beyond the optimization phase.
 *
 * @ingroup scc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file strip_conformity_checks.c
 *
 * Prefix: SCC
 *
 *****************************************************************************/
#include "strip_conformity_checks.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "SCC"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lhs;
    node *preassigns;
    bool scrapassign;
};

/**
 * A template entry in the template info structure
 */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_SCRAPASSIGN(n) ((n)->scrapassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_SCRAPASSIGN (result) = FALSE;

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
 * @fn node *SCCdoStripConformityChecks( node *syntax_tree)
 *
 *****************************************************************************/
node *
SCCdoStripConformityChecks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting strip conformity checks traversal.");

    TRAVpush (TR_scc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT_TAG ("TEMP", "Strip conformity checks traversal complete.");

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
 * @fn node *RenameOrReplaceRets( int skip, int no, node *ids, node *args,
 *                                node **assigns)
 *
 * @brief Replaces the first no LHS IDS_AVIS with the corresponding
 *        RHS ID_AVIS by setting AVIS_SUBST accordingly. The
 *        first skip arguments are ignored.
 *
 * @param skip number of arguments to skip
 * @param no   number of arguments to alias
 * @param ids  lhs ids to be aliased
 * @param args the arguments
 * @param assigns used to append aliasing assignments to
 *
 * @return new lhs ids
 ******************************************************************************/
static node *
RenameOrReplaceRets (int skip, size_t no, node *ids, node *args, node **assigns)
{
    node *tmp;

    DBUG_ENTER ();

    if (skip != 0) {
        ids = RenameOrReplaceRets (skip - 1, no, ids, EXPRS_NEXT (args), assigns);
    } else if (no != 0) {
        IDS_NEXT (ids) = RenameOrReplaceRets (skip, no - 1, IDS_NEXT (ids),
                                              EXPRS_NEXT (args), assigns);

        if (NODE_TYPE (EXPRS_EXPR (args)) == N_id) {
            /*
             * we mark the avis for substitution and deletion
             */
            DBUG_PRINT ("Aliasing %s ...", IDS_NAME (ids));

            DBUG_ASSERT (AVIS_SUBST (IDS_AVIS (ids)) == NULL, "AVIS_SUBST already set!");

            AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (args));
        } else {
            /*
             * insert a substitution assign
             */
            DBUG_PRINT ("Inserting substitution for %s ...", IDS_NAME (ids));

            tmp = ids;
            ids = IDS_NEXT (ids);
            IDS_NEXT (tmp) = NULL;

            *assigns = TBmakeAssign (TBmakeLet (tmp, DUPdoDupTree (EXPRS_EXPR (args))),
                                     *assigns);
            AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
        }
    } else if (ids != NULL) {
        IDS_NEXT (ids) = RenameOrReplaceRets (skip, no, IDS_NEXT (ids), args, assigns);
        /*
         * the remaining lhs identifiers must be guards, so we emit
         * ids = TRUE assigns
         */

        DBUG_PRINT ("Setting %s to TRUE...", IDS_NAME (ids));

        tmp = ids;
        ids = IDS_NEXT (ids);
        IDS_NEXT (tmp) = NULL;

        *assigns = TBmakeAssign (TBmakeLet (tmp, TBmakeBool (TRUE)), *assigns);
        AVIS_SSAASSIGN (IDS_AVIS (tmp)) = *assigns;
    }

    DBUG_RETURN (ids);
}

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
 * @fn node *SCCblock(node *arg_node, info *arg_info)
 *
 * @brief Traverses first the code in the block and then the
 *        vardec chain. Furthermore, the INFO_LHS and INFO_PREASSIGNS
 *        is stacked.
 *
 *****************************************************************************/
node *
SCCblock (node *arg_node, info *arg_info)
{
    node *oldlhs, *preassigns;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;
    preassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    INFO_LHS (arg_info) = oldlhs;
    INFO_PREASSIGNS (arg_info) = preassigns;

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCCassign(node *arg_node, info *arg_info)
 *
 * @brief Inserts preassignments from INFO_PREASSIGN.
 *
 *****************************************************************************/
node *
SCCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (!INFO_SCRAPASSIGN (arg_info), "SCRAPASSIGN already set!");

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SCRAPASSIGN (arg_info)) {
        DBUG_PRINT ("Scrapping assignment...");

        arg_node = FREEdoFreeNode (arg_node);
    }

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        DBUG_PRINT ("Inserting preassigns...");

        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    if (INFO_SCRAPASSIGN (arg_info)) {
        INFO_SCRAPASSIGN (arg_info) = FALSE;
        arg_node = TRAVopt (arg_node, arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCClet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SCClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCCprf(node *arg_node, info *arg_info)
 *
 * @brief Marks conformity check prfs as identity on their first n
 *        arguments by setting AVIS_SUBST.
 *
 *****************************************************************************/
node *
SCCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_PRINT ("Looking at prf %s...", PRF_NAME (PRF_PRF (arg_node)));

    switch (PRF_PRF (arg_node)) {
    /* prfs with one identity on first arg */

    /* dataflow and extrema guards never reach code generator */
    case F_noteminval:
    case F_notemaxval:
    case F_noteintersect:
        INFO_LHS (arg_info)
          = RenameOrReplaceRets (0, 1, INFO_LHS (arg_info), PRF_ARGS (arg_node),
                                 &INFO_PREASSIGNS (arg_info));
        INFO_SCRAPASSIGN (arg_info) = TRUE;
        break;

    case F_afterguard:
    case F_non_neg_val_V:
    case F_non_neg_val_S:
    case F_val_le_val_VxV:
    case F_val_le_val_SxS:
    case F_val_lt_val_SxS:
    case F_shape_matches_dim_VxA:
    case F_val_lt_shape_VxA:
    case F_prod_matches_prod_shape_VxA:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info)
              = RenameOrReplaceRets (0, 1, INFO_LHS (arg_info), PRF_ARGS (arg_node),
                                     &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /* prfs with two identities on first args */
    case F_same_shape_AxA:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info)
              = RenameOrReplaceRets (0, 2, INFO_LHS (arg_info), PRF_ARGS (arg_node),
                                     &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    /* prfs with one identity on second arg */
    case F_type_constraint:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info)
              = RenameOrReplaceRets (1, 1, INFO_LHS (arg_info), PRF_ARGS (arg_node),
                                     &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

        /* prfs with n-1 identities */
    case F_guard:
        if (!global.runtimecheck.conformity && global.insertconformitychecks) {
            INFO_LHS (arg_info)
              = RenameOrReplaceRets (0, TCcountExprs (PRF_ARGS (arg_node)) - 1,
                                     INFO_LHS (arg_info), PRF_ARGS (arg_node),
                                     &INFO_PREASSIGNS (arg_info));
            INFO_SCRAPASSIGN (arg_info) = TRUE;
        }
        break;

    default:; /* do nothing for the rest */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCCid(node *arg_node, info *arg_info)
 *
 * @brief Substitutes the AVIS if needed.
 *
 *****************************************************************************/
node *
SCCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCCvardec(node *arg_node, info *arg_info)
 *
 * @brief Removes unused vardecs.
 *
 *****************************************************************************/
node *
SCCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (VARDEC_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("Removing variable %s...", AVIS_NAME (VARDEC_AVIS (arg_node)));

        arg_node = FREEdoFreeNode (arg_node);
        if (arg_node != NULL) {
            arg_node = TRAVdo (arg_node, arg_info);
        }
    } else {
        VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
