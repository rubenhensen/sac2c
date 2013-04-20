#ifndef _SAC_OMP_FIND_PRIVATE_H_
#define _SAC_OMP_FIND_PRIVATE_H_

#include "types.h"

extern node *OFPdoFindPrivate (node *syntaxtree);

extern node *OFPmodule (node *arg_node, info *arg_info);
extern node *OFPfundef (node *arg_node, info *arg_info);
extern node *OFPlet (node *arg_node, info *arg_info);
extern node *OFPwith2 (node *arg_node, info *arg_info);
extern node *OFPwith (node *arg_node, info *arg_info);
extern node *OFPids (node *arg_node, info *arg_info);
extern node *OFPid (node *arg_node, info *arg_info);
extern node *OFPwithid (node *arg_node, info *arg_info);

#endif /* _SAC_OMP_FIND_PRIVATE_H_ */
