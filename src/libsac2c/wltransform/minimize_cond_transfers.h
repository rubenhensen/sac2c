

#ifndef _SAC_MINIMIZE_COND_TRANSFERS_H_
#define _SAC_MINIMIZE_COND_TRANSFERS_H_

#include "types.h"

extern node *MCTRANdoMinimizeCondTransfers (node *arg_node);
extern node *MCTRANprf (node *arg_node, info *arg_info);
extern node *MCTRANassign (node *arg_node, info *arg_info);
extern node *MCTRANlet (node *arg_node, info *arg_info);
extern node *MCTRANap (node *arg_node, info *arg_info);
extern node *MCTRANfundef (node *arg_node, info *arg_info);
extern node *MCTRANid (node *arg_node, info *arg_info);
extern node *MCTRANarg (node *arg_node, info *arg_info);
extern node *MCTRANfuncond (node *arg_node, info *arg_info);
extern node *MCTRANreturn (node *arg_node, info *arg_info);

#endif
