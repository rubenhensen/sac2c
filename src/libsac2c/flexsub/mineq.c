
/** <!--********************************************************************-->
 *
 * @file mineq.c
 *
 * prefix: TFMIN
 *
 * description: This file generates a minimum equivalent graph from the
 * subtyping relationships.
 *
 * TODO: This algorithm assumes a single edge between two vertices. If there are
 * multiple edges between two pairs of vertices, the algorithm may trip.
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "graphtypes.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "mineq.h"
#include "graphutils.h"

/** <!--********************************************************************-->
 *
 * @fn node *TFMINdoReduceGraph( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TFMINdoReduceTFGraph (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_tfmin);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFMINtfdag( node *arg_node, info *arg_info)
 *
 *   @brief  For each vertex sorted in topological order, run the TFMINtfvertex
 *   function
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TFMINtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    nodelist *nl;
    compinfo *ci = TFDAG_INFO (arg_node);

    if (ci != NULL) {
        nl = COMPINFO_TOPOLIST (ci);
        while (nl != NULL) {
            TRAVdo (NODELIST_NODE (nl), arg_info);
            nl = NODELIST_NEXT (nl);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFMINtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here. If we find that for a given
 *   edge, there is an alternate path from the source to the target of the edge,
 *   we identify the edge as a superfluous edge and remove it.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFMINtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs, *parents_itr1, *parents_itr2, *edge;
    nodelist *nl_next;
    int children_visited, total_children, inlist = 0;

    defs = arg_node;

    parents_itr1 = TFVERTEX_PARENTS (defs);

    while (parents_itr1 != NULL) {

        /*
         * Check if the parent belongs to the ancestor list of any of the other
         * parents of the vertex
         */

        parents_itr2 = TFVERTEX_PARENTS (defs);

        while (parents_itr2 != NULL) {

            inlist = GUvertInList (TFEDGE_TARGET (parents_itr1),
                                   TFVERTEX_ANCESTORS (TFEDGE_TARGET (parents_itr2)));

            if (inlist)
                break;

            parents_itr2 = TFEDGE_NEXT (parents_itr2);
        }

        if (inlist) {

            CTIwarn (EMPTY_LOC, "Removing superfluous edge between %d and %d.\n",
                     TFVERTEX_PRE (TFEDGE_TARGET (parents_itr1)),
                     TFVERTEX_PRE (arg_node));
            edge = parents_itr1;
            parents_itr1 = TFEDGE_NEXT (parents_itr1);
            GUremoveEdge (TFEDGE_TARGET (edge), arg_node);

        } else {

            parents_itr1 = TFEDGE_NEXT (parents_itr1);
        }
    }

    /*
     * Now we should check to find if there are any parent vertices for whom all
     * its children have been visited. In this case, we can free the ancestor set
     * of the vertex.
     */

    parents_itr1 = TFVERTEX_PARENTS (defs);

    while (parents_itr1 != NULL) {

        /*
         * Increment the number of children visited during creation of the minimum
         * equivalent graph for each parent.
         */
        children_visited = TFVERTEX_MINEQCHILDVISITS (TFEDGE_TARGET (parents_itr1))++;
        total_children = TFVERTEX_NUMCHILDREN (TFEDGE_TARGET (parents_itr1));

        /*
         * Build the ancestor list of the node.
         */

        TFVERTEX_ANCESTORS (arg_node)
          = GUmergeLists (TFVERTEX_ANCESTORS (arg_node),
                          TFVERTEX_ANCESTORS (TFEDGE_TARGET (parents_itr1)));

        nl_next = TFVERTEX_ANCESTORS (arg_node);
        TFVERTEX_ANCESTORS (arg_node) = (nodelist *)MEMmalloc (sizeof (nodelist));
        NODELIST_NODE (TFVERTEX_ANCESTORS (arg_node)) = TFEDGE_TARGET (parents_itr1);
        NODELIST_NEXT (TFVERTEX_ANCESTORS (arg_node)) = nl_next;

        /*
         * If the number of children visited equals the total number of children for
         * any of the parents, we are done with that parent and we can free its
         * ancestor list.
         */

        if (children_visited == total_children) {
            TFVERTEX_ANCESTORS (TFEDGE_TARGET (parents_itr1))
              = FREEfreeNodelist (TFVERTEX_ANCESTORS (TFEDGE_TARGET (parents_itr1)));
        }

        parents_itr1 = TFEDGE_NEXT (parents_itr1);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
