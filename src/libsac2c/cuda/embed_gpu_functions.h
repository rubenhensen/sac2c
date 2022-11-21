#ifndef _SAC_EMBED_GPU_FUNCTIONS_H_
#define _SAC_EMBED_GPU_FUNCTIONS_H_

#include "types.h"

extern node * EGFdoEmbedGpuFunctions (node *arg_node);

extern node * EGFmodule (node *arg_node, info *arg_info);
extern node * EGFfundef (node *arg_node, info *arg_info);
extern node * EGFarg (node *arg_node, info *arg_info);
extern node * EGFret (node *arg_node, info *arg_info);

#endif
