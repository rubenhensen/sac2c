#ifndef _SAC_CUDA_EMR_LOOP_H_
#define _SAC_CUDA_EMR_LOOP_H_

#include "types.h"

extern node *EMRTUdoEMRUpdateFun (node *syntax_tree);
extern node *EMRTUdoEMRUpdateAp (node *syntax_tree);

extern node *EMRTUfundef (node *arg_node, info *arg_info);
extern node *EMRTUap (node *arg_node, info *arg_info);
extern node *EMRTUmodarray (node *arg_node, info *arg_info);
extern node *EMRTUgenarray (node *arg_node, info *arg_info);
extern node *EMRTUlet (node *arg_node, info *arg_info);
extern node *EMRTUprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUDA_EMR_LOOP_H_ */
