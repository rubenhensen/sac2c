/*
 *
 * $Log$
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
extern node *EMSRReuseElimination (node *syntax_tree);

extern node *EMREassign (node *arg_node, info *arg_info);
extern node *EMREblock (node *arg_node, info *arg_info);
extern node *EMRElet (node *arg_node, info *arg_info);
extern node *EMREprf (node *arg_node, info *arg_info);
extern node *EMREwithop (node *arg_node, info *arg_info);

#endif
