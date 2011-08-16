/*
 * $Id$
 */

#ifndef _SAC_ASSOCIATIVE_LAW_H_
#define _SAC_ASSOCIATIVE_LAW_H_

#include "types.h"

/******************************************************************************
 *
 * Associative Law Optimization
 *
 * prefix: AL
 *
 *****************************************************************************/

extern node *ALdoAssocLawOptimization (node *arg_node);

extern node *ALmodule (node *arg_node, info *arg_info);
extern node *ALfundef (node *arg_node, info *arg_info);
extern node *ALarg (node *arg_node, info *arg_info);
extern node *ALblock (node *arg_node, info *arg_info);
extern node *ALassign (node *arg_node, info *arg_info);
extern node *ALlet (node *arg_node, info *arg_info);
extern node *ALids (node *arg_node, info *arg_info);
extern node *ALwith (node *arg_node, info *arg_info);
extern node *ALcode (node *arg_node, info *arg_info);
extern node *ALpart (node *arg_node, info *arg_info);
extern node *ALprf (node *arg_node, info *arg_info);

#endif /* _SAC_ASSOCIATIVE_LAW_H_ */
