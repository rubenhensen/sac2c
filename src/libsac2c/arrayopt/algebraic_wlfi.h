/*
 * $Id: algebraic_wlfi.h 16002 2009-02-18 22:03:23Z rbe $
 */
#ifndef _SAC_ALGEBRAIC_WLFI_H_
#define _SAC_ALGEBRAIC_WLFI_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Algebraic With-Loop-Folding-Inference traversal
 *
 * Prefix: AWLFI
 *
 *****************************************************************************/
extern node *AWLFIdoAlgebraicWithLoopFoldingOneFunction (node *arg_node);

extern node *AWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns,
                                     node *restypeavis);
extern node *AWLFIfundef (node *arg_node, info *arg_info);
extern node *AWLFIblock (node *arg_node, info *arg_info);
extern node *AWLFIassign (node *arg_node, info *arg_info);
extern node *AWLFIlet (node *arg_node, info *arg_info);
extern node *AWLFIcond (node *arg_node, info *arg_info);
extern node *AWLFIfuncond (node *arg_node, info *arg_info);
extern node *AWLFIwhile (node *arg_node, info *arg_info);
extern node *AWLFIid (node *arg_node, info *arg_info);
extern node *AWLFIwith (node *arg_node, info *arg_info);
extern node *AWLFIpart (node *arg_node, info *arg_info);
extern node *AWLFIcode (node *arg_node, info *arg_info);
extern node *AWLFImodarray (node *arg_node, info *arg_info);
extern node *AWLFIgenerator (node *arg_node, info *arg_info);
extern node *AWLFIids (node *arg_node, info *arg_info);
extern node *AWLFIprf (node *arg_node, info *arg_info);
extern node *AWLFIavis (node *arg_node, info *arg_info);

#endif /* _SAC_ALGEBRAIC_WLFI_H_ */
