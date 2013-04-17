#ifndef _SAC_LOOP_INVARIANT_REMOVEL_H_
#define _SAC_LOOP_INVARIANT_REMOVEL_H_

#include "types.h"

/*****************************************************************************
 *
 * Loop invariants removal ( ssalir_tab)
 *
 * prefix: DLIR
 *
 * description:
 *   this module does loop invariant removal on a function in ssa form.
 *   traversal functions to infer loop invariant expressions
 *
 *****************************************************************************/
extern node *DLIRdoLoopInvariantRemoval (node *arg_node);

extern node *DLIRfundef (node *arg_node, info *arg_info);
extern node *DLIRarg (node *arg_node, info *arg_info);
extern node *DLIRvardec (node *arg_node, info *arg_info);
extern node *DLIRblock (node *arg_node, info *arg_info);
extern node *DLIRassign (node *arg_node, info *arg_info);
extern node *DLIRlet (node *arg_node, info *arg_info);
extern node *DLIRid (node *arg_node, info *arg_info);
extern node *DLIRap (node *arg_node, info *arg_info);
extern node *DLIRcond (node *arg_node, info *arg_info);
extern node *DLIRreturn (node *arg_node, info *arg_info);
extern node *DLIRwith (node *arg_node, info *arg_info);
extern node *DLIRwithid (node *arg_node, info *arg_info);
extern node *DLIRexprs (node *arg_node, info *arg_info);
extern node *DLIRids (node *arg_node, info *arg_info);
extern node *DLIRmodule (node *arg_node, info *arg_info);

/*****************************************************************************
 *
 * traversal functions to move loop invariant expressions
 *
 * prefix: LIRMOV
 *
 * description:
 *
 *****************************************************************************/
extern node *DLIRMOVid (node *arg_node, info *arg_info);
extern node *DLIRMOVwithid (node *arg_node, info *arg_info);
extern node *DLIRMOVblock (node *arg_node, info *arg_info);
extern node *DLIRMOVassign (node *arg_node, info *arg_info);
extern node *DLIRMOVlet (node *arg_node, info *arg_info);
extern node *DLIRMOVids (node *arg_node, info *arg_info);
#endif /* _SAC_LOOP_INVARIANT_REMOVEL_H_ */
