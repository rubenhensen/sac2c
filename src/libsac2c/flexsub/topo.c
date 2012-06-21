
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
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "types.h"
#include "graphutils.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "topo.h"

/*
 * INFO structure
 * pre is the topological id for the vertices in the dependency graph.
 */

struct INFO {
    int topo;
    nodelist *head;
    nodelist *list;
};

/*
 * INFO macros
 */
#define INFO_TOPO(n) n->topo
#define INFO_HEAD(n) n->head
#define INFO_LIST(n) n->list

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));
    INFO_TOPO (result) = 1;
    INFO_HEAD (result) = NULL;
    INFO_LIST (result) = NULL;
    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTOPdoTopoSort( node *syntax_tree)
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

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_tftop);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTOPtfdag( node *arg_node, info *arg_info)
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
TFTOPtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    TRAVdo (TFDAG_ROOT (arg_node), arg_info);
    COMPINFO_TOPOLIST (TFDAG_INFO (arg_node)) = INFO_HEAD (arg_info);

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

    DBUG_ENTER ();

    node *defs, *children;

    defs = arg_node;

    children = TFVERTEX_CHILDREN (defs);
    TFVERTEX_TOPO (defs) = INFO_TOPO (arg_info)++;

    if (INFO_HEAD (arg_info) == NULL) {

        /*
         * We also maintain a list of topologically sorted vertices for future
         * processing here.
         */

        INFO_HEAD (arg_info) = MEMmalloc (sizeof (nodelist));
        INFO_LIST (arg_info) = INFO_HEAD (arg_info);

        NODELIST_NODE (INFO_HEAD (arg_info)) = arg_node;

    } else if (NODELIST_NEXT (INFO_LIST (arg_info)) == NULL) {

        NODELIST_NEXT (INFO_LIST (arg_info)) = MEMmalloc (sizeof (nodelist));
        INFO_LIST (arg_info) = NODELIST_NEXT (INFO_LIST (arg_info));
        NODELIST_NODE (INFO_LIST (arg_info)) = arg_node;
        NODELIST_NEXT (INFO_LIST (arg_info)) = NULL;
    }

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

#undef DBUG_PREFIX
