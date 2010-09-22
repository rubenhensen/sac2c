
/** <!--********************************************************************-->
 *
 * @file dfwalk.c
 *
 * prefix: TFTOP
 *
 * description: depth first walk of the subtyping hierarchy
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "dfwalk.h"
#include "structures.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "topo.h"

/*
 * INFO structure
 * pre is the topological id for the vertices in the dependency graph.
 */

struct INFO {
    int topo;
};

/*
 * INFO macros
 */
#define INFO_TOPO(n) n->topo

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_TOPO (result) = 1;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTOPdoTOPalk( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFTOPdoTopoSort (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TFTOPdoTopoSort");

    arg_info = MakeInfo ();

    TRAVpush (TR_tftop);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTOPtfspec( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We loop through the defs in the type family specifications to
 *   identify any potential root nodes and start a topological sort if we find one.
 *
 *   TODO: Ensure that DAG is a DAG by adding a top vertex if required.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFTOPtfspec (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TFTOPtfspec");

    node *defs;

    defs = TFSPEC_DEFS (arg_node);

    /*
     * First label nodes for tree reachability
     */

    while (defs != NULL) {
        if (TFVERTEX_PARENTS (defs) == NULL) {
            /*
             * Here could be the start of a potentially new type
             * component since we have multiple components in a type.
             *
             * For example, \alpha and [*] both require a fresh depth first
             * walk.
             *
             * TODO: Ensure that there are no multi-head DAGs
             */

            if (INFO_TOPO (arg_info) != 1) {
                INFO_TOPO (arg_info) = 1;
            }

            TRAVdo (defs, arg_info);
        }

        defs = TFVERTEX_NEXT (defs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTOPtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here to check if the number of parents
 *   of a vertex equals the number of times its visited in the traversal of the
 *   DAG. If it is the same, then the topological number of the vertex can be
 *   done.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFTOPtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TFTOPtfvertex");

    node *defs, *children;

    defs = arg_node;

    children = TFVERTEX_CHILDREN (defs);
    TFVERTEX_TOPO (defs) = INFO_TOPO (arg_info)++;

    while (children != NULL) {

        /*
         * Check if the number of visits in the topological walk equals the number
         * of parents for the vertex. If so, topological numbering can proceed for
         * the vertex.
         */

        if (TFVERTEX_NUMPARENTS (TFEDGE_TARGET (children))
            == ++TFVERTEX_NUMTOPOVISITS (TFEDGE_TARGET (children))) {
            TRAVdo (TFEDGE_TARGET (children), arg_info);
        }

        children = TFEDGE_NEXT (children);
    }

    DBUG_RETURN (arg_node);
}
