/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:24  sacbase
 * new release made
 *
 * Revision 1.3  1995/06/02 12:11:50  asi
 * Added NodeBehindCast
 *
 * Revision 1.2  1995/05/15  08:49:25  asi
 * LIRfundef, LIRMblock, LIRassign, LIRMassign, LIRloop, LIRMloop, LIRcond added.
 * First version of loop invariant removal implemented.
 *
 * Revision 1.1  1995/04/05  15:16:10  asi
 * Initial revision
 *
 *
 */

#ifndef _LoopInvariantRemoval_h

#define _LoopInvariantRemoval_h

extern node *LoopInvariantRemoval (node *arg_node, node *arg_info);

extern node *LIRfundef (node *arg_node, node *arg_info);
extern node *LIRloop (node *arg_node, node *arg_info);
extern node *LIRcond (node *arg_node, node *arg_info);
extern node *LIRassign (node *arg_node, node *arg_info);

extern node *LIRMassign (node *arg_node, node *arg_info);
extern node *LIRMblock (node *arg_node, node *arg_info);
extern node *LIRMloop (node *arg_node, node *arg_info);

extern node *NodeBehindCast (node *arg_node);

#endif /* _LoopInvariantRemoval_h */
