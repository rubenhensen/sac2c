/*
 *
 * $Log$
 * Revision 1.3  1995/06/14 13:31:23  asi
 * added Unroll, UNRdo, UNRwhile and UNRassign
 *
 * Revision 1.2  1995/06/02  11:32:51  asi
 * Added Unroll, and UNRfundef.
 *
 * Revision 1.1  1995/05/26  14:22:26  asi
 * Initial revision
 *
 *
 */

#ifndef _Unroll_h

#define _Unroll_h

extern node *Unroll (node *arg_node, node *arg_info);

extern node *UNRdo (node *arg_node, node *arg_info);
extern node *UNRwhile (node *arg_node, node *arg_info);
extern node *UNRassign (node *arg_node, node *arg_info);

#endif /* _Unroll_h */
