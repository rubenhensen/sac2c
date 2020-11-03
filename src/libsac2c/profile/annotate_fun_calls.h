#ifndef _SAC_ANNOTATE_FUN_CALLS_H_
#define _SAC_ANNOTATE_FUN_CALLS_H_

#include "types.h"

extern node *PFdoProfileFunCalls (node *fundef);

extern node *PFfundef (node *arg_node, info *arg_info);
extern node *PFassign (node *arg_node, info *arg_info);
extern node *PFap (node *arg_node, info *arg_info);
extern node *PFwith (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATE_FUN_CALLS_H_ */
