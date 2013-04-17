#ifndef _SAC_ELIM_ALPHA_TYPES_H_
#define _SAC_ELIM_ALPHA_TYPES_H_

#include "types.h"

extern node *EATdoEliminateAlphaTypes (node *arg_node);
extern node *EATmodule (node *arg_node, info *arg_info);
extern node *EATfundef (node *arg_node, info *arg_info);
extern node *EATap (node *arg_node, info *arg_info);
extern node *EATavis (node *arg_node, info *arg_info);
extern node *EATblock (node *arg_node, info *arg_info);
extern node *EATarray (node *arg_node, info *arg_info);
extern node *EATlet (node *arg_node, info *arg_info);
extern node *EATpart (node *arg_node, info *arg_info);
extern node *EATwithid (node *arg_node, info *arg_info);
extern node *EATwith (node *arg_node, info *arg_info);

#endif /* _SAC_ELIM_ALPHA_TYPES_H_ */
