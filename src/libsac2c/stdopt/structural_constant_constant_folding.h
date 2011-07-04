/*
 * $Id$
 */
#ifndef _SAC_structural_constant_constant_folding_h_
#define _SAC_structural_constant_constant_folding_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   structural_constant_constant_folding.h
 *
 * prefix: SCCF
 *
 *****************************************************************************/

extern node *SCCFprf_reshape (node *arg_node, info *arg_info);
extern node *SCCFprf_sel (node *arg_node, info *arg_info);
extern node *SCCFprf_idx_sel (node *arg_node, info *arg_info);
extern node *SCCFprf_modarray_AxVxS (node *arg_node, info *arg_info);
extern node *SCCFprf_modarray_AxVxA (node *arg_node, info *arg_info);
extern node *SCCFprf_idx_modarray (node *arg_node, info *arg_info);
extern node *SCCFprf_cat_VxV (node *arg_node, info *arg_info);
extern node *SCCFprf_take_SxV (node *arg_node, info *arg_info);
extern node *SCCFprf_drop_SxV (node *arg_node, info *arg_info);
extern node *SCCFprf_idx_shape_sel (node *arg_node, info *arg_info);
extern node *SCCFprf_mesh_VxVxV (node *arg_node, info *arg_info);
extern node *SCCFprf_mask_VxVxV (node *arg_node, info *arg_info);

extern struct_constant *SCCFfreeStructConstant (struct_constant *struc_co);
extern struct_constant *SCCFexpr2StructConstant (node *expr);
extern node *SCCFdupStructConstant2Expr (struct_constant *struc_co);

#endif /* structural_constant_constant_folding_h_ */
