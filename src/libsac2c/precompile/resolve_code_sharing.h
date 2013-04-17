#ifndef _SAC_RESOLVE_CODE_SHARING_H_
#define _SAC_RESOLVE_CODE_SHARING_H_

#include "types.h"

extern node *RCSdoResolveCodeSharing (node *arg_node);

extern node *RCSwith (node *arg_node, info *arg_info);
extern node *RCSwith2 (node *arg_node, info *arg_info);
extern node *RCScode (node *arg_node, info *arg_info);
extern node *RCSpart (node *arg_node, info *arg_info);
extern node *RCSwlgrid (node *arg_node, info *arg_info);

#endif /* _SAC_RESOLVE_CODE_SHARING_H_ */
