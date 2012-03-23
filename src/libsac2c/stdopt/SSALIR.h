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

#ifndef _SAC_SSALIR_H_
#define _SAC_SSALIR_H_

#include "types.h"

/*****************************************************************************
 *
 * Loop invariants removal ( ssalir_tab)
 *
 * prefix: LIR
 *
 * description:
 *   this module does loop invariant removal on a function in ssa form.
 *   traversal functions to infer loop invariant expressions
 *
 *****************************************************************************/
extern node *LIRdoLoopInvariantRemoval (node *arg_node);

extern node *LIRfundef (node *arg_node, info *arg_info);
extern node *LIRarg (node *arg_node, info *arg_info);
extern node *LIRvardec (node *arg_node, info *arg_info);
extern node *LIRblock (node *arg_node, info *arg_info);
extern node *LIRassign (node *arg_node, info *arg_info);
extern node *LIRlet (node *arg_node, info *arg_info);
extern node *LIRid (node *arg_node, info *arg_info);
extern node *LIRap (node *arg_node, info *arg_info);
extern node *LIRcond (node *arg_node, info *arg_info);
extern node *LIRreturn (node *arg_node, info *arg_info);
extern node *LIRwith (node *arg_node, info *arg_info);
extern node *LIRwithid (node *arg_node, info *arg_info);
extern node *LIRexprs (node *arg_node, info *arg_info);
extern node *LIRids (node *arg_node, info *arg_info);
extern node *LIRcode (node *arg_node, info *arg_info);

/*****************************************************************************
 *
 * traversal functions to move loop invariant expressions
 *
 * prefix: LIRMOV
 *
 * description:
 *
 *****************************************************************************/
extern node *LIRMOVid (node *arg_node, info *arg_info);
extern node *LIRMOVwithid (node *arg_node, info *arg_info);
extern node *LIRMOVblock (node *arg_node, info *arg_info);
extern node *LIRMOVassign (node *arg_node, info *arg_info);
extern node *LIRMOVlet (node *arg_node, info *arg_info);
extern node *LIRMOVids (node *arg_node, info *arg_info);
#endif /* _SAC_SSALIR_H_ */
