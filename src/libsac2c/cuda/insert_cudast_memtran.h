

#ifndef _SAC_INSERT_CUDAST_MEMTRAN_H_
#define _SAC_INSERT_CUDAST_MEMTRAN_H_

#include "types.h"

extern node *ICSMEMdoInsertCudastMemtran (node *arg_node);
extern node *ICSMEMfundef (node *arg_node, info *arg_info);
extern node *ICSMEMap (node *arg_node, info *arg_info);
extern node *ICSMEMid (node *arg_node, info *arg_info);
extern node *ICSMEMassign (node *arg_node, info *arg_info);
extern node *ICSMEMids (node *arg_node, info *arg_info);
extern node *ICSMEMcudast (node *arg_node, info *arg_info);
extern node *ICSMEMlet (node *arg_node, info *arg_info);
extern node *ICSMEMfuncond (node *arg_node, info *arg_info);

#endif
