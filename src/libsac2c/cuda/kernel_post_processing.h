

#ifndef _SAC_KERNEL_POST_PROCESSING_H_
#define _SAC_KERNEL_POST_PROCESSING_H_

#include "types.h"

extern node *KPPdoKernelPostProcessing (node *arg_node);
extern node *KPPfundef (node *arg_node, info *arg_info);
extern node *KPPassign (node *arg_node, info *arg_info);
extern node *KPPlet (node *arg_node, info *arg_info);
extern node *KPPid (node *arg_node, info *arg_info);
extern node *KPPprf (node *arg_node, info *arg_info);
extern node *KPPwith3 (node *arg_node, info *arg_info);
extern node *KPPrange (node *arg_node, info *arg_info);

#endif
