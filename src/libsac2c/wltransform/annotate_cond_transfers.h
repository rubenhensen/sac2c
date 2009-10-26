#ifndef _SAC_ANNOTATE_COND_TRANSFERS_H_
#define _SAC_ANNOTATE_COND_TRANSFERS_H_

#include "types.h"

extern node *ACTRANdoAnnotateCondTransfers (node *arg_node);
extern node *ACTRANprf (node *arg_node, info *arg_info);
extern node *ACTRANassign (node *arg_node, info *arg_info);
extern node *ACTRANfundef (node *arg_node, info *arg_info);
extern node *ACTRANid (node *arg_node, info *arg_info);
extern node *ACTRANlet (node *arg_node, info *arg_info);
extern node *ACTRANfuncond (node *arg_node, info *arg_info);

#endif
