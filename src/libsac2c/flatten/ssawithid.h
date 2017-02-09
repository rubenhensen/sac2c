/******************************************************************************
 *
 * SSA Withid traversal
 *
 * Prefix: SSAW
 *
 *****************************************************************************/

#ifndef _SAC_SSAWITHID_H_
#define _SAC_SSAWITHID_H_

#include "types.h"

extern node *SSAWdoTransformToSSA (node *fundef);
extern node *SSAWdoTransformFromSSA (node *fundef);

extern node *SSAWfundef (node *arg_node, info *arg_info);
extern node *SSAWblock (node *arg_node, info *arg_info);
extern node *SSAWassign (node *arg_node, info *arg_info);
extern node *SSAWwith (node *arg_node, info *arg_info);
extern node *SSAWpart (node *arg_node, info *arg_info);
extern node *SSAWwithid (node *arg_node, info *arg_info);
extern node *SSAWids (node *arg_node, info *arg_info);

#endif // _SAC_SSAWITHID_H_
