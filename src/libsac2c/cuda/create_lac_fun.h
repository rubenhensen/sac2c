/*****************************************************************************
 *
 * file:   create_lac_fun.h
 *
 * description:
 *
 *****************************************************************************/

#ifndef _CREATE_LAC_FUN_H_
#define _CREATE_LAC_FUN_H_

#include "types.h"

extern node *CLACFdoCreateLacFun (bool condfun, node *fundef, node *assigns,
                                  node *predicate, node *iterator, node *loop_bound,
                                  node *in_mem, node *out_mem, node **lacfun_p);
extern node *CLACFassign (node *arg_node, info *arg_info);
extern node *CLACFids (node *arg_node, info *arg_info);
extern node *CLACFid (node *arg_node, info *arg_info);

#endif /* _CREATE_LAC_FUN_H_ */
