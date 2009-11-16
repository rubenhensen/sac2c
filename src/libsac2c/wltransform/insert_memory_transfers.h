

#ifndef _SAC_INSERT_MEMORY_TRANSFERS_H_
#define _SAC_INSERT_MEMORY_TRANSFERS_H_

#include "types.h"

extern node *IMEMdoInsertMemoryTransfers (node *arg_node);
extern node *IMEMfundef (node *arg_node, info *arg_info);
extern node *IMEMap (node *arg_node, info *arg_info);
extern node *IMEMid (node *arg_node, info *arg_info);
extern node *IMEMlet (node *arg_node, info *arg_info);
extern node *IMEMassign (node *arg_node, info *arg_info);
extern node *IMEMwith (node *arg_node, info *arg_info);
extern node *IMEMids (node *arg_node, info *arg_info);
extern node *IMEMgenarray (node *arg_node, info *arg_info);
extern node *IMEMmodarray (node *arg_node, info *arg_info);
extern node *IMEMcode (node *arg_node, info *arg_info);

#endif
