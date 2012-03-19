/*****************************************************************************
 *
 * file:   create_loop_fun.h
 *
 * description:
 *
 *****************************************************************************/

#ifndef _CREATE_LOOP_FUN_H_
#define _CREATE_LOOP_FUN_H_

#include "types.h"

extern node *CLFdoCreateLoopFun (node *fundef, node *loop_assigns, node *iterator,
                                 node *loop_bound, node *in_mem, node *out_mem,
                                 node **fun_p);
extern node *CLFassign (node *arg_node, info *arg_info);
extern node *CLFids (node *arg_node, info *arg_info);
extern node *CLFid (node *arg_node, info *arg_info);

#endif /* _CREATE_LOOP_FUN_H_ */
