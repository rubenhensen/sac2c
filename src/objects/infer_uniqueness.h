/* $Id$ */

#ifndef _SAC_INFER_UNIQUENESS_H_
#define _SAC_INFER_UNIQUENESS_H_

#include "types.h"

extern node *IUQvardec (node *arg_node, info *arg_info);
extern node *IUQarg (node *arg_node, info *arg_info);
extern node *IUQfundef (node *arg_node, info *arg_info);
extern node *IUQblock (node *arg_node, info *arg_info);
extern node *IUQassign (node *arg_node, info *arg_info);
extern node *IUQlet (node *arg_node, info *arg_info);
extern node *IUQap (node *arg_node, info *arg_info);
extern node *IUQprf (node *arg_node, info *arg_info);
extern node *IUQid (node *arg_node, info *arg_info);
extern node *IUQwith (node *arg_node, info *arg_info);
extern node *IUQreturn (node *arg_node, info *arg_info);
extern node *IUQfuncond (node *arg_node, info *arg_info);
extern node *IUQcond (node *arg_node, info *arg_info);

extern node *IUQdoInferUniqueness (node *syntax_tree);

#endif /* _SAC_INFER_UNIQUENESS_H_ */
