/*
 *
 * $Log$
 * Revision 1.2  1995/03/13 15:18:47  hw
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
extern node *RCwhile (node *arg_node, node *arg_info);
extern node *RCid (node *arg_node, node *arg_info);
extern node *RClet (node *arg_node, node *arg_info);
extern node *RCcond (node *arg_node, node *arg_info);
extern node *RCfundef (node *arg_node, node *arg_info);
extern node *Refcount (node *arg_node);

#endif /* _refcount_h */
