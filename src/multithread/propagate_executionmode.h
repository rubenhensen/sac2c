/*
 * $Log$
 * Revision 1.2  2004/07/23 10:05:46  skt
 * complete redesign
 *
 * Revision 1.1  2004/07/06 12:31:20  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   propagate_executionmode.h
 *
 * description:
 *   header file for propagate_executionmode.c
 *
 *****************************************************************************/

#ifndef PROPAGATE_EXECUTIONMODE_H

#define PROPAGATE_EXECUTIONMODE_H

/* access macros for arg_info
 *   int        ANYCHANGE     (is 1, if there was a change of the executionmode
 *                             somewhere in the current traversal)
 *   int        FIRSTTRAV     (holds the information, wheter this is the first
 *                             traversal (=>1) or not (=>0)
 *   int        FUNEXECMODE   (holds the current executionmode of the function)
 *   node*      ASSIGN        (the current assignment, the traversal.mechanism
 *                             is in)
 */
#define INFO_PEM_ANYCHANGE(n) (n->flag)
#define INFO_PEM_FIRSTTRAV(n) (n->counter)
#define INFO_PEM_ACTFUNDEF(n) (n->node[0])
#define INFO_PEM_MYASSIGN(n) (n->node[1])
#define INFO_PEM_LASTCONDASSIGN(n) (n->node[2])
#define INFO_PEM_LASTWITHASSIGN(n) (n->node[3])

#define PEM_DEBUG 0

extern node *PropagateExecutionmode (node *arg_node, node *arg_info);

extern node *PEMfundef (node *arg_node, node *arg_info);

extern node *PEMassign (node *arg_node, node *arg_info);

extern node *PEMap (node *arg_node, node *arg_info);

extern node *PEMcond (node *arg_node, node *arg_info);

extern node *PEMwith2 (node *arg_node, node *arg_info);

void UpdateExecmodes (node *assign, node *arg_info);

void UpdateFundefExecmode (node *fundef, int execmode);

void UpdateCondExecmode (node *condassign, int execmode);

void UpdateWithExecmode (node *withloop_assign, int execmode);

#if PEM_DEBUG
char *DecodeExecmode (int execmode);
#endif

#endif /* PROPAGATE_EXECUTIONMODE_H */
