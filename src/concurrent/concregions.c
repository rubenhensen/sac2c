/*
 *
 * $Log$
 * Revision 1.3  1998/04/03 21:07:33  dkr
 * changed ConcAssign
 *
 * Revision 1.2  1998/04/03 11:59:40  dkr
 * include order is now correct :)
 *
 * Revision 1.1  1998/04/03 11:37:21  dkr
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"

#include "DupTree.h"
#include "dbug.h"

#include "concregions.h"

/******************************************************************************
 *
 * function:
 *   node *ConcAssign(node *arg_node, node *arg_info)
 *
 * description:
 *   at the moment we simply generate one concurrent region for each
 *    first level with-loop.
 *
 ******************************************************************************/

node *
ConcAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ConcAssign");

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_Nwith)) {
        /*
         * current assignment contains a with-loop
         *  -> create a concurrent region containing the current assignment only
         */
        ASSIGN_INSTR (arg_node) = MakeConc (MakeAssign (ASSIGN_INSTR (arg_node), NULL));
        /*
         * we only traverse the following assignments to prevent nested
         *  concurrent regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ConcRegions(node *syntax_tree)
 *
 * description:
 *   in this compiler phase we mark the regions of the syntax-tree
 *    (chains of assignments actually) we want to generate
 *    concurrent C-code for.
 *
 ******************************************************************************/

node *
ConcRegions (node *syntax_tree)
{
    DBUG_ENTER ("ConcRegions");

    /*
     * set new function-table for traverse
     */
    act_tab = concregions_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_RETURN (syntax_tree);
}
