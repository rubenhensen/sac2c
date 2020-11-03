#ifndef _SAC_CUDA_ASYNC_DELAY_EXPAND_H_
#define _SAC_CUDA_ASYNC_DELAY_EXPAND_H_

#include "types.h"

extern node * CUADEdoAsyncDelayExpand (node *syntax_tree);

extern node * CUADEfundef (node *arg_node, info *arg_info);
extern node * CUADEassign (node *arg_node, info *arg_info);
extern node * CUADElet (node *arg_node, info *arg_info);
extern node * CUADEwith (node *arg_node, info *arg_info);
extern node * CUADEcond (node *arg_node, info *arg_info);
extern node * CUADEids (node *arg_node, info *arg_info);
extern node * CUADEid (node *arg_node, info *arg_info);
extern node * CUADEprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUDA_ASYNC_DELAY_EXPAND_H_ */
