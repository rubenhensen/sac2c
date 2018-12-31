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
extern node *WRCIdoWithloopExtendedReuseCandidateInference (node *syntax_tree);

extern node *WRCIfundef (node *arg_node, info *arg_info);
extern node *WRCIap (node *arg_node, info *arg_info);
extern node *WRCIcond (node *arg_node, info *arg_info);
extern node *WRCIarg (node *arg_node, info *arg_info);
extern node *WRCIassign (node *arg_node, info *arg_info);
extern node *WRCIlet (node *arg_node, info *arg_info);
extern node *WRCIprf (node *arg_node, info *arg_info);
extern node *WRCIids (node *arg_node, info *arg_info);
extern node *WRCIwith (node *arg_node, info *arg_info);
extern node *WRCIgenarray (node *arg_node, info *arg_info);
extern node *WRCImodarray (node *arg_node, info *arg_info);
extern node *WRCIfold (node *arg_node, info *arg_info);
extern node *WRCIgenerator (node *arg_node, info *arg_info);

extern node *WRCIprintRCs (node *arg_node, info *arg_info);

/*
 * EMR Loop Memory Propogation
 */
extern node *ELMPdoExtendLoopMemoryPropagation (node *syntax_tree);
extern node *ELMPfundef (node *arg_node, info *arg_info);
extern node *ELMPap (node *arg_node, info *arg_info);
extern node *ELMPlet (node *arg_node, info *arg_info);
extern node *ELMPwith (node *arg_node, info *arg_info);
extern node *ELMPgenarray (node *arg_node, info *arg_info);
extern node *ELMPmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_WRCI_H_ */
