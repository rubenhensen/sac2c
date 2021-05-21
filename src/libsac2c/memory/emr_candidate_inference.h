#ifndef _SAC_EMRCI_H_
#define _SAC_EMRCI_H_

#include "types.h"

/**
 * EMRCI - Extended Memory Reuse Candidate Inference
 */

extern node *EMRCIdoWithloopExtendedReuseCandidateInference (node *syntax_tree);
extern node *EMRCIdoWithloopExtendedReuseCandidateInferencePrf (node *syntax_tree);
extern node *EMRCIprintPreFun (node *arg_node, info *arg_info);

extern node *EMRCIfundef (node *arg_node, info *arg_info);
extern node *EMRCIap (node *arg_node, info *arg_info);
extern node *EMRCIcond (node *arg_node, info *arg_info);
extern node *EMRCIarg (node *arg_node, info *arg_info);
extern node *EMRCIassign (node *arg_node, info *arg_info);
extern node *EMRCIlet (node *arg_node, info *arg_info);
extern node *EMRCIprf (node *arg_node, info *arg_info);
extern node *EMRCIids (node *arg_node, info *arg_info);
extern node *EMRCIwith (node *arg_node, info *arg_info);
extern node *EMRCIgenarray (node *arg_node, info *arg_info);
extern node *EMRCImodarray (node *arg_node, info *arg_info);

#endif /* _SAC_EMRCI_H_ */
