/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:47  sacbase
 * new release made
 *
 * Revision 1.2  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.1  1999/01/15 16:58:18  sbs
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

/******************************************************************************
 *
 * function:
 *  node *FreeMasks(node *arg_node)
 *
 * description:
 *   This traversal eliminates all masks needed during optimization.
 *   This is done by using two functions only:
 *     TravSons   for all nodes that do NOT carry masks during
 *                optimization, and
 *     FMTravSons for all nodes that carry mask during optimization,
 *                i.e., N_fundef, N_block, N_assign, N_cond, N_do,
 *                N_while, N_with, N_gen, N_operator, N_Nwithop,
 *                N_Ncode, and N_Npart.
 *
 ******************************************************************************/

node *
FreeMasks (node *arg_node)
{
    funptr *tmp_tab;
    DBUG_ENTER ("FreeMasks");

    DBUG_PRINT ("OPT", ("FREEMASKS"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
    tmp_tab = act_tab;
    act_tab = freemask_tab;

    arg_node = Trav (arg_node, NULL);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *FMTravSons(node *arg_node, node *arg_info)
 *
 * description:
 *   Basically, this function is exactly the same as TravSons, but it also
 *   frees the masks of the node.
 *   ATTENTION: this file breaks the convention of using the appropriate
 *   access macros for the sake of breavity - I hate the type-system of C!!
 *
 ******************************************************************************/

node *
FMTravSons (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FMTravSons");

    for (i = 0; i < MAX_MASK; i++)
        FREE (arg_node->mask[i]);

    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    DBUG_RETURN (arg_node);
}
