#ifndef _SAC_CUDA_MEMRT_H_
#define _SAC_CUDA_MEMRT_H_

#include "types.h"

extern node *MEMRTdoMinimizeEMRTransfers (node *syntax_tree);

extern node *MEMRTfundef (node *arg_node, info *arg_info);
extern node *MEMRTarg (node *arg_node, info *arg_info);
extern node *MEMRTassign (node *arg_node, info *arg_info);
extern node *MEMRTlet (node *arg_node, info *arg_info);
extern node *MEMRTid (node *arg_node, info *arg_info);
extern node *MEMRTap (node *arg_node, info *arg_info);
extern node *MEMRTprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUDA_MEMRT_H_ */
