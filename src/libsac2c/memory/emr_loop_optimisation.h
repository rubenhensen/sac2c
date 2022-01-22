#ifndef _SAC_EMRL_H_
#define _SAC_EMRL_H_

#include "types.h"

/*
 * EMR Loop Memory Propogation
 */
extern node *EMRLfundef (node *arg_node, info *arg_info);
extern node *EMRLassign (node *arg_node, info *arg_info);
extern node *EMRLap (node *arg_node, info *arg_info);
extern node *EMRLlet (node *arg_node, info *arg_info);
extern node *EMRLwith (node *arg_node, info *arg_info);
extern node *EMRLgenarray (node *arg_node, info *arg_info);
extern node *EMRLmodarray (node *arg_node, info *arg_info);

extern node *EMRLdoExtendLoopMemoryPropagation (node *syntax_tree);

#endif /* _SAC_EMRL_H_ */
