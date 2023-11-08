#ifndef _SAC_WITHLOOP_INVARIANT_REMOVAL_H_
#define _SAC_WITHLOOP_INVARIANT_REMOVAL_H_

#include "types.h"

/*****************************************************************************
 *
 * Withloop invariants removal; this traversal relies on WLIRI being run
 *          directly before running this traversal.
 *
 * prefix: WLIR
 *
 *
 *****************************************************************************/
extern node *WLIRdoLoopInvariantRemoval (node *arg_node);

extern node *WLIRmodule (node *arg_node, info *arg_info);
extern node *WLIRfundef (node *arg_node, info *arg_info);
extern node *WLIRassign (node *arg_node, info *arg_info);
extern node *WLIRwith (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOP_INVARIANT_REMOVAL_H_ */
