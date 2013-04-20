#ifndef _SAC_INB_H_
#define _SAC_INB_H_

#include "types.h"

/******************************************************************************
 *
 * Identify noop conditional branch
 *
 * Prefix: INB
 *
 *****************************************************************************/
extern node *INBdoIdentifyNoopBranch (node *hotpart);

extern node *INBassign (node *arg_node, info *arg_info);
extern node *INBap (node *arg_node, info *arg_info);
extern node *INBfundef (node *arg_node, info *arg_info);
extern node *INBpart (node *arg_node, info *arg_info);
extern node *INBcode (node *arg_node, info *arg_info);
extern node *INBcond (node *arg_node, info *arg_info);
extern node *INBfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_INB_H_ */
