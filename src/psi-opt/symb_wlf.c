/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup symbolic with loop folding
 *
 * Module description goes here.
 *
 * For an example, take a look at src/refcount/explicitcopy.c
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file symb_wlf.c
 *
 * Prefix: SWLF
 *
 *****************************************************************************/
#include "symb_wlf.h"

/*
 * Other includes go here
 */
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    int swlaction;
    node *exprs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_SWLACTION(n) ((n)->swlaction)
#define INFO_EXPRS(n) ((n)->exprs)

typedef enum { SWL_pass, SWL_replace } swl_action_t;

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_SWLACTION (result) = SWL_pass;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 * @fn node *SWLFdoSymbolicWithLoopFolding( node *syntax_tree)
 *
 *****************************************************************************/
node *
SWLFdoSymbolicWithLoopFolding (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SWLFdoSymbolicWithLoopFolding");

    info = MakeInfo ();

    DBUG_PRINT ("SWLF", ("Starting symbolic with loop folding."));

    TRAVpush (TR_swlf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("SWLF", ("Symbolic with loop folding complete."));

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
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static node *
DummyStaticHelper (node *arg_node)
{
    DBUG_ENTER ("DummyStaticHelper");

    DBUG_RETURN (DummyStaticHelper (arg_node));
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
 * @fn node *SWLFfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
SWLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFfundef");

    if (FUNDEF_BODY (arg_node)) {
        DBUG_PRINT ("SWLF", ("Symbolic With-Loops fusion in function %s",
                             FUNDEF_NAME (arg_node)));

        INFO_FUNDEF (arg_info) = arg_node;

        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);

        DBUG_PRINT ("SWLF", ("Symbolic With-Loops fusion in function %s complete",
                             FUNDEF_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node SWLFblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SWLFblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFblock");

    if (BLOCK_INSTR (arg_node)) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node SWLFassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SWLFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFassign");

    if (ASSIGN_INSTR (arg_node)) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_INSTRTYPE (arg_node) == N_let && NODE_TYPE (ASSIGN_RHS (arg_node)) == N_prf
        && PRF_PRF (ASSIGN_RHS (arg_node)) == F_sel) {
        PRTdoPrintNode (ASSIGN_RHS (arg_node));

        if (INFO_SWLACTION (arg_info) == SWL_replace) {
            ;
        }
    }

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node SWLFwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SWLFwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFwith");

    if (WITH_CODE (arg_node)) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
