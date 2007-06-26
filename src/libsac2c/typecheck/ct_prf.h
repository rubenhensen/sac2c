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
extern ntype *NTCCTprf_saabind (te_info *info, ntype *args);
extern ntype *NTCCTprf_dim_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_reshape_VxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_sel_VxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxVxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarray_AxVxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_toi_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tof_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tod_S (te_info *info, ntype *args);
extern ntype *NTCCTprf_tob_S (te_info *info, ntype *args);
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

#endif /* _SAC_CT_PRF_H_ */
