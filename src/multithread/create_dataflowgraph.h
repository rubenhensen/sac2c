/*
 * $Log$
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

/* access macros for arg_info
 *
 *   node*      DATAFLOWGRAPH  (the dataflowgraph of the currunt function
 *                              as far as it has been constructed)
 *   int        EXECUTIONMODE  (the current execution mode)
 */
#define INFO_CDFG_DATAFLOWGRAPH(n) (n->node[0])
#define INFO_CDFG_OUTERASSIGN(n) (n->node[1])
#define INFO_CDFG_ACTNODE(n) (n->node[2])
#define INFO_CDFG_EXECUTIONMODE(n) (n->flag)
#define INFO_CDFG_WITHDEEP(n) (n->refcnt)

#define CDFG_DEBUG 1

extern node *CreateDataflowgraph (node *arg_node, node *arg_info);

extern node *CDFGfundef (node *arg_node, node *arg_info);

extern node *CDFGassign (node *arg_node, node *arg_info);

extern node *CDFGlet (node *arg_node, node *arg_info);

extern node *CDFGid (node *arg_node, node *arg_info);

extern node *CDFGwith2 (node *arg_node, node *arg_info);

/* Some functions to create, administrate and delete dataflowgraphs */

node *InitiateDataflowgraph (char *name, node *arg_info);

int PrintDataflowgraph (node *dataflowgraph, char *name);

int PrintDataflownode (node *datanode);

node *GetReturnNode (node *graph);

node *UpdateReturn (node *graph, node *arg_node);

node *MakeDataflowNode (char *name, node *inner_assign, node *arg_info);

node *AddDataflowNode (node *graph, node *newnode);

node *UpdateDependencies (node *graph, node *avisnode, node *actnode);

#endif /* CREATE_DATAFLOWGRAPH_H */
