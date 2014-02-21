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

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "create_dataflowgraph.h"
#include "print.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "CDFG"
#include "debug.h"

#include "globals.h"

#define CDFG_DEBUG 0

/*
 * INFO structure
 */
struct INFO {
    node *currentdfg;
    node *outermostdfg;
    node *currentdfn;
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
 *
 */
#define INFO_CDFG_CURRENTDFG(n) (n->currentdfg)
#define INFO_CDFG_OUTERMOSTDFG(n) (n->outermostdfg)
#define INFO_CDFG_CURRENTDFN(n) (n->currentdfn)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_CDFG_CURRENTDFG (result) = NULL;
    INFO_CDFG_OUTERMOSTDFG (result) = NULL;
    INFO_CDFG_CURRENTDFN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* some declaration */
static node *UpdateDependency (node *dfn_assign, node *outer_graph, node *current_node);

static node *FindAssignCorrespondingNode (node *graph, node *dfn_assign);

static node *LowestCommonLevel (node *node_one, node *node_two);

static void UpdateDataflowgraph (node *graph, node *node_one, node *two);

static bool FirstIsWithinSecond (node *node_one, node *node_two);

static char *GetName (node *assign);

/** <!--********************************************************************-->
 *
 * @fn  node *CDFGdoCreateDataflowgraph(node *arg_node)
 *
 * @brief inits the traversal for this phase
 *
 * @param arg_node
 * @return
 *
 *****************************************************************************/
node *
CDFGdoCreateDataflowgraph (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "CDFGdoCreateDataflowgraph expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_cdfg);

    DBUG_PRINT ("trav into module-funs");
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("trav from module-funs");

    traversaltable = TRAVpop ();
    DBUG_ASSERT (traversaltable == TR_cdfg, "Popped incorrect traversal table");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CDFGblock(node *arg_node, info *arg_info)
 *
 * @brief builds the dataflowgraph for this block and traverses into its
 *        instructions to enable dataflowgraph-building in their subblocks
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
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_block, "node is not a N_block");

    /* push info... */
    old_dataflowgraph = INFO_CDFG_CURRENTDFG (arg_info);

    /* Initialisation of the dataflowgraph for this N_block */
    BLOCK_DATAFLOWGRAPH (arg_node) = TBmakeDataflowgraph ();
    INFO_CDFG_CURRENTDFG (arg_info) = BLOCK_DATAFLOWGRAPH (arg_node);

    /* we have a new outermost dataflowgraph, if no old_dataflowgraph exists;
     * otherwise the current dataflownode gets a new dataflowgraph*/
    if (old_dataflowgraph == NULL) {
        INFO_CDFG_OUTERMOSTDFG (arg_info) = INFO_CDFG_CURRENTDFG (arg_info);
    }
    /* so we've got a "home dataflownode" */
    else {
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
    DBUG_PRINT ("trav into instruction(s)");
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    DBUG_PRINT ("trav from instruction(s)");

    /* As a fact of beeing very complex, additional output will only take place
     * if the compilation breaks with the cdfg-specifier */

#ifndef PRODUCTION
    /*
     * The production version of sac2c does not contain the compiler phase
     * PH_mt_cdfg. In principle, the whole code in subdirectory multithread
     * should be put behind an ifndef PRODUCTION.
     */
    if (global.break_after_subphase == PH_mt3_cdfg) {
        fprintf (stdout, "A N_block...\n");
        PRTdoPrintNode (arg_node);
        fprintf (stdout, "...and its dataflowgraph:\n");
        PRTdoPrintNode (INFO_CDFG_CURRENTDFG (arg_info));
    }
#endif

    /* pop info... */
    INFO_CDFG_CURRENTDFG (arg_info) = old_dataflowgraph;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CDFGassign(node *arg_node, info *arg_info)
 *
 * @brief if arg_node isn't the last assignment of its block, CDFGassign builds
 *        a new dataflownode for this assignment;
 *        updates the INFO_CDFG_CURRENTDFN
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

    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "node is not a N_assign");

    /* push info... */
    old_dataflownode = INFO_CDFG_CURRENTDFN (arg_info);

    /* Are we the last mohican?
     * no => let's build a new dataflownode */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        INFO_CDFG_CURRENTDFN (arg_info)
          = TBmakeDataflownode (INFO_CDFG_CURRENTDFG (arg_info), arg_node,
                                GetName (arg_node));
    }
    /* yes => the coresponding dataflownode still exists as DF__sink
     * (it had been builded by the first call of TBmakeDataflowgraph) */
    else {
        INFO_CDFG_CURRENTDFN (arg_info)
          = DATAFLOWGRAPH_SINK (INFO_CDFG_CURRENTDFG (arg_info));
        DATAFLOWNODE_ASSIGN (INFO_CDFG_CURRENTDFN (arg_info)) = arg_node;
        DATAFLOWNODE_EXECMODE (INFO_CDFG_CURRENTDFN (arg_info))
          = ASSIGN_EXECMODE (arg_node);
    }

    /* set the back-pointer, too */
    ASSIGN_DATAFLOWNODE (arg_node) = INFO_CDFG_CURRENTDFN (arg_info);

    /* continue traversal */
    DBUG_PRINT ("trav into instruction");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    DBUG_PRINT ("trav from instruction");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("trav into next");
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("trav from next");
    }

    /* pop info ... */
    INFO_CDFG_CURRENTDFN (arg_info) = old_dataflownode;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CDFGid(node *arg_node, info *arg_info)
 *
 * @brief updates the dependenciy of the INDO_CDFG_CURRENTDFN, concerning
 *        the N_id (arg_node) and its defining assignment
 *        (in AVIS_SSAASSIGN(ID_AVIS(arg_node)))
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_id, "node is not a N_id");

#if CDFG_DEBUG
    fprintf (stdout, "act. id = %s\n", ID_NAME (arg_node));
#endif

    INFO_CDFG_OUTERMOSTDFG (arg_info)
      = UpdateDependency (AVIS_SSAASSIGN (ID_AVIS (arg_node)),
                          INFO_CDFG_OUTERMOSTDFG (arg_info),
                          INFO_CDFG_CURRENTDFN (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CDFGwithid(node *arg_node, info *arg_info)
 *
 * @brief updates the dependency of the INDO_CDFG_CURRENTDFN, concerning
 *        the N_id (arg_node) and its defining assignment
 *        (in AVIS_SSAASSIGN(IDS_AVIS(NWITHID_VEC(arg_node))) and in
 *         AVIS_SSAASSIGN(IDS_AVIS(NWITHID_IDS(arg_node)))) respectively
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
CDFGwithid (node *arg_node, info *arg_info)
{
    node *iterator;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_withid, "node is not a N_withid");

    /* handle the with-id vector */
    INFO_CDFG_OUTERMOSTDFG (arg_info)
      = UpdateDependency (AVIS_SSAASSIGN (IDS_AVIS (WITHID_VEC (arg_node))),
                          INFO_CDFG_OUTERMOSTDFG (arg_info),
                          INFO_CDFG_CURRENTDFN (arg_info));

    /* handle the with-id vector elements */
    iterator = WITHID_IDS (arg_node);
    while (iterator != NULL) {
        INFO_CDFG_OUTERMOSTDFG (arg_info)
          = UpdateDependency (AVIS_SSAASSIGN (IDS_AVIS (iterator)),
                              INFO_CDFG_OUTERMOSTDFG (arg_info),
                              INFO_CDFG_CURRENTDFN (arg_info));
        iterator = IDS_NEXT (iterator);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *UpdateDependency(node *dfn_assign, node *outer_graph,
 *                                   node *current_node)
 *
 * @brief Updates the dependency between the assignment dfn_assign and the
 *        current dataflownode current_node in the dataflowgraph outer_graph
 *
 * @param dfn_assign
 * @param outer_graph the dataflowgraph of the N_block in top-level
 *        (i.e. the dataflowgraph, who contains in some deepnees of its
 *         structure both the current_node and the dataflownode belonging to
 *         dfn_assign; also outer_graph has no MYHOMEDFN)
 * @param current_node the dataflownode the traversal is in
 * @return
 *
 *****************************************************************************/
static node *
UpdateDependency (node *dfn_assign, node *outer_graph, node *current_node)
{
    node *node_found;
    node *common_graph;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (outer_graph) == N_dataflowgraph,
                 "2nd parameter is no N_dataflowgraph");
    DBUG_ASSERT (NODE_TYPE (current_node) == N_dataflownode,
                 "3rd parameter is no N_dataflownode");

    /* Is there an assignment to depend on?
     * yes -> then let's search for it in the dataflowgraph(s) */
    if (dfn_assign != NULL) {
        DBUG_ASSERT (NODE_TYPE (dfn_assign) == N_assign, "1st parameter is no N_assign");

        /* first you've to find the dataflownode which assignment is dfn_assign */
        /*node_found = FindAssignCorrespondingNode(outer_graph, dfn_assign);

        DBUG_ASSERT((node_found == ASSIGN_DATAFLOWNODE(dfn_assign)),
        "oops - should two identical dataflownodes exist?");

        DBUG_ASSERT((node_found != NULL),"No corresponding node found");*/

        node_found = ASSIGN_DATAFLOWNODE (dfn_assign);

        /* then you've to find the graph which the lowest level, which includes
         * both nodes */
        common_graph = LowestCommonLevel (node_found, current_node);

        DBUG_ASSERT (common_graph != NULL, "don't found lowest common level");

        /* finally you've to update the dependency whitin the common_graph;
         * finding the nodes of the graph's to-level that corresponds to
         * node_found and current_node is made, too */
        UpdateDataflowgraph (common_graph, node_found, current_node);
    }
    /* no -> nothing to do */
    DBUG_RETURN (outer_graph);
}

/** <!--********************************************************************-->
 *
 * @fn static node *FindAssignCorrespondingNode(node *graph, node *dfn_assign)
 *
 * @brief finds the dataflownode in the graph, which assignment equals
 *        dfn_assign, and returns the dataflownode
 *
 * @param graph
 * @param dfn_assign
 * @return the node, somewhere in graph, which has dfn_assign as its assignment
 *
 *****************************************************************************/
static node *
FindAssignCorrespondingNode (node *graph, node *dfn_assign)
{
    node *result;
    nodelist *member_iterator;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (graph) == N_dataflowgraph,
                 "1st parameter is no N_dataflowgraph");
    DBUG_ASSERT (dfn_assign != NULL, "2nd parameter is NULL");
    DBUG_ASSERT (NODE_TYPE (dfn_assign) == N_assign, "2nd parameter is no N_assign");

#if CDFG_DEBUG
    /*fprintf(stdout,"searching for node which corresponds to");
      PRTPrintNode(dfn_assign);*/
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
                result = FindAssignCorrespondingNode (DATAFLOWNODE_DFGTHEN (
                                                        NODELIST_NODE (member_iterator)),
                                                      dfn_assign);
                /* or within the else-branch? */
                if ((result == NULL)
                    && (DATAFLOWNODE_DFGELSE (NODELIST_NODE (member_iterator)) != NULL)) {
                    result
                      = FindAssignCorrespondingNode (DATAFLOWNODE_DFGELSE (
                                                       NODELIST_NODE (member_iterator)),
                                                     dfn_assign);
                }
            }
        } /* else */

        member_iterator = NODELIST_NEXT (member_iterator);
    } /* while */

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *LowestCommonLevel(node *node_one, node *node_two)
 *
 * @brief finds the graph, which consists both, node_one and node_two somewhere
 *        in its structure and which is also the smallest one that matches
 * <pre> example:
 *       let node_one be B and node_two be D
 *
 *       DFG1: A->B->C
 *       DFG2: DATAFLOWNODE_DFGTHEN(B): BA->BB->BC->BD
 *       DFG3: DATAFLOWNODE_DFGTHEN(BA): BAA->BAB->BAC->BAD
 *       DFG4: DATAFLOWNODE_DFGELSE(BA): BAa->BAb->BAc->BAd
 *       let node_one be B and node_two be C => return(DFG1)
 *       let node_one be BA and node_two be C => return(DFG1)
 *       let node_one be BAA and node_two be C => return(DFG1)
 *       let node_one be BA and node_two be BAA => return(DFG2)
 *       let node_one be BA and node_two be BAa => return(DFG2)
 *       let node_one be BAA and node_two be BAa => return(DFG2)
 *       let node_one be BAA and node_two be BAC => return(DFG3)
 * </pre>
 * @param node_one
 * @param node_two
 * @return the dataflowgraph who contains both, node_one and node_two, could
 *         be NULL if no dataflowgraph with this properties exists
 *
 *****************************************************************************/
node *
LowestCommonLevel (node *node_one, node *node_two)
{
    node *result;
    node *iterator;
    bool found_lcl;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (node_one) == N_dataflownode,
                 "1st parameter is no N_dataflownode");
    DBUG_ASSERT (NODE_TYPE (node_two) == N_dataflownode,
                 "2nd parameter is no N_dataflownode");

    result = DATAFLOWNODE_GRAPH (node_one);

    found_lcl = FALSE;

    /* as long as you haven't found the common graph, iterate over the dataflow-
       graphs which include node_one */
    while ((found_lcl == FALSE) && (result != NULL)) {
        iterator = DATAFLOWNODE_GRAPH (node_two);

        /* as long as you haven't found the common graph, iterate over the data-
         * flowgraphs which includes node_two */
        while ((found_lcl == FALSE) && (iterator != NULL)) {
            /* compare the dataflowgraph, which contains node_two (iterator), with
             * the dataflowgraph, which contains node_one (result) */
            if (iterator == result) {
                found_lcl = TRUE;
            }
            /* if the comparision of both dataflowgraph fails, "increment" the
             * dataflowgraph of node_two (iterator)... */
            else if (DATAFLOWGRAPH_MYHOMEDFN (iterator) != NULL) {
                iterator = DATAFLOWNODE_GRAPH (DATAFLOWGRAPH_MYHOMEDFN (iterator));
            }
            /* ... as long as this is possible */
            else {
                iterator = NULL;
            }
        } /* while */

        /* still found => go the dataflowgraph-level of node_one/result one step
         * up, is possible; set result to NULL otherwise */
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

/** <!--********************************************************************-->
 *
 * @fn void UpdateDataflowgraph(node *graph, node *node_one,
 *                                  node *node_two)
 *
 * @brief Updates the dependency between node_one and node_two in graph;
 *        as a fact node_one and node_two needn't to be in the same level
 *        in this graph, the coresponding nodes of graph-level must be found;
 *        no dependency will be adde if the corresponding nodes are equal or
 *        the dependency still exists
 *
 * @param graph the common dataflowgraph of node_one and node_two
 * @param node_one a dataflownode somewhere in graph
 * @param node_two a dataflownode somewhere in graph
 * @return the dataflowgraph graph with updated dependency
 *
 *****************************************************************************/
void
UpdateDataflowgraph (node *graph, node *node_one, node *node_two)
{
    nodelist *iterator;
    node *from_node;
    node *to_node;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (graph) == N_dataflowgraph,
                 "1st parameter is no N_dataflowgraph");
    DBUG_ASSERT (NODE_TYPE (node_one) == N_dataflownode,
                 "2nd parameter is no N_dataflownode");
    DBUG_ASSERT (NODE_TYPE (node_two) == N_dataflownode,
                 "3rd parameter is no N_dataflownode");

    from_node = NULL;
    to_node = NULL;

    /* good luck - if the dataflowgraphs of both dataflownodes are identical */
    if (DATAFLOWNODE_GRAPH (node_one) == DATAFLOWNODE_GRAPH (node_two)) {
        from_node = node_one;
        to_node = node_two;
    } else {
        /* bad luck - iterate over the members of the dataflowgraph graph and
         * search for dataflownodes, who contains node_one and/or node_two */
        iterator = DATAFLOWGRAPH_MEMBERS (graph);
        while ((iterator != NULL) && ((from_node == NULL) || (to_node == NULL))) {

            /* find from_node, if it's node done yet */
            if ((from_node == NULL)
                && (FirstIsWithinSecond (node_one, NODELIST_NODE (iterator)) == TRUE)) {
                from_node = NODELIST_NODE (iterator);
            }
            /* find to_node, if it's node done yet */
            if ((to_node == NULL)
                && (FirstIsWithinSecond (node_two, NODELIST_NODE (iterator)) == TRUE)) {
                to_node = NODELIST_NODE (iterator);
            }
            iterator = NODELIST_NEXT (iterator);
        }
        DBUG_ASSERT (((to_node != NULL) || (from_node != NULL)),
                     "don't found to_node and from_node");
        DBUG_ASSERT (from_node != NULL, "don't found from_node");
        DBUG_ASSERT (to_node != NULL, "don't found to_node");
    }

    /* update dependency only if both nodes are not identical and the dependency
     * does not exist yet */
    if ((to_node != from_node)
        && (TCnodeListFind (DATAFLOWNODE_DEPENDENT (from_node), to_node) == NULL)) {
        DATAFLOWNODE_DEPENDENT (from_node)
          = TCnodeListAppend (DATAFLOWNODE_DEPENDENT (from_node), to_node, NULL);
        DATAFLOWNODE_REFCOUNT (to_node)++;
        DATAFLOWNODE_USEDNODES (to_node)
          = TCnodeListAppend (DATAFLOWNODE_USEDNODES (to_node), from_node, NULL);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn bool FirstIsWithinSecond(node *inner_node, node* outer_node)
 *
 * @brief
 *
 * @param inner_node
 * @param outer_node
 * @return true, if inner_node is somewhere within the outer_node, else false
 *
 *****************************************************************************/
static bool
FirstIsWithinSecond (node *inner_node, node *outer_node)
{
    bool result;
    bool continue_search;
    DBUG_ENTER ();
    DBUG_ASSERT (((NODE_TYPE (inner_node) == N_dataflownode)
                  && (NODE_TYPE (outer_node) == N_dataflownode)),
                 "dataflownodes as parameters (1st,2nd) expected");

    continue_search = TRUE;

    while (continue_search == TRUE) {
        /* stop the search
         * - if you'd found a common dataflowgraph
         * - if there's no upper dataflowgraph left */
        if ((DATAFLOWNODE_GRAPH (inner_node) == DATAFLOWNODE_GRAPH (outer_node))
            || (DATAFLOWGRAPH_MYHOMEDFN (DATAFLOWNODE_GRAPH (inner_node)) == NULL)) {
            continue_search = FALSE;
        } else {
            inner_node = DATAFLOWGRAPH_MYHOMEDFN (DATAFLOWNODE_GRAPH (inner_node));
        }
    }

    /* test of identity - if it fails, inner_node and outer_node a members of the
     * same dataflowgraph, but that's is */
    if (inner_node == outer_node) {
        result = TRUE;
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn char *GetName(node* assign)
 *
 * @brief gets a name for the assignment, to store it in the dataflowgraph
 *        usually it's the name of the leftmost ids (at a let-instruction);
 *        other possibilities are "DF__void" (if an assignment hat no return
 *        value - see below) or "DF__conditional"
 *
 *  <pre> as far as I know is the only possibility to get an let-assignment
 *        a programm like this:
 *        {
 *        if(cond) {
 *          a=1;
 *        }
 *        return(0);
 *
 *  </pre>
 *
 * @param assign the assignment
 * @return
 *
 *****************************************************************************/
static char *
GetName (node *assign)
{
    node *instr;
    char *return_value;
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "GetName expects a N_assign");

    instr = ASSIGN_STMT (assign);
    return_value = NULL;
    if (NODE_TYPE (instr) == N_let) {
        if (LET_IDS (ASSIGN_STMT (assign)) != NULL) {
            return_value = IDS_NAME (LET_IDS (ASSIGN_STMT (assign)));
        } else {
            return_value = STRcpy ("DF__void");
        }
    } else if (NODE_TYPE (instr) == N_cond) {
        return_value = STRcpy ("DF__conditional");
    } else {
        DBUG_UNREACHABLE ("GetName was called with an invalid assignment");
    }
    DBUG_RETURN (return_value);
}

/**
 * @}
 **/

#undef DBUG_PREFIX
