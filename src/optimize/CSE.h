/*
 *
 * $Log$
 * Revision 1.1  1996/01/17 15:54:09  asi
 * Initial revision
 *
 *
 */

#ifndef _CSE_h

#define _CSE_h

extern node *CSE (node *arg_node, node *arg_info);

extern node *CSEfundef (node *arg_node, node *arg_info);
extern node *CSEwhile (node *arg_node, node *arg_info);
extern node *CSEdo (node *arg_node, node *arg_info);
extern node *CSEcond (node *arg_node, node *arg_info);
extern node *CSEwith (node *arg_node, node *arg_info);

#endif /* _CSE_h */
