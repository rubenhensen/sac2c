/*
 * $Log$
 * Revision 1.4  2004/08/09 03:47:34  skt
 * some very painful bugfixing
 * added support for dataflowgraphs within with-loops
 * (I hope someone'll use it in future)
 *
 * Revision 1.3  2004/08/06 17:20:24  skt
 * some adaptions for creating the dataflowgraph
 *
 * Revision 1.2  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.1  2004/07/29 08:38:53  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   create_dataflowgraph.h
 *
 * description:
 *   header file for create_dataflowgraph.c
 *
 *****************************************************************************/

#ifndef CREATE_DATAFLOWGRAPH_H

#define CREATE_DATAFLOWGRAPH_H

#define CDFG_DEBUG 0

extern node *CreateDataflowgraph (node *arg_node);

extern node *CDFGblock (node *arg_node, info *arg_info);

extern node *CDFGassign (node *arg_node, info *arg_info);

extern node *CDFGcond (node *arg_node, info *arg_info);

extern node *CDFGid (node *arg_node, info *arg_info);

/*extern node *CDFGwith2(node *arg_node, info *arg_info);*/

/* Some functions to create, administrate and delete dataflowgraphs */

void PrintDataflowgraph (node *dataflowgraph);

void PrintDataflownode (node *datanode);

node *CDFGUpdateDependencies (node *dfn_assign, node *current_graph, node *outer_graph,
                              node *current_node, node *outer_node);

node *CDFGFindAssignCorrespondingNode (node *graph, node *dfn_assign);

node *CDFGLowestCommonLevel (node *node_one, node *node_two);

void CDFGUpdateDataflowgraph (node *graph, node *node_one, node *two);

bool CDFGFirstIsWithinSecond (node *node_one, node *node_two);

char *CDFGSetName (node *assign);

#endif /* CREATE_DATAFLOWGRAPH_H */
