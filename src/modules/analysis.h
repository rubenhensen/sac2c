/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:41:58  sacbase
 * new release made
 *
 * Revision 1.3  1999/01/07 10:50:49  cg
 * Added function ANAnwithop() and ANAfoldfun() which infer the dependence
 * of a function on its fold operations both for the old and for the new
 * with-loop.
 *
 * Revision 1.2  1995/10/19 14:31:06  cg
 * renamed function analysis to Analysis
 *
 * Revision 1.1  1995/10/19  11:04:05  cg
 * Initial revision
 *
 *
 *
 */

extern node *Analysis (node *syntaxtree);
extern node *ANAmodul (node *arg_node, node *arg_info);
extern node *ANAfundef (node *arg_node, node *arg_info);
extern node *ANAvardec (node *arg_node, node *arg_info);
extern node *ANAid (node *arg_node, node *arg_info);
extern node *ANAap (node *arg_node, node *arg_info);
extern node *ANAnwithop (node *arg_node, node *arg_info);
extern node *ANAfoldfun (node *arg_node, node *arg_info);
