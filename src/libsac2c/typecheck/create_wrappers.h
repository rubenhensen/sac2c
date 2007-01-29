/*
 *
 * $Id$
 *
 */

#ifndef _SAC_CREATE_WRAPPERS_H_
#define _SAC_CREATE_WRAPPERS_H_

#include "types.h"

extern node *CRTWRPdoCreateWrappers (node *arg_node);
extern ntype *CRTWRPcreateFuntype (node *fundef);

extern node *CRTWRPmodule (node *arg_node, info *arg_info);
extern node *CRTWRPfundef (node *arg_node, info *arg_info);
extern node *CRTWRPlet (node *arg_node, info *arg_info);
extern node *CRTWRPspap (node *arg_node, info *arg_info);
extern node *CRTWRPgenarray (node *arg_node, info *arg_info);
extern node *CRTWRPspfold (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WRAPPERS_H_ */
