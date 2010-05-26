

#ifndef _SPLIT_CUDD_FOLD_H_
#define _SPLIT_CUDD_FOLD_H_

#include "types.h"

extern node *SCUFdoSplitCudaFold (node *arg_node);
extern node *SCUFfundef (node *arg_node, info *arg_info);
extern node *SCUFassign (node *arg_node, info *arg_info);
extern node *SCUFlet (node *arg_node, info *arg_info);
extern node *SCUFwith (node *arg_node, info *arg_info);
extern node *SCUFpart (node *arg_node, info *arg_info);
extern node *SCUFgenerator (node *arg_node, info *arg_info);
extern node *SCUFfold (node *arg_node, info *arg_info);
extern node *SCUFcode (node *arg_node, info *arg_info);
extern node *SCUFprf (node *arg_node, info *arg_info);

#endif
