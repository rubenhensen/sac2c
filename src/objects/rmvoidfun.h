/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:26  sacbase
 * new release made
 *
 * Revision 1.2  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
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
extern node *RMVcast (node *arg_node, node *arg_info);

#endif /* _sac_rmvoidfun_h */
