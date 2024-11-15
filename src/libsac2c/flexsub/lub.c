/** <!--********************************************************************-->
 *
 * @file lub.c
 *
 * prefix: TFPLB
 *
 * description: This file calls functions to preprocess the tpye hierarchy graph
 * for answering least upper bound queries. This is done with the aid of a
 * compiler pass.
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
#include "lubtree.h"
#include "lubcross.h"
#include "binheap.h"
#include <time.h>

/*
 * INFO structure
 * pre is the depth first walk id for the nodes in the dependency
 * graph. premax is the maximum value of the pre of the tree
 * decendants of a node
 */
struct INFO {
    dynarray *euler;
};

/*
 * INFO macros
 */
#define INFO_EULER(n) n->euler

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_EULER (result) = NULL;
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
 * @fn node *TFPLBQdoPreprocessForLUBQueries( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFPLBdoLUBPreprocessing (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_tfplb);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

void
randNumGen (int max, int *testpre)
{

    testpre[0] = rand () % (max);
    testpre[1] = rand () % (max);
}

void
testPriorityQueue (void)
{

    int i, j, random, totalelements;
    dynarray *q;

    srand ((unsigned int)time (NULL));

    for (j = 0; j < 10; j++) {

        q = (dynarray *)MEMmalloc (sizeof (dynarray));
        initDynarray (q);

        for (i = 0; i < 10; i++) {
            random = rand () % 10 + 1;
            PQinsert (random, q);
        }

        PQprint (q);

        totalelements = DYNARRAY_TOTALELEMS (q);

        for (i = 0; i < totalelements; i++) {
            printf ("%d,", PQgetMin (q));
            PQdeleteMin (q);
        }

        printf ("\n-----------\n");

        freeDynarray (q);
    }
}

void
testlubtree (node *arg_node)
{

    dynarray *prearr;
    int j, nodecount;
    int testpre[2];
    node *n1, *n2, *result;

    unsigned int iseed = (unsigned int)time (NULL);
    srand (iseed);

    prearr = COMPINFO_PREARR (TFDAG_INFO (arg_node));
    nodecount = DYNARRAY_TOTALELEMS (prearr);
    printDepthAndPre (COMPINFO_EULERTOUR (TFDAG_INFO (arg_node)));
    printLubInfo (TFDAG_INFO (arg_node));

    for (j = 0; j < nodecount; j++) {
        randNumGen (nodecount, testpre);
        n1 = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (prearr, testpre[0]));
        n2 = (node *)ELEM_DATA (DYNARRAY_ELEMS_POS (prearr, testpre[1]));
        printf ("lub(%d,%d) = ", TFVERTEX_PRE (n1), TFVERTEX_PRE (n2));
        result = LUBtreeLCAfromNodes (n1, n2, TFDAG_INFO (arg_node));
        printf ("Result = %d \n", TFVERTEX_PRE (result));
        fflush (stdout);
    }
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPLBtfdag( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFPLBtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs;
    compinfo *ci;

    defs = TFDAG_DEFS (arg_node);

    /*
     * First label nodes for tree reachability
     */

    INFO_EULER (arg_info) = NULL;

    TRAVdo (TFDAG_ROOT (defs), arg_info);

    ci = TFDAG_INFO (arg_node);

    COMPINFO_EULERTOUR (ci) = INFO_EULER (arg_info);
    COMPINFO_LUB (ci) = LUBcreatePartitions (COMPINFO_EULERTOUR (ci));

    LUBincorporateCrossEdges (ci);

    // testlubtree( arg_node);
    // testPriorityQueue();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPLBtfvertex( node *arg_node, info *arg_info)
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
TFPLBtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *defs, *children;
    elem *e;

    defs = arg_node;

    children = TFVERTEX_CHILDREN (defs);

    if (INFO_EULER (arg_info) == NULL) {
        INFO_EULER (arg_info) = (dynarray *)MEMmalloc (sizeof (dynarray));
        initDynarray (INFO_EULER (arg_info));
    }

    e = (elem *)MEMmalloc (sizeof (elem));
    ELEM_IDX (e) = TFVERTEX_DEPTH (defs);

    /*
     * ELEM_DATA(e) is a void pointer. So, we have to take this into account while
     * initialising it.
     */

    ELEM_DATA (e) = MEMmalloc (2 * sizeof (int));
    ((int *)ELEM_DATA (e))[0] = TFVERTEX_PRE (defs);
    ((int *)ELEM_DATA (e))[1] = 0;

    addToArray (INFO_EULER (arg_info), e);

    TFVERTEX_EULERID (defs) = DYNARRAY_TOTALELEMS (INFO_EULER (arg_info)) - 1;

    while (children != NULL) {

        if (TFEDGE_EDGETYPE (children) == edgetree) {

            TRAVdo (TFEDGE_TARGET (children), arg_info);

            /*
             * We add the parent vertex once again upon return from the traversal.
             */

            e = (elem *)MEMmalloc (sizeof (elem));
            ELEM_IDX (e) = TFVERTEX_DEPTH (defs);

            ELEM_DATA (e) = MEMmalloc (2 * sizeof (int));
            ((int *)ELEM_DATA (e))[0] = TFVERTEX_PRE (defs);
            ((int *)ELEM_DATA (e))[1] = 0;

            addToArray (INFO_EULER (arg_info), e);
        }

        children = TFEDGE_NEXT (children);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
