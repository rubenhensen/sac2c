#ifndef _SAC_MOVESHAREDMEMINSTR_H_
#define _SAC_MOVESHAREDMEMINSTR_H_

#include "types.h"

/******************************************************************************
 *
 * $Id$
 *
 * Move shared memory management instructions
 *
 * Prefix: MVSMI
 *
 *****************************************************************************/
extern node *MVSMIdoMoveSharedMemoryManagementInstructions (node *syntax_tree);

extern node *MVSMIfundef (node *arg_node, info *arg_info);
extern node *MVSMIap (node *arg_node, info *arg_info);
extern node *MVSMIassign (node *arg_node, info *arg_info);

extern node *COSMIfundef (node *arg_node, info *arg_info);
extern node *COSMIret (node *arg_node, info *arg_info);
extern node *COSMIarg (node *arg_node, info *arg_info);
extern node *COSMIblock (node *arg_node, info *arg_info);
extern node *COSMIvardec (node *arg_node, info *arg_info);
extern node *COSMIassign (node *arg_node, info *arg_info);
extern node *COSMIlet (node *arg_node, info *arg_info);
extern node *COSMIprf (node *arg_node, info *arg_info);
extern node *COSMIwith2 (node *arg_node, info *arg_info);
extern node *COSMIgenarray (node *arg_node, info *arg_info);
extern node *COSMImodarray (node *arg_node, info *arg_info);
extern node *COSMIfold (node *arg_node, info *arg_info);

#endif /* _SAC_MOVESHAREDMEMINSTR_H_ */
