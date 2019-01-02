#ifndef _SAC_WRCI_H_
#define _SAC_WRCI_H_

#include "types.h"

/******************************************************************************
 *
 * With-Loop reuse candidate inference
 *
 * Prefix: WRCI
 *
 *****************************************************************************/
extern node *WRCIdoWithloopReuseCandidateInference (node *syntax_tree);
extern node *WRCIprintPreFun (node *arg_node, info *arg_info);

extern node *WRCIfundef (node *arg_node, info *arg_info);
extern node *WRCIassign (node *arg_node, info *arg_info);
extern node *WRCIlet (node *arg_node, info *arg_info);
extern node *WRCIwith (node *arg_node, info *arg_info);
extern node *WRCIgenarray (node *arg_node, info *arg_info);
extern node *WRCImodarray (node *arg_node, info *arg_info);
extern node *WRCIfold (node *arg_node, info *arg_info);
extern node *WRCIgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_WRCI_H_ */
