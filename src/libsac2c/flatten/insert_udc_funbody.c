#include <stdio.h>
#include "globals.h"

#define DBUG_PREFIX "IUCFB"
#include "debug.h"

#include "types.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "handle_mops.h"
#include "while2do.h"
#include "handle_condexpr.h"
#include "namespaces.h"

#include "insert_udc_funbody.h"

node *
IUCFBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

node *
IUCFBudcs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* IUCFBdoInsertUdc2Funbody(node* syntax_tree)
 *
 * description:
 *   Insert udcs(user-defined constraints to function body).
 *
 ******************************************************************************/

node *
IUCFBdoInsertUdc2Funbody (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_iucfb);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
#undef DBUG_PREFIX
