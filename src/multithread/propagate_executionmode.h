/*
 * $Log$
 * Revision 1.4  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 1.3  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
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

#define PEM_DEBUG 0

extern node *PropagateExecutionmode (node *arg_node);

extern node *PEMfundef (node *arg_node, info *arg_info);

extern node *PEMassign (node *arg_node, info *arg_info);

extern node *PEMap (node *arg_node, info *arg_info);

extern node *PEMcond (node *arg_node, info *arg_info);

extern node *PEMwith2 (node *arg_node, info *arg_info);

void UpdateExecmodes (node *assign, info *arg_info);

void UpdateFundefExecmode (node *fundef, mtexecmode_t execmode);

void UpdateCondExecmode (node *condassign, mtexecmode_t execmode);

void UpdateWithExecmode (node *withloop_assign, mtexecmode_t execmode);

#if PEM_DEBUG
char *DecodeExecmode (execmode_t execmode);
#endif

#endif /* PROPAGATE_EXECUTIONMODE_H */
