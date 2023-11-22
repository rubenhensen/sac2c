/** <!--********************************************************************-->
 *
 * @defgroup esv Eliminate Shape Variables
 *
 * This traversal removes all expressions for dim and shape from AVIS nodes.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file elim_shapevars.c
 *
 * Prefix: ESV
 *
 *****************************************************************************/
#include "elim_shapevars.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "ESV"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lhs;
};

#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;

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
 * @fn node *ESVdoEliminateShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ESVdoEliminateShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "ESV called on non N_module node");

    TRAVpush (TR_esv);
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
static void
ResetAvis (node *avis)
{
    DBUG_ENTER ();

    AVIS_HASDTTHENPROXY (avis) = FALSE;
    AVIS_HASDTELSEPROXY (avis) = FALSE;

    if (TRUE == AVIS_HASSAAARGUMENTS (avis)) {
        AVIS_HASSAAARGUMENTS (avis) = FALSE;
    }

    AVIS_DIM (avis) = FREEoptFreeNode(AVIS_DIM (avis));

    AVIS_SHAPE (avis) = FREEoptFreeNode(AVIS_SHAPE (avis));

    AVIS_SUBST (avis) = NULL;

    DBUG_RETURN ();
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
 * @fn node *ESVfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ResetAvis (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (PRF_PRF (arg_node) == F_saabind) {
        AVIS_SUBST (IDS_AVIS (INFO_LHS (arg_info))) = ID_AVIS (PRF_ARG3 (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESVid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/

#undef DBUG_PREFIX
