

#ifndef _SAC_ANNOTATE_CUDA_LOOP_H_
#define _SAC_ANNOTATE_CUDA_LOOP_H_

#include "types.h"

extern node *ACULdoAnnotateCUDALoop (node *arg_node);
extern node *ACULfundef (node *arg_node, info *arg_info);
extern node *ACULap (node *arg_node, info *arg_info);
extern node *ACULgenarray (node *arg_node, info *arg_info);

#endif
