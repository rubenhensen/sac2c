/*
 *
 * $Log$
 * Revision 1.2  2004/08/10 16:14:33  ktr
 * RIicm added.
 *
 * Revision 1.1  2004/08/10 13:30:12  ktr
 * Initial revision
 *
 */
#ifndef _sac_reuse_h
#define _sac_reuse_h

extern node *ReuseInference (node *arg_node);

extern node *RIarg (node *arg_node, info *arg_info);
extern node *RIassign (node *arg_node, info *arg_info);
extern node *RIcode (node *arg_node, info *arg_info);
extern node *RIcond (node *arg_node, info *arg_info);
extern node *RIfundef (node *arg_node, info *arg_info);
extern node *RIicm (node *arg_node, info *arg_info);
extern node *RIlet (node *arg_node, info *arg_info);
extern node *RIprf (node *arg_node, info *arg_info);
extern node *RIwith2 (node *arg_node, info *arg_info);

#endif
