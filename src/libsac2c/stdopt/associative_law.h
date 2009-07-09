/*
 * $Id$
 */

#ifndef _SAC_ALIATIVE_LAW_H_
#define _SAC_ALIATIVE_LAW_H_

#include "types.h"

/******************************************************************************
 *
 * Associative Law Optimization
 *
 * prefix: AL
 *
 *****************************************************************************/

extern node *ALdoAssocLawOptimizationModule (node *arg_node);
extern node *ALdoAssocLawOptimizationOneFundef (node *arg_node);

extern node *ALfundef (node *arg_node, info *arg_info);
extern node *ALblock (node *arg_node, info *arg_info);
extern node *ALassign (node *arg_node, info *arg_info);
extern node *ALlet (node *arg_node, info *arg_info);
extern node *ALids (node *arg_node, info *arg_info);
extern node *ALprf (node *arg_node, info *arg_info);

#endif /* _SAC_ALIATIVE_LAW_H_ */
