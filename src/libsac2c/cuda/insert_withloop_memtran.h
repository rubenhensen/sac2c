

#ifndef _SAC_INSERT_WITHLOOP_MEMTRAN_H_
#define _SAC_INSERT_WITHLOOP_MEMTRAN_H_

#include "types.h"

extern node *IWLMEMdoInsertWithloopMemtran (node *arg_node);
extern node *IWLMEMfundef (node *arg_node, info *arg_info);
extern node *IWLMEMap (node *arg_node, info *arg_info);
extern node *IWLMEMid (node *arg_node, info *arg_info);
extern node *IWLMEMlet (node *arg_node, info *arg_info);
extern node *IWLMEMassign (node *arg_node, info *arg_info);
extern node *IWLMEMwith (node *arg_node, info *arg_info);
extern node *IWLMEMids (node *arg_node, info *arg_info);
extern node *IWLMEMgenarray (node *arg_node, info *arg_info);
extern node *IWLMEMmodarray (node *arg_node, info *arg_info);
extern node *IWLMEMcode (node *arg_node, info *arg_info);
extern node *IWLMEMfuncond (node *arg_node, info *arg_info);

#endif
