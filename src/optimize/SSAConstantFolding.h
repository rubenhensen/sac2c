/*
 * $Log$
 * Revision 1.11  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 1.10  2004/09/25 14:35:52  ktr
 * Whenever a generator is known to cover just one index, the withid is assumed
 * to be constant inside thate corresponding code.
 *
 * Revision 1.9  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.8  2004/03/05 19:14:27  mwe
 * SSACFfuncond added
 *
 * Revision 1.7  2003/09/16 18:15:26  ktr
 * Index vectors are now treated as structural constants.
 *
 * Revision 1.6  2002/10/09 12:43:24  dkr
 * structural constants exported now
 *
 * Revision 1.5  2001/05/23 15:48:11  nmw
 * comments added, code beautified
 *
 * Revision 1.4  2001/05/15 15:52:05  nmw
 * access to PrfFolding for external modules implemented SSACFFoldPrfExpr
 *
 * Revision 1.3  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.2  2001/03/29 16:31:21  nmw
 * Constant Folding for Loops and Conditionals implemented
 *
 * Revision 1.1  2001/03/20 16:16:54  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   SSAConstantFolding.h
 *
 * prefix: SSACF
 *
 * description:
 *   this module does constant folding on code in ssa form.
 *
 *
 *****************************************************************************/

#ifndef _SAC_ConstantFolding_h_
#define _SAC_ConstantFolding_h_

include "types.h"

  /*
   * functions to handle SCOs
   */
  struct_constant *
  SCOexpr2StructConstant (node *expr);
struct_constant *SCOarray2StructConstant (node *expr);
struct_constant *SCOwithidVec2StructConstant (node *expr);
struct_constant *SCOscalar2StructConstant (node *expr);
node *SCOdupStructConstant2Expr (struct_constant *struc_co);
struct_constant *SCOfreeStructConstant (struct_constant *struc_co);

extern node *CFdoConstantFolding (node *fundef, node *modul);

extern node *CFfundef (node *arg_node, info *arg_info);
extern node *CFblock (node *arg_node, info *arg_info);
extern node *CFarg (node *arg_node, info *arg_info);
extern node *CFassign (node *arg_node, info *arg_info);
extern node *CFcond (node *arg_node, info *arg_info);
extern node *CFreturn (node *arg_node, info *arg_info);
extern node *CFlet (node *arg_node, info *arg_info);
extern node *CFap (node *arg_node, info *arg_info);
extern node *CFid (node *arg_node, info *arg_info);
extern node *CFarray (node *arg_node, info *arg_info);
extern node *CFprf (node *arg_node, info *arg_info);
extern node *CFwith (node *arg_node, info *arg_info);
extern node *CFpart (node *arg_node, info *arg_info);
extern node *CFcode (node *arg_node, info *arg_info);
extern node *CFgen (node *arg_node, info *arg_info);
extern node *CFfuncond (node *arg_node, info *arg_info);

extern node *CFfoldPrfExpr (prf op, node **arg_expr);

#endif /* SAC_ConstantFolding_h */
