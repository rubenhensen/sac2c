/*
 *
 * $Log$
 * Revision 1.1  1995/11/16 19:47:38  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_rmvoidfun_h
#define _sac_rmvoidfun_h

extern node *RemoveVoidFunctions (node *syntax_tree);

extern node *RMVblock (node *arg_node, node *arg_info);
extern node *RMVassign (node *arg_node, node *arg_info);
extern node *RMVfundef (node *arg_node, node *arg_info);
extern node *RMVmodul (node *arg_node, node *arg_info);

#endif /* _sac_rmvoidfun_h */
