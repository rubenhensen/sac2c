/*
 *
 * $Log$
 * Revision 1.4  1998/04/03 12:22:11  srs
 * added CSENcode()
 *
 * Revision 1.3  1998/02/25 15:20:20  srs
 * added support for new WL
 *
 * Revision 1.2  1996/01/22 14:33:48  asi
 * basic algorithm added
 *
 * Revision 1.1  1996/01/17  15:54:09  asi
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
extern node *CSEassign (node *arg_node, node *arg_info);
extern node *CSEid (node *arg_node, node *arg_info);
extern node *CSENwith (node *arg_node, node *arg_info);
extern node *CSENcode (node *arg_node, node *arg_info);

#endif /* _CSE_h */
