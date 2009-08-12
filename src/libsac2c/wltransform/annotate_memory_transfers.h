#ifndef _SAC_ANNOTATE_MEMORY_TRANSFERS_H_
#define _SAC_ANNOTATE_MEMORY_TRANSFERS_H_

#include "types.h"

extern node *AMTRANdoAnnotateMemoryTransfers (node *arg_node);
extern node *AMTRANprf (node *arg_node, info *arg_info);
extern node *AMTRANassign (node *arg_node, info *arg_info);
extern node *AMTRANap (node *arg_node, info *arg_info);
extern node *AMTRANfundef (node *arg_node, info *arg_info);
extern node *AMTRANid (node *arg_node, info *arg_info);
extern node *AMTRANarg (node *arg_node, info *arg_info);

#endif
