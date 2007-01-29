/*
 * $Id$
 */
#ifndef _SAC_DISTRIBUTIVITY_H_
#define _SAC_DISTRIBUTIVITY_H_

#include "types.h"

/******************************************************************************
 *
 * Distributivity optimization
 *
 * prefix: DISTRIB
 *
 *****************************************************************************/
extern node *DISTRIBdoDistributivityOptimization (node *arg_node);
extern node *DISTRIBdoDistributivityOptimizationOneFundef (node *arg_node);

extern node *DISTRIBfundef (node *arg_node, info *arg_info);
extern node *DISTRIBblock (node *arg_node, info *arg_info);
extern node *DISTRIBassign (node *arg_node, info *arg_info);
extern node *DISTRIBlet (node *arg_node, info *arg_info);
extern node *DISTRIBids (node *arg_node, info *arg_info);
extern node *DISTRIBprf (node *arg_node, info *arg_info);

#endif /* _SAC_DISTRIBUTIVITY_H_ */
