/*
 * $Log$
 * Revision 1.7  2004/08/19 15:01:03  skt
 * rearranging algorithm improved
 *
 * Revision 1.6  2004/08/12 12:52:19  skt
 * some debugging...
 *
 * Revision 1.5  2004/08/11 09:31:54  skt
 * ASMRAPrintCluster bug fixed
 *
 * Revision 1.4  2004/08/11 08:38:44  skt
 * full redesigned, still under construction but looks well
 *
 * Revision 1.3  2004/07/29 00:41:50  skt
 * build compilable intermediate version
 * work in progress
 *
 * Revision 1.2  2004/04/30 14:10:05  skt
 * some debugging
 *
 * Revision 1.1  2004/04/27 09:59:21  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   assignments_rearrange.h
 *
 * description:
 *   header file for assignments_rearrange.c
 *
 *****************************************************************************/

#ifndef ASSIGNMENTS_REARRANGE_H

#define ASSIGNMENTS_REARRANGE_H

#define ASMRA_DEBUG 0

/*
 * some structures
 */

struct asmra_cluster_s {
    node *dfn;
    int distance;
    struct asmra_cluster_s *next;
};

struct asmra_list_s {
    void *element;
    struct asmra_list_s *next;
};

/*
 * some access macros
 *   node*                    ASMRA_CLUSTER_DFN
 *   int                      ASMRA_CLUSTER_DISTANCE
 *   struct asmra_cluster_s*  ASMRA_CLUSTER_NEXT
 *
 *   void*                    ASMRA_LIST_ELEMENT
 *   struct asmra_list_s*     ASMRA_LIST_NEXT
 */
#define ASMRA_CLUSTER_DFN(n) (n->dfn)
#define ASMRA_CLUSTER_DISTANCE(n) (n->distance)
#define ASMRA_CLUSTER_NEXT(n) (n->next)
#define ASMRA_LIST_ELEMENT(n) (n->element)
#define ASMRA_LIST_NEXT(n) (n->next)

extern node *AssignmentsRearrange (node *arg_node);

extern node *ASMRAblock (node *arg_node, info *arg_info);

extern node *ASMRACreateNewAssignmentOrder (node *arg_node);

struct asmra_list_s *ASMRABuildListOfCluster (node *graph);

struct asmra_cluster_s *ASMRABuildCluster (node *graph, int execmode);

node *ASMRAFindElement (node *graph, int execmode);

struct asmra_list_s *ASMRADissolveAllCluster (struct asmra_list_s *list_of_cluster);

struct asmra_cluster_s *ASMRACalculateDistances (struct asmra_cluster_s *cluster,
                                                 struct asmra_list_s *list);

bool ASMRAOneNodeIsDependent (struct asmra_cluster_s *cluster,
                              struct asmra_cluster_s *dependent_cluster);

bool ASMRAFoundDependent (nodelist *dependent_nodes, struct asmra_cluster_s *search_area);

bool ASMRAIsInCluster (node *dfn, struct asmra_cluster_s *search_area);

node *ASMRAGetNodeWithLowestDistance (struct asmra_cluster_s *cluster,
                                      struct asmra_list_s *list);

int ASMRAGetMinDistanceToFather (node *dfn, struct asmra_list_s *list);

node *ASMRABuildNewAssignmentChain (struct asmra_list_s *list_of_dfn, node *arg_node);

struct asmra_cluster_s *ASMRAMakeCluster (node *arg_node);

struct asmra_cluster_s *ASMRAFreeCluster (struct asmra_cluster_s *cluster);

struct asmra_cluster_s *ASMRAClusterAdd (struct asmra_cluster_s *cluster, node *dfn);

struct asmra_cluster_s *ASMRAClusterMerge (struct asmra_cluster_s *cluster_1,
                                           struct asmra_cluster_s *cluster_2);

struct asmra_cluster_s *ASMRAClusterRefUpdate (struct asmra_cluster_s *cluster);

void ASMRAPrintCluster (struct asmra_cluster_s *cluster);

struct asmra_list_s *ASMRAMakeList (void *element);

struct asmra_list_s *ASMRAFreeList (struct asmra_list_s *list);

struct asmra_list_s *ASMRAListAppend (struct asmra_list_s *list, void *element);

node *ASMRAPrepareDataflowgraph (node *graph);

#endif /* ASSIGNMENTS_REARRANGE_H */
