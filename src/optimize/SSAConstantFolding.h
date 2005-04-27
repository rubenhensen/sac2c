/*
 * $Log$
 * Revision 1.16  2005/04/27 07:52:25  ktr
 * Stripped out superfluous rules
 *
 * Revision 1.15  2005/04/20 19:15:29  ktr
 * removed CFarg, CFlet. Codebrushing.
 *
 * Revision 1.14  2005/02/14 15:51:48  mwe
 * CFids removed
 *
 * Revision 1.13  2004/11/26 19:44:47  khf
 * corrected traversal functions
 *
 * Revision 1.12  2004/11/23 20:58:19  sbs
 * include bug elimnated
 *
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

#include "types.h"

/*
 * functions to handle SCOs
 */
extern struct_constant *CFscoExpr2StructConstant (node *expr);
extern struct_constant *CFscoArray2StructConstant (node *expr);
extern struct_constant *CFscoWithidVec2StructConstant (node *expr);
extern struct_constant *CFscoScalar2StructConstant (node *expr);
extern node *CFscoDupStructConstant2Expr (struct_constant *struc_co);
extern struct_constant *CFscoFreeStructConstant (struct_constant *struc_co);

extern node *CFdoConstantFolding (node *fundef, node *modul);

extern node *CFfundef (node *arg_node, info *arg_info);
extern node *CFblock (node *arg_node, info *arg_info);
extern node *CFassign (node *arg_node, info *arg_info);
extern node *CFcond (node *arg_node, info *arg_info);
extern node *CFap (node *arg_node, info *arg_info);
extern node *CFlet (node *arg_node, info *arg_info);
extern node *CFarray (node *arg_node, info *arg_info);
extern node *CFprf (node *arg_node, info *arg_info);
extern node *CFwith (node *arg_node, info *arg_info);
extern node *CFpart (node *arg_node, info *arg_info);
extern node *CFcode (node *arg_node, info *arg_info);
extern node *CFgenerator (node *arg_node, info *arg_info);
extern node *CFfuncond (node *arg_node, info *arg_info);

extern node *CFfoldPrfExpr (prf op, node **arg_expr);

#endif /* SAC_ConstantFolding_h */
