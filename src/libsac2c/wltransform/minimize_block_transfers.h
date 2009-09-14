

#ifndef _SAC_MINIMIZE_BLOCK_TRANSFERS_H_
#define _SAC_MINIMIZE_BLOCK_TRANSFERS_H_

#include "types.h"

extern node *MBTRANdoMinimizeBlockTransfers (node *arg_node);
extern node *MBTRANassign (node *arg_node, info *arg_info);
extern node *MBTRANlet (node *arg_node, info *arg_info);
extern node *MBTRANid (node *arg_node, info *arg_info);
extern node *MBTRANwith (node *arg_node, info *arg_info);
extern node *MBTRANfundef (node *arg_node, info *arg_info);
extern node *MBTRANcond (node *arg_node, info *arg_info);

#endif
