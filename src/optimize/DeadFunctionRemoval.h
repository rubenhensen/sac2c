/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:15  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/07 17:37:03  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _DeadFunctionRemoval_h

#define _DeadFunctionRemoval_h

extern node *DeadFunctionRemoval (node *arg_node, node *info_node);

extern node *DFRmodul (node *arg_node, node *arg_info);
extern node *DFRfundef (node *arg_node, node *arg_info);
extern node *DFRap (node *arg_node, node *arg_info);

#endif /* _DeadFunctionRemoval_h */
