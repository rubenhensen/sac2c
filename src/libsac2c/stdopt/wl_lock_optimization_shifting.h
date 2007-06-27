#ifndef _WL_LOCK_OPTIMIZATION_SHIFTING_H_
#define _WL_LOCK_OPTIMIZATION_SHIFTING_H_

#include "types.h"

extern node *WLLOSprf (node *arg_node, info *arg_info);
extern node *WLLOSassign (node *arg_node, info *arg_info);
extern node *WLLOSlet (node *arg_node, info *arg_info);
extern node *WLLOSwith (node *arg_node, info *arg_info);
extern node *WLLOSdoLockOptimizationShifting (node *syntax_tree);

#endif /*_WL_LOCK_OPTIMIZATION_SHIFTING_H_*/
