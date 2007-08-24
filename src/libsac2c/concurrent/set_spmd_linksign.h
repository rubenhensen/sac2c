#ifndef _SET_SPMD_LINKSIGN_H_
#define _SET_SPMD_LINKSIGN_H_

#include "types.h"

extern node *SSPMDLSarg (node *arg_node, info *arg_info);
extern node *SSPMDLSid (node *arg_node, info *arg_info);
extern node *SSPMDLSexprs (node *arg_node, info *arg_info);
extern node *SSPMDLSreturn (node *arg_node, info *arg_info);
extern node *SSPMDLSgenarray (node *arg_node, info *arg_info);
extern node *SSPMDLSmodarray (node *arg_node, info *arg_info);
extern node *SSPMDLSbreak (node *arg_node, info *arg_info);
extern node *SSPMDLSwith2 (node *arg_node, info *arg_info);
extern node *SSPMDLSlet (node *arg_node, info *arg_info);
extern node *SSPMDLSret (node *arg_node, info *arg_info);
extern node *SSPMDLSfundef (node *arg_node, info *arg_info);
extern node *SSPMDLSmodule (node *arg_node, info *arg_info);
extern node *SSPMDLSdoSetSpmdLinksign (node *syntax_tree);

#endif /* _SET_SPMD_LINKSIGN_H_*/
