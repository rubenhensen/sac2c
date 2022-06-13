

#ifndef _SAC_ANNOTATE_CUDA_WITHLOOP_H_
#define _SAC_ANNOTATE_CUDA_WITHLOOP_H_

#include "types.h"

extern node *ACUWLdoAnnotateCUDAWL (node *arg_node);
extern node *ACUWLfundef (node *arg_node, info *arg_info);
extern node *ACUWLwith (node *arg_node, info *arg_info);
extern node *ACUWLcode (node *arg_node, info *arg_info);
extern node *ACUWLid (node *arg_node, info *arg_info);
extern node *ACUWLap (node *arg_node, info *arg_info);
extern node *ACUWLlet (node *arg_node, info *arg_info);
extern node *ACUWLfold (node *arg_node, info *arg_info);
extern node *ACUWLbreak (node *arg_node, info *arg_info);
extern node *ACUWLpropagate (node *arg_node, info *arg_info);
extern node *ACUWLgenarray (node *arg_node, info *arg_info);
extern node *ACUWLmodarray (node *arg_node, info *arg_info);

#endif
