/*
 *
 * $Log$
 * Revision 1.1  1995/02/13 16:38:56  asi
 * Initial revision
 *
 *
 */

#ifndef _DeadCodeRemoval_h

#define _DeadCodeRemoval_h

extern node *DeadCodeRemoval (node *arg_node, node *arg_info);

extern node *DEADfundef (node *arg_node, node *arg_info);
extern node *DEADassign (node *arg_node, node *arg_info);
extern node *DEADblock (node *arg_node, node *arg_info);
extern node *DEADvardec (node *arg_node, node *arg_info);
extern node *DEADcond (node *arg_node, node *arg_info);
extern node *DEADloop (node *arg_node, node *arg_info);

#endif /* _DeadCodeRemoval_h */
