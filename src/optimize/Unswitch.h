/*
 *
 * $Log$
 * Revision 1.1  1995/07/07 13:40:15  asi
 * Initial revision
 *
 */

#ifndef _Unswitch_h

#define _Unswitch_h

extern node *Unswitsh (node *arg_node, node *arg_info);

extern node *UNSfundef (node *arg_node, node *arg_info);
extern node *UNSdo (node *arg_node, node *arg_info);
extern node *UNSwhile (node *arg_node, node *arg_info);
extern node *UNSwith (node *arg_node, node *arg_info);
extern node *UNScond (node *arg_node, node *arg_info);
extern node *UNSlet (node *arg_node, node *arg_info);
extern node *UNSassign (node *arg_node, node *arg_info);

#endif /* _Unswitch_h */
