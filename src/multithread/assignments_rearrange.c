/*
 * $Log$
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "assignments_rearrange.h"
#include "print.h"
#include "multithread.h"

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

    if (INFO_ASMRA_NEXT (info) != NULL) {
        info = FreeInfo (info);
    }
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

/** <!--********************************************************************->>
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

    if (NODE_TYPE (BLOCK_INSTR (arg_node)) == N_assign) {
        arg_node = ASMRACreateNewAssignmentOrder (arg_node);
    }

    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into instruction(s)"));
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from instruction(s)"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *ASMRACreateNewAssignmentOrder(node *arg_node)
 *
 * @brief Top-level rearrange arg_node's assignment-chain
 *
 * @param arg_node a N_block
 * @return N_block with rearranged assignment chain on top-level
 *
 ****************************************************************************/

node *
ASMRACreateNewAssignmentOrder (node *arg_node)
{
    struct asmra_list_s *my_list;
    DBUG_ENTER ("ASMRACreateNewAssignmentOrder");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

    my_list = ASMRABuildListOfCluster (BLOCK_DATAFLOWGRAPH (arg_node));

    my_list = ASMRADissolveAllCluster (my_list);

    arg_node = ASMRABuildNewAssignmentChain (my_list, arg_node);

    DBUG_RETURN (arg_node);
}

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
    while (DATAFLOWNODE_REFCOUNT (DATAFLOWGRAPH_SINK (graph)) != 0) {
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

struct asmra_cluster_s *
ASMRABuildCluster (node *graph, int execmode)
{
    node *nextnode;
    struct asmra_cluster_s *result;
    bool node_added;
    DBUG_ENTER ("ASMRABuildCluster");

    nextnode = ASMRAFindElement (graph, execmode);
    result = NULL;

    do {
        node_added = FALSE;
        while (nextnode != NULL) {
            result = ASMRAClusterAdd (result, nextnode);
            node_added = TRUE;
            nextnode = ASMRAFindElement (graph, execmode);
        }

        ASMRAPrintCluster (result);
        result = ASMRAClusterRefUpdate (result);
    } while ((execmode != MUTH_MULTI) && (node_added == TRUE));

    DBUG_RETURN (result);
}

/*
 * returns a dataflownode from the graph with no references on it (exept the
 * one by the source) and which executionmode equals MUTH_ANY or execmode
 */
node *
ASMRAFindElement (node *graph, int execmode)
{
    nodelist *member_iterator;
    node *result;
    DBUG_ENTER ("ASMRAListFindElement");

    result = NULL;
    member_iterator = DATAFLOWGRAPH_MEMBERS (graph);

    while (member_iterator != NULL) {
        if (((DATAFLOWNODE_EXECMODE (NODELIST_NODE (member_iterator)) == execmode)
             || (DATAFLOWNODE_EXECMODE (NODELIST_NODE (member_iterator)) == MUTH_ANY))
            && (DATAFLOWNODE_REFCOUNT (NODELIST_NODE (member_iterator)) == 1)
            && (DATAFLOWNODE_REFLEFT (NODELIST_NODE (member_iterator)) == 0)) {
            result = NODELIST_NODE (member_iterator);

            /* set the executionmode of the result
             * (important for ASMRADissolveCLuster) */
            DATAFLOWNODE_EXECMODE (result) = execmode;
            DATAFLOWNODE_REFLEFT (result) = -1;
            member_iterator = NULL;
        } else {
            member_iterator = NODELIST_NEXT (member_iterator);
        }
    }
    DBUG_RETURN (result);
}

struct asmra_list_s *
ASMRADissolveAllCluster (struct asmra_list_s *list)
{
    struct asmra_list_s *list_of_dfn, *iterator;
    struct asmra_cluster_s *act_cluster;
    node *act_node;

    DBUG_ENTER ("ASMRADissolveAllCluster");
    list_of_dfn = NULL;
    act_node = NULL;
    iterator = list;

    while (ASMRA_LIST_ELEMENT (iterator) != NULL) {
        act_cluster = ASMRA_LIST_ELEMENT (iterator);
        act_cluster = ASMRACalculateDistances (act_cluster, iterator);

        act_node = ASMRAGetNodeWithLowestDistance (act_cluster);
        while (act_node != NULL) {
            list_of_dfn = ASMRAListAppend (list_of_dfn, act_node);
            act_node = ASMRAGetNodeWithLowestDistance (act_cluster);
        }

        ASMRAFreeCluster (ASMRA_LIST_ELEMENT (iterator));
        iterator = ASMRA_LIST_NEXT (iterator);
    }

    ASMRAFreeList (list);

    DBUG_RETURN (list_of_dfn);
}

struct asmra_cluster_s *
ASMRACalculateDistances (struct asmra_cluster_s *cluster, struct asmra_list_s *list)
{
    struct asmra_cluster_s *act_member;
    struct asmra_list_s *list_iterator;

    DBUG_ENTER ("ASMRACalculateDistances");

    act_member = cluster;

    while (act_member != NULL) {
        ASMRA_CLUSTER_DISTANCE (act_member) = 0;
        list_iterator = list;
        while ((list_iterator != NULL)
               && (ASMRAFoundDependent (DATAFLOWNODE_DEPENDENT (
                                          ASMRA_CLUSTER_DFN (act_member)),
                                        ASMRA_LIST_ELEMENT (list_iterator))
                   == FALSE))
            ;
        {

            list_iterator = ASMRA_LIST_NEXT (list_iterator);
            ASMRA_CLUSTER_DISTANCE (act_member)++;
        }
    }
    DBUG_RETURN (cluster);
}

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

bool
ASMRAIsInCluster (node *dfn, struct asmra_cluster_s *search_area)
{
    bool result;
    DBUG_ENTER ("ASMRAIsInCluster");
    result = FALSE;

    while (search_area != NULL) {
        if (dfn == ASMRA_CLUSTER_DFN (search_area)) {
            result = TRUE;
            search_area = NULL;
        }
    }

    DBUG_RETURN (result);
}

node *
ASMRAGetNodeWithLowestDistance (struct asmra_cluster_s *act_cluster)
{
    node *result;
    int lowest_distance;
    struct asmra_cluster_s *iterator;
    DBUG_ENTER ("ASMRAGetNodeWithLowestDistance");

    iterator = act_cluster;
    if (iterator != NULL) {
        result = ASMRA_CLUSTER_DFN (iterator);
        lowest_distance = ASMRA_CLUSTER_DISTANCE (iterator);
        iterator = ASMRA_CLUSTER_NEXT (iterator);
    }

    while (iterator != NULL) {
        if (ASMRA_CLUSTER_DISTANCE (iterator) < lowest_distance) {
            lowest_distance = ASMRA_CLUSTER_DISTANCE (iterator);
            result = ASMRA_CLUSTER_DFN (iterator);
        }
        iterator = ASMRA_CLUSTER_NEXT (iterator);
    }

    DBUG_RETURN (result);
}

node *
ASMRABuildNewAssignmentChain (struct asmra_list_s *list_of_dfn, node *arg_node)
{
    struct asmra_list_s *list_iterator;
    node *act_assign, *last_assign;
    DBUG_ENTER ("ASMRABuildNewAssignmentChain");

    list_iterator = list_of_dfn;
    act_assign = ASMRA_LIST_ELEMENT (list_iterator);
    BLOCK_INSTR (arg_node) = act_assign;

    list_iterator = ASMRA_LIST_NEXT (list_iterator);

    while (list_iterator != NULL) {
        last_assign = act_assign;
        act_assign = ASMRA_LIST_ELEMENT (list_iterator);
        ASSIGN_NEXT (last_assign) = act_assign;
        list_iterator = ASMRA_LIST_NEXT (list_iterator);
    }

    list_of_dfn = ASMRAFreeList (list_of_dfn);
    DBUG_RETURN (arg_node);
}

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

struct asmra_cluster_s *
ASMRAFreeCluster (struct asmra_cluster_s *cluster)
{
    DBUG_ENTER ("ASMRAFreeCluster");

    if (ASMRA_CLUSTER_NEXT (cluster) != NULL) {
        ASMRA_CLUSTER_NEXT (cluster) = ASMRAFreeCluster (ASMRA_CLUSTER_NEXT (cluster));
    }
    free (ASMRA_CLUSTER_DFN (cluster));
    free (ASMRA_CLUSTER_NEXT (cluster));

    return cluster;
}

struct asmra_cluster_s *
ASMRAClusterAdd (struct asmra_cluster_s *cluster, node *dfn)
{
    struct asmra_cluster_s *tmp;
    DBUG_ENTER ("ASMRAClusterAdd");
    tmp = ASMRAMakeCluster (dfn);
    ASMRA_CLUSTER_NEXT (tmp) = cluster;

    DBUG_RETURN (tmp);
}

struct asmra_cluster_s *
ASMRAClusterRefUpdate (struct asmra_cluster_s *cluster)
{
    struct asmra_cluster_s *tmp;
    nodelist *dependent_iterator;
    DBUG_ENTER ("ASMRAClusterRefUpdate");

    tmp = cluster;
    while (tmp != NULL) {
        /* update the refcounts in the member of the cluster */
        DATAFLOWNODE_REFCOUNT (ASMRA_CLUSTER_DFN (tmp))--;

        /* update the refcount in its dependent nodes */
        dependent_iterator = DATAFLOWNODE_DEPENDENT (ASMRA_CLUSTER_DFN (tmp));
        while (dependent_iterator != NULL) {
            DATAFLOWNODE_REFCOUNT (NODELIST_NODE (dependent_iterator))--;
            dependent_iterator = NODELIST_NEXT (dependent_iterator);
        }

        tmp = ASMRA_CLUSTER_NEXT (tmp);
    }
    DBUG_RETURN (cluster);
}

void
ASMRAPrintCluster (struct asmra_cluster_s *cluster)
{
    DBUG_ENTER ("ASMRAPrintCluster");

    if (cluster != NULL) {
        fprintf (stdout, "%s," DATAFLOWNODE_NAME (ASMRA_CLUSTER_DFN (cluster)));
        ASMRAPrintCluster (cluster);
    }
    DBUG_VOID_RETURN;
}

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

struct asmra_list_s *
ASMRAFreeList (struct asmra_list_s *list)
{
    DBUG_ENTER ("ASMRAFreeList");

    if (ASMRA_LIST_NEXT (list) != NULL) {
        ASMRA_LIST_NEXT (list) = ASMRAFreeList (ASMRA_LIST_NEXT (list));
    }
    free (ASMRA_LIST_ELEMENT (list));
    free (ASMRA_LIST_NEXT (list));

    return list;
}

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

/**
 * @}
 **/
