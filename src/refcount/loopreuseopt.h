/*
 *
 * $Log$
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/11/09 20:54:20  ktr
 * Typo corrected.
 *
 * Revision 1.1  2004/11/02 14:26:57  ktr
 * Initial revision
 *
 */
#ifndef _SAC_LOOPREUSEOPT_H_
#define _SAC_LOOPREUSEOPT_H_

#include "types.h"

extern node *EMLRdoLoopReuseOptimization (node *syntax_tree);

/*****************************************************************************
 *
 * Loop Reuse Traversal (emlr_tab)
 *
 * Nodes which must not be traversed:
 *  - N_objdef
 *
 * Prefix: EMLR
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
 * Prefix: EMLRO
 *
 ***************************************************************************/
extern node *EMLROap (node *arg_node, info *arg_info);
extern node *EMLROarg (node *arg_node, info *arg_info);
extern node *EMLROfundef (node *arg_node, info *arg_info);
extern node *EMLROid (node *arg_node, info *arg_info);
extern node *EMLROprf (node *arg_node, info *arg_info);

#endif /* _SAC_LOOPREUSEOPT_H_ */
