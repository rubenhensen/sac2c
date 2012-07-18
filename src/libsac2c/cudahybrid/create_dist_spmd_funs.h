/*****************************************************************************
 *
 * Creating Distributed SPMD functions
 *
 * prefix: MTDSPMDF
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_DIST_SPMD_FUNS_H_
#define _SAC_CREATE_DIST_SPMD_FUNS_H_

#include "types.h"

extern node *MTDSPMDFdoCreateDistributedSpmdFuns (node *syntax_tree);

extern node *MTDSPMDFmodule (node *arg_node, info *arg_info);
extern node *MTDSPMDFfundef (node *arg_node, info *arg_info);
extern node *MTDSPMDFwith2 (node *arg_node, info *arg_info);
extern node *MTDSPMDFwith (node *arg_node, info *arg_info);
extern node *MTDSPMDFlet (node *arg_node, info *arg_info);
extern node *MTDSPMDFid (node *arg_node, info *arg_info);
extern node *MTDSPMDFids (node *arg_node, info *arg_info);
extern node *MTDSPMDFassign (node *arg_node, info *arg_info);
extern node *MTDSPMDFwithid (node *arg_node, info *arg_info);
extern node *MTDSPMDFcond (node *arg_node, info *arg_info);
extern node *MTDSPMDFprf (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_DIST_SPMD_FUNS_H_ */
