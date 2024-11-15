/** <!--********************************************************************-->
 *
 * @file dfwalk.c
 *
 * prefix: TFDFW
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
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "dfwalk.h"
#include "tfprintutils.h"

/*
 * INFO structure
 * pre is the depth first walk id for the nodes in the dependency
 * graph. premax is the maximum value of the pre of the tree
 * decendants of a node
 */
struct INFO {
    int pre;
    int post;
    dynarray *prearr;
};

/*
 * INFO macros
 */
#define INFO_PRE(n) n->pre
#define INFO_POST(n) n->post
#define INFO_PREARR(n) n->prearr

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_PRE (result) = 1;
    INFO_POST (result) = 1;
    INFO_PREARR (result) = NULL;

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
 * @fn node *TFDFWdoDFWalk( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFDFWdoDFWalk (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_tfdfw);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFDFWtfdag( node *arg_node, info *arg_info)
 *
 *   @brief
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
TFDFWtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    /*
     * First label nodes for tree reachability
     */

    INFO_PREARR (arg_info) = NULL;

    TRAVdo (TFDAG_ROOT (arg_node), arg_info);

    COMPINFO_PREARR (TFDAG_INFO (arg_node)) = INFO_PREARR (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFDFWtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here. If the node has not
 *   been visited i.e. its pre is 0, we update the pre of
 *   the node. Then, we check the children (subtypes) of the def and if
 *   they are not visited, we visit them.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFDFWtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs, *children;
    elem *e;

    defs = arg_node;

    children = TFVERTEX_CHILDREN (defs);
    TFVERTEX_PRE (defs) = INFO_PRE (arg_info)++;

    if (INFO_PREARR (arg_info) == NULL) {
        INFO_PREARR (arg_info) = (dynarray *)MEMmalloc (sizeof (dynarray));
        initDynarray (INFO_PREARR (arg_info));
    }

    e = (elem *)MEMmalloc (sizeof (elem));
    ELEM_IDX (e) = TFVERTEX_PRE (defs);
    ELEM_DATA (e) = arg_node;
    addToArray (INFO_PREARR (arg_info), e);

    while (children != NULL) {

        if (TFVERTEX_PRE (TFEDGE_TARGET (children)) == 0) {
            /*
             * Tree branch
             */
            TFEDGE_EDGETYPE (children) = edgetree;
            TFEDGE_WASCLASSIFIED (children) = 1;

            TFVERTEX_DEPTH (TFEDGE_TARGET (children)) = TFVERTEX_DEPTH (defs) + 1;

            TRAVdo (TFEDGE_TARGET (children), arg_info);
        }

        children = TFEDGE_NEXT (children);
    }

    /*
     * We have traversed all descendants of this node. Its time to
     * update the value of premax which is the maximum value of the
     * pre of all tree descendant of this node
     */

    TFVERTEX_PREMAX (defs) = INFO_PRE (arg_info);
    TFVERTEX_POST (defs) = INFO_POST (arg_info)++;

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
