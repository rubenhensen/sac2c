/** <!--********************************************************************-->
 *
 * @file preprocess_graph.c
 *
 * prefix: TFRCH
 *
 * description: We label vertices for answering reachability queries in this files. The
 * labeling scheme is based on the paper "Dual Labeling: Answering Graph Reachability
 * Queries in Constant Time" by Haixun Wang et. al. that appeared in ICDE '06.
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
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "reachlabel.h"

/*
 * INFO structure
 *
 * collabel is a variable used to label vertices with numbers to index the columns of
 * cross edge based reachability matrix. This is referred to the transitive link matrix in
 * "Dual Labeling: Answering Graph Reachability Queries in Constant Time".
 *
 * totalcols is the the total number of cross edge sources.
 */

struct INFO {
    int collabel;
    int totalcols;
    dynarray *csrc;
    dynarray *ctar;
    elemstack **estack;
};

/*
 * INFO macros
 */
#define INFO_COLLABEL(n) n->collabel
#define INFO_TOTALCOLS(n) n->totalcols
#define INFO_CSRC(n) n->csrc
#define INFO_CTAR(n) n->ctar
#define INFO_ESTACK(n) n->estack

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_COLLABEL (result) = 0;
    INFO_TOTALCOLS (result) = 0;
    INFO_CSRC (result) = NULL;
    INFO_CTAR (result) = NULL;
    INFO_ESTACK (result) = NULL;

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
 * @fn node *TFPPGdoPreprocessTFGraph( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/

node *
TFRCHdoReachabilityAnalysis (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_tfrch);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFRCHtfdag( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We loop through the defs in the type family specifications to
 *   identify any potential apex nodes.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/

node *
TFRCHtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    compinfo *ci;

    ci = TFDAG_INFO (arg_node);

    if (ci != NULL && COMPINFO_TLTABLE (ci) != NULL) {

        INFO_TOTALCOLS (arg_info) = DYNARRAY_TOTALELEMS (COMPINFO_CSRC (ci));
        INFO_CSRC (arg_info) = COMPINFO_CSRC (ci);
        INFO_CTAR (arg_info) = COMPINFO_CTAR (ci);
        INFO_ESTACK (arg_info) = (elemstack **)MEMmalloc (sizeof (elemstack *));
        INFO_COLLABEL (arg_info) = 0;

        TRAVdo (TFDAG_ROOT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFRCHtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here. If the node has not
 *   been visited i.e. its pre is 0, we update the pre of
 *   the node. Then, we check the subs (subtypes) of the def and if
 *   they are not visited, we visit them.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/

node *
TFRCHtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs, *children, *parents;

    defs = arg_node;

    /*
     * Assign non-tree labels for reachability now
     */

    parents = TFVERTEX_PARENTS (defs);

    int pop = 0, i;

    /*
     * If the current vertex has an incoming cross edges, then we push that vertex onto
     * the stack and set a flag to pop at the end of this function.
     */

    while (parents != NULL) {

        if (TFEDGE_EDGETYPE (parents) == edgecross) {

            elem *e = (elem *)MEMmalloc (sizeof (elem));
            ELEM_DATA (e) = NULL;

            /*
             * TODO: The following loop "may" be unnecessary. Instead we can use a
             * variable like we use for collabel (see below).
             */

            for (i = 0; i < DYNARRAY_TOTALELEMS (INFO_CTAR (arg_info)); i++) {

                if (TFVERTEX_PRE (arg_node)
                    == ELEM_IDX (DYNARRAY_ELEMS (INFO_CTAR (arg_info))[i])) {
                    ELEM_IDX (e) = i;
                }
            }

            pushElemstack (INFO_ESTACK (arg_info), e);
            pop = 1;
            break;
        }

        parents = TFEDGE_NEXT (parents);
    }

    /*
     * We maintain a variable called xpre which is the preorder number of a vertex that
     * has a preorder number higher than "defs" and is a cross-edge source.
     */

    int xpre;

    xpre = ELEM_IDX (DYNARRAY_ELEMS (INFO_CSRC (arg_info))[INFO_COLLABEL (arg_info)]);

    if (INFO_COLLABEL (arg_info) < INFO_TOTALCOLS (arg_info)) {
        if (TFVERTEX_PRE (defs) <= xpre) {
            TFVERTEX_REACHCOLA (defs) = INFO_COLLABEL (arg_info);
            TFVERTEX_ISRCHCOLAMARKED (defs) = 1;
        }
        /*
         * else just maintain the default REACHCOLA
         */
    }

    /*
     * Call the children recursively
     */

    children = TFVERTEX_CHILDREN (defs);

    while (children != NULL) {

        if (TFEDGE_EDGETYPE (children) == edgetree) {
            TRAVdo (TFEDGE_TARGET (children), arg_info);
        }

        children = TFEDGE_NEXT (children);
    }

    if (INFO_COLLABEL (arg_info) < INFO_TOTALCOLS (arg_info)) {

        /*
         * Update the values of xpre after the recursive call returns
         */

        xpre = ELEM_IDX (DYNARRAY_ELEMS (INFO_CSRC (arg_info))[INFO_COLLABEL (arg_info)]);

        if (TFVERTEX_PREMAX (defs) > xpre) {
            INFO_COLLABEL (arg_info)++;
        }

        if (INFO_COLLABEL (arg_info) < INFO_TOTALCOLS (arg_info)) {
            TFVERTEX_REACHCOLB (defs) = INFO_COLLABEL (arg_info);
            TFVERTEX_ISRCHCOLBMARKED (defs) = 1;
        }
    }

    /*
     * Now we update the vertices with the cluster id. These clusters are sets of
     * cross-edge-free regions.
     */

    if (*(INFO_ESTACK (arg_info)) != NULL) {

        if (ELEMSTACK_CURR (*(INFO_ESTACK (arg_info))) != NULL) {
            TFVERTEX_ROW (defs) = ELEM_IDX (ELEMSTACK_CURR (*(INFO_ESTACK (arg_info))));
            TFVERTEX_ISROWMARKED (defs) = 1;
        }
    }

    /*
     * Pop the vertex that is the root of the cluster now.
     */

    if (pop == 1) {
        freeElem (popElemstack (INFO_ESTACK (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
