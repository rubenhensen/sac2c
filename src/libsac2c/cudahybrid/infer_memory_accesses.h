
#ifndef _SAC_INFER_MEM_ACCESSES_H_
#define _SAC_INFER_MEM_ACCESSES_H_

#include "types.h"

typedef struct {
    int min;
    int max;
    bool own;
} offset_t;

extern node *IMAdoInferMemoryAccesses (node *arg_node);
extern node *IMAfundef (node *arg_node, info *arg_info);
extern node *IMAlet (node *arg_node, info *arg_info);
extern node *IMAwith (node *arg_node, info *arg_info);
extern node *IMAwith2 (node *arg_node, info *arg_info);
extern node *IMAgenarray (node *arg_node, info *arg_info);
extern node *IMAmodarray (node *arg_node, info *arg_info);
extern node *IMAprf (node *arg_node, info *arg_info);

#endif
