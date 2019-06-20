#ifndef _SAC_ANNOTATE_PL_MEMORY_H_
#define _SAC_ANNOTATE_PL_MEMORY_H_

#include "types.h"

extern node * EMAPMdoAnnotatePagelockedMem (node *syntax_tree);

extern node * EMAPMprintPreFun (node *arg_node, info *arg_info);

extern node * EMAPMfundef (node *arg_node, info *arg_info);
extern node * EMAPMassign (node *arg_node, info *arg_info);
extern node * EMAPMlet (node *arg_node, info *arg_info);
extern node * EMAPMfuncond (node *arg_node, info *arg_info);
extern node * EMAPMap (node *arg_node, info *arg_info);
extern node * EMAPMprf (node *arg_node, info *arg_info);
extern node * EMAPMid (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATE_PL_MEMORY_H_ */
