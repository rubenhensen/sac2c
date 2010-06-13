/*
 * $Id$
 */

#ifndef _SAC_CONSTANT_PROPAGATION_H_
#define _SAC_CONSTANT_PROPAGATION_H_

#include "types.h"

/******************************************************************************
 *
 * Constant propagation traversal ( cp_tab)
 *
 * Prefix: CP
 *
 *****************************************************************************/
extern node *CPdoConstantPropagation (node *arg_node);
extern node *CPdoConstantPropagationOneFundef (node *arg_node);
extern node *CPfundef (node *arg_node, info *arg_info);
extern node *CPassign (node *arg_node, info *arg_info);
extern node *CPprf (node *arg_node, info *arg_info);
extern node *CPid (node *arg_node, info *arg_info);
extern node *CParray (node *arg_node, info *arg_info);
extern node *CPgenarray (node *arg_node, info *arg_info);
extern node *CPavis (node *arg_node, info *arg_info);

#endif /* _SAC_CONSTANT_PROPAGATION_H_  */
