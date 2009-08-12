

#ifndef _SAC_MINIMIZE_SEQUENTIAL_TRANSFERS_H_
#define _SAC_MINIMIZE_SEQUENTIAL_TRANSFERS_H_

#include "types.h"

extern node *MSTRANdoMinimizeTransfers (node *arg_node, bool *flag);
// extern node *MSTRANdoMinimizeTransfers(node *arg_node);
extern node *MSTRANassign (node *arg_node, info *arg_info);
extern node *MSTRANlet (node *arg_node, info *arg_info);
extern node *MSTRANid (node *arg_node, info *arg_info);
extern node *MSTRANwith (node *arg_node, info *arg_info);
extern node *MSTRANfundef (node *arg_node, info *arg_info);

#endif
