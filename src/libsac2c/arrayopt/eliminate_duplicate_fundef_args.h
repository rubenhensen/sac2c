#ifndef _SAC_ELIMINATE_DUPLICATE_FUNDEF_ARGS_H_
#define _SAC_ELIMINATE_DUPLICATE_FUNDEF_ARGS_H_

#include "types.h"

extern node *EDFAdoEliminateDuplicateFundefArgs (node *arg_node);
extern bool EDFAisLoopFunInvariant (node *arg_node, node *argid, node *rca);

extern node *EDFAmodule (node *arg_node, info *arg_info);
extern node *EDFAfundef (node *arg_node, info *arg_info);
// node *EDFAarg(node *arg_node, info *arg_info);
// node *EDFAblock(node *arg_node, info *arg_info);
// node *EDFAassign(node *arg_node, info *arg_info);
// node *EDFAlet(node *arg_node, info *arg_info);
extern node *EDFAap (node *arg_node, info *arg_info);
// node *EDFAret(node *arg_node, info *arg_info);
// node *EDFAreturn(node *arg_node, info *arg_info);
// node *EDFAids(node *arg_node, info *arg_info);
// node *EDFAexprs(node *arg_node, info *arg_info);
// node *EDFAid(node *arg_node, info *arg_info);

#endif /*_SAC_ELIMINATE_DUPLICATE_FUNDEF_ARGS_H_*/
