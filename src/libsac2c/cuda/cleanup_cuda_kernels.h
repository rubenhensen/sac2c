

#ifndef _SAC_CLEANUP_CUDA_KERNELS_H_
#define _SAC_CLEANUP_CUDA_KERNELS_H_

#include "types.h"

extern node *CLKNLdoCleanupCUDAKernels (node *arg_node);
extern node *CLKNLfundef (node *arg_node, info *arg_info);
extern node *CLKNLassign (node *arg_node, info *arg_info);
extern node *CLKNLlet (node *arg_node, info *arg_info);
extern node *CLKNLid (node *arg_node, info *arg_info);
extern node *CLKNLprf (node *arg_node, info *arg_info);
extern node *CLKNLrange (node *arg_node, info *arg_info);

#endif
