/*
 * $Log$
 * Revision 1.6  2004/08/12 12:39:28  skt
 * killed a bug in CDFGFirstIsWithinSecond
 * moved PrintDataflowgraph and PrintDataflownode to print
 *
 * Revision 1.5  2004/08/11 08:37:05  skt
 * output for a compiler stop by 'cdfg' added
 *
 * Revision 1.4  2004/08/09 03:47:34  skt
 * some very painful bugfixing
 * added support for dataflowgraphs within with-loops
 * (I hope someone'll use it in future)
 *
 * Revision 1.3  2004/08/06 17:24:38  skt
 * some adaptions made
 *
 * Revision 1.2  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.1  2004/07/29 08:39:12  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup cdfg Create Dataflowgraph
 * @ingroup muth
 *
 * @brief Create dataflowgraphs for the code
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file create_dataflowgraph.c
 *
 * prefix: CDFG
 *
 * description:
 *   creates a dataflowgraph for each function-definition, to use ist later on
 *   in other subphases of mt-mode 3 like rearranging the code or inserting
 *   a communication-structure
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "create_dataflowgraph.h"
#include "multithread.h"
#include "print.h"

/*
 * INFO structure
 */
struct INFO {
    node *currentdfg;
    node *outermostdfg;
    node *currentdfn;
    node *outermostdfn;
    int withdeep;
};

/*
 * INFO macros
 *   node*      CURRENTDFG     (the current (innermost) dataflowgraph the
 *                              traversal is; usually the same as OUTERMOSTDFG
 *                              (exception: within conditionals))
 *   node*      OUTERMOSTDFG   (the current outermost dataflowgraph, the graph
 *                              of the block which belongs direct to the
 *                              function)
 *   node*      CURRENTDFN     (the dataflownode in CURRENTDFG, belonging to
 *                              the current assignment)
 *   node*      OUTERMOSTDFN   (the dataflownode in OUTERMOSTDFG, belonging to
 *                              the current assignment)
 *   a little bit confused? It doesn't matter - here's an example:
 *
 *   int tutu(bool decision) {
 *     sense = 42;
 *     if(decision)
 *       result_then = sense; (AAA)
 *     else
 *       result_else = 0;
 *     result_tutu = Funcond(decision, result_then, result_else);
 *     return(return_tutu);
 *
 *   the belonging dataflowgraph(s):
 *     dfg_tutu:
 *       source_tutu -> sense -> conditional -> result_tutu -> return/sink
 *     dfg_then:
 *       source_then -> result_then/sink
 *     dfg_else:
 *       source_else -> result_else/sink
 *
 *   let the traversal-mechanismn be in AAA:
 *     CURRENTDFG points to dfg_then
 *     OUTERMOSTDFG points to dfg_tutu
 *     CURRENTDFN points to result_then/sink
 *     OUTERMOSTDFN points to conditional
 *
 */
#define INFO_CDFG_CURRENTDFG(n) (n->currentdfg)
#define INFO_CDFG_OUTERMOSTDFG(n) (n->outermostdfg)
#define INFO_CDFG_CURRENTDFN(n) (n->currentdfn)
#define INFO_CDFG_OUTERMOSTDFN(n) (n->outermostdfn)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CDFG_CURRENTDFG (result) = NULL;
    INFO_CDFG_OUTERMOSTDFG (result) = NULL;
    INFO_CDFG_CURRENTDFN (result) = NULL;
    INFO_CDFG_OUTERMOSTDFN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CreateDataflowgraph(node *arg_node)
 *
 * description:
 *   Inits the traversal for this phase
 *
 ******************************************************************************/
node *
CreateDataflowgraph (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("CreateDataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "CreateDataflowgraph expects a N_modul as arg_node");

    arg_info = MakeInfo ();

    /* push info ... */
    old_tab = act_tab;
    act_tab = cdfg_tab;

    DBUG_PRINT ("CDFG", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return the block-node with its belonging dataflowgraph
 *
 *****************************************************************************/
node *
CDFGblock (node *arg_node, info *arg_info)
{
    node *old_dataflowgraph;
    DBUG_ENTER ("CDFGblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

    /* push info... */
    old_dataflowgraph = INFO_CDFG_CURRENTDFG (arg_info);

    /* Initialisation of the dataflowgraph for this N_block */
    BLOCK_DATAFLOWGRAPH (arg_node) = MakeDataflowgraph ();
    INFO_CDFG_CURRENTDFG (arg_info) = BLOCK_DATAFLOWGRAPH (arg_node);

    /* we have a new outermost dataflowgraph, if no old_dataflowgraph exists;
     * otherwise the current dataflownode gets a new dataflowgraph*/
    if (old_dataflowgraph == NULL) {
        INFO_CDFG_OUTERMOSTDFG (arg_info) = INFO_CDFG_CURRENTDFG (arg_info);
    } else {
        /* so we've got a "home dataflownode" */
        DATAFLOWGRAPH_MYHOMEDFN (BLOCK_DATAFLOWGRAPH (arg_node))
          = INFO_CDFG_CURRENTDFN (arg_info);

        /* the "home dataflownode" must have a pointer to this (its)
         * daughter-dataflowgraph. Only the question of "then-daughter" or
         * "else-daughter" has ot be checked */
        if (DATAFLOWNODE_DFGTHEN (INFO_CDFG_CURRENTDFN (arg_info)) == NULL) {
            DATAFLOWNODE_DFGTHEN (INFO_CDFG_CURRENTDFN (arg_info))
              = BLOCK_DATAFLOWGRAPH (arg_node);
        } else {
            DATAFLOWNODE_DFGELSE (INFO_CDFG_CURRENTDFN (arg_info))
              = BLOCK_DATAFLOWGRAPH (arg_node);
        }
    }

    /* continue traversal */
    DBUG_PRINT ("CDFG", ("trav into instruction(s)"));
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from instruction(s)"));

    if ((break_after == PH_multithread) && (strcmp ("cdfg", break_specifier) == 0)) {
        PrintNode (arg_node);
        PrintNode (INFO_CDFG_CURRENTDFG (arg_info));
    }

    /* pop info... */
    INFO_CDFG_CURRENTDFG (arg_info) = old_dataflowgraph;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGassign (node *arg_node, info *arg_info)
{
    node *old_dataflownode;

    DBUG_ENTER ("CDFGassign");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "node is not a N_assign");

    /* push info... */
    old_dataflownode = INFO_CDFG_CURRENTDFN (arg_info);

    /* only assignments on the top-level will be represented in the dataflowgraph
     */
    /* Are we the last mohican? */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        INFO_CDFG_CURRENTDFN (arg_info)
          = MakeDataflownode (INFO_CDFG_CURRENTDFG (arg_info), arg_node,
                              CDFGSetName (arg_node));
    }
    /* yes => the coresponding dataflownode still exists as DF__sink */
    else {
        INFO_CDFG_CURRENTDFN (arg_info)
          = DATAFLOWGRAPH_SINK (INFO_CDFG_CURRENTDFG (arg_info));
        DATAFLOWNODE_ASSIGN (INFO_CDFG_CURRENTDFN (arg_info)) = arg_node;
        DATAFLOWNODE_EXECMODE (INFO_CDFG_CURRENTDFN (arg_info))
          = ASSIGN_EXECMODE (arg_node);
    }
    /* Do we have to update the outermost dataflownode? */
    if (INFO_CDFG_CURRENTDFG (arg_info) == INFO_CDFG_OUTERMOSTDFG (arg_info)) {
        INFO_CDFG_OUTERMOSTDFN (arg_info) = INFO_CDFG_CURRENTDFN (arg_info);
    }

    DBUG_PRINT ("CDFG", ("trav into instruction"));
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    DBUG_PRINT ("CDFG", ("trav from instruction"));

    /* continue traversal */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("CDFG", ("trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("CDFG", ("trav from next"));
    }

    /* pop info ... */
    INFO_CDFG_CURRENTDFN (arg_info) = old_dataflownode;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CDFGid");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id), "node is not a N_id");

    /*#if CDFG_DEBUG*/
    /*fprintf(stdout,"act. id = %s\n",ID_NAME(arg_node));*/
    /* if (AVIS_SSAASSIGN(ID_AVIS(arg_node)) == NULL)
       fprintf(stdout,"ssaassign is NULL!\n");*/
    /* fprintf(stdout,"outermost DFG:\n");
       fprintf(stdout,"\n");*/
    /*#endif*/

    INFO_CDFG_OUTERMOSTDFG (arg_info)
      = CDFGUpdateDependencies (AVIS_SSAASSIGN (ID_AVIS (arg_node)),
                                INFO_CDFG_CURRENTDFG (arg_info),
                                INFO_CDFG_OUTERMOSTDFG (arg_info),
                                INFO_CDFG_CURRENTDFN (arg_info),
                                INFO_CDFG_OUTERMOSTDFN (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *CDFGwithid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGwithid (node *arg_node, info *arg_info)
{
    ids *iterator;
    DBUG_ENTER ("CDFGwithid");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_nwithid), "node is not a N_nwithid");

    /* handle the with-id vector */
    iterator = NWITHID_VEC (arg_node);
    INFO_CDFG_OUTERMOSTDFG (arg_info)
      = CDFGUpdateDependencies (AVIS_SSAASSIGN (IDS_AVIS (NWITHID_VEC (arg_node))),
                                INFO_CDFG_CURRENTDFG (arg_info),
                                INFO_CDFG_OUTERMOSTDFG (arg_info),
                                INFO_CDFG_CURRENTDFN (arg_info),
                                INFO_CDFG_OUTERMOSTDFN (arg_info));

    /* handle the with-id vector elements */
    iterator = NWITHID_IDS (arg_node);
    while (iterator != NULL) {
        INFO_CDFG_OUTERMOSTDFG (arg_info)
          = CDFGUpdateDependencies (AVIS_SSAASSIGN (IDS_AVIS (iterator)),
                                    INFO_CDFG_CURRENTDFG (arg_info),
                                    INFO_CDFG_OUTERMOSTDFG (arg_info),
                                    INFO_CDFG_CURRENTDFN (arg_info),
                                    INFO_CDFG_OUTERMOSTDFN (arg_info));
        iterator = IDS_NEXT (iterator);
    }
    DBUG_RETURN (arg_node);
}

node *
CDFGUpdateDependencies (node *dfn_assign, node *current_graph, node *outer_graph,
                        node *current_node, node *outer_node)
{
    node *node_found;
    node *common_graph;
    DBUG_ENTER ("CDFGUpdateDependencies");
    DBUG_ASSERT ((NODE_TYPE (current_graph) == N_dataflowgraph),
                 "CDFGUpdadeDependencies's 2nd parameter is no N_dataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (outer_graph) == N_dataflowgraph),
                 "CDFGUpdadeDependencies's 3rd parameter is no N_dataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (current_node) == N_dataflownode),
                 "CDFGUpdadeDependencies's 4th parameter is no N_dataflownode");
    DBUG_ASSERT ((NODE_TYPE (outer_node) == N_dataflownode),
                 "CDFGUpdadeDependencies's 5th parameter is no N_dataflownode");

    /* Is there an assignment to depend on?
     * yes -> then let's search for it in the dataflowgraph(s) */
    if (dfn_assign != NULL) {
        DBUG_ASSERT ((NODE_TYPE (dfn_assign) == N_assign),
                     "CDFGUpdadeDependencies's 1st parameter is no N_assign");

        node_found = CDFGFindAssignCorrespondingNode (outer_graph, dfn_assign);
        /*fprintf(stdout,"Assignment:");
        PrintNode(dfn_assign);
        fprintf(stdout,"corresponds to:");
        PrintNode(DATAFLOWNODE_ASSIGN(node_found));*/

        DBUG_ASSERT ((node_found != NULL), "No corresponding node found");

        common_graph = CDFGLowestCommonLevel (node_found, current_node);

        DBUG_ASSERT ((common_graph != NULL), "don't found lowest common level");

        CDFGUpdateDataflowgraph (common_graph, node_found, current_node);
    }
    /* no -> nothing to do */
    DBUG_RETURN (outer_graph);
}

node *
CDFGFindAssignCorrespondingNode (node *graph, node *dfn_assign)
{
    node *result;
    nodelist *member_iterator;
    DBUG_ENTER ("CDFGFindAssignCorrespondingNode");
    DBUG_ASSERT ((NODE_TYPE (graph) == N_dataflowgraph),
                 "FindCorrespondingNode's 1st parameter is no N_dataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (dfn_assign) == N_assign),
                 "CDFGFindAssignCorrespondingNode's 2nd parameter is no N_assign");
    DBUG_ASSERT ((dfn_assign != NULL),
                 "CDFGFindAssignCorrespondingNode's 2nd parameter is NULL");

#if CDFG_DEBUG
    /*fprintf(stdout,"searching for node which corresponds to");
      PrintNode(dfn_assign);*/
#endif

    result = NULL;
    member_iterator = DATAFLOWGRAPH_MEMBERS (graph);
    while ((result == NULL) && (member_iterator != NULL)) {
        /* Is this node the one? */
        if ((DATAFLOWNODE_ASSIGN (NODELIST_NODE (member_iterator))) == dfn_assign) {
            result = NODELIST_NODE (member_iterator);
        }
        /* not -> well, perhaps within its dataflowgraphs (if exist) - rekursion */
        else {
            /* within the then-branch? */
            if (DATAFLOWNODE_DFGTHEN (NODELIST_NODE (member_iterator)) != NULL) {
                result
                  = CDFGFindAssignCorrespondingNode (DATAFLOWNODE_DFGTHEN (
                                                       NODELIST_NODE (member_iterator)),
                                                     dfn_assign);
                /* or within the else-branch? */
                if ((result == NULL)
                    && (DATAFLOWNODE_DFGELSE (NODELIST_NODE (member_iterator)) != NULL)) {
                    result = CDFGFindAssignCorrespondingNode (DATAFLOWNODE_DFGELSE (
                                                                NODELIST_NODE (
                                                                  member_iterator)),
                                                              dfn_assign);
                }
            }
        } /* else */
        member_iterator = NODELIST_NEXT (member_iterator);
    }

    DBUG_RETURN (result);
}

node *
CDFGLowestCommonLevel (node *node_one, node *node_two)
{
    node *result;
    node *iterator;
    bool found_lcl;
    DBUG_ENTER ("CDFGLowestCommonLevel");

    result = DATAFLOWNODE_GRAPH (node_one);

    found_lcl = FALSE;

    while ((found_lcl == FALSE) && (result != NULL)) {
        iterator = DATAFLOWNODE_GRAPH (node_two);
        while ((found_lcl == FALSE) && (iterator != NULL)) {

            if (iterator == result) {
                found_lcl = TRUE;
            } else if (DATAFLOWGRAPH_MYHOMEDFN (iterator) != NULL) {
                iterator = DATAFLOWNODE_GRAPH (DATAFLOWGRAPH_MYHOMEDFN (iterator));
            } else {
                iterator = NULL;
            }
        } /* while */
        if (found_lcl == FALSE) {
            if (DATAFLOWGRAPH_MYHOMEDFN (result) != NULL) {
                result = DATAFLOWNODE_GRAPH (DATAFLOWGRAPH_MYHOMEDFN (result));
            } else {
                result = NULL;
            }
        }
    } /* while */

    DBUG_RETURN (result);
}

void
CDFGUpdateDataflowgraph (node *graph, node *node_one, node *node_two)
{
    nodelist *iterator;
    node *from_node;
    node *to_node;
    DBUG_ENTER ("CDFGUpdateDataflowgraph");

    from_node = NULL;
    to_node = NULL;

    if (DATAFLOWNODE_GRAPH (node_one) == DATAFLOWNODE_GRAPH (node_two)) {
        from_node = node_one;
        to_node = node_two;
    } else {
        iterator = DATAFLOWGRAPH_MEMBERS (graph);
        while ((iterator != NULL) && ((from_node == NULL) || (to_node == NULL))) {

            if ((from_node == NULL)
                && (CDFGFirstIsWithinSecond (node_one, NODELIST_NODE (iterator))
                    == TRUE)) {
                from_node = NODELIST_NODE (iterator);
            }
            if ((to_node == NULL)
                && (CDFGFirstIsWithinSecond (node_two, NODELIST_NODE (iterator))
                    == TRUE)) {
                to_node = NODELIST_NODE (iterator);
            }
            iterator = NODELIST_NEXT (iterator);
        }
        DBUG_ASSERT (((to_node != NULL) || (from_node != NULL)),
                     "don't found to_node and from_node");
        DBUG_ASSERT ((from_node != NULL), "don't found from_node");

        DBUG_ASSERT ((to_node != NULL), "don't found to_node");
    }

    /* update dependency only if both nodes are not identical */
    if (to_node != from_node) {
        if (NodeListFind (DATAFLOWNODE_DEPENDENT (from_node), to_node) == NULL) {
            DATAFLOWNODE_DEPENDENT (from_node)
              = NodeListAppend (DATAFLOWNODE_DEPENDENT (from_node), to_node, NULL);
            DATAFLOWNODE_REFCOUNT (to_node)++;
        }
    }

    DBUG_VOID_RETURN;
}

bool
CDFGFirstIsWithinSecond (node *inner_node, node *outer_node)
{
    bool result;
    bool continue_search;
    DBUG_ENTER ("CDFGFirstIsWithinSecond");
    DBUG_ASSERT (((NODE_TYPE (inner_node) == N_dataflownode)
                  && (NODE_TYPE (outer_node) == N_dataflownode)),
                 "dataflownodes as parameter expected");

    continue_search = TRUE;

    while (continue_search == TRUE) {
        if ((DATAFLOWNODE_GRAPH (inner_node) == DATAFLOWNODE_GRAPH (outer_node))
            || (DATAFLOWGRAPH_MYHOMEDFN (DATAFLOWNODE_GRAPH (inner_node)) == NULL)) {
            continue_search = FALSE;
        } else {
            inner_node = DATAFLOWGRAPH_MYHOMEDFN (DATAFLOWNODE_GRAPH (inner_node));
        }
    }

    if (inner_node == outer_node) {
        result = TRUE;
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

char *
CDFGSetName (node *assign)
{
    node *instr;
    char *return_value;
    DBUG_ENTER ("SetName");
    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "SetName expects a N_assign");

    instr = ASSIGN_INSTR (assign);
    return_value = NULL;
    if (NODE_TYPE (instr) == N_let) {
        return_value = IDS_NAME (LET_IDS (ASSIGN_INSTR (assign)));
    } else if (NODE_TYPE (instr) == N_cond) {
        return_value = StringCopy ("DF__condition");
    } else {
        PrintNode (assign);
        DBUG_ASSERT (0, "SetName was called with an invalid assignment");
    }
    DBUG_RETURN (return_value);
}

/**
 * @}
 **/
