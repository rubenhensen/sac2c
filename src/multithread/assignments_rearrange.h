/*
 * $Log$
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

/* access macros for arg_info
 *
 *   node*      DATAFLOWGRAPH  (the dataflowgraph of the currunt function
 *                              as far as it has been constructed)
 *   int        EXECUTIONMODE  (the current execution mode)
 *   odelist*   USEDNODES      (list of identifiers of the variables used by
 *                              the current assignment)
 */
#define INFO_ASMRA_DATAFLOWGRAPH(n) (n->node[0])
#define INFO_ASMRA_OUTERASSIGN(n) (n->node[1])
#define INFO_ASMRA_ACTNODE(n) (n->node[2])
#define INFO_ASMRA_EXECUTIONMODE(n) (n->flag)
#define INFO_ASMRA_WITHDEEP(n) (n->refcnt)
#define INFO_ASMRA_USEDIDS(n) (n->info.ids)

/* access macros for N_infos to be used as notes of a dataflowgraph
 *
 *   node*       ASSIGNMENT     (the corresponding assignment)
 *   int         EXECUTIONMODE  (some info about the execution of the
 *                               corresponding assignment;
 *                               any-, multi- or singlethreaded)
 *   ids*        DEFINEDVARS    (list of identifiers of the variables defined
 *                               by this node)
 *   nodelist*   DEPENDENT (list of (dataflow) nodes that depends on the
 *                               DEFINEDVARS of this node)
 */

#define DATA_ASMRA_INNERASSIGN(n) (n->node[0])
#define DATA_ASMRA_OUTERASSIGN(n) (n->node[1])
#define DATA_ASMRA_EXECUTIONMODE(n) (n->flag)
#define DATA_ASMRA_DEPENDENT(n) ((nodelist *)(n->dfmask[1]))

/* definitions for storing the information about the current execution mode */
#define ASMRA_ANY 0
#define ASMRA_ST 1
#define ASMRA_MT 2

extern node *AssignmentsRearrange (node *arg_node, node *arg_info);

extern node *ASMRAfundef (node *arg_node, node *arg_info);

node *CreateDataflowgraph (node *args, int mode);

int DeleteDataflowgraph (node *graph);

extern node *ASMRAassign (node *arg_node, node *arg_info);

extern node *ASMRAlet (node *arg_node, node *arg_info);

node *MakeNode (ids *identifier, node *arg_info);

node *MakeReturnnode (node *assign_node, node *arg_info);

node *AddNode (node *graph, node *newnode);

extern node *ASMRAid (node *arg_node, node *arg_info);

node *UpdateDependencies (node *graph, node *avisnode, node *actnode);

/*extern node *ASMRAreturn(node *arg_node, node *arg_info);*/

extern node *ASMRAst (node *arg_node, node *arg_info);

extern node *ASMRAmt (node *arg_node, node *arg_info);

#endif /* ASSIGNMENTS_REARRANGE_H */
