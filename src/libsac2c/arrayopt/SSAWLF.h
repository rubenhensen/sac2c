#ifndef _SAC_WLF_H_
#define _SAC_WLF_H_

#include "types.h"

extern node *WLFdoWLF (node *arg_node);

extern node *WLFmodule (node *arg_node, info *arg_info);
extern node *WLFfundef (node *arg_node, info *arg_info);
extern node *WLFassign (node *arg_node, info *arg_info);
extern node *WLFid (node *arg_node, info *arg_info);
extern node *WLFwith (node *arg_node, info *arg_info);
extern node *WLFlet (node *arg_node, info *arg_info);
extern node *WLFcode (node *arg_node, info *arg_info);

#endif /* _SAC_SSAWLF_H_ */
