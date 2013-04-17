#ifndef _SAC_SPMDFUN_FIX_H_
#define _SAC_SPMDFUN_FIX_H_

#include <types.h>

node *FSFSap (node *arg_node, info *arg_info);
node *FSFSfundef (node *arg_node, info *arg_info);
node *FSFSlet (node *arg_node, info *arg_info);
node *FSFSreturn (node *arg_node, info *arg_info);
node *FSFSwith2 (node *arg_node, info *arg_info);

node *FSFSdoFixSpmdFunctionSignatures (node *syntax_tree);

#endif /* _SAC_SPMDFUN_FIX_H_ */
