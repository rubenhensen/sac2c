/*
 *
 * $Log$
 * Revision 1.1  2004/11/02 14:26:57  ktr
 * Initial revision
 *
 */
#ifndef _sac_loopreuseopt_h
#define _sac_loopreuseopt_h

extern node *EMLRLoopReuseInference (node *syntax_tree);

/*****************************************************************************
 *
 * Loop Reuse Traversal (emlr_tab)
 *
 * Nodes which must not be traversed:
 *  - N_objdef
 *
 ****************************************************************************/
extern node *EMLRap (node *arg_node, info *arg_info);
extern node *EMLRassign (node *arg_node, info *arg_info);
extern node *EMLRfundef (node *arg_node, info *arg_info);

/****************************************************************************
 *
 * Loop Reuse Optimization Traversal (emlro_tab)
 *
 * Nodes which must not be traversed:
 *  - N_objdef
 *
 ***************************************************************************/
extern node *EMLROap (node *arg_node, info *arg_info);
extern node *EMLROarg (node *arg_node, info *arg_info);
extern node *EMLROfundef (node *arg_node, info *arg_info);
extern node *EMLROid (node *arg_node, info *arg_info);
extern node *EMLROprf (node *arg_node, info *arg_info);

#endif
