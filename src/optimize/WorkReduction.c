/*
 *
 * $Log$
 * Revision 1.1  1995/03/17 17:45:41  asi
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "WorkReduction.h"

extern char filename[]; /* is set temporary; will be set later on in main.c */

/*
 *
 *  functionname  : WorkReduction
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates work reduction for the intermediate sac-code:
                    - call OptTrav to start constant-folding
 *  global vars   : syntax_tree, act_tab, wr_stack
 *  internal funs : ---
 *  external funs : Trav, MakeNode, MAlloc
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 *
 */
node *
WorkReduction (node *arg_node, node *info_node)
{
    DBUG_ENTER ("WorkReduction");
    act_tab = wr_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    free (info_node);
    DBUG_RETURN (arg_node);
}
