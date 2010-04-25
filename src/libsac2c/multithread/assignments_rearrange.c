/*
 * $Id$
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

#include <limits.h> /* for INT_MAX */
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "assignments_rearrange.h"
#include "multithread_lib.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "globals.h"

/*
 * some local structures
 */

struct asmra_cluster_s {
    node *dfn;
    int distance;
    struct asmra_cluster_s *next;
};

struct asmra_list_s {
    node *node;
    struct asmra_cluster_s *cluster;
    struct asmra_list_s *next;
};

struct INFO {
    node *next;
};

/*
 * some access macros
 *   node*                    ASMRA_CLUSTER_DFN
 *   int                      ASMRA_CLUSTER_DISTANCE
 *   struct asmra_cluster_s*  ASMRA_CLUSTER_NEXT
 *
 *   node*                    ASMRA_LIST_NODEELEM
 *   struct asmra_cluster_s*  ASMRA_LIST_CLUSTERELEM
 *   struct asmra_list_s*     ASMRA_LIST_NEXT
 */
#define ASMRA_CLUSTER_DFN(n) (n->dfn)
#define ASMRA_CLUSTER_DISTANCE(n) (n->distance)
#define ASMRA_CLUSTER_NEXT(n) (n->next)
#define ASMRA_LIST_NODEELEM(n) (n->node)
#define ASMRA_LIST_CLUSTERELEM(n) (n->cluster)
#define ASMRA_LIST_NEXT(n) (n->next)

/*
 * INFO macros
 *   node*      NEXT
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

    result = MEMmalloc (sizeof (info));

    INFO_ASMRA_NEXT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* some local declarations */
static node *CreateNewAssignmentOrder (node *arg_node);

static struct asmra_list_s *BuildListOfCluster (node *graph);

static struct asmra_cluster_s *BuildCluster (node *graph, mtexecmode_t execmode);

static node *FindElement (node *graph, mtexecmode_t execmode);

static struct asmra_list_s *DissolveAllCluster (struct asmra_list_s *list_of_cluster);

static struct asmra_cluster_s *CalculateDistances (struct asmra_cluster_s *cluster,
                                                   struct asmra_list_s *list);
static bool FoundDependent (nodelist *dependent_nodes,
                            struct asmra_cluster_s *search_area);
static bool IsInCluster (node *dfn, struct asmra_cluster_s *search_area);

static node *GetNodeWithLowestDistance (struct asmra_cluster_s *cluster,
                                        struct asmra_list_s *list);
static int GetMinDistanceToFather (node *dfn, struct asmra_list_s *list);

static node *BuildNewAssignmentChain (struct asmra_list_s *list_of_dfn, node *arg_node);
static struct asmra_cluster_s *MakeCluster (node *arg_node);

static struct asmra_cluster_s *FreeCluster (struct asmra_cluster_s *cluster);

static struct asmra_cluster_s *ClusterAdd (struct asmra_cluster_s *cluster, node *dfn);
static struct asmra_cluster_s *ClusterMerge (struct asmra_cluster_s *cluster_1,
                                             struct asmra_cluster_s *cluster_2);
static struct asmra_cluster_s *ClusterRefUpdate (struct asmra_cluster_s *cluster);

#if 0
/*
 * Note, this function is currently not used and hence its definition causes 
 * a compiler warning.
 */
static
void PrintCluster(struct asmra_cluster_s *cluster);
#endif

static struct asmra_list_s *FreeList (struct asmra_list_s *list);

static struct asmra_list_s *ListAppend (struct asmra_list_s *list, node *node,
                                        struct asmra_cluster_s *cluster);

static node *PrepareDataflowgraph (node *graph);

/*** <!--*******************************************************************-->
 *
 * @fn:node *ASMRAdoAssignmentsRearrange(node *arg_node)
 *
 * @brief Inits the traversal for this phase
 *
 * @param arg_node
 * @return
 *
 *****************************************************************************/
node *
ASMRAdoAssignmentsRearrange (node *arg_node)
{
    info *arg_info;
    trav_t traversaltable;
    DBUG_ENTER ("ASMRAdoAssignmentsRearrange");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "ASMRAdoAssignmentsRearrange expects a N_module as arg_node");

    arg_info = MakeInfo ();

    TRAVpush (TR_asmra);

    DBUG_PRINT ("ASMRA", ("trav into module-funs"));
    MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from module-funs"));

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_asmra), "Popped incorrect traversal table");

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

    if (BLOCK_INSTR (arg_node) != NULL) {
        if (NODE_TYPE (BLOCK_INSTR (arg_node)) == N_assign) {
            if (ASSIGN_EXECMODE (BLOCK_INSTR (arg_node)) != MUTH_MULTI_SPECIALIZED) {
                arg_node = CreateNewAssignmentOrder (arg_node);
            }
        }
    }

    /* continue traversal */
    DBUG_PRINT ("ASMRA", ("trav into instruction(s)"));
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    DBUG_PRINT ("ASMRA", ("trav from instruction(s)"));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateNewAssignmentOrder(node *arg_node)
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
static node *
CreateNewAssignmentOrder (node *arg_node)
{
    struct asmra_list_s *my_list;
    node *graph;
    DBUG_ENTER ("CreateNewAssignmentOrder");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "node is not a N_block");

    graph = BLOCK_DATAFLOWGRAPH (arg_node);

    graph = PrepareDataflowgraph (graph);

    my_list = BuildListOfCluster (graph);

    my_list = DissolveAllCluster (my_list);

    arg_node = BuildNewAssignmentChain (my_list, arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_list_s *BuildListOfCluster(node *graph)
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
static struct asmra_list_s *
BuildListOfCluster (node *graph)
{
    struct asmra_list_s *list_of_cluster;
    struct asmra_cluster_s *new_cluster;
    mtexecmode_t next_cluster_execmode;
    DBUG_ENTER ("BuildListOfCluster");

    next_cluster_execmode = MUTH_EXCLUSIVE;
    /* create initial list */
    list_of_cluster = NULL;

    /* deal with the graph as long as it contains more than one unused node */
    while (DATAFLOWNODE_REFLEFT (DATAFLOWGRAPH_SINK (graph)) != 0) {
        new_cluster = NULL;

        while (new_cluster == NULL) {
            new_cluster = BuildCluster (graph, next_cluster_execmode);

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
            case MUTH_ANY:
                break;
            case MUTH_MULTI_SPECIALIZED:
                break;
            default:
                break;
            }
        }
        list_of_cluster = ListAppend (list_of_cluster, NULL, new_cluster);
    }
    DBUG_RETURN (list_of_cluster);
}

/** <!--********************************************************************-->
 *
 * @fn struct asmra_cluster_s *BuildCluster(node *graph, mtexecmode_t execmode)
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
static struct asmra_cluster_s *
BuildCluster (node *graph, mtexecmode_t execmode)
{
    node *nextnode;
    struct asmra_cluster_s *result;
    struct asmra_cluster_s *result_part;
    bool node_added;

    DBUG_ENTER ("BuildCluster");

    result = NULL;
    result_part = NULL;
    do {
        node_added = FALSE;

        /*
         * first, find all dataflownodes with execmode, that only depend on the
         * source and aren't in use
         */
        nextnode = FindElement (graph, execmode);
        while (nextnode != NULL) {
            result_part = ClusterAdd (result_part, nextnode);
            nextnode = FindElement (graph, execmode);
        }

        /*
         * second, find all dataflownodes with MUTH_ANY, that only depend on the
         * source and aren't in use
         */
        nextnode = FindElement (graph, MUTH_ANY);
        while (nextnode != NULL) {
            result_part = ClusterAdd (result_part, nextnode);
            nextnode = FindElement (graph, MUTH_ANY);
        }

        /*
         * third, update the referencecounter of the nodes, now new in use
         */
        if (result_part != NULL) {
            result_part = ClusterRefUpdate (result_part);
            result = ClusterMerge (result, result_part);
            result_part = NULL;
            node_added = TRUE;
        }

    } while ((node_added == TRUE) && (execmode != MUTH_MULTI));

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static node *FindElement(node *graph, mtexecmode_t execmode)
 *
 * @brief finds and returns a dataflownode of the dataflowgraph with
 *        DATAFLOWNODE_EXECMODE(element) = execmode,
 *        DATAFLOWNODE_REFLEFT(element) = 1, (only depended on the source) and
 *        DATAFLOWNODE_ISUSED(element) = FALSE (it is not used, yet)
 *
 * @param graph the dataflowgraph with used and unused nodes
 * @param execmode
 * @return see above
 *
 ****************************************************************************/
static node *
FindElement (node *graph, mtexecmode_t execmode)
{
    nodelist *member_iterator;
    node *result;
    DBUG_ENTER ("ListFindElement");

    result = NULL;
    member_iterator = DATAFLOWGRAPH_MEMBERS (graph);

    while (member_iterator != NULL) {
        if ((DATAFLOWNODE_EXECMODE (NODELIST_NODE (member_iterator)) == execmode)
            && (DATAFLOWNODE_REFLEFT (NODELIST_NODE (member_iterator)) == 1)

            && (!DATAFLOWNODE_ISUSED (NODELIST_NODE (member_iterator)))) {
            result = NODELIST_NODE (member_iterator);

            /* set the executionmode of the result
             * (important for DissolveCluster) */
            DATAFLOWNODE_ISUSED (result) = TRUE;
            member_iterator = NULL;
        } else {
            member_iterator = NODELIST_NEXT (member_iterator);
        }
    }
    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 *@fn static struct asmra_list_s *DissolveAllCluster(struct asmra_list_s *list)
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
static struct asmra_list_s *
DissolveAllCluster (struct asmra_list_s *list)
{
    struct asmra_list_s *list_of_dfn, *iterator;
    struct asmra_cluster_s *act_cluster;
    node *act_node;

    DBUG_ENTER ("DissolveAllCluster");
    list_of_dfn = NULL;
    act_node = NULL;
    iterator = list;

    /* do the appending for all list elements (cluster) */
    while (iterator != NULL) {
        act_cluster = ASMRA_LIST_CLUSTERELEM (iterator);
        act_cluster = CalculateDistances (act_cluster, iterator);

        act_node = GetNodeWithLowestDistance (act_cluster, list_of_dfn);

        /* appends the current node with the lowest distance to the list as long
         * as the cluster is empty (equals act_node == NULL */
        while (act_node != NULL) {
            DATAFLOWNODE_ISUSED (act_node) = FALSE;
            list_of_dfn = ListAppend (list_of_dfn, act_node, NULL);

            act_node = GetNodeWithLowestDistance (act_cluster, list_of_dfn);
        }

        FreeCluster (ASMRA_LIST_CLUSTERELEM (iterator));
        iterator = ASMRA_LIST_NEXT (iterator);
    }

    FreeList (list);

    DBUG_RETURN (list_of_dfn);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_cluster_s *CalculateDistances(
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
static struct asmra_cluster_s *
CalculateDistances (struct asmra_cluster_s *cluster, struct asmra_list_s *list)
{
    struct asmra_cluster_s *act_member;
    struct asmra_list_s *list_iterator;
    bool found_dep;
    nodelist *dependent_nodes;

    DBUG_ENTER ("CalculateDistances");

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
                found_dep = FoundDependent (dependent_nodes,
                                            ASMRA_LIST_CLUSTERELEM (list_iterator));
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
 * @fn static bool FoundDependent(nodelist* dependent_nodes,
 *                                struct asmra_cluster_s *search_area)
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
static bool
FoundDependent (nodelist *dependent_nodes, struct asmra_cluster_s *search_area)
{
    bool result;
    DBUG_ENTER ("FoundDependent");

    result = FALSE;

    while ((dependent_nodes != NULL) && (result == FALSE)) {
        result = IsInCluster (NODELIST_NODE (dependent_nodes), search_area);
        dependent_nodes = NODELIST_NEXT (dependent_nodes);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static bool IsInCluster(node *dfn, struct asmra_cluster_s *search_area)
 *
 * @brief true, if dfn element of the search_area, false otherwise
 *
 * @param dfn
 * @param search_area
 * @return see above
 *
 ****************************************************************************/
static bool
IsInCluster (node *dfn, struct asmra_cluster_s *search_area)
{
    bool result;
    DBUG_ENTER ("IsInCluster");
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
 * @fn static node *GetNodeWithLowestDistance(struct asmra_cluster_s *cluster,
 *                                            struct asmra_list_s *list)
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
static node *
GetNodeWithLowestDistance (struct asmra_cluster_s *cluster, struct asmra_list_s *list)
{
    node *result;
    int lowest_distance;
    int father_distance;
    int father_distance_tmp;
    struct asmra_cluster_s *iterator;
    DBUG_ENTER ("GetNodeWithLowestDistance");

    result = NULL;
    iterator = cluster;
    lowest_distance = INT_MAX;
    father_distance = -1;

    while (iterator != NULL) {
        /* is the current node unused and is its cluster-distance no the next
         * depending node equal or lower than the actual result? */
        if (DATAFLOWNODE_ISUSED (ASMRA_CLUSTER_DFN (iterator))
            && (ASMRA_CLUSTER_DISTANCE (iterator) <= lowest_distance)) {

            father_distance_tmp
              = GetMinDistanceToFather (ASMRA_CLUSTER_DFN (iterator), list);

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
        DATAFLOWNODE_ISUSED (result) = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static int GetMinDistanceToFather(node *dfn, struct asmra_list_s *list )
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
static int
GetMinDistanceToFather (node *dfn, struct asmra_list_s *list)
{
    int distance;
    node *list_dfn;
    DBUG_ENTER ("GetMinDistanceToFather");

    distance = 0;

    /* find the list elements who has dfn as dependent node */
    while (list != NULL) {
        list_dfn = ASMRA_LIST_NODEELEM (list);

        /* search in the dependent nodes of the actual list_dfn for dfn */
        if (TCnodeListFind (DATAFLOWNODE_DEPENDENT (list_dfn), dfn) != NULL) {
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
 * @fn static node *BuildNewAssignmentChain(struct asmra_list_s *list_of_dfn,
 *                                          node *arg_node)
 *
 * @brief gets a list of dataflownodes and builds an assignment-chain out of
 *        their assignments for the N_block arg_node *
 *
 * @param list_of_dfn list of dataflownodes, will be killed afterwards
 * @param arg_node a N_block
 * @return arg_node with a new assignment-chain in BLOCK_INSTR
 *
 ****************************************************************************/
static node *
BuildNewAssignmentChain (struct asmra_list_s *list_of_dfn, node *arg_node)
{
    struct asmra_list_s *list_iterator;
    node *act_dfn;
    node *act_assign, *last_assign;
    DBUG_ENTER ("BuildNewAssignmentChain");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_block), "N_block expected");

    list_iterator = list_of_dfn;

    /* the first assignment of the block */
    act_dfn = ASMRA_LIST_NODEELEM (list_iterator);
    act_assign = DATAFLOWNODE_ASSIGN (act_dfn);
    BLOCK_INSTR (arg_node) = act_assign;

    list_iterator = ASMRA_LIST_NEXT (list_iterator);

    /* adds assignments as long as the list of dataflownodes is empty */
    while (list_iterator != NULL) {

        last_assign = act_assign;
        act_dfn = ASMRA_LIST_NODEELEM (list_iterator);
        act_assign = DATAFLOWNODE_ASSIGN (act_dfn);

        ASSIGN_NEXT (last_assign) = act_assign;
        list_iterator = ASMRA_LIST_NEXT (list_iterator);
    }
    ASSIGN_NEXT (act_assign) = NULL;

    list_of_dfn = FreeList (list_of_dfn);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_cluster_s *MakeCluster(node *dfn)
 *
 * @brief creates a one elemented cluster with its element dfn
 *
 * @param dfn the initial-element of the cluster
 * @return the new one-elemented cluster
 *
 ****************************************************************************/
static struct asmra_cluster_s *
MakeCluster (node *dfn)
{
    struct asmra_cluster_s *result;

    DBUG_ENTER ("MakeCluster");

    result = MEMmalloc (sizeof (struct asmra_cluster_s));

    ASMRA_CLUSTER_DFN (result) = dfn;
    ASMRA_CLUSTER_DISTANCE (result) = 0;
    ASMRA_CLUSTER_NEXT (result) = NULL;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_cluster_s *FreeCluster(struct asmra_cluster_s *cluster)
 *
 * @brief frees the cluster datastructure
 *
 * @param cluster
 * @return
 *
 ****************************************************************************/
static struct asmra_cluster_s *
FreeCluster (struct asmra_cluster_s *cluster)
{
    DBUG_ENTER ("FreeCluster");

    if (ASMRA_CLUSTER_NEXT (cluster) != NULL) {
        ASMRA_CLUSTER_NEXT (cluster) = FreeCluster (ASMRA_CLUSTER_NEXT (cluster));
    }
    cluster = MEMfree (cluster);

    DBUG_RETURN (cluster);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_cluster_s *ClusterAdd(
 *                                  struct asmra_cluster_s *cluster, node *dfn)
 *
 * @brief adds a dataflownode to a cluster
 *
 * @param cluster
 * @param dfn the dataflownode to be added to cluster
 * @return cluster incl. its new member dfn
 *
 ****************************************************************************/
static struct asmra_cluster_s *
ClusterAdd (struct asmra_cluster_s *cluster, node *dfn)
{
    struct asmra_cluster_s *tmp;
    DBUG_ENTER ("ClusterAdd");
    tmp = MakeCluster (dfn);
    ASMRA_CLUSTER_NEXT (tmp) = cluster;

    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_cluster_s *ClusterMerge
 *                                          (struct asmra_cluster_s *cluster_1,
 *                                           struct asmra_cluster_s *cluster_2)
 *
 * @brief merges two cluster by appending cluster_2 to cluster_1
 *
 * @param cluster_1
 * @param cluster_2
 * @return cluster_1 -> cluster_2
 *
 ****************************************************************************/
static struct asmra_cluster_s *
ClusterMerge (struct asmra_cluster_s *cluster_1, struct asmra_cluster_s *cluster_2)
{
    struct asmra_cluster_s *tmp;
    struct asmra_cluster_s *old_tmp;
    DBUG_ENTER ("ClusterMerge");

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
 * @fn static struct asmra_cluster_s *ClusterRefUpdate(
 *                                             struct asmra_cluster_s *cluster)
 *
 * @brief updates the REFLEFT attributes of all dataflownode, that depend
 *        on the member of this cluster
 *
 * @param cluster
 * @return
 *
 ****************************************************************************/
static struct asmra_cluster_s *
ClusterRefUpdate (struct asmra_cluster_s *cluster)
{
    struct asmra_cluster_s *tmp;
    nodelist *dependent_iterator;
    static int cell_id = 0;
    DBUG_ENTER ("ClusterRefUpdate");

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

#if 0

/*
 * Note, this function is currently not used and hence its definition causes 
 * a compiler warning.
 */

/** <!--********************************************************************-->
 *
 * @fn static void PrintCluster(struct asmra_cluster_s *cluster)
 *
 * @brief Prints cluster to stdout
 *
 * @param cluster
 *
 ****************************************************************************/
static 
void PrintCluster(struct asmra_cluster_s *cluster) 
{
  DBUG_ENTER("PrintCluster");
  
  if (cluster != NULL) {
    fprintf(stdout,"%s dist:%i execm:%s; ",
            DATAFLOWNODE_NAME(ASMRA_CLUSTER_DFN(cluster)),
            ASMRA_CLUSTER_DISTANCE(cluster),
            MUTHLIBdecodeExecmode(DATAFLOWNODE_EXECMODE(ASMRA_CLUSTER_DFN(cluster))));
    PrintCluster(ASMRA_CLUSTER_NEXT(cluster));
    fflush(stdout);
  }
  DBUG_VOID_RETURN;  
}

#endif

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_list_s *FreeList(struct asmra_list_s *list)
 *
 * @brief frees the list list
 *
 * @param list the superflous list
 * @return
 *
 ****************************************************************************/
static struct asmra_list_s *
FreeList (struct asmra_list_s *list)
{
    DBUG_ENTER ("FreeList");

    if (ASMRA_LIST_NEXT (list) != NULL) {
        list = FreeList (ASMRA_LIST_NEXT (list));
    }
    list = MEMfree (list);

    DBUG_RETURN (list);
}

/** <!--********************************************************************-->
 *
 * @fn static struct asmra_list_s *ListAppend(struct asmra_list_s *list,
 *                                            node* node,
 *                                            struct* asmra_cluster_s)
 *
 * @brief appends an element to a list at its tail
 *
 * @param list the list, might be NULL - you will get a new list then
 * @param element the new element
 * @return the list with its new added elenemt
 *
 ****************************************************************************/
static struct asmra_list_s *
ListAppend (struct asmra_list_s *list, node *node, struct asmra_cluster_s *cluster)
{
    struct asmra_list_s *iter;
    DBUG_ENTER ("ListAppend");

    iter = list;

    if (list != NULL) {
        while (ASMRA_LIST_NEXT (iter) != NULL) {
            iter = ASMRA_LIST_NEXT (iter);
        }
        ASMRA_LIST_NEXT (iter) = MEMmalloc (sizeof (struct asmra_list_s));
        if (node != NULL) {
            ASMRA_LIST_NODEELEM (ASMRA_LIST_NEXT (iter)) = node;
        } else {
            ASMRA_LIST_CLUSTERELEM (ASMRA_LIST_NEXT (iter)) = cluster;
        }
        ASMRA_LIST_NEXT (ASMRA_LIST_NEXT (iter)) = NULL;
    } else {

        list = MEMmalloc (sizeof (struct asmra_list_s));
        if (node != NULL) {
            ASMRA_LIST_NODEELEM (ASMRA_LIST_NEXT (list)) = node;
        } else {
            ASMRA_LIST_CLUSTERELEM (ASMRA_LIST_NEXT (list)) = cluster;
        }

        ASMRA_LIST_NEXT (list) = NULL;
    }

    DBUG_RETURN (list);
}

/** <!--********************************************************************-->
 *
 * @fn static node *PrepareDataflowgraph(node* graph)
 *
 * @brief nomen est omen - prepares the dataflowgraph for its use in asmra:
 *        sets the dataflownode-attributes REFLEFT and USED
 *
 * @param graph the dataflowgraph to prepare
 * @return the prepared dataflowgraph
 *
 ****************************************************************************/
static node *
PrepareDataflowgraph (node *graph)
{
    nodelist *iter;
    DBUG_ENTER ("PrepareDataflowgraph");
    DBUG_ASSERT ((NODE_TYPE (graph) == N_dataflowgraph), "N_dataflowgraph expected");

    iter = DATAFLOWGRAPH_MEMBERS (graph);

    while (iter != NULL) {
        DATAFLOWNODE_REFLEFT (NODELIST_NODE (iter))
          = DATAFLOWNODE_REFCOUNT (NODELIST_NODE (iter));
        DATAFLOWNODE_ISUSED (NODELIST_NODE (iter)) = FALSE;
        iter = NODELIST_NEXT (iter);
    }

    /* guarantee the non-use of the source */
    DATAFLOWNODE_ISUSED (DATAFLOWGRAPH_SOURCE (graph)) = TRUE;

    DBUG_RETURN (graph);
}

/**
 * @}
 **/
