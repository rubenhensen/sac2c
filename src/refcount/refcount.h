/*
 *
 * $Log$
 * Revision 1.8  1995/12/07 17:59:56  cg
 * added declaration of function IsNonUniqueHidden
 *
 * Revision 1.7  1995/04/28  17:24:50  hw
 * added RCgen
 *
 * Revision 1.6  1995/04/11  15:10:55  hw
 * changed args of functio IsArray
 *
 * Revision 1.5  1995/03/29  12:03:45  hw
 * declaration of IsArray inserted
 *
 * Revision 1.4  1995/03/16  17:39:13  hw
 * RCwith and RCcon (used for N_genarray and N_modarray) inserted
 *
 * Revision 1.3  1995/03/14  18:45:21  hw
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
extern node *RCwith (node *arg_node, node *arg_info);
extern node *RCcon (node *arg_node, node *arg_info);
extern node *Refcount (node *arg_node);
extern node *RCgen (node *arg_node, node *arg_info);

extern int IsArray (types *type);
extern int IsNonUniqueHidden (types *type);

#endif /* _refcount_h */
