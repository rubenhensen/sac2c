/*
 *
 * $Log$
 * Revision 1.4  2004/11/07 14:25:24  ktr
 * ongoing implementation.
 *
 * Revision 1.3  2004/11/02 14:33:25  ktr
 * Reuseelimination is now performed seperately for each branch of a cond.
 *
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/22 14:13:56  ktr
 * Initial revision
 *
 */
#ifndef _reuseelimination_h
#define _reuseelimination_h

/******************************************************************************
 *
 * Reuse elimination traversal
 *
 * Prefix: EMRE
 *
 *****************************************************************************/
extern node *EMREReuseElimination (node *syntax_tree);

extern node *EMREassign (node *arg_node, info *arg_info);
extern node *EMREcond (node *arg_node, info *arg_info);
extern node *EMREfundef (node *arg_node, info *arg_info);
extern node *EMRElet (node *arg_node, info *arg_info);
extern node *EMREprf (node *arg_node, info *arg_info);
extern node *EMREvardec (node *arg_node, info *arg_info);
extern node *EMREwithop (node *arg_node, info *arg_info);

#endif
