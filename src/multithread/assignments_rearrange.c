/*
 * $Log$
 * Revision 1.11  2004/08/26 17:23:56  skt
 * changed inclusion of multithread.h into multithread_lib.h
 *
 * Revision 1.10  2004/08/19 15:01:03  skt
 * rearranging algorithm improved
 *
 * Revision 1.9  2004/08/17 10:29:06  skt
 * changed two return into DBUG_RETURN
 *
 * Revision 1.8  2004/08/13 16:16:39  skt
 * some comments added
 *
 * Revision 1.7  2004/08/12 12:52:19  skt
 * some debugging...
 *
 * Revision 1.6  2004/08/11 09:31:54  skt
 * ASMRAPrintCluster bug fixed
 *
 * Revision 1.5  2004/08/11 08:38:44  skt
 * full redesigned, still under construction but looks well
 *
 * Revision 1.4  2004/08/05 10:52:22  skt
 * initialization of border & new_borderelem added
 *
 * Revision 1.3  2004/07/29 00:41:50  skt
 * build compilable intermediate version
 * work in progress
 *
 * Revision 1.2  2004/04/30 14:10:05  skt
 * some debugging
 *
 * Revision 1.1  2004/04/27 09:59:04  skt
 * Initial revision
 *
 */

/**
 *
 * @defgroup asmra Assignments Rearrange
 * @ingroup muth
 *
 * @brief Rearranges the assignments of each function to avoid superflous synchronisation
 * @{
 **/

/** <!--********************************************************************-->
 *
 * @file assignments_rearrange.c
 *
 * prefix: ASMRA
 *
 * description:
 *   rearranges the assignment of each funcion to avoid superflous
 *   synchronisations
 *
 *****************************************************************************/

#define NEW_INFO

#include "dbug.h"
#include <limits.h> /* for INT_MAX */
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "assignments_rearrange.h"
#include "print.h"
#include "multithread_lib.h"

/*
 * INFO structure
 */
struct INFO {
    node *next;
};

/*
 * INFO macros
 *   node*      NEXT
 *
 */
#define INFO_ASMRA_NEXT(n) (n->next)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_ASMRA_NEXT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/*** <!--*******************************************************************-->
 *
 * @fn:node *AssignmentsRearrange(node *arg_node)
 *
 * @brief Inits the traversal for this phase
 *
 * @param arg_node
 * @return
 *
 *****************************************************************************/
node *
AssignmentsRearrange (node *arg_node)
{
    funtab *old_tab;
    info *arg_info;
    DBUG_ENTER ("AssignmentsRearrange");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "AssignmentsRearrange expects a N_modul as arg_node");

    arg_info = MakeInfo ();

    /* push info ... */
    old_tab = act_tab;
    act_tab = asmra_tab;

    DBUG_PRINT ("ASMRA", ("trav into modul-funs"));
    MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from modul-funs"));

    /* pop info ... */
    act_tab = old_tab;

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRAblock(node *arg_node, info *arg_info)
 *
 * @brief Rearrange the assignments of this block on all levels
 *
 * @param arg_node
 * @param arg_info
 * @return N_block with rearranged assignment chain(s)
 *
 ****************************************************************************/
node *
ASMRAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASMRAblock");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

#if ASMRA_DEBUG
    fprintf (stdout, "before");
    PrintNode (arg_node);
#endif

    if (NODE_TYPE (BLOCK_INSTR (arg_node)) == N_assign) {
        arg_node = ASMRACreateNewAssignmentOrder (arg_node);
    }

#if ASMRA_DEBUG
    fprintf (stdout, "after:\n");
    PrintNode (arg_node);
#endif

    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into instruction(s)"));
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from instruction(s)"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRACreateNewAssignmentOrder(node *arg_node)
 *
 * @brief Top-level rearrange arg_node's assignment-chain
 *   <pre> the rearranging proceeds in several steps
 *         1. Initialize the dataflowgraph of the block (preparation)
 *         2. Build a list of dataflownode-cluster
 *            each cluster consists of dataflownodes with identical
 *            executionmodes and MUTH_ANY-executionmodes
 *            if their executionmode is MUTH_EXCLUSIVE or MUTH_SINGLE, the
 *            different nodes of a cluster can depend on each other, otherwise
 *            (MUTH_MULTI) this is forbitten
 *         3. Arrange the elements of each cluster and dissolve the cluster
 *            into a list of dataflownodes
 *         4. Build a new assignmentchain for arg_node out of this list
 *  </pre>
 * @param arg_node a N_block
 * @return N_block with rearranged assignment chain on top-level
 *
 ****************************************************************************/
node *
ASMRACreateNewAssignmentOrder (node *arg_node)
{
    struct asmra_list_s *my_list;
    node *graph;
    DBUG_ENTER ("ASMRACreateNewAssignmentOrder");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

    graph = BLOCK_DATAFLOWGRAPH (arg_node);

    graph = ASMRAPrepareDataflowgraph (graph);

    my_list = ASMRABuildListOfCluster (graph);

    my_list = ASMRADissolveAllCluster (my_list);

    arg_node = ASMRABuildNewAssignmentChain (my_list, arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_list_s *ASMRABuildListOfCluster(node *graph)
 *
 * @brief build cluster of dataflownodes, belonging together;
 *        the specific cluster are arranges in a list;
 *        the sequence of the list-members fits the sequence the assignment of
 *        the members has to be executed
 *
 * @param graph
 * @return
 *
 ****************************************************************************/
struct asmra_list_s *
ASMRABuildListOfCluster (node *graph)
{
    struct asmra_list_s *list_of_cluster;
    struct asmra_cluster_s *new_cluster;
    int next_cluster_execmode;
    DBUG_ENTER ("ASMRABuildListOfCluster");

    next_cluster_execmode = MUTH_EXCLUSIVE;
    /* create initial list */
    list_of_cluster = NULL;

    /* deal with the graph as long as it contains more than one unused node */
    while (DATAFLOWNODE_REFLEFT (DATAFLOWGRAPH_SINK (graph)) != 0) {
        new_cluster = NULL;

        while (new_cluster == NULL) {
            new_cluster = ASMRABuildCluster (graph, next_cluster_execmode);

            /* "increment" next_cluster_mode (MUTH_MULTI only if the building of the
             * cluster failed) */
            switch (next_cluster_execmode) {
            case MUTH_EXCLUSIVE:
                next_cluster_execmode = MUTH_SINGLE;
                break;
            case MUTH_SINGLE:
                next_cluster_execmode = MUTH_MULTI;
                break;
            case MUTH_MULTI:
                if (new_cluster == NULL) {
                    next_cluster_execmode = MUTH_EXCLUSIVE;
                }
                break;
            }
        }
        list_of_cluster = ASMRAListAppend (list_of_cluster, new_cluster);
    }
    DBUG_RETURN (list_of_cluster);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRABuildCluster(node *graph, int execmode)
 *
 * @brief builds a cluster with execmode and MUTH_ANY out of the
 *        dataflowgraph graph; if execmode is MUTH_MULTI, use only the nodes
 *        with depends on noone, otherwise you can iterate i.e.update the
 *        reference-counter REFLEFT in-between;
 *  <pre> examples:
 *        graph:  A(MT) -> B(MT) -> C(MT) -> D(MT) -> E(MT)
 *        execmode: MT => return <A>
 *        graph:  A(ST) -> B(ST) -> C(ST) -> D(ST) -> E(MT)
 *        execmode: ST => return <A,B,C,D>
 *  <pre>
 *
 * @param graph
 * @param execmode
 * @return
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRABuildCluster (node *graph, int execmode)
{
    node *nextnode;
    struct asmra_cluster_s *result;
    struct asmra_cluster_s *result_part;
    bool node_added;

    DBUG_ENTER ("ASMRABuildCluster");

    result = NULL;
    result_part = NULL;
    do {
        node_added = FALSE;

        /* first, find all dataflownodes with execmode, that only depend on the
         * source and aren't in use */
        nextnode = ASMRAFindElement (graph, execmode);
        while (nextnode != NULL) {
            result_part = ASMRAClusterAdd (result_part, nextnode);
            nextnode = ASMRAFindElement (graph, execmode);
        }

        /* second, find all dataflownodes with MUTH_ANY, that only depend on the
         * source and aren't in use */
        nextnode = ASMRAFindElement (graph, MUTH_ANY);
        while (nextnode != NULL) {
            result_part = ASMRAClusterAdd (result_part, nextnode);
            nextnode = ASMRAFindElement (graph, MUTH_ANY);
        }

        /* third, update the referencecounter of the nodes, now new in use */
        if (result_part != NULL) {
            result_part = ASMRAClusterRefUpdate (result_part);
            result = ASMRAClusterMerge (result, result_part);
            result_part = NULL;
            node_added = TRUE;
        }

    } while ((node_added == TRUE) && (execmode != MUTH_MULTI));

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRAFindElement(node *graph, int execmode)
 *
 * @brief finds and returns a dataflownode of the dataflowgraph with
 *        DATAFLOWNODE_EXECMODE(element) = execmode,
 *        DATAFLOWNODE_REFLEFT(element) = 1, (only depended on the source) and
 *        DATAFLOWNODE_USED(element) = FALSE (it is not used, yet)
 *
 * @param graph the dataflowgraph with used and unused nodes
 * @param execmode
 * @return see above
 *
 ****************************************************************************/
node *
ASMRAFindElement (node *graph, int execmode)
{
    nodelist *member_iterator;
    node *result;
    DBUG_ENTER ("ASMRAListFindElement");

    result = NULL;
    member_iterator = DATAFLOWGRAPH_MEMBERS (graph);

    while (member_iterator != NULL) {
        if ((DATAFLOWNODE_EXECMODE (NODELIST_NODE (member_iterator)) == execmode)
            && (DATAFLOWNODE_REFLEFT (NODELIST_NODE (member_iterator)) == 1)
            && (DATAFLOWNODE_USED (NODELIST_NODE (member_iterator)) == FALSE)) {
            result = NODELIST_NODE (member_iterator);

            /* set the executionmode of the result
             * (important for ASMRADissolveCluster) */
            DATAFLOWNODE_USED (result) = TRUE;
            member_iterator = NULL;
        } else {
            member_iterator = NODELIST_NEXT (member_iterator);
        }
    }
    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_list_s *ASMRADissolveAllCluster(struct asmra_list_s *list)
 *
 * @brief gets a list of cluster and returns a list of the dataflownodes out
 *        of the cluster, whereas the sequence of the nodes corresponds to the
 *        sequence of the cluster and the sequence of the nodes of one cluster
 *        is optimal for split-phase communication
 *
 * @param list list of cluster
 * @return list of dataflownodes
 *
 ****************************************************************************/
struct asmra_list_s *
ASMRADissolveAllCluster (struct asmra_list_s *list)
{
    struct asmra_list_s *list_of_dfn, *iterator;
    struct asmra_cluster_s *act_cluster;
    node *act_node;

#if ASMRA_DEBUG
    int counter = 0;
#endif

    DBUG_ENTER ("ASMRADissolveAllCluster");
    list_of_dfn = NULL;
    act_node = NULL;
    iterator = list;

    /* do the appending for all list elements (cluster) */
    while (iterator != NULL) {
        act_cluster = ASMRA_LIST_ELEMENT (iterator);
        act_cluster = ASMRACalculateDistances (act_cluster, iterator);

#if ASMRA_DEBUG
        fprintf (stdout, " cluster no. %i:\n", ++counter);
        ASMRAPrintCluster (act_cluster);
        fprintf (stdout, "\n\n");
#endif

        act_node = ASMRAGetNodeWithLowestDistance (act_cluster, list_of_dfn);

        /* appends the current node with the lowest distance to the list as long
         * as the cluster is empty (equals act_node == NULL */
        while (act_node != NULL) {
            DATAFLOWNODE_USED (act_node) = FALSE;
            list_of_dfn = ASMRAListAppend (list_of_dfn, act_node);

            act_node = ASMRAGetNodeWithLowestDistance (act_cluster, list_of_dfn);
        }

        ASMRAFreeCluster (ASMRA_LIST_ELEMENT (iterator));
        iterator = ASMRA_LIST_NEXT (iterator);
    }

    ASMRAFreeList (list);

    DBUG_RETURN (list_of_dfn);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRACalculateDistances(
 *                                             struct asmra_cluster_s *cluster,
 *                                             struct asmra_list_s *list)
 *
 * @brief calculates the distances between each nodes of the cluster and their
 *        dependend nodes in the list
 *
 * @param cluster
 * @param list
 * @return cluster with correct distances
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRACalculateDistances (struct asmra_cluster_s *cluster, struct asmra_list_s *list)
{
    struct asmra_cluster_s *act_member;
    struct asmra_list_s *list_iterator;
    bool found_dep;
    nodelist *dependent_nodes;

    DBUG_ENTER ("ASMRACalculateDistances");

    act_member = cluster;

    /* is impossible to calculate a distance to someone without someone */
    if (list != NULL) {
        /* is impossible to calculate a distance from someone without someone */
        while (act_member != NULL) {
            /* calculate the distances one by one */

            ASMRA_CLUSTER_DISTANCE (act_member) = 0;
            list_iterator = list;
            dependent_nodes = DATAFLOWNODE_DEPENDENT (ASMRA_CLUSTER_DFN (act_member));

            /* as long as you do not find a dependend node in the actual cluster,
             * test the next one and increment the distance */
            while (list_iterator != NULL) {
                found_dep = ASMRAFoundDependent (dependent_nodes,
                                                 ASMRA_LIST_ELEMENT (list_iterator));
                if (found_dep == FALSE) {
                    ASMRA_CLUSTER_DISTANCE (act_member)++;
                    list_iterator = ASMRA_LIST_NEXT (list_iterator);
                } else {
                    list_iterator = NULL;
                }
            }

            act_member = ASMRA_CLUSTER_NEXT (act_member);
        }
    }
    DBUG_RETURN (cluster);
}

/** <!--********************************************************************-->
 *
 * @fn bool ASMRAFoundDependent(nodelist* dependent_nodes,
 *                              struct asmra_cluster_s *search_area)
 *
 * @brief detect whether a node of dependent_nodes is witin the search_area
 *        or not
 *
 * @param dependent_nodes
 * @param search_area
 * @return true, if there is a dependent_node within the saerch_area,
 *         false otherwise
 *
 ****************************************************************************/
bool
ASMRAFoundDependent (nodelist *dependent_nodes, struct asmra_cluster_s *search_area)
{
    bool result;
    DBUG_ENTER ("ASMRAFoundDependent");

    result = FALSE;

    while ((dependent_nodes != NULL) && (result == FALSE)) {
        result = ASMRAIsInCluster (NODELIST_NODE (dependent_nodes), search_area);
        dependent_nodes = NODELIST_NEXT (dependent_nodes);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn bool ASMRAIsInCluster(node *dfn, struct asmra_cluster_s *search_area)
 *
 * @brief true, if dfn element of the search_area, false otherwise
 *
 * @param dfn
 * @param search_area
 * @return see above
 *
 ****************************************************************************/
bool
ASMRAIsInCluster (node *dfn, struct asmra_cluster_s *search_area)
{
    bool result;
    DBUG_ENTER ("ASMRAIsInCluster");
    result = FALSE;

    while ((search_area != NULL) && (result == FALSE)) {
        if (dfn == ASMRA_CLUSTER_DFN (search_area)) {
            result = TRUE;
        }
        search_area = ASMRA_CLUSTER_NEXT (search_area);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRAGetNodeWithLowestDistance(struct asmra_cluster_s *cluster,
 *                                          struct asmra_list_s *list)
 *
 * @brief finds the dataflownode in the cluster, which has the lowest attribute
 *        ASMRA_CLUSTER_DISTANCE, tags it as !USED and returns it;
 *        if more than one node with this properties exists, it searchs in the
 *        current list of nodes for father-nodes, they (the 2 or more nodes)
 *        depend. the one which fathers are farer away will be returned
 *
 * @param cluster the search area
 * @param list the current list of nodes, still in the correct order, to
 *        decide which node to take if 2 (or more) have identical distances
 * @return the dataflownode out of the cluster with the lowest distance, could
 *         be NULL
 *
 ****************************************************************************/
node *
ASMRAGetNodeWithLowestDistance (struct asmra_cluster_s *cluster,
                                struct asmra_list_s *list)
{
    node *result;
    int lowest_distance;
    int father_distance;
    int father_distance_tmp;
    struct asmra_cluster_s *iterator;
    DBUG_ENTER ("ASMRAGetNodeWithLowestDistance");

    result = NULL;
    iterator = cluster;
    lowest_distance = INT_MAX;
    father_distance = -1;

    while (iterator != NULL) {
        /* is the current node unused and is its cluster-distance no the next
         * depending node equal or lower than the actual result? */
        if ((DATAFLOWNODE_USED (ASMRA_CLUSTER_DFN (iterator)) == TRUE)
            && (ASMRA_CLUSTER_DISTANCE (iterator) <= lowest_distance)) {

            father_distance_tmp
              = ASMRAGetMinDistanceToFather (ASMRA_CLUSTER_DFN (iterator), list);

#if ASMRA_DEBUG
            fprintf (stdout, "Node: %s, distance: %i, fatherdistance: %i\n",
                     DATAFLOWNODE_NAME (ASMRA_CLUSTER_DFN (iterator)),
                     ASMRA_CLUSTER_DISTANCE (iterator), father_distance_tmp);
#endif

            /* yes - and is it's distance to the father(s) greater than the
             * father-distance of the actual result? */
            if (father_distance_tmp > father_distance) {
                /* yes again - well done, we've found a new result candidate */
                result = ASMRA_CLUSTER_DFN (iterator);
                lowest_distance = ASMRA_CLUSTER_DISTANCE (iterator);
                father_distance = father_distance_tmp;
            }
        }
        iterator = ASMRA_CLUSTER_NEXT (iterator);
    }
    /* mark the result node as "used" - caution, this is a inversion of the
     * original intention */
    if (result != NULL) {
        DATAFLOWNODE_USED (result) = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn int ASMRAGetMinDistanceToFather(node *dfn, struct asmra_list_s *list )
 *
 * @brief calculates the distance from dfn to its nearest father in list, if
 *        G would be appended to list, e.g.
 *        list: A->B->C->D->E->F (->G)
 *        dfn: G
 *        let A & C be fathers of G
 *            so the nearest father would be C and the distance to G is 3.
 *
 * @param list_of_dfn list of dataflownodes, will be killed afterwards
 * @param arg_node a N_block
 * @return arg_node with a new assignment-chain in BLOCK_INSTR
 *
 ****************************************************************************/
int
ASMRAGetMinDistanceToFather (node *dfn, struct asmra_list_s *list)
{
    int distance;
    node *list_dfn;
    DBUG_ENTER ("ASMRAGetMinDistanceToFather");

    distance = 0;

    /* find the list elements who has dfn as dependent node */
    while (list != NULL) {
        list_dfn = ASMRA_LIST_ELEMENT (list);

        /* search in the dependent nodes of the actual list_dfn for dfn */
        if (NodeListFind (DATAFLOWNODE_DEPENDENT (list_dfn), dfn) != NULL) {
            /* if you found some, list_dfn must be a father of dfn, so you've got
             * to reset the distance counter */
            distance = 0;
            /*fprintf(stdout,"Found %s in %s\n",DATAFLOWNODE_NAME(dfn),
                    DATAFLOWNODE_NAME(list_dfn));
                    PrintNode(list_dfn);*/
        } else {
            /* otherwise list_dfn is no father => the distance increases */
            distance++;
        }
        list = ASMRA_LIST_NEXT (list);
    }

    DBUG_RETURN (distance);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRABuildNewAssignmentChain(struct asmra_list_s *list_of_dfn,
 *                                        node *arg_node)
 *
 * @brief gets a list of dataflownodes and builds an assignment-chain out of
 *        their assignments for the N_block arg_node *
 *
 * @param list_of_dfn list of dataflownodes, will be killed afterwards
 * @param arg_node a N_block
 * @return arg_node with a new assignment-chain in BLOCK_INSTR
 *
 ****************************************************************************/
node *
ASMRABuildNewAssignmentChain (struct asmra_list_s *list_of_dfn, node *arg_node)
{
    struct asmra_list_s *list_iterator;
    node *act_dfn;
    node *act_assign, *last_assign;
    DBUG_ENTER ("ASMRABuildNewAssignmentChain");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "N_block expected");

    list_iterator = list_of_dfn;

    /* the first assignment of the block */
    act_dfn = ASMRA_LIST_ELEMENT (list_iterator);
    act_assign = DATAFLOWNODE_ASSIGN (act_dfn);
    BLOCK_INSTR (arg_node) = act_assign;

    list_iterator = ASMRA_LIST_NEXT (list_iterator);

    /* adds assignments as long as the list of dataflownodes is empty */
    while (list_iterator != NULL) {

        last_assign = act_assign;
        act_dfn = ASMRA_LIST_ELEMENT (list_iterator);
        act_assign = DATAFLOWNODE_ASSIGN (act_dfn);

        ASSIGN_NEXT (last_assign) = act_assign;
        list_iterator = ASMRA_LIST_NEXT (list_iterator);
    }
    ASSIGN_NEXT (act_assign) = NULL;

    list_of_dfn = ASMRAFreeList (list_of_dfn);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRAMakeCluster(node *dfn)
 *
 * @brief creates a one elemented cluster with its element dfn
 *
 * @param dfn the initial-element of the cluster
 * @return the new one-elemented cluster
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRAMakeCluster (node *dfn)
{
    struct asmra_cluster_s *result;

    DBUG_ENTER ("ASMRAMakeCluster");

    result = Malloc (sizeof (struct asmra_cluster_s));

    ASMRA_CLUSTER_DFN (result) = dfn;
    ASMRA_CLUSTER_DISTANCE (result) = 0;
    ASMRA_CLUSTER_NEXT (result) = NULL;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRAFreeCluster(struct asmra_cluster_s *cluster)
 *
 * @brief frees the cluster datastructure
 *
 * @param cluster
 * @return
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRAFreeCluster (struct asmra_cluster_s *cluster)
{
    DBUG_ENTER ("ASMRAFreeCluster");

    if (ASMRA_CLUSTER_NEXT (cluster) != NULL) {
        ASMRA_CLUSTER_NEXT (cluster) = ASMRAFreeCluster (ASMRA_CLUSTER_NEXT (cluster));
    }
    cluster = Free (cluster);

    DBUG_RETURN (cluster);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRAClusterAdd(struct asmra_cluster_s *cluster,
 *                                             node *dfn)
 *
 * @brief adds a dataflownode to a cluster
 *
 * @param cluster
 * @param dfn the dataflownode to be added to cluster
 * @return cluster incl. its new member dfn
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRAClusterAdd (struct asmra_cluster_s *cluster, node *dfn)
{
    struct asmra_cluster_s *tmp;
    DBUG_ENTER ("ASMRAClusterAdd");
    tmp = ASMRAMakeCluster (dfn);
    ASMRA_CLUSTER_NEXT (tmp) = cluster;

    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRAClusterMerge
 *                                        (struct asmra_cluster_s *cluster_1,
 *                                         struct asmra_cluster_s *cluster_2)
 *
 * @brief merges two cluster by appending cluster_2 to cluster_1
 *
 * @param cluster_1
 * @param cluster_2
 * @return cluster_1 -> cluster_2
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRAClusterMerge (struct asmra_cluster_s *cluster_1, struct asmra_cluster_s *cluster_2)
{
    struct asmra_cluster_s *tmp;
    struct asmra_cluster_s *old_tmp;
    DBUG_ENTER ("ASMRAClusterMerge");

    if (cluster_1 == NULL) {
        cluster_1 = cluster_2;
    } else {
        tmp = cluster_1;
        do {
            old_tmp = tmp;
            tmp = ASMRA_CLUSTER_NEXT (tmp);
        } while (tmp != NULL);

        ASMRA_CLUSTER_NEXT (old_tmp) = cluster_2;
    }

    DBUG_RETURN (cluster_1);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *ASMRAClusterRefUpdate(struct asmra_cluster_s
 *                                                   *cluster)
 *
 * @brief updates the REFLEFT attributes of all dataflownode, that depend
 *        on the member of this cluster
 *
 * @param cluster
 * @return
 *
 ****************************************************************************/
struct asmra_cluster_s *
ASMRAClusterRefUpdate (struct asmra_cluster_s *cluster)
{
    struct asmra_cluster_s *tmp;
    nodelist *dependent_iterator;
    static int cell_id = 0;
    DBUG_ENTER ("ASMRAClusterRefUpdate");

    tmp = cluster;
    while (tmp != NULL) {
        /* update the reflefts in the member of the cluster */
        DATAFLOWNODE_REFLEFT (ASMRA_CLUSTER_DFN (tmp))--;

        /* set the cell_id for this cluster */
        ASSIGN_CELLID (DATAFLOWNODE_ASSIGN (ASMRA_CLUSTER_DFN (tmp))) = cell_id;

        /* update the reflefts in its dependent nodes */
        dependent_iterator = DATAFLOWNODE_DEPENDENT (ASMRA_CLUSTER_DFN (tmp));
        while (dependent_iterator != NULL) {
            DATAFLOWNODE_REFLEFT (NODELIST_NODE (dependent_iterator))--;
            dependent_iterator = NODELIST_NEXT (dependent_iterator);
        }

        tmp = ASMRA_CLUSTER_NEXT (tmp);
    }
    cell_id++;
    DBUG_RETURN (cluster);
}

/** <!--********************************************************************-->
 *
 * @fn void ASMRAPrintCluster(struct asmra_cluster_s *cluster)
 *
 * @brief Prints cluster to stdout
 *
 * @param cluster
 *
 ****************************************************************************/
void
ASMRAPrintCluster (struct asmra_cluster_s *cluster)
{
    DBUG_ENTER ("ASMRAPrintCluster");

    if (cluster != NULL) {
        fprintf (stdout, "%s dist:%i execm:%s; ",
                 DATAFLOWNODE_NAME (ASMRA_CLUSTER_DFN (cluster)),
                 ASMRA_CLUSTER_DISTANCE (cluster),
                 MUTHDecodeExecmode (
                   DATAFLOWNODE_EXECMODE (ASMRA_CLUSTER_DFN (cluster))));
        ASMRAPrintCluster (ASMRA_CLUSTER_NEXT (cluster));
        fflush (stdout);
    }
    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_list_s *ASMRAMakeList(void *element)
 *
 * @brief create a one elemented list with element as its element
 *
 * @param the new element
 * @return the new list
 *
 ****************************************************************************/
struct asmra_list_s *
ASMRAMakeList (void *element)
{
    struct asmra_list_s *result;

    DBUG_ENTER ("ASMRAMakeList");

    result = Malloc (sizeof (struct asmra_list_s));
    ASMRA_LIST_ELEMENT (result) = element;
    ASMRA_LIST_NEXT (result) = NULL;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_list_s *ASMRAFreeList(struct asmra_list_s *list)
 *
 * @brief frees the list list
 *
 * @param list the superflous list
 * @return
 *
 ****************************************************************************/
struct asmra_list_s *
ASMRAFreeList (struct asmra_list_s *list)
{
    DBUG_ENTER ("ASMRAFreeList");

    if (ASMRA_LIST_NEXT (list) != NULL) {
        list = ASMRAFreeList (ASMRA_LIST_NEXT (list));
    }
    list = Free (list);

    DBUG_RETURN (list);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_list_s *ASMRAListAppend(struct asmra_list_s *list,
 *                                          void* element)
 *
 * @brief appends an element to a list at its tail
 *
 * @param list the list, might be NULL
 * @param element the new element
 * @return the list with its new added elenemt
 *
 ****************************************************************************/
struct asmra_list_s *
ASMRAListAppend (struct asmra_list_s *list, void *element)
{
    struct asmra_list_s *iter;
    DBUG_ENTER ("ASMRAListAppend");

    iter = list;

    if (list != NULL) {
        while (ASMRA_LIST_NEXT (iter) != NULL) {
            iter = ASMRA_LIST_NEXT (iter);
        }

        ASMRA_LIST_NEXT (iter) = ASMRAMakeList (element);
    } else {
        list = ASMRAMakeList (element);
    }

    DBUG_RETURN (list);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASMRAPrepareDataflowgraph(node* graph)
 *
 * @brief nomen est omen - prepares the dataflowgraph for its use in asmra:
 *        sets the dataflownode-attributes REFLEFT and USED
 *
 * @param graph the dataflowgraph to prepare
 * @return the prepared dataflowgraph
 *
 ****************************************************************************/
node *
ASMRAPrepareDataflowgraph (node *graph)
{
    nodelist *iter;
    DBUG_ENTER ("ASMRAPrepareDataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (graph) == N_dataflowgraph), "N_dataflowgraph expected");

    iter = DATAFLOWGRAPH_MEMBERS (graph);

    while (iter != NULL) {
        DATAFLOWNODE_REFLEFT (NODELIST_NODE (iter))
          = DATAFLOWNODE_REFCOUNT (NODELIST_NODE (iter));
        DATAFLOWNODE_USED (NODELIST_NODE (iter)) = FALSE;
        iter = NODELIST_NEXT (iter);
    }

    /* guarantee the non-use of the source */
    DATAFLOWNODE_USED (DATAFLOWGRAPH_SOURCE (graph)) = TRUE;

    DBUG_RETURN (graph);
}

/**
 * @}
 **/
