#ifndef _SAC_ALGEBRAIC_WLF_H_
#define _SAC_ALGEBRAIC_WLF_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Algebraic With-Loop folding  traversal ( awlf_tab)
 *
 * Prefix: AWLF
 *
 *****************************************************************************/
extern node *AWLFdoAlgebraicWithLoopFolding (node *arg_node);

extern node *AWLFfundef (node *arg_node, info *arg_info);
extern node *AWLFassign (node *arg_node, info *arg_info);
extern node *AWLFlet (node *arg_node, info *arg_info);
extern node *AWLFwith (node *arg_node, info *arg_info);
extern node *AWLFpart (node *arg_node, info *arg_info);
extern node *AWLFcode (node *arg_node, info *arg_info);
extern node *AWLFcond (node *arg_node, info *arg_info);
extern node *AWLFfuncond (node *arg_node, info *arg_info);
extern node *AWLFwhile (node *arg_node, info *arg_info);
extern node *AWLFids (node *arg_node, info *arg_info);
extern node *AWLFprf (node *arg_node, info *arg_info);

#endif /* _SAC_ALGEBRAIC_WLF_H_ */
