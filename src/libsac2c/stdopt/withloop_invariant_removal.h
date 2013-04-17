#ifndef _SAC_WITHLOOP_INVARIANT_REMOVAL_H_
#define _SAC_WITHLOOP_INVARIANT_REMOVAL_H_

#include "types.h"

/*****************************************************************************
 *
 * Withloop invariants removal ( ssalir_tab)
 *
 * prefix: WLIR
 *
 * description:
 *   this module does loop invariant removal on a function in ssa form.
 *   traversal functions to infer loop invariant expressions
 *
 *****************************************************************************/
extern node *WLIRdoLoopInvariantRemoval (node *fundef);

extern node *WLIRmodule (node *arg_node, info *arg_info);
extern node *WLIRfundef (node *arg_node, info *arg_info);
extern node *WLIRarg (node *arg_node, info *arg_info);
extern node *WLIRvardec (node *arg_node, info *arg_info);
extern node *WLIRblock (node *arg_node, info *arg_info);
extern node *WLIRassign (node *arg_node, info *arg_info);
extern node *WLIRlet (node *arg_node, info *arg_info);
extern node *WLIRid (node *arg_node, info *arg_info);
extern node *WLIRwith (node *arg_node, info *arg_info);
extern node *WLIRwithid (node *arg_node, info *arg_info);
extern node *WLIRids (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOP_INVARIANT_REMOVAL_H_ */
