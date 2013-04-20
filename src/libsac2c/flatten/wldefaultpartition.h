#ifndef _SAC_WLDEFAULTPARTITION_H_
#define _SAC_WLDEFAULTPARTITION_H_

#include "types.h"

/******************************************************************************
 *
 * With-Loop default partition traversal
 *
 * Prefix: WLDP
 *
 *****************************************************************************/
extern node *WLDPdoWlDefaultPartition (node *arg_node);

extern node *WLDPmodule (node *arg_node, info *arg_info);
extern node *WLDPfundef (node *arg_node, info *arg_info);

extern node *WLDPassign (node *arg_node, info *arg_info);
extern node *WLDPwith (node *arg_node, info *arg_info);
extern node *WLDPpart (node *arg_node, info *arg_info);
extern node *WLDPgenarray (node *arg_node, info *arg_info);
extern node *WLDPmodarray (node *arg_node, info *arg_info);
extern node *WLDPpropagate (node *arg_node, info *arg_info);

/* function which is also used by WLPartitionGeneration.c */
extern node *CreateZeros (ntype *array_type, node *fundef);

#endif /* _SAC_WLDEFAULTPARTITION_H_ */
