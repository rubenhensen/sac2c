#ifndef _HANDLE_DOTS_H_
#define _HANDLE_DOTS_H_

#include "types.h"

extern node *HDdoEliminateSelDots (node *arg_node);
extern node *HDwith (node *arg_node, info *arg_info);
extern node *HDgenarray (node *arg_node, info *arg_info);
extern node *HDmodarray (node *arg_node, info *arg_info);
extern node *HDfold (node *arg_node, info *arg_info);
extern node *HDpart (node *arg_node, info *arg_info);
extern node *HDgenerator (node *arg_node, info *arg_info);
extern node *HDdot (node *arg_node, info *arg_info);
extern node *HDspap (node *arg_node, info *arg_info);
extern node *HDprf (node *arg_node, info *arg_info);
extern node *HDassign (node *arg_node, info *arg_info);
extern node *HDsetwl (node *arg_node, info *arg_info);
extern node *HDspid (node *arg_node, info *arg_info);

#endif /* _HANDLE_DOTS_H_ */
