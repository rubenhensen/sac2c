

#ifndef _SAC_CREATE_DIST_WL_COND_H_
#define _SAC_CREATE_DIST_WL_COND_H_

#include "types.h"

extern node *DISTCONDdoCreateDistWlCond (node *arg_node);
extern node *DISTCONDfundef (node *arg_node, info *arg_info);
extern node *DISTCONDassign (node *arg_node, info *arg_info);
extern node *DISTCONDlet (node *arg_node, info *arg_info);
extern node *DISTCONDwiths (node *arg_node, info *arg_info);
extern node *DISTCONDwith (node *arg_node, info *arg_info);
extern node *DISTCONDwith2 (node *arg_node, info *arg_info);

#endif
