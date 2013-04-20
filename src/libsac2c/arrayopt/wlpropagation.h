#ifndef _SAC_WLPROPAGATION_H_
#define _SAC_WLPROPAGATION_H_

#include "types.h"

extern node *WLPROPdoWithloopPropagation (node *arg_node);

extern node *WLPROPfundef (node *arg_node, info *arg_info);
extern node *WLPROPassign (node *arg_node, info *arg_info);
extern node *WLPROPap (node *arg_node, info *arg_info);
extern node *WLPROPexprs (node *arg_node, info *arg_info);
extern node *WLPROPid (node *arg_node, info *arg_info);

#endif
