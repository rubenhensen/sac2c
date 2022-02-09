#ifndef _SAC_INPLACECOMP_H_
#define _SAC_INPLACECOMP_H_

#include "types.h"

/******************************************************************************
 *
 * Inplace Computation traversal
 *
 * Prefix: IPC
 *
 *****************************************************************************/
extern node *IPCdoInplaceComputation (node *syntax_tree);

extern node *IPCap (node *arg_node, info *arg_info);
extern node *IPCcode (node *arg_node, info *arg_info);
extern node *IPCrange (node *arg_node, info *arg_info);
extern node *IPCcond (node *arg_node, info *arg_info);
extern node *IPCfundef (node *arg_node, info *arg_info);
extern node *IPClet (node *arg_node, info *arg_info);
extern node *IPCprf (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * Inplace Computation helper traversal
 *
 * Prefix: IPCH
 *
 *****************************************************************************/
extern node *IPCHap (node *arg_node, info *arg_info);
extern node *IPCHassign (node *arg_node, info *arg_info);
extern node *IPCHid (node *arg_node, info *arg_info);

#endif /* _SAC_DATAREUSE_H_ */
