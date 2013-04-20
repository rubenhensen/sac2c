#ifndef _SAC_LOOP_AND_COND_SCALARIZATION__IN_H_
#define _SAC_LOOP_AND_COND_SCALARIZATION__IN_H_

#include "types.h"

extern node *LACSIdoLoopScalarization (node *arg_node);

extern node *LACSImodule (node *arg_node, info *arg_info);
extern node *LACSIfundef (node *arg_node, info *arg_info);
extern node *LACSIexprs (node *arg_node, info *arg_info);
extern node *LACSIassign (node *arg_node, info *arg_info);
extern node *LACSIap (node *arg_node, info *arg_info);
extern node *LACSIprf (node *arg_node, info *arg_info);
extern node *LACSIid (node *arg_node, info *arg_info);

#endif /* _SAC_LOOP_AND_COND_SCALARIZATION__IN_H_ */
