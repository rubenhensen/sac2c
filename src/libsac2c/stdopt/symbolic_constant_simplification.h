/*
 * $Id$
 */
#ifndef _SAC_symbolic_constant_simplification_h_
#define _SAC_symbolic_constant_simplification_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   symbolic_constant_simplification.h
 *
 * prefix: SCS
 *
 *****************************************************************************/
extern void SCSinitSymbolicConstantSimplification ();
extern void SCSfinalizeSymbolicConstantSimplification ();
extern node *MakeTrue (node *prfarg);
extern node *MakeFalse (node *prfarg);
extern simpletype GetBasetypeOfExpr (node *expr);

extern node *SCSprf_tobool_S (node *arg_node, info *arg_info);
extern node *SCSprf_toc_S (node *arg_node, info *arg_info);
extern node *SCSprf_tob_S (node *arg_node, info *arg_info);
extern node *SCSprf_tos_S (node *arg_node, info *arg_info);
extern node *SCSprf_toi_S (node *arg_node, info *arg_info);
extern node *SCSprf_tol_S (node *arg_node, info *arg_info);
extern node *SCSprf_toll_S (node *arg_node, info *arg_info);
extern node *SCSprf_toub_S (node *arg_node, info *arg_info);
extern node *SCSprf_tous_S (node *arg_node, info *arg_info);
extern node *SCSprf_toui_S (node *arg_node, info *arg_info);
extern node *SCSprf_toul_S (node *arg_node, info *arg_info);
extern node *SCSprf_toull_S (node *arg_node, info *arg_info);
extern node *SCSprf_tof_S (node *arg_node, info *arg_info);
extern node *SCSprf_tod_S (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_add_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_add_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_sub (node *arg_node, info *arg_info);
extern node *SCSprf_sub_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_sub_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_mul_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_div_XxS (node *arg_node, info *arg_info);
extern node *SCSprf_div_SxX (node *arg_node, info *arg_info);
extern node *SCSprf_div_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_not (node *arg_node, info *arg_info);
extern node *SCSprf_and_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_and_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_and_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_and_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_or_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_or_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_or_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_or_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_mod (node *arg_node, info *arg_info);
extern node *SCSprf_lege (node *arg_node, info *arg_info);
extern node *SCSprf_nlege (node *arg_node, info *arg_info);
extern node *SCSprf_minmax (node *arg_node, info *arg_info);
extern node *SCSprf_shape (node *arg_node, info *arg_info);
extern node *SCSprf_guard (node *arg_node, info *arg_info);
extern node *SCSprf_afterguard (node *arg_node, info *arg_info);
extern node *SCSprf_same_shape_AxA (node *arg_node, info *arg_info);
extern node *SCSprf_shape_matches_dim_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_non_neg_val_V (node *arg_node, info *arg_info);
extern node *SCSprf_val_lt_shape_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_val_le_val_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_prod_matches_prod_shape_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_sel_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_idx_shape_sel (node *arg_node, info *arg_info);
extern node *SCSprf_reshape (node *arg_node, info *arg_info);
extern node *SCSprf_neg_S (node *arg_node, info *arg_info);
extern node *SCSprf_neg_V (node *arg_node, info *arg_info);
extern node *SCSprf_reciproc_S (node *arg_node, info *arg_info);
extern node *SCSprf_reciproc_V (node *arg_node, info *arg_info);

#endif /* symbolic_constant_simplification.h */
