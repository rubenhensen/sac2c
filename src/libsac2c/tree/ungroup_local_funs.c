/*****************************************************************************
 *
 * file:   ungroup_local_funs.c
 *
 * prefix: UGLF
 *
 * description:
 *
 *   This traversal dissolves local fundef chains into the global fundef spine.
 *
 *****************************************************************************/

/**
 * Includes
 */

#include "ungroup_local_funs.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

/**
 * Traversal start function
 */

node *
UGLFdoUngroupLocalFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = NULL;

    TRAVpush (TR_uglf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/**
 * Traversal functions
 */

node *
UGLFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    global.local_funs_grouped = FALSE;

    DBUG_RETURN (arg_node);
}

node *
UGLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    FUNDEF_NEXT (arg_node)
      = TCappendFundef (FUNDEF_LOCALFUNS (arg_node), FUNDEF_NEXT (arg_node));

    FUNDEF_LOCALFUNS (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
