/*
 *
 * $Log$
 * Revision 1.1  2000/03/17 15:55:04  dkr
 * Initial revision
 *
 */

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

/*
 * usage of arg_info (INFO_...)
 * ------------------------------------
 *
 * ...
 */

/******************************************************************************
 *
 * Function:
 *   node *CleanupDecls( node *syntax_tree)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CleanupDecls (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("CleanupDecls");

    info_node = MakeInfo ();

    act_tab = cudecls_tab;
    syntax_tree = Trav (syntax_tree, info_node);

    info_node = FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
