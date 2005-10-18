/*
 * $Id$
 */
#ifndef _SAC_ASSOCIATIVITY_H_
#define _SAC_ASSOCIATIVITY_H_

#include "types.h"

/******************************************************************************
 *
 * Associativity optimization
 *
 * prefix: ASSOC
 *
 *****************************************************************************/
extern node *ASSOCdoAssociativityOptimization (node *arg_node);
extern node *ASSOCdoAssociativityOptimizationOneFundef (node *arg_node);

extern node *ASSOCfundef (node *arg_node, info *arg_info);
extern node *ASSOCblock (node *arg_node, info *arg_info);
extern node *ASSOCassign (node *arg_node, info *arg_info);
extern node *ASSOClet (node *arg_node, info *arg_info);
extern node *ASSOCids (node *arg_node, info *arg_info);
extern node *ASSOCprf (node *arg_node, info *arg_info);

#endif /* _SAC_INFERNEEDCOUNTERS_H_ */
