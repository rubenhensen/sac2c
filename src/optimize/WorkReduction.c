/*
 *
 * $Log$
 * Revision 3.4  2001/05/17 12:46:31  nmw
 * MALLOC/FREE changed to Malloc/Free, result of Free() used
 *
 * Revision 3.3  2001/03/22 21:14:58  dkr
 * include of tree.h elimianted
 *
 * Revision 3.2  2001/02/13 17:21:26  dkr
 * MakeNode() eliminated
 *
 * Revision 3.1  2000/11/20 18:00:39  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:41:44  sacbase
 * new release made
 *
 * Revision 1.3  1995/12/30 17:00:48  cg
 * removed external declaration of 'filename'
 *
 * Revision 1.2  1995/04/05  14:01:53  asi
 * added WRfundef
 *
 * Revision 1.1  1995/03/17  17:45:41  asi
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "WorkReduction.h"

/*
 *
 *  functionname  : WorkReduction
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates work reduction for the intermediate sac-code:
 *  global vars   : syntax_tree, act_tab, wr_stack
 *
 */

node *
WorkReduction (node *arg_node, node *info_node)
{
    DBUG_ENTER ("WorkReduction");
    act_tab = wr_tab;
    info_node = MakeInfo ();

    arg_node = Trav (arg_node, info_node);

    info_node = FreeTree (info_node);
    DBUG_RETURN (arg_node);
}

node *
WRfundef (node *arg_node, node *info_node)
{
    DBUG_ENTER ("WRfundef");

    DBUG_RETURN (arg_node);
}
