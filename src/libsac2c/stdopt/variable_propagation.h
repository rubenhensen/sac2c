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
extern node *VPfundef (node *arg_node, info *arg_info);
extern node *VPassign (node *arg_node, info *arg_info);
extern node *VPid (node *arg_node, info *arg_info);
extern node *VPavis (node *arg_node, info *arg_info);

#endif /* _SAC_VARIABLEPROPAGATION_H_ */
