/*
 * $Log$
 * Revision 1.4  2004/12/19 14:33:45  sbs
 * made functional
 *
 * Revision 1.3  2004/12/08 17:59:15  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.2  2004/11/24 17:16:49  mwe
 * SacDevCamp: Compiles!
 *
 * Revision 1.1  2004/11/19 10:50:43  mwe
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   ToOldTypes.h
 *
 * prefix: TOT
 *
 * description:
 *
 *   This module restores all types-structures from ntype-structures.
 *   All ntype-structures will be removed.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "ToOldTypes.h"
#include "type_utils.h"

/***************************************************************
 *
 * function:
 *   node *TOTlet(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Traverses in expr.
 *
 ***************************************************************/

node *
TOTlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TOTlet");

    if (LET_EXPR (arg_node) != NULL)
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTassign(node *arg_node, node *arg_info)
 *
 *
 * description:
 *
 ***************************************************************/
node *
TOTassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TOTassign");

    if (ASSIGN_INSTR (arg_node) != NULL)
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL)
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTvardec(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTvardec (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TOTvardec");

    type = AVIS_TYPE (VARDEC_AVIS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntype information");

    if (VARDEC_TYPE (arg_node) != NULL) {
        VARDEC_TYPE (arg_node) = FREEfreeAllTypes (VARDEC_TYPE (arg_node));
    }

    VARDEC_TYPE (arg_node) = TYtype2OldType (type);

    AVIS_TYPE (VARDEC_AVIS (arg_node)) = TYfreeType (AVIS_TYPE (VARDEC_AVIS (arg_node)));

    if (VARDEC_NEXT (arg_node) != NULL)
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTarg(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TOTarg");

    type = AVIS_TYPE (ARG_AVIS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntype information");

    if (ARG_TYPE (arg_node) != NULL)
        ARG_TYPE (arg_node) = FREEfreeAllTypes (ARG_TYPE (arg_node));

    ARG_TYPE (arg_node) = TYtype2OldType (type);

    AVIS_TYPE (ARG_AVIS (arg_node)) = TYfreeType (AVIS_TYPE (ARG_AVIS (arg_node)));

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTblock(node *arg_node, node *arg_info)
 *
 *
 * description:
 *
 ***************************************************************/

node *
TOTblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TOTblock");

    if (BLOCK_INSTR (arg_node) != NULL)
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL)
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTfundef(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Traverses in sons.
 *   Maybe later also transformation from ntype to types
 *
 ***************************************************************/

node *
TOTfundef (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("TOTfundef");

    if (FUNDEF_BODY (arg_node) != NULL)
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_ARGS (arg_node) != NULL)
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

    if (FUNDEF_TYPES (arg_node) != NULL) {
        FUNDEF_TYPES (arg_node) = FREEfreeAllTypes (FUNDEF_TYPES (arg_node));
    }
    type = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
    DBUG_ASSERT ((type != NULL), "missing ntypes in N_rets!");
    FUNDEF_TYPES (arg_node) = TYtype2OldType (type);
    type = TYfreeType (type);

    if (FUNDEF_NEXT (arg_node) != NULL)
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *ToOldTypes(node *syntax_tree)
 *
 *
 * description:
 *   Starting function of tree-traversal.
 *
 ***************************************************************/

node *
TOTdoToOldTypes (node *syntax_tree)
{

    DBUG_ENTER ("ToOldTypes");

    if (global.compiler_phase > PH_typecheck) {

        TRAVpush (TR_tot);
        syntax_tree = TRAVdo (syntax_tree, NULL);
        TRAVpop ();
    }

    DBUG_RETURN (syntax_tree);
}
