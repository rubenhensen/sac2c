/*
 *
 * $Log$
 * Revision 1.1  1995/04/05 15:16:10  asi
 * Initial revision
 *
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
#include "LoopInvariantRemoval.h"

extern char filename[]; /* is set temporary; will be set later on in main.c */

/*
 *
 *  functionname  : LoopInvariantRemoval
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates loop invariant removal reduction for the intermediate
 *                  sac-code
 *  global vars   : syntax_tree, act_tab, wr_stack
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 *
 */
node *
LoopInvariantRemoval (node *arg_node, node *info_node)
{
    DBUG_ENTER ("LoopInvariantRemoval");
    act_tab = lir_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}
