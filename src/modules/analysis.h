/*
 *
 * $Log$
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
