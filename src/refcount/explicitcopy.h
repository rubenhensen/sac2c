/*
 *
 * $Log$
 * Revision 1.1  2004/11/09 22:15:14  ktr
 * Initial revision
 *
 */
#ifndef _sac_explicitcopy_h
#define _sac_explicitcopy_h

extern node *EMECExplicitCopy (node *syntax_tree);

extern node *EMECassign (node *arg_node, info *arg_info);
extern node *EMECfundef (node *arg_node, info *arg_info);
extern node *EMECprf (node *arg_node, info *arg_info);

#endif
