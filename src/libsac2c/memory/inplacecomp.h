#ifndef _SAC_INPLACECOMP_H_
#define _SAC_INPLACECOMP_H_

#include "types.h"

/******************************************************************************
 *
 * Inplace Computation traversal
 *
 * Prefix: EMIP
 *
 *****************************************************************************/
extern node *EMIPdoInplaceComputation (node *syntax_tree);

extern node *EMIPap (node *arg_node, info *arg_info);
extern node *EMIPcode (node *arg_node, info *arg_info);
extern node *EMIPcond (node *arg_node, info *arg_info);
extern node *EMIPfundef (node *arg_node, info *arg_info);
extern node *EMIPlet (node *arg_node, info *arg_info);
extern node *EMIPprf (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * Inplace Computation helper traversal
 *
 * Prefix: EMIPH
 *
 *****************************************************************************/
extern node *EMIPHap (node *arg_node, info *arg_info);
extern node *EMIPHassign (node *arg_node, info *arg_info);
extern node *EMIPHid (node *arg_node, info *arg_info);

#endif /* _SAC_DATAREUSE_H_ */
