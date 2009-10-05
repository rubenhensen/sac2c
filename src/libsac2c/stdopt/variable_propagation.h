/*
 * $Id$
 */

#ifndef _SAC_VARIABLEPROPAGATION_H_
#define _SAC_VARIABLEPROPAGATION_H_

#include "types.h"

/******************************************************************************
 *
 * Variable propagation traversal ( vp_tab)
 *
 * Prefix: VP
 *
 *****************************************************************************/
extern node *VPdoVarPropagation (node *arg_node);
extern node *VPdoVarPropagationOneFundef (node *arg_node);

extern node *VPfundef (node *arg_node, info *arg_info);
extern node *VPassign (node *arg_node, info *arg_info);
extern node *VPlet (node *arg_node, info *arg_info);
extern node *VPcode (node *arg_node, info *arg_info);
extern node *VPrange (node *arg_node, info *arg_info);
extern node *VPcond (node *arg_node, info *arg_info);
extern node *VPgenerator (node *arg_node, info *arg_info);
extern node *VPgenarray (node *arg_node, info *arg_info);
extern node *VPmodarray (node *arg_node, info *arg_info);
extern node *VPfold (node *arg_node, info *arg_info);
extern node *VPap (node *arg_node, info *arg_info);
extern node *VPprf (node *arg_node, info *arg_info);
extern node *VPid (node *arg_node, info *arg_info);
extern node *VPreturn (node *arg_node, info *arg_info);
extern node *VParray (node *arg_node, info *arg_info);
extern node *VPfuncond (node *arg_node, info *arg_info);
extern node *VPavis (node *arg_node, info *arg_info);

#endif /* _SAC_VARIABLEPROPAGATION_H_ */
