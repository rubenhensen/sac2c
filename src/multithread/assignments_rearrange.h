/*
 * $Log$
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
 *   node*       INNERASSIGNMENT (the corresponding assignment)
 *   node*       OUTERASSIGNMENT (the assignment in the list of assignments,
 *                                that has to be taken for rearranging the
 *                                list. Usually the same as INNERASSIGNMENT,
 *                                but neccessary to handle ST-/MT-Blocks
 *                                see ASMRAassign for details)
 *   int         EXECUTIONMODE   (some info about the execution of the
 *                                corresponding assignment;
 *                                any-, multi- or singlethreaded)
 *   nodelist*   DEPENDENT       (list of (dataflow) nodes that depends on this
 *                                node)
 *   char*       NAME            (name of the dataflownode; NOT substantial
 *                                for the functionality, but helpful to print
 *                                and understand the dataflowgraph)
 */

#define DATA_ASMRA_INNERASSIGN(n) (n->node[0])
#define DATA_ASMRA_OUTERASSIGN(n) (n->node[1])
#define DATA_ASMRA_EXECUTIONMODE(n) (n->flag)
#define DATA_ASMRA_DEPENDENT(n) ((nodelist *)(n->dfmask[0]))
#define DATA_ASMRA_NAME(n) ((char *)(n->dfmask[1]))

/* definitions for storing the information about the current execution mode */
#define ASMRA_ANY 0
#define ASMRA_ST 1
#define ASMRA_MT 2

extern node *AssignmentsRearrange (node *arg_node, node *arg_info);

extern node *ASMRAfundef (node *arg_node, node *arg_info);

node *CreateDataflowgraph (char *name, node *arg_info);

int DeleteDataflowgraph (node *graph);

int PrintDataflowgraph (node *dataflowgraph, char *name);

int PrintDataflownode (node *datanode);

extern node *ASMRAassign (node *arg_node, node *arg_info);

node *GetReturnNode (node *graph);

node *UpdateReturn (node *graph, node *arg_node);

extern node *ASMRAlet (node *arg_node, node *arg_info);

node *MakeNode (char *name, node *inner_assign, node *arg_info);

node *AddNode (node *graph, node *newnode);

extern node *ASMRAid (node *arg_node, node *arg_info);

node *UpdateDependencies (node *graph, node *avisnode, node *actnode);

/*extern node *ASMRAreturn(node *arg_node, node *arg_info);*/

extern node *ASMRAst (node *arg_node, node *arg_info);

extern node *ASMRAmt (node *arg_node, node *arg_info);

extern node *ASMRAwith2 (node *arg_node, node *arg_info);

#endif /* ASSIGNMENTS_REARRANGE_H */
