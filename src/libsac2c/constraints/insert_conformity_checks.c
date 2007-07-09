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
#include "memory.h"
#include "insert_domain_constraints.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *postassigns;
    node *lhs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_POSTASSIGNS (result) = NULL;
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

#if 0
/** <!--********************************************************************-->
 *
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static 
node *DummyStaticHelper(node *arg_node)
{
  DBUG_ENTER( "DummyStaticHelper");

  DBUG_RETURN( arg_node);
}

#endif

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

    if (FUNDEF_BODY (arg_node) != NULL) {
        arg_node = IDCinitialize (arg_node, FALSE);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

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
    DBUG_ENTER ("ICCassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_POSTASSIGNS (arg_info)) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("ICCprf");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Conformity Checks -->
 *****************************************************************************/
