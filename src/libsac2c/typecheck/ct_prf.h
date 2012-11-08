/*
 * $Id$
 */

#ifndef _SAC_CT_PRF_H_
#define _SAC_CT_PRF_H_

#include "types.h"

extern ntype *NTCCTprf_dummy (te_info *info, ntype *args);
extern ntype *NTCCTprf_id (te_info *info, ntype *args);
extern ntype *NTCCTprf_array (te_info *info, ntype *elems);
extern ntype *NTCCTprf_cast (te_info *info, ntype *elems);
extern ntype *NTCCTprf_type_conv (te_info *info, ntype *args);
extern ntype *NTCCTprf_enclose (te_info *info, ntype *args);
extern ntype *NTCCTprf_disclose (te_info *info, ntype *args);
extern ntype *NTCCTprf_dispatch_error (te_info *info, ntype *args);
extern ntype *NTCCTprf_guard (te_info *info, ntype *args);
extern ntype *NTCCTprf_guardhold (te_info *info, ntype *args);
extern ntype *NTCCTprf_afterguard (te_info *info, ntype *args);
extern ntype *NTCCTprf_noteminval (te_info *info, ntype *args);
extern ntype *NTCCTprf_notemaxval (te_info *info, ntype *args);
extern ntype *NTCCTprf_noteintersect (te_info *info, ntype *args);
extern ntype *NTCCTprf_type_constraint (te_info *info, ntype *args);
extern ntype *NTCCTprf_same_shape (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_dim (te_info *info, ntype *args);
extern ntype *NTCCTprf_non_neg_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_non_neg_V (te_info *info, ntype *args);
extern ntype *NTCCTprf_val_shape_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_val_shape_V (te_info *info, ntype *args);
extern ntype *NTCCTprf_val_le_val_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_val_lt_val_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_val_le_val_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_prod_shape (te_info *info, ntype *args);
extern ntype *NTCCTprf_nested_shape (te_info *info, ntype *args);
extern ntype *NTCCTprf_saabind (te_info *info, ntype *args);
extern ntype *NTCCTprf_dim_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_reshape_VxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_sel_VxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_sel_VxIA (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_vect2offset (te_info *info, ntype *args);
extern ntype *NTCCTprf_idxs2offset (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxSxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxVxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxVxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxSxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_hideValue_SxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_hideShape_SxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_hideDim_SxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_tob_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tos_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toi_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tol_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toll_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toub_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tous_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toui_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toul_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toull_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tof_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tod_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tobool_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_toc_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_VxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_V (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_rel_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_rel_op_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_rel_op_VxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_rel_op_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_VxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_V (te_info *info, ntype *args);
extern ntype *NTCCTprf_int_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_int_op_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_int_op_VxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_int_op_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_drop_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_take_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_cat_VxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_mask_VxVxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_mask_VxVxS (te_info *info, ntype *args);

#endif /* _SAC_CT_PRF_H_ */
