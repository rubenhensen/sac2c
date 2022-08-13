
/** <!--********************************************************************-->
 *
 * @file classifyedges.c
 *
 * prefix: TFCTR
 *
 * description: In this file, we first classify the edges in subtyping hierarchy DAG.
 * Then, we build a transitive link table for each DAG under consideration.
 *
 * The transitive link table holds information about which cross edge sources
 * reach what cross edge target vertices. More details can be found in the paper
 * on reachability analysis using dual labeling.
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
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "ctransitive.h"
#include "reachhelper.h"

/*
 * INFO structure
 */
struct INFO {
    dynarray *tltable;
    dynarray *arrX;
    dynarray *arrY;
};

/*
 * INFO macros
 */
#define INFO_TLTABLE(n) n->tltable

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_TLTABLE (result) = NULL;

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
 * @fn node *TFCTRdoClassifyEdges( node *dag)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFCTRdoCrossClosure (node *dag)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_tfctr);

    dag = TRAVdo (dag, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (dag);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFCTRtfdag( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFCTRtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    compinfo *ci;

    TRAVdo (TFDAG_ROOT (arg_node), arg_info);

    /*
     * Do the following if we have at least one cross edge in the DAG.
     */

    if (INFO_TLTABLE (arg_info) != NULL) {
        /*
         * We maintain a list of all cross edge sources and all cross edge
         * targets in a DAG.
         */
        ci = TFDAG_INFO (arg_node);
        setSrcTarArrays (INFO_TLTABLE (arg_info), &(COMPINFO_CSRC (ci)),
                         &(COMPINFO_CTAR (ci)));

        /*
         * For each cross edge source, compute all the source edge targets that
         * it can potentially reach transitively. This will add a few more
         * entries in the transitive link table.
         */

        if (DYNARRAY_TOTALELEMS (INFO_TLTABLE (arg_info)) > 0) {
            COMPINFO_TLTABLE (ci) = INFO_TLTABLE (arg_info);
            /*
             * The transitive link table and cross closure are two different
             * things.
             */
            buildTransitiveLinkTable (COMPINFO_TLTABLE (ci));
            COMPINFO_TLC (ci) = computeTLCMatrix (INFO_TLTABLE (arg_info),
                                                  COMPINFO_CSRC (ci), COMPINFO_CTAR (ci));
        }

        // freeDynarray( INFO_TLTABLE( arg_info));
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFCTRtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here.
 *   If the edge has not been classified, we classify the edge based
 *   on the pre and post order numbers of the source and target
 *   vertices for the edge.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/

node *
TFCTRtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs, *children, *parents;

    defs = arg_node;

    int pre_parent, pre_child, post_parent, post_child, premax_child;

    children = TFVERTEX_CHILDREN (defs);
    pre_parent = TFVERTEX_PRE (arg_node);
    post_parent = TFVERTEX_POST (arg_node);

    while (children != NULL) {

        if (!TFEDGE_WASCLASSIFIED (children)) {

            /* Tree edges have already been classified during depth first walk of the
             * graph.
             */

            pre_child = TFVERTEX_PRE (TFEDGE_TARGET (children));
            premax_child = TFVERTEX_PREMAX (TFEDGE_TARGET (children));
            post_child = TFVERTEX_POST (TFEDGE_TARGET (children));

            if (pre_parent < pre_child && post_child < post_parent) {

                /*
                 * This is a forward edge. Since back and forward edges are
                 * disallowed, throw an error here.
                 *
                 * TODO: Dont throw an error. Instead, ignore the forward edge and show
                 * a warning.
                 */

                CTIabort (EMPTY_LOC, "Forward edge found in subtyping hierarchy");

            } else if (pre_child < pre_parent && post_parent < post_child) {

                /*
                 * This is a back edge. Since back and forward edges are
                 * disallowed, throw an error here.
                 */

                CTIabort (EMPTY_LOC, "Back edge found in subtyping hierarchy");

            } else if (pre_child < pre_parent && post_child < post_parent) {

                /*
                 * This must be a cross edge. Add this to the transitive
                 * link table
                 */

                TFEDGE_EDGETYPE (children) = edgecross;

                /*
                 * Set the parent relationship to be a cross edge as well.
                 * This will be used in the non-tree labeling
                 */

                parents = TFVERTEX_PARENTS (TFEDGE_TARGET (children));

                while (parents != NULL) {
                    if (TFEDGE_TARGET (parents) == defs) {
                        TFEDGE_EDGETYPE (parents) = edgecross;
                    }
                    parents = TFEDGE_NEXT (parents);
                }

                /*
                 * Now that we have discovered a cross edge, we must add it
                 * to the transitive link table.
                 */

                if (INFO_TLTABLE (arg_info) == NULL) {
                    INFO_TLTABLE (arg_info) = (dynarray *)MEMmalloc (sizeof (dynarray));
                    initDynarray (INFO_TLTABLE (arg_info));
                }

                /*
                 * The transitive link table is actually a list that consists of three
                 * integers a,b and c in the form a->[b,c) which signifies that a vertex
                 * with a preorder number of "a" reaches another vertex with a preorder
                 * number "b" and "c" is the maximum preorder number of the children of
                 * "b" increased by 1. Hence, the open interval ")".
                 */

                elem *e = (elem *)MEMmalloc (sizeof (elem));
                ELEM_DATA (e) = MEMmalloc (2 * sizeof (int));
                ELEM_IDX (e) = pre_parent;
                *((int *)ELEM_DATA (e)) = pre_child;
                *((int *)ELEM_DATA (e) + 1) = premax_child;

                addToArray (INFO_TLTABLE (arg_info), e);

            } else {

                CTIabort (EMPTY_LOC, "Unclassifiable edge found in subtyping hierarchy");
            }

            TFEDGE_WASCLASSIFIED (children) = 1;

        } else {

            TRAVdo (TFEDGE_TARGET (children), arg_info);
        }

        children = TFEDGE_NEXT (children);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
