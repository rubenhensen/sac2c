#ifndef _HANDLE_WITH_LOOP_DOTS_H_
#define _HANDLE_WITH_LOOP_DOTS_H_

#include "types.h"

extern node *HWLDdoEliminateSelDots (node *arg_node);
extern node *HWLDwith (node *arg_node, info *arg_info);
extern node *HWLDgenarray (node *arg_node, info *arg_info);
extern node *HWLDmodarray (node *arg_node, info *arg_info);
extern node *HWLDfold (node *arg_node, info *arg_info);
extern node *HWLDpart (node *arg_node, info *arg_info);
extern node *HWLDwithid (node *arg_node, info *arg_info);
extern node *HWLDgenerator (node *arg_node, info *arg_info);

#endif /* _HANDLE_WITH_LOOP_DOTS_H_ */
