#ifndef _SAC_EXPLICITACCUMULATE_H_
#define _SAC_EXPLICITACCUMULATE_H_

#include "types.h"

/******************************************************************************
 *
 * Explicit accumulate traversal ( ea_tab)
 *
 * Prefix: EA
 *
 *****************************************************************************/
extern node *EAdoExplicitAccumulate (node *arg_node);

extern node *EAmodule (node *arg_node, info *arg_info);
extern node *EAfundef (node *arg_node, info *arg_info);
extern node *EAassign (node *arg_node, info *arg_info);
extern node *EAlet (node *arg_node, info *arg_info);

extern node *EAwith (node *arg_node, info *arg_info);
extern node *EApropagate (node *arg_node, info *arg_info);
extern node *EAfold (node *arg_node, info *arg_info);
extern node *EAcode (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITACCUMULATE_H_ */
