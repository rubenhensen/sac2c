/*
 * $Id$
 */
#ifndef _SAC_ConstantFolding_h_
#define _SAC_ConstantFolding_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   SSAConstantFolding.h
 *
 * prefix: SSACF
 *
 *****************************************************************************/
extern node *CFdoConstantFolding (node *fundef);
extern node *CFdoConstantFoldingModule (node *syntax_tree);

extern node *CFfundef (node *arg_node, info *arg_info);
extern node *CFblock (node *arg_node, info *arg_info);
extern node *CFassign (node *arg_node, info *arg_info);
extern node *CFcond (node *arg_node, info *arg_info);
extern node *CFlet (node *arg_node, info *arg_info);
extern node *CFids (node *arg_node, info *arg_info);
extern node *CFarray (node *arg_node, info *arg_info);
extern node *CFprf (node *arg_node, info *arg_info);
extern node *CFwith (node *arg_node, info *arg_info);
extern node *CFpart (node *arg_node, info *arg_info);
extern node *CFcode (node *arg_node, info *arg_info);
extern node *CFfuncond (node *arg_node, info *arg_info);

/** <!--********************************************************************-->
 *
 * functions to handle SCOs
 *
 *****************************************************************************/
extern struct_constant *CFscoExpr2StructConstant (node *expr);
extern node *CFscoDupStructConstant2Expr (struct_constant *struc_co);
extern struct_constant *CFscoFreeStructConstant (struct_constant *struc_co);

#endif /* SAC_ConstantFolding_h */