/*
 *
 * $Log$
 * Revision 1.3  1995/03/14 18:45:21  hw
 * renamed RCwhile to RCloop
 * this version handles do- and while-loops correct.
 * conditionals are not implemented
 *
 * Revision 1.2  1995/03/13  15:18:47  hw
 * RCfundef and Refcount inserted
 *
 * Revision 1.1  1995/03/09  16:17:01  hw
 * Initial revision
 *
 *
 */
#ifndef _refcount_h

#define _refcount_h

extern node *RCassign (node *arg_node, node *arg_info);
extern node *RCloop (node *arg_node, node *arg_info);
extern node *RCid (node *arg_node, node *arg_info);
extern node *RClet (node *arg_node, node *arg_info);
extern node *RCcond (node *arg_node, node *arg_info);
extern node *RCfundef (node *arg_node, node *arg_info);
extern node *Refcount (node *arg_node);

#endif /* _refcount_h */
