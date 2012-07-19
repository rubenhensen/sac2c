/*****************************************************************************
 *
 * Lifting memory operations from conditional
 *
 * prefix: MTLMOFC
 *
 *****************************************************************************/

#ifndef _SAC_LIFT_MEMOPS_COND_H_
#define _SAC_LIFT_MEMOPS_COND_H_

#include "types.h"

extern node *MTLMOFCdoCreateDistributedSpmdFuns (node *syntax_tree);

extern node *MTLMOFCmodule (node *arg_node, info *arg_info);
extern node *MTLMOFCfundef (node *arg_node, info *arg_info);
extern node *MTLMOFCwith2 (node *arg_node, info *arg_info);
extern node *MTLMOFCwith (node *arg_node, info *arg_info);
extern node *MTLMOFCassign (node *arg_node, info *arg_info);
extern node *MTLMOFCcond (node *arg_node, info *arg_info);
extern node *MTLMOFCprf (node *arg_node, info *arg_info);
extern node *MTLMOFClet (node *arg_node, info *arg_info);
extern node *MTLMOFCids (node *arg_node, info *arg_info);

#endif /* _SAC_LIFT_MEMOPS_COND_H_ */
