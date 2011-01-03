/*
 * $Log$
 * Revision 1.9  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.8  2005/07/20 13:12:37  ktr
 * removed FUNDEF_EXTASSIGN/INTASSIGN
 *
 * Revision 1.7  2004/11/26 21:59:14  mwe
 * LIRids, LIRMOVids added
 *
 * Revision 1.6  2004/11/22 18:33:19  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.5  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.4  2001/04/12 12:40:14  nmw
 * SSALIRexprs added
 *
 * Revision 1.3  2001/04/09 15:57:08  nmw
 * first implementation of code move up (not tested yet)
 *
 * Revision 1.2  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.1  2001/03/26 15:37:45  nmw
 * Initial revision
 *
 */

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
#endif /* _SAC_SSALIR_H_ */
