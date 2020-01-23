#ifndef _SAC_CUDA_MAN_MEMORY_H_
#define _SAC_CUDA_MAN_MEMORY_H_

#include "types.h"

extern node * CUMMdoManagedMemory (node *syntax_tree);

extern node * CUMMavis (node *arg_node, info *arg_info);
extern node * CUMMlet (node *arg_node, info *arg_info);
extern node * CUMMprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUDA_MAN_MEMORY_H_ */
