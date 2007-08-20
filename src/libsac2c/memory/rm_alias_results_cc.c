/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup racc Remove Alias Results from Conformity Checks
 *
 * Various conformity check operations yield aliases their arguments in order
 * to prevent instruction sequences from being shuffled around.
 * At the same time, they return a proper boolean result.
 * To use the memory management's fill operation, we strip the alias results
 * from the following operations:
 *   - type_constraint
 *   - same_shape_AxA
 *   - shape_matches_dim_VxA
 *   - non_neg_val_V
 *   - val_lt_shape_VxA
 *   - val_le_val_VxV
 *   - prod_matches_prod_shape_VxA
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file rm_alias_results_cc.c
 *
 * Prefix: RACC
 *
 *****************************************************************************/
#include "rm_alias_results_cc.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *let;
};

/**
 * A pointer to the last LET node
 */
#define INFO_LET(n) (n->let)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LET (result) = NULL;

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
 * @fn node *EMRACCdoRemoveAliasResultsFromConformityChecks( node *syntax_tree)
 *
 *****************************************************************************/
node *
EMRACCdoRemoveAliasResultsFromConformityChecks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRACCdoRemoveAliasResultsFromConformityChecks");

    info = MakeInfo ();

    DBUG_PRINT ("RACC", ("Starting to remove Alias Results from Conformity Checks."));

    TRAVpush (TR_emracc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("RACC", ("Removal of Alias Results from Conformity Checks complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain entering the body
 *
 *****************************************************************************/
node *
EMRACCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCarg");

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCvardec");

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACClet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACClet");

    INFO_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCprf");

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    switch (PRF_PRF (arg_node)) {
    case F_type_constraint:
        /*
         * v,p = type_constraint( t, a); R
         * => p = type_constraint( t, a); R[v\a]
         */
        AVIS_SUBST (IDS_AVIS (LET_IDS (INFO_LET (arg_info))))
          = ID_AVIS (PRF_ARG2 (arg_node));
        LET_IDS (INFO_LET (arg_info)) = FREEdoFreeNode (LET_IDS (INFO_LET (arg_info)));
        break;

    case F_same_shape_AxA:
        /*
         * va,vb,p = same_shape_AxA(a,b); R
         * => p = same_shape_AxA(a,b); R[va\a][vb\b]
         */
        AVIS_SUBST (IDS_AVIS (LET_IDS (INFO_LET (arg_info))))
          = ID_AVIS (PRF_ARG1 (arg_node));
        LET_IDS (INFO_LET (arg_info)) = FREEdoFreeNode (LET_IDS (INFO_LET (arg_info)));
        AVIS_SUBST (IDS_AVIS (LET_IDS (INFO_LET (arg_info))))
          = ID_AVIS (PRF_ARG2 (arg_node));
        LET_IDS (INFO_LET (arg_info)) = FREEdoFreeNode (LET_IDS (INFO_LET (arg_info)));
        break;

    case F_shape_matches_dim_VxA:
    case F_non_neg_val_V:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
    case F_prod_matches_prod_shape_VxA:
        /*
         * v,p = constraint(a,b); R
         * => p = constraint(a,b) R[v\a]
         */
        AVIS_SUBST (IDS_AVIS (LET_IDS (INFO_LET (arg_info))))
          = ID_AVIS (PRF_ARG1 (arg_node));
        LET_IDS (INFO_LET (arg_info)) = FREEdoFreeNode (LET_IDS (INFO_LET (arg_info)));
        break;

    default:; /* do nothing */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRACCid");

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
