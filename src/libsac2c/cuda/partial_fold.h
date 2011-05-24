

#ifndef _SAC_PARTIAL_FOLD_H_
#define _SAC_PARTIAL_FOLD_H_

#include "types.h"

extern node *PFDdoPartialFold (node *arg_node);
extern node *PFDwith (node *arg_node, info *arg_info);
extern node *PFDwithid (node *arg_node, info *arg_info);
extern node *PFDpart (node *arg_node, info *arg_info);
extern node *PFDgenerator (node *arg_node, info *arg_info);
extern node *PFDfold (node *arg_node, info *arg_info);
extern node *PFDlet (node *arg_node, info *arg_info);
extern node *PFDfundef (node *arg_node, info *arg_info);
extern node *PFDassign (node *arg_node, info *arg_info);
extern node *PFDcode (node *arg_node, info *arg_info);
extern node *PFDprf (node *arg_node, info *arg_info);
extern node *PFDmodule (node *arg_node, info *arg_info);

#endif
