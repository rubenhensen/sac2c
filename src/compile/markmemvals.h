/*
 *
 * $Log$
 * Revision 1.1  2004/07/21 16:52:46  ktr
 * Initial revision
 *
 *
 */

#ifndef _sac_markmemvals_h
#define _sac_markmemvals_h

extern node *MarkMemVals (node *syntax_tree);

extern node *MMVfundef (node *arg_node, info *arg_info);
extern node *MMVid (node *arg_node, info *arg_info);
extern node *MMVlet (node *arg_node, info *arg_info);
extern node *MMVprf (node *arg_node, info *arg_info);
extern node *MMVwith (node *arg_node, info *arg_info);
extern node *MMVwith2 (node *arg_node, info *arg_info);
extern node *MMVwithop (node *arg_node, info *arg_info);

#endif /* _sac_precompile_h */
