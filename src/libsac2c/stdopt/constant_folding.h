/*
 * $Id: constantfolding.h 15176 2007-01-29 12:14:40Z cg $
 */
#ifndef _SAC_constantfolding_h_
#define _SAC_constantfolding_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   constantfolding.h
 *
 * prefix: CF
 *
 *****************************************************************************/
extern node *CFdoConstantFolding (node *fundef);

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

extern node *CFprf_dim (node *arg_node, info *arg_info);
extern node *CFprf_shape (node *arg_node, info *arg_info);
extern node *CFprf_reshape (node *arg_node, info *arg_info);

/** <!--********************************************************************-->
 *
 * functions to handle SCOs
 *
 *****************************************************************************/
extern struct_constant *CFscoExpr2StructConstant (node *expr);
extern node *CFscoDupStructConstant2Expr (struct_constant *struc_co);
extern struct_constant *CFscoFreeStructConstant (struct_constant *struc_co);

#endif /* _SAC_constantfolding_h_ */
