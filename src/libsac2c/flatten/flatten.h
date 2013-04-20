#ifndef _SAC_FLATTEN_H_
#define _SAC_FLATTEN_H_

#include "types.h"

/******************************************************************************
 *
 * Flatten traversal ( flat_tab)
 *
 * Prefix: FLAT
 *
 *****************************************************************************/

extern node *FLATdoFlatten (node *syntax_tree);

extern node *FLATblock (node *arg_node, info *arg_info);
extern node *FLATassign (node *arg_node, info *arg_info);
extern node *FLATexprs (node *arg_node, info *arg_info);
extern node *FLATcond (node *arg_node, info *arg_info);
extern node *FLATdo (node *arg_node, info *arg_info);
extern node *FLATmodule (node *arg_node, info *arg_info);
extern node *FLATfundef (node *arg_node, info *arg_info);
extern node *FLATspap (node *arg_node, info *arg_info);
extern node *FLATarray (node *arg_node, info *arg_info);
extern node *FLATreturn (node *arg_node, info *arg_info);
extern node *FLATspid (node *arg_node, info *arg_info);
extern node *FLATlet (node *arg_node, info *arg_info);
extern node *FLATcast (node *arg_node, info *arg_info);
extern node *FLATarg (node *arg_node, info *arg_info);
extern node *FLATprf (node *arg_node, info *arg_info);
extern node *FLATwith (node *arg_node, info *arg_info);
extern node *FLATwithid (node *arg_node, info *arg_info);
extern node *FLATpart (node *arg_node, info *arg_info);
extern node *FLATgenerator (node *arg_node, info *arg_info);
extern node *FLATgenarray (node *arg_node, info *arg_info);
extern node *FLATmodarray (node *arg_node, info *arg_info);
extern node *FLATspfold (node *arg_node, info *arg_info);
extern node *FLATpropagate (node *arg_node, info *arg_info);
extern node *FLATcode (node *arg_node, info *arg_info);

#endif /* _SAC_FLATTEN_H_ */
