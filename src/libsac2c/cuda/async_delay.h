#ifndef _SAC_CUDA_ASYNC_DELAY_H_
#define _SAC_CUDA_ASYNC_DELAY_H_

#include "types.h"

extern node * CUADdoAsyncDelay (node *syntax_tree);

extern node * CUADfundef (node *arg_node, info *arg_info);
extern node * CUADassign (node *arg_node, info *arg_info);
extern node * CUADlet (node *arg_node, info *arg_info);
extern node * CUADprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUDA_ASYNC_DELAY_H_ */
