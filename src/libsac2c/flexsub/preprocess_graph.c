/** <!--********************************************************************-->
 *
 * @file tf_build_graph.c
 *
 * prefix: TFBDG
 *
 * description:
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "preprocess_graph.h"
#include "structures.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "types.h"

/*
 * INFO structure
 * pre is the depth first walk id for the nodes in the dependency
 * graph. premax is the maximum value of the pre of the tree
 * decendants of a node
 */
struct INFO {
    int pre;
    int post;
    graph_label_mode labelmode;
    dynarray *tltable;
    dynarray *arrX;
    dynarray *arrY;
    elemstack **estack;
    int nontreeidx;
    matrix **tlcmatrices;
};

/*
 * INFO macros
 */
#define INFO_PRE(n) n->pre
#define INFO_POST(n) n->post
#define INFO_LABELMODE(n) n->labelmode
#define INFO_TLTABLE(n) n->tltable
#define INFO_ARRX(n) n->arrX
#define INFO_ARRY(n) n->arrY
#define INFO_NONTREEIDX(n) n->nontreeidx
#define INFO_ESTACK(n) n->estack
#define INFO_TLCMATRICES(n) n->tlcmatrices

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_PRE (result) = 1;
    INFO_POST (result) = 1;
    INFO_LABELMODE (result) = tree_labeling;
    INFO_TLTABLE (result) = NULL;
    INFO_ARRX (result) = NULL;
    INFO_ARRY (result) = NULL;
    INFO_ESTACK (result) = NULL;
    INFO_NONTREEIDX (result) = 0;
    INFO_TLCMATRICES (result) = NULL;

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
 * @fn node *TFPPGdoPreprocessTFGraph( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFPPGdoPreprocessTFGraph (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TFPPGdoPreprocessTFGraph");

    arg_info = MakeInfo ();

    TRAVpush (TR_tfppg);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPPGtfspec( node *arg_node, info *arg_info)
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
TFPPGtfspec (node *arg_node, info *arg_info)
{
    node *defs;
    DBUG_ENTER ("TFPPGtfspec");
    defs = TFSPEC_DEFS (arg_node);
    /*
     * First label nodes for tree reachability
     */
    INFO_LABELMODE (arg_info) = tree_labeling;
    while (defs != NULL) {
        if (TFDEF_PRE (defs) == 0 && TFDEF_SUPERS (defs) == NULL) {
            /*
             * Here could be the start of a potentially new type family
             * component since we have multiple components in one family.
             *
             * For example, \alpha and [*] both require a fresh depth first
             * walk.
             */
            if (INFO_PRE (arg_info) != 1) {
                INFO_PRE (arg_info) = 1;
            }
            if (INFO_POST (arg_info) != 1) {
                INFO_POST (arg_info) = 1;
            }
            TRAVdo (defs, arg_info);
        }
        defs = TFDEF_NEXT (defs);
    }
    /*
     * Then classify the edges and throw an error if forward and back
     * edges are present. The list of cross edges is used to build the
     * trasitive link count matrix.
     */
    defs = TFSPEC_DEFS (arg_node);
    while (defs != NULL) {
        if (TFDEF_SUPERS (defs) == NULL) {
            INFO_LABELMODE (arg_info) = edge_labeling;
            TRAVdo (defs, arg_info);
            /* Do the non-tree labeling part only if we have cross edges */
            int currsize = ++TFSPEC_NUMHIERAR (arg_node);
            void *_tlcmats
              = MEMrealloc (TFSPEC_TLCMATRICES (arg_node), currsize * sizeof (matrix *),
                            (currsize - 1) * sizeof (matrix *));
            if (!_tlcmats) {
                CTIabort ("Memory reallocation error in non-tree labeling!\n");
            }
            MEMfree (TFSPEC_TLCMATRICES (arg_node));
            TFSPEC_TLCMATRICES (arg_node) = (matrix **)_tlcmats;
            TFSPEC_TLCMATRICES (arg_node)[currsize - 1] = NULL;
            if (INFO_TLTABLE (arg_info) != NULL) {
                if (INFO_ARRX (arg_info) != NULL)
                    freeDynarray (INFO_ARRX (arg_info));
                if (INFO_ARRY (arg_info) != NULL)
                    freeDynarray (INFO_ARRY (arg_info));
                buildTransitiveLinkTable (INFO_TLTABLE (arg_info));
                setXYarrays (INFO_TLTABLE (arg_info), &(INFO_ARRX (arg_info)),
                             &(INFO_ARRY (arg_info)));
                if (DYNARRAY_TOTALELEMS (INFO_TLTABLE (arg_info)) > 0) {
                    TFSPEC_TLCMATRICES (arg_node)
                    [currsize - 1]
                      = computeTLCMatrix (INFO_TLTABLE (arg_info), INFO_ARRX (arg_info),
                                          INFO_ARRY (arg_info));
                }
                freeDynarray (INFO_TLTABLE (arg_info));
                INFO_TLTABLE (arg_info) = NULL;
                /*
                 * Now label nodes for non-tree reachability.
                 * The labels are in the form of triples {x,y,z}
                 */
                INFO_LABELMODE (arg_info) = nontree_labeling;
                INFO_ESTACK (arg_info) = MEMmalloc (sizeof (elemstack *));
                INFO_NONTREEIDX (arg_info) = 0;
                TRAVdo (defs, arg_info);
            }
        }
        defs = TFDEF_NEXT (defs);
    }
    /*
    int i;
    for(i=0;i<TFSPEC_NUMHIERAR(arg_node);i++){
      if(TFSPEC_TLCMATRICES(arg_node)[i]!=NULL){
        printMatrix(TFSPEC_TLCMATRICES(arg_node)[i]);
        printf("\n");
      }
    }
    */
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPPGtfdef( node *arg_node, info *arg_info)
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
TFPPGtfdef (node *arg_node, info *arg_info)
{
    node *defs, *subs, *supers;
    DBUG_ENTER ("TFPPGtfdef");
    defs = arg_node;
    if (INFO_LABELMODE (arg_info) == tree_labeling) {
        subs = TFDEF_SUBS (defs);
        TFDEF_PRE (defs) = INFO_PRE (arg_info)++;
        while (subs != NULL) {
            if (TFDEF_PRE (TFEDGE_TYPEFAMILY (subs)) == 0) {
                /*
                 * Tree branch
                 */
                TFEDGE_EDGETYPE (subs) = edgetree;
                TRAVdo (TFEDGE_TYPEFAMILY (subs), arg_info);
            } else {
                /*
                 * Cross/Back/Forward branch here.
                 * Do nothing, these are dealt with in edge labeling.
                 * Refer literature on Directed Acyclic Graphs.
                 */
                TFEDGE_EDGETYPE (subs) = -1;
            }
            subs = TFEDGE_NEXT (subs);
        }
        /*
         * We have traversed all descendants of this node. Its time to
         * update the value of premax which is the maximum value of the
         * pre of all tree descendant of this node
         */
        TFDEF_PREMAX (defs) = INFO_PRE (arg_info);
        TFDEF_POST (defs) = INFO_POST (arg_info)++;
    } else if (INFO_LABELMODE (arg_info) == edge_labeling) {
        int pre_super, pre_sub, post_super, post_sub, premax_sub;
        subs = TFDEF_SUBS (defs);
        pre_super = TFDEF_PRE (arg_node);
        post_super = TFDEF_POST (arg_node);
        while (subs != NULL) {
            if (TFEDGE_EDGETYPE (subs) != edgetree) {
                pre_sub = TFDEF_PRE (TFEDGE_TYPEFAMILY (subs));
                premax_sub = TFDEF_PREMAX (TFEDGE_TYPEFAMILY (subs));
                post_sub = TFDEF_POST (TFEDGE_TYPEFAMILY (subs));
                if (pre_super < pre_sub && post_sub < post_super) {
                    /* This is a forward edge. Since back and forward edges are
                     * disallowed, throw an error here
                     */
                    CTIabort ("Forward edge found in subtyping hierarchy");
                } else if (pre_sub < pre_super && post_super < post_sub) {
                    /* This is a back edge. Since back and forward edges are
                     * disallowed, throw an error here
                     */
                    CTIabort ("Back edge found in subtyping hierarchy");
                } else if (pre_sub < pre_super && post_sub < post_super) {
                    /*
                     * This must be a cross edge. Add this to the transitive
                     * link table
                     */
                    TFEDGE_EDGETYPE (subs) = edgecross;
                    /*
                     * Set the super relationship to be a cross edge as well.
                     * This will be used in the non-tree labeling
                     */
                    supers = TFDEF_SUPERS (TFEDGE_TYPEFAMILY (subs));
                    while (supers != NULL) {
                        if (TFEDGE_TYPEFAMILY (supers) == defs) {
                            TFEDGE_EDGETYPE (supers) = edgecross;
                        }
                        supers = TFEDGE_NEXT (supers);
                    }
                    if (INFO_TLTABLE (arg_info) == NULL) {
                        INFO_TLTABLE (arg_info) = MEMmalloc (sizeof (dynarray));
                        initDynarray (INFO_TLTABLE (arg_info));
                    }
                    elem *e = MEMmalloc (sizeof (elem));
                    ELEM_DATA (e) = MEMmalloc (2 * sizeof (int));
                    ELEM_IDX (e) = pre_super;
                    *((int *)ELEM_DATA (e)) = pre_sub;
                    *((int *)ELEM_DATA (e) + 1) = premax_sub;
                    addToArray (INFO_TLTABLE (arg_info), e);
                } else {
                    CTIabort ("Unclassifiable edge found in subtyping hierarchy");
                }
            } else {
                TRAVdo (TFEDGE_TYPEFAMILY (subs), arg_info);
            }
            subs = TFEDGE_NEXT (subs);
        }
    } else if (INFO_LABELMODE (arg_info) == nontree_labeling) {
        /*
         * Assign non-tree labels now
         */
        supers = TFDEF_SUPERS (defs);
        int pop = 0;
        while (supers != NULL) {
            if (TFEDGE_EDGETYPE (supers) == edgecross) {
                elem *e = MEMmalloc (sizeof (elem));
                ELEM_DATA (e) = NULL;
                int i;
                for (i = 0; i < DYNARRAY_TOTALELEMS (INFO_ARRY (arg_info)); i++) {
                    if (TFDEF_PRE (arg_node)
                        == ELEM_IDX (DYNARRAY_ELEMS (INFO_ARRY (arg_info))[i])) {
                        ELEM_IDX (e) = i;
                    }
                }
                pushElemstack (INFO_ESTACK (arg_info), e);
                pop = 1;
                break;
            }
            supers = TFEDGE_NEXT (supers);
        }
        if (INFO_NONTREEIDX (arg_info) < DYNARRAY_TOTALELEMS (INFO_ARRX (arg_info))) {
            if (TFDEF_PRE (defs) <= ELEM_IDX (DYNARRAY_ELEMS (INFO_ARRX (
                                      arg_info))[INFO_NONTREEIDX (arg_info)])) {
                TFDEF_NONTREEX (defs) = INFO_NONTREEIDX (arg_info);
            }
        }
        subs = TFDEF_SUBS (defs);
        while (subs != NULL) {
            if (TFDEF_NONTREEX (TFEDGE_TYPEFAMILY (subs)) == -1
                && TFEDGE_EDGETYPE (subs) == edgetree) {
                TRAVdo (TFEDGE_TYPEFAMILY (subs), arg_info);
            }
            subs = TFEDGE_NEXT (subs);
        }
        if (INFO_NONTREEIDX (arg_info) < DYNARRAY_TOTALELEMS (INFO_ARRX (arg_info))) {
            int lhs, rhs;
            lhs = TFDEF_PREMAX (defs);
            rhs = ELEM_IDX (
              DYNARRAY_ELEMS (INFO_ARRX (arg_info))[INFO_NONTREEIDX (arg_info)]);
            if (lhs > rhs) {
                INFO_NONTREEIDX (arg_info)++;
            }
        }
        if (INFO_NONTREEIDX (arg_info) < DYNARRAY_TOTALELEMS (INFO_ARRX (arg_info))) {
            TFDEF_NONTREEY (defs) = INFO_NONTREEIDX (arg_info);
        }
        if (*(INFO_ESTACK (arg_info)) != NULL) {
            if (ELEMSTACK_CURR (*(INFO_ESTACK (arg_info))) != NULL) {
                TFDEF_NONTREEZ (defs)
                  = ELEM_IDX (ELEMSTACK_CURR (*(INFO_ESTACK (arg_info))));
            }
        }
        if (pop == 1) {
            popElemstack (INFO_ESTACK (arg_info));
        }
    }
    DBUG_RETURN (arg_node);
}
