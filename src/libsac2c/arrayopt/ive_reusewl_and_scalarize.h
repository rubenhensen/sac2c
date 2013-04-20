#ifndef _SAC_IVE_REUSEWL_AND_SCALARIZE_H_
#define _SAC_IVE_REUSEWL_AND_SCALARIZE_H_

#include "types.h"

extern node *IVERASlet (node *arg_node, info *arg_info);
extern node *IVERASwith (node *arg_node, info *arg_info);
extern node *IVERASpart (node *arg_node, info *arg_info);
extern node *IVERAScode (node *arg_node, info *arg_info);
extern node *IVERASprf (node *arg_node, info *arg_info);
extern node *IVERASfundef (node *arg_node, info *arg_info);
extern node *IVERASassign (node *arg_node, info *arg_info);

extern node *IVERASdoWithloopReuseAndOptimisation (node *syntax_tree);

#endif /* _SAC_IVE_REUSEWL_AND_SCALARIZE_H_ */
