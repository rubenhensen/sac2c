/*
 * $Log$
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
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "ToOldTypes.h"

/***************************************************************
 *
 * function:
 *   node *TOTcast(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTcast (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTcast");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != CAST_NTYPE (arg_node)), "missing ntype information");

    if (CAST_TYPE (arg_node) != NULL)
        CAST_TYPE (arg_node) = FreeAllTypes (CAST_TYPE (arg_node));

    CAST_TYPE (arg_node) = TYType2OldType (CAST_NTYPE (arg_node));

    CAST_NTYPE (arg_node) = TYFreeType (CAST_NTYPE (arg_node));
#endif

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *  node *TOTarray(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTarray (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTarray");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != ARRAY_NTYPE (arg_node)), "missing ntype information");

    if (ARRAY_TYPE (arg_node) != NULL)
        ARRAY_TYPE (arg_node) = FreeAllTypes (ARRAY_TYPE (arg_node));

    ARRAY_TYPE (arg_node) = TYType2OldType (ARRAY_NTYPE (arg_node));

    ARRAY_NTYPE (arg_node) = TYFreeType (ARRAY_NTYPE (arg_node));
#endif

    DBUG_RETURN (arg_node);
}

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
TOTlet (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTlet");

    if (LET_EXPR (arg_node) != NULL)
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
TOTassign (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTassign");

    if (ASSIGN_INSTR (arg_node) != NULL)
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL)
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

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
TOTvardec (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTvardec");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != VARDEC_NTYPE (arg_node)), "missing ntype information");

    if (VARDEC_TYPE (arg_node) != NULL)
        VARDEC_TYPE (arg_node) = FreeAllTypes (VARDEC_TYPE (arg_node));

    VARDEC_TYPE (arg_node) = TYType2OldType (VARDEC_NTYPE (arg_node));

    VARDEC_NTYPE (arg_node) = TYFreeType (VARDEC_NTYPE (arg_node));
#endif

    if (VARDEC_NEXT (arg_node) != NULL)
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);

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
TOTarg (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTarg");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != ARG_NTYPE (arg_node)), "missing ntype information");

    if (ARG_TYPE (arg_node) != NULL)
        ARG_TYPE (arg_node) = FreeAllTypes (ARG_TYPE (arg_node));

    ARG_TYPE (arg_node) = TYType2OldType (ARG_NTYPE (arg_node));

    ARG_NTYPE (arg_node) = TYFreeType (ARG_NTYPE (arg_node));
#endif

    if (ARG_NEXT (arg_node) != NULL)
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

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
TOTblock (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTblock");

    if (BLOCK_INSTR (arg_node) != NULL)
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL)
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
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
TOTfundef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTfundef");

    if (FUNDEF_BODY (arg_node) != NULL)
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    if (FUNDEF_ARGS (arg_node) != NULL)
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);

#ifdef MWE_NTYPE_READY

#endif

    if (FUNDEF_NEXT (arg_node) != NULL)
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTobjdef(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTobjdef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTobjdef");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != OBJDEF_NTYPE (arg_node)), "missing ntype information");

    if (OBJDEF_TYPE (arg_node) != NULL)
        OBJDEF_TYPE (arg_node) = FreeAllTypes (OBJDEF_TYPE (arg_node));

    OBJDEF_TYPE (arg_node) = TYType2OldType (OBJDEF_NTYPE (arg_node));

    OBJDEF_NTYPE (arg_node) = TYFreeType (OBJDEF_NTYPE (arg_node));
#endif

    if (OBJDEF_NEXT (arg_node) != NULL)
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/***************************************************************
 *
 * function:
 *   node *TOTtypedef(node *arg_node, node *arg_info)
 *
 *
 * description:
 *   Transforms ntype-structure to types-structure.
 *   Removes ntype-structure.
 *
 ***************************************************************/

node *
TOTtypedef (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("TOTtypedef");

#ifdef MWE_NTYPE_READY
    DBUG_ASSERT ((NULL != TYPEDEF_NTYPE (arg_node)), "missing ntype information");

    if (TYPEDEF_TYPE (arg_node) != NULL)
        TYPEDEF_TYPE (arg_node) = FreeAllTypes (TYPEDEF_TYPE (arg_node));

    TYPEDEF_TYPE (arg_node) = TYType2OldType (TYPEDEF_NTYPE (arg_node));

    TYPEDEF_NTYPE (arg_node) = TYFreeType (TYPEDEF_NTYPE (arg_node));
#endif

    if (TYPEDEF_NEXT (arg_node) != NULL)
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);

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
ToOldTypes (node *syntax_tree)
{
    funtab *old_tab;

    DBUG_ENTER ("ToOldTypes");

    old_tab = act_tab;
    act_tab = tot_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}
