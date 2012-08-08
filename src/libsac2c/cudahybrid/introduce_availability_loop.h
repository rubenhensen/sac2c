

#ifndef _SAC_INTRODUCE_AVAILABILITY_LOOP_H_
#define _SAC_INTRODUCE_AVAILABILITY_LOOP_H_

#include "types.h"

extern node *IALdoIntroduceAvailabilityLoops (node *arg_node);
extern node *IALfundef (node *arg_node, info *arg_info);
extern node *IALassign (node *arg_node, info *arg_info);
extern node *IALprf (node *arg_node, info *arg_info);
extern node *IALwith (node *arg_node, info *arg_info);
extern node *IALpart (node *arg_node, info *arg_info);
extern node *IALgenerator (node *arg_node, info *arg_info);
extern node *IALexprs (node *arg_node, info *arg_info);
extern node *IALgenarray (node *arg_node, info *arg_info);
extern node *IALmodarray (node *arg_node, info *arg_info);

#endif
