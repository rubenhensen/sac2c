#ifndef _SAC_WLIDXS_H_
#define _SAC_WLIDXS_H_

#include "types.h"

extern node *WLIDXdoAnnotateWithloopIdxs (node *syntax_tree);

extern node *WLIDXfundef (node *arg_node, info *arg_info);
extern node *WLIDXlet (node *arg_node, info *arg_info);
extern node *WLIDXwith (node *arg_node, info *arg_info);
extern node *WLIDXwithid (node *arg_node, info *arg_info);

#endif /* _SAC_WLIDXS_H_ */
