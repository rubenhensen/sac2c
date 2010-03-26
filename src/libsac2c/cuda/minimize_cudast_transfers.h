

#ifndef _SAC_MINIMIZE_CUDAST_TRANSFERS_H_
#define _SAC_MINIMIZE_CUDAST_TRANSFERS_H_

#include "types.h"

extern node *MCSTRANdoMinimizeCudastTransfers (node *arg_node);
extern node *MCSTRANprf (node *arg_node, info *arg_info);
extern node *MCSTRANassign (node *arg_node, info *arg_info);
extern node *MCSTRANlet (node *arg_node, info *arg_info);
extern node *MCSTRANcudast (node *arg_node, info *arg_info);

#endif
