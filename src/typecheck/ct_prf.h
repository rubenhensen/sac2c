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
extern ntype *NTCCTprf_dim (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape (te_info *info, ntype *args);
extern ntype *NTCCTprf_reshape (te_info *info, ntype *args);
extern ntype *NTCCTprf_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarrayS (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarrayA (te_info *info, ntype *args);
extern ntype *NTCCTprf_toiS (te_info *info, ntype *args);
extern ntype *NTCCTprf_toiA (te_info *info, ntype *args);
extern ntype *NTCCTprf_tofS (te_info *info, ntype *args);
extern ntype *NTCCTprf_tofA (te_info *info, ntype *args);
extern ntype *NTCCTprf_todS (te_info *info, ntype *args);
extern ntype *NTCCTprf_todA (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_SxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_AxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_AxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_ari_op_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_rel_op_AxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_AxA (te_info *info, ntype *args);
extern ntype *NTCCTprf_log_op_A (te_info *info, ntype *args);
extern ntype *NTCCTprf_int_op_SxS (te_info *info, ntype *args);
extern ntype *NTCCTprf_drop_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_take_SxV (te_info *info, ntype *args);
extern ntype *NTCCTprf_cat_VxV (te_info *info, ntype *args);

#endif /* _SAC_CT_PRF_H_ */
