

#ifndef _SAC_INSERT_MEMTRAN_DIST_H_
#define _SAC_INSERT_MEMTRAN_DIST_H_

#include "types.h"

extern node *IMEMDISTdoInsertMemtranDist (node *arg_node);
extern node *IMEMDISTfundef (node *arg_node, info *arg_info);
extern node *IMEMDISTap (node *arg_node, info *arg_info);
extern node *IMEMDISTid (node *arg_node, info *arg_info);
extern node *IMEMDISTlet (node *arg_node, info *arg_info);
extern node *IMEMDISTassign (node *arg_node, info *arg_info);
extern node *IMEMDISTwith (node *arg_node, info *arg_info);
extern node *IMEMDISTids (node *arg_node, info *arg_info);
extern node *IMEMDISTgenarray (node *arg_node, info *arg_info);
extern node *IMEMDISTmodarray (node *arg_node, info *arg_info);
extern node *IMEMDISTcode (node *arg_node, info *arg_info);
extern node *IMEMDISTfuncond (node *arg_node, info *arg_info);
extern node *IMEMDISTreturn (node *arg_node, info *arg_info);

#endif
