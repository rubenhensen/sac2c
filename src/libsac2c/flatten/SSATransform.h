/******************************************************************************
 *
 * $Id$
 *
 * SSATransform traversal ( ssafrm_tab)
 *
 * Prefix: SSAT
 *
 *****************************************************************************/

#ifndef _SAC_SSATRANSFORM_H_
#define _SAC_SSATRANSFORM_H_

#include "types.h"

extern node *SSATdoTransform (node *ast);
extern node *SSATdoTransformAllowGOs (node *ast);

extern node *SSATap (node *arg_node, info *arg_info);
extern node *SSATassign (node *arg_node, info *arg_info);
extern node *SSATblock (node *arg_node, info *arg_info);
extern node *SSATcode (node *arg_node, info *arg_info);
extern node *SSATcond (node *arg_node, info *arg_info);
extern node *SSATfundef (node *arg_node, info *arg_info);
extern node *SSATlet (node *arg_node, info *arg_info);
extern node *SSATarg (node *arg_node, info *arg_info);
extern node *SSATvardec (node *arg_node, info *arg_info);
extern node *SSATid (node *arg_node, info *arg_info);
extern node *SSATwith (node *arg_node, info *arg_info);
extern node *SSATwith2 (node *arg_node, info *arg_info);
extern node *SSATpart (node *arg_node, info *arg_info);
extern node *SSATwithid (node *arg_node, info *arg_info);
extern node *SSATfuncond (node *arg_node, info *arg_info);
extern node *SSATreturn (node *arg_node, info *arg_info);
extern node *SSATids (node *arg_node, info *arg_info);

#endif /* _SAC_SSATRANSFORM_H_ */
