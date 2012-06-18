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

#endif /* _SAC_SSALIR_H_ */
