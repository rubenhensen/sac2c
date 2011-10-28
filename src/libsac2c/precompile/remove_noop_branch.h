/*
 * $Id$
 */
#ifndef _SAC_RNB_H_
#define _SAC_RNB_H_

#include "types.h"

/******************************************************************************
 *
 * Remove noop conditional branch
 *
 * Prefix: RNB
 *
 *****************************************************************************/
extern node *RNBdoRemoveNoopBranch (node *hotpart);

extern node *RNBassign (node *arg_node, info *arg_info);
extern node *RNBlet (node *arg_node, info *arg_info);
extern node *RNBwith (node *arg_node, info *arg_info);
extern node *RNBwith2 (node *arg_node, info *arg_info);
extern node *RNBcode (node *arg_node, info *arg_info);
extern node *RNBcond (node *arg_node, info *arg_info);

#endif /* _SAC_RNB_H_ */
