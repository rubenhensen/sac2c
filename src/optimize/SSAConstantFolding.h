/*
 * $Log$
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

#ifndef _SAC_SSAConstantFolding_h_
#define _SAC_SSAConstantFolding_h_

/*
 * structural constant (SCO) should be integrated in constants.[ch] in future
 */
typedef struct STRUCT_CONSTANT struct_constant;

/*
 * functions to handle SCOs
 */
struct_constant *SCOExpr2StructConstant (node *expr);
struct_constant *SCOArray2StructConstant (node *expr);
struct_constant *SCOWithidVec2StructConstant (node *expr);
struct_constant *SCOScalar2StructConstant (node *expr);
node *SCODupStructConstant2Expr (struct_constant *struc_co);
struct_constant *SCOFreeStructConstant (struct_constant *struc_co);

extern node *SSAConstantFolding (node *fundef, node *modul);

/* traversal functions */
extern node *SSACFfundef (node *arg_node, node *arg_info);
extern node *SSACFblock (node *arg_node, node *arg_info);
extern node *SSACFarg (node *arg_node, node *arg_info);
extern node *SSACFassign (node *arg_node, node *arg_info);
extern node *SSACFcond (node *arg_node, node *arg_info);
extern node *SSACFreturn (node *arg_node, node *arg_info);
extern node *SSACFlet (node *arg_node, node *arg_info);
extern node *SSACFap (node *arg_node, node *arg_info);
extern node *SSACFid (node *arg_node, node *arg_info);
extern node *SSACFarray (node *arg_node, node *arg_info);
extern node *SSACFprf (node *arg_node, node *arg_info);
extern node *SSACFNgen (node *arg_node, node *arg_info);

extern node *SSACFFoldPrfExpr (prf op, node **arg_expr);

#endif /* SAC_SSAConstantFolding_h */
