#ifndef _SAC_ELIM_BOTTOM_TYPES_H_
#define _SAC_ELIM_BOTTOM_TYPES_H_

#include "types.h"

extern node *EBTdoEliminateBottomTypes (node *arg_node);
extern node *EBTmodule (node *arg_node, info *arg_info);
extern node *EBTfundef (node *arg_node, info *arg_info);
extern node *EBTap (node *arg_node, info *arg_info);
extern node *EBTprf (node *arg_node, info *arg_info);
extern node *EBTvardec (node *arg_node, info *arg_info);
extern node *EBTlet (node *arg_node, info *arg_info);
extern node *EBTassign (node *arg_node, info *arg_info);
extern node *EBTblock (node *arg_node, info *arg_info);
extern node *EBTids (node *arg_node, info *arg_info);
extern node *EBTcond (node *arg_node, info *arg_info);
extern node *EBTfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_ELIM_BOTTOM_TYPES_H_ */
