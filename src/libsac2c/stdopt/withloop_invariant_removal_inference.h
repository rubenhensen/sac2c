#ifndef _SAC_WITHLOOP_INVARIANT_REMOVAL_INFERENCE_H_
#define _SAC_WITHLOOP_INVARIANT_REMOVAL_INFERENCE_H_

#include "types.h"

/*****************************************************************************
 *
 * Withloop invariants removal inference; tighly related to 
 *          Withloop invariants removal (WLIR) which relies on this traversal.
 *
 * prefix: WLIRI
 *
 *
 *****************************************************************************/
extern node *WLIRIdoLoopInvariantRemovalInference (node *arg_node);

extern node *WLIRImodule (node *arg_node, info *arg_info);
extern node *WLIRIfundef (node *arg_node, info *arg_info);
extern node *WLIRIarg (node *arg_node, info *arg_info);
extern node *WLIRIassign (node *arg_node, info *arg_info);
extern node *WLIRIlet (node *arg_node, info *arg_info);
extern node *WLIRIid (node *arg_node, info *arg_info);
extern node *WLIRIwith (node *arg_node, info *arg_info);
extern node *WLIRIwithid (node *arg_node, info *arg_info);
extern node *WLIRIids (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOP_INVARIANT_REMOVAL_INFERENCE_H_ */
