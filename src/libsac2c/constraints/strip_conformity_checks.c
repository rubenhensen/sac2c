/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup scc Strip Conformity Checks
 *
 * Removes all conformity checks from the dataflow graph so that they
 * can be safely removed by dead code removal.
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
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lhs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
 * @fn node *SCCdoStripConformityChecks( node *syntax_tree)
 *
 *****************************************************************************/
node *
SCCdoStripConformityChecks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SCCdoStripConformityChecks");

    info = MakeInfo ();

    DBUG_PRINT ("SCC", ("Starting strip conformity checks traversal."));

    TRAVpush (TR_scc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("TEMP", ("Strip conformity checks traversal complete."));

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
 * @fn node *RenameRets( int skip, int no, node *ids, node *args)
 *
 * @brief Replaces the first no LHS IDS_AVIS with the corresponding
 *        RHS ID_AVIS by setting AVIS_SUBST accordingly. The
 *        first skip arguments are ignored.
 *
 *****************************************************************************/
static node *
RenameRets (int skip, int no, node *ids, node *args)
{
    DBUG_ENTER ("RenameRets");

    if (skip != 0) {
        ids = RenameRets (skip - 1, no, ids, EXPRS_NEXT (args));
    } else if (no != 0) {
        IDS_NEXT (ids) = RenameRets (skip, no - 1, IDS_NEXT (ids), EXPRS_NEXT (args));

        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (args)) == N_id),
                     "non-N_id arg to prf found!");

        AVIS_SUBST (IDS_AVIS (ids)) = ID_AVIS (EXPRS_EXPR (args));
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
 *        vardec chain. Furthermore, the INFO_LHS is stacked.
 *
 *****************************************************************************/
node *
SCCblock (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ("SCCblock");

    oldlhs = INFO_LHS (arg_info);
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    INFO_LHS (arg_info) = oldlhs;

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
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
    DBUG_ENTER ("SCClet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

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
    DBUG_ENTER ("SCCprf");

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    switch (PRF_PRF (arg_node)) {
    /* prfs with one identitie on first arg */
    case F_guard:
    case F_afterguard:
    case F_non_neg_val_V:
        INFO_LHS (arg_info) = RenameRets (0, 1, INFO_LHS (arg_info), PRF_ARGS (arg_node));
        break;

    /* prfs with two identities on first args */
    case F_same_shape_AxA:
    case F_shape_matches_dim_VxA:
    case F_val_matches_shape_VxA:
    case F_val_matches_val_VxV:
    case F_prod_matches_prod_shape_VxA:
        INFO_LHS (arg_info) = RenameRets (0, 2, INFO_LHS (arg_info), PRF_ARGS (arg_node));
        break;

    /* prfs with one identity on second arg */
    case F_type_constraint:
        INFO_LHS (arg_info) = RenameRets (1, 1, INFO_LHS (arg_info), PRF_ARGS (arg_node));
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
    DBUG_ENTER ("SCCid");

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCCavis(node *arg_node, info *arg_info)
 *
 * @brief Resets the AVIS_SUBST attribute to NULL.
 *
 *****************************************************************************/
node *
SCCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCCavis");

    AVIS_SUBST (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
