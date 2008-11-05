/*****************************************************************************
 *
 * $Id$
 *
 * file:   group_local_funs.h
 *
 * prefix: GLF
 *
 *****************************************************************************/

#ifndef _SAC_GROUP_LOCAL_FUNS_H_
#define _SAC_GROUP_LOCAL_FUNS_H_

#include "types.h"

node *GLFdoGroupLocalFuns (node *syntax_tree);

node *GLFmodule (node *arg_node, info *arg_info);
node *GLFfundef (node *arg_node, info *arg_info);
node *GLFap (node *arg_node, info *arg_info);

bool GLFisLocalFun (node *fundef);

#endif /* _SAC_GROUP_LOCAL_FUNS_H_ */
