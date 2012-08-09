

#ifndef _SAC_PREPARE_DIST_SCHEDULER_H_
#define _SAC_PREPARE_DIST_SCHEDULER_H_

#include "types.h"

extern node *PDSdoPrepareDistributedScheduler (node *arg_node);
extern node *PDSfundef (node *arg_node, info *arg_info);
extern node *PDSassign (node *arg_node, info *arg_info);
extern node *PDScond (node *arg_node, info *arg_info);

#endif
