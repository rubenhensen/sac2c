

#ifndef _SAC_MINIMIZE_LOOP_TRANSFERS_H_
#define _SAC_MINIMIZE_LOOP_TRANSFERS_H_

#include "types.h"

extern node *MLTRANdoMinimizeLoopTransfers (node *arg_node);
extern node *MLTRANprf (node *arg_node, info *arg_info);
extern node *MLTRANassign (node *arg_node, info *arg_info);
extern node *MLTRANlet (node *arg_node, info *arg_info);
extern node *MLTRANap (node *arg_node, info *arg_info);
extern node *MLTRANfundef (node *arg_node, info *arg_info);
extern node *MLTRANid (node *arg_node, info *arg_info);
extern node *MLTRANarg (node *arg_node, info *arg_info);
extern node *MLTRANfuncond (node *arg_node, info *arg_info);
extern node *MLTRANreturn (node *arg_node, info *arg_info);

#endif
