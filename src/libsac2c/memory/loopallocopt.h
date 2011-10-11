#ifndef _SAC_LOOPALLOCOPT_H_
#define _SAC_LOOPALLOCOPT_H_

#include "types.h"

extern node *EMLAOdoLoopAllocationOptimization (node *syntax_tree);

/*****************************************************************************
 *
 * Loop Alloc Optimization Traversal (emlao_tab)
 *
 * Prefix: EMLAO
 *
 ****************************************************************************/
extern node *EMLAOap (node *arg_node, info *arg_info);
extern node *EMLAOassign (node *arg_node, info *arg_info);
extern node *EMLAOfundef (node *arg_node, info *arg_info);
extern node *EMLAOarg (node *arg_node, info *arg_info);
extern node *EMLAOmodarray (node *arg_node, info *arg_info);
extern node *EMLAOprf (node *arg_node, info *arg_info);
extern node *EMLAOwith (node *arg_node, info *arg_info);
extern node *EMLAOwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_LOOPALLOCOPT_H_ */
