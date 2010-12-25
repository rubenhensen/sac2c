/*****************************************************************************
 *
 * file:   create_cond_fun.h
 *
 * description:
 *
 *****************************************************************************/

#ifndef _CREATE_COND_FUN_H_
#define _CREATE_COND_FUN_H_

#include "types.h"

extern node *CCFdoCreateCondFun (bool condfun, node *fundef, node *assigns,
                                 node *predicate, node *iterator, node *loop_bound,
                                 node *in_mem, node *out_mem, node **lacfun_p);
extern node *CCFassign (node *arg_node, info *arg_info);
extern node *CCFids (node *arg_node, info *arg_info);
extern node *CCFid (node *arg_node, info *arg_info);

#endif /* _CREATE_COND_FUN_H_ */
