/*****************************************************************************
 *
 * Creating SPMD functions
 *
 * prefix: MTSPMDF
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_SPMD_FUNS_H_
#define _SAC_CREATE_SPMD_FUNS_H_

#include "types.h"

extern node *MTSPMDFdoSpmdLift (node *syntax_tree);

extern node *MTSPMDFmodule (node *arg_node, info *arg_info);
extern node *MTSPMDFfundef (node *arg_node, info *arg_info);
extern node *MTSPMDFdo (node *arg_node, info *arg_info);
extern node *MTSPMDFvardec (node *arg_node, info *arg_info);
extern node *MTSPMDFwiths (node *arg_node, info *arg_info);
extern node *MTSPMDFwith2 (node *arg_node, info *arg_info);
extern node *MTSPMDFwith (node *arg_node, info *arg_info);
extern node *MTSPMDFlet (node *arg_node, info *arg_info);
extern node *MTSPMDFid (node *arg_node, info *arg_info);
extern node *MTSPMDFids (node *arg_node, info *arg_info);
extern node *MTSPMDFwithid (node *arg_node, info *arg_info);
extern node *MTSPMDFfold (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_SPMD_FUNS_H_ */
