/*
 * $Log$
 * Revision 1.1  2004/07/06 12:31:20  skt
 * Initial revision
 *
 */

/* to do in node_info.mac
 * add pem_tab
 *     - PEMfundef
 *     - PEMassign
 *     - PEMap
 * TravNone for N_nwith2, N_array und N_prf
 * TravNone for N_return for PEM & TEM
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
#define INFO_PEM_FUNEXECMODE(n) (n->refcnt)
#define INFO_PEM_ASSIGN(n) (n->node[0])
#define PEM_DEBUG 0

extern node *PropagateExecutionmode (node *arg_node, node *arg_info);

extern node *PEMfundef (node *arg_node, node *arg_info);

extern node *PEMassign (node *arg_node, node *arg_info);

extern node *PEMap (node *arg_node, node *arg_info);

int UpdateFunexecmode (int fun_execmode, int assign_execmode);

#if PEM_DEBUG
char *DecodeExecmode (int execmode);
#endif

#endif /* PROPAGATE_EXECUTIONMODE_H */
