/*
 * $Log$
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

/* access macros for arg_info
 *
 *   node*      DATAFLOWGRAPH  (the dataflowgraph of the currunt function
 *                              as far as it has been constructed)
 *   int        EXECUTIONMODE  (the current execution mode)
 */
/*#define INFO_CDFG_DATAFLOWGRAPH(n)  (n->node[0])
#define INFO_CDFG_OUTERASSIGN(n)    (n->node[1])
#define INFO_CDFG_ACTNODE(n)        (n->node[2])
#define INFO_CDFG_EXECUTIONMODE(n)  (n->flag)
#define INFO_CDFG_WITHDEEP(n)       (n->refcnt)*/

#define CDFG_DEBUG 0

extern node *CreateDataflowgraph (node *arg_node);

extern node *CDFGblock (node *arg_node, info *arg_info);

extern node *CDFGassign (node *arg_node, info *arg_info);

extern node *CDFGcond (node *arg_node, info *arg_info);

extern node *CDFGid (node *arg_node, info *arg_info);

extern node *CDFGwith2 (node *arg_node, info *arg_info);

/* Some functions to create, administrate and delete dataflowgraphs */

node *InitiateDataflowgraph (char *name, info *arg_info);

void PrintDataflowgraph (node *dataflowgraph);

void PrintDataflownode (node *datanode);

node *AddDataflownode (node *graph, node *newnode);

node *UpdateDependencies (node *graph, node *avisnode, node *actnode);

char *SetName (node *assign);

#endif /* CREATE_DATAFLOWGRAPH_H */
