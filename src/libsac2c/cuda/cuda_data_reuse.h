

#ifndef _CUDA_DATA_REUSE_H_
#define _CUDA_DATA_REUSE_H_

#include "types.h"

extern node *CUDRdoCudaDataReuse (node *syntax_tree);
extern node *CUDRmodule (node *arg_node, info *arg_info);
extern node *CUDRfundef (node *arg_node, info *arg_info);
extern node *CUDRap (node *arg_node, info *arg_info);
extern node *CUDRassign (node *arg_node, info *arg_info);
extern node *CUDRwith (node *arg_node, info *arg_info);
extern node *CUDRpart (node *arg_node, info *arg_info);
extern node *CUDRcode (node *arg_node, info *arg_info);
extern node *CUDRwith3 (node *arg_node, info *arg_info);
extern node *CUDRrange (node *arg_node, info *arg_info);
extern node *CUDRprf (node *arg_node, info *arg_info);

#endif
