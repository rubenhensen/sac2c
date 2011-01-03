/*
 * $Id$
 */

#ifndef _SAC_DISTRIBUTIVE_LAW_H_
#define _SAC_DISTRIBUTIVE_LAW_H_

#include "types.h"

/******************************************************************************
 *
 * Distributivity optimization
 *
 * prefix: DL
 *
 *****************************************************************************/

extern node *DLdoDistributiveLawOptimization (node *arg_node);

extern node *DLfundef (node *arg_node, info *arg_info);
extern node *DLblock (node *arg_node, info *arg_info);
extern node *DLassign (node *arg_node, info *arg_info);
extern node *DLlet (node *arg_node, info *arg_info);
extern node *DLids (node *arg_node, info *arg_info);
extern node *DLprf (node *arg_node, info *arg_info);

#endif /* _SAC_DISTRIBUTIVE_LAW_H_ */
