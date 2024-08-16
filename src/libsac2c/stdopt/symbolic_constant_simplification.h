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
/*
 * These functions seem to be helpers.... not sure they should be in this
 * module.
 */
extern bool SCSisSelOfShape (node *arg_node);
extern bool SCSisPositive (node *arg_node);
extern bool SCSisNegative (node *arg_node);
extern bool SCSisNonPositive (node *arg_node);
extern bool SCSisNonNegative (node *arg_node);
extern void SCSinitSymbolicConstantSimplification (void);
extern void SCSfinalizeSymbolicConstantSimplification (void);
extern node *SCSmakeTrue (node *prfarg);
extern node *SCSmakeFalse (node *prfarg);
extern node *SCSmakeZero (node *prfarg);
extern bool SCSisConstantZero (node *arg_node);
extern bool SCSisConstantNonZero (node *arg_node);
extern bool SCSisConstantOne (node *arg_node);
extern simpletype SCSgetBasetypeOfExpr (node *expr);
extern node *SCSrecurseWithExtrema (node *arg_node, info *arg_info, node *arg1,
                                    node *arg2, node *(*fun) (node *, info *));
extern node *SCSmakeVectorArray (shape *shp, node *scalarval);
extern bool SCSisRelationalOnMinMax (prf fun, node *arg1, node *arg2, info *arg_info);
extern bool SCScanOptGEOnDyadicFn (node *arg1, node *arg2, bool *res);
extern bool SCScanOptMAXOnDyadicFn (node *arg1, node *arg2, bool *res);
extern bool SCScanOptMINOnDyadicFn (node *arg1, node *arg2, bool *res);
extern bool SCSisMatchPrfargs (node *arg_node, info *arg_info);

/*
 * All functions below are triggered from CF through the function tables
 * defined in prf_info.mac
 */
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

/* SIMD operations.  */
extern node *SCSprf_add_SMxSM (node *arg_node, info *arg_info);
extern node *SCSprf_sub_SMxSM (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SMxSM (node *arg_node, info *arg_info);
extern node *SCSprf_div_SMxSM (node *arg_node, info *arg_info);
extern node *SCSprf_simd_sel_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_simd_sel_SxS (node *arg_node, info *arg_info);

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
extern node *SCSprf_mod_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_aplmod (node *arg_node, info *arg_info);
extern node *SCSprf_aplmod_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_eq_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_eq_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_eq_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_eq_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_neq_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_neq_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_neq_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_neq_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_lege (node *arg_node, info *arg_info);
extern node *SCSprf_nlege (node *arg_node, info *arg_info);
extern node *SCSprf_min_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_min_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_max_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_max_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_shape (node *arg_node, info *arg_info);
extern node *SCSprf_conditional_error (node *arg_node, info *arg_info);
extern node *SCSprf_guard (node *arg_node, info *arg_info);
extern node *SCSprf_noteminval (node *arg_node, info *arg_info);
extern node *SCSprf_notemaxval (node *arg_node, info *arg_info);
extern node *SCSprf_same_shape_AxA (node *arg_node, info *arg_info);
extern node *SCSprf_shape_matches_dim_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_non_neg_val_S (node *arg_node, info *arg_info);
extern node *SCSprf_non_neg_val_V (node *arg_node, info *arg_info);
extern node *SCSprf_val_lt_shape_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_val_lt_val_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_val_le_val_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_val_le_val_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_prod_matches_prod_shape_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_sel_VxA (node *arg_node, info *arg_info);
extern node *SCSprf_all_V (node *arg_node, info *arg_info);
extern node *SCSprf_sel_VxIA (node *arg_node, info *arg_info);
extern node *SCSprf_idx_shape_sel (node *arg_node, info *arg_info);
extern node *SCSprf_reshape (node *arg_node, info *arg_info);
extern node *SCSprf_neg_S (node *arg_node, info *arg_info);
extern node *SCSprf_neg_V (node *arg_node, info *arg_info);
extern node *SCSprf_reciproc_S (node *arg_node, info *arg_info);
extern node *SCSprf_reciproc_V (node *arg_node, info *arg_info);
extern node *SCSprf_abs_S (node *arg_node, info *arg_info);
extern node *SCSprf_abs_V (node *arg_node, info *arg_info);

#endif /* symbolic_constant_simplification.h */
