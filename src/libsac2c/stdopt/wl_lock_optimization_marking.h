#ifndef _WL_LOCK_OPTIMIZATION_MARKING_H_
#define _WL_LOCK_OPTIMIZATION_MARKING_H_

#include "types.h"

extern node *WLLOMprf (node *arg_node, info *arg_info);
extern node *WLLOMfundef (node *arg_node, info *arg_info);
extern node *WLLOMid (node *arg_node, info *arg_info);
extern node *WLLOMids (node *arg_node, info *arg_info);
extern node *WLLOMassign (node *arg_node, info *arg_info);
extern node *WLLOMlet (node *arg_node, info *arg_info);
extern node *WLLOMwith (node *arg_node, info *arg_info);
extern node *WLLOMdoLockOptimizationMarking (node *syntax_tree);

#endif /*_WL_LOCK_OPTIMIZATION_MARKING_H_*/
