/*****************************************************************************
 *
 * file:   ungroup_local_funs.h
 *
 * prefix: UGLF
 *
 *****************************************************************************/

#ifndef _SAC_UNGROUP_LOCAL_FUNS_H_
#define _SAC_UNGROUP_LOCAL_FUNS_H_

#include "types.h"

node *UGLFdoUngroupLocalFuns (node *syntax_tree);

node *UGLFmodule (node *arg_node, info *arg_info);
node *UGLFfundef (node *arg_node, info *arg_info);

#endif /* _SAC_UNGROUP_LOCAL_FUNS_H_ */
