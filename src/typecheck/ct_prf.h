/*
 * $Log$
 * Revision 1.13  2005/08/29 16:43:03  ktr
 * added support for prfs F_idx_sel, F_shape_sel, F_idx_shape_sel
 *
 * Revision 1.12  2005/08/19 17:27:33  sbs
 * added NTCCTprf_type_conv
 *
 * Revision 1.11  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.10  2004/02/27 11:49:15  sbs
 * NTCPRF_phi deleted
 *
 * Revision 1.9  2004/02/03 11:25:44  sbs
 * NTCPRF_phi added.
 *
 * Revision 1.8  2003/09/10 09:42:35  sbs
 * NTCPRF_drop_SxV improved /
 * NTCPRF_take_SxV added.
 *
 * Revision 1.7  2003/04/09 15:35:57  sbs
 * NTCPRF_toiS, NTCPRF_toiA, NTCPRF_tofS, NTCPRF_tofA, NTCPRF_todS, NTCPRF_todA,
 * NTCPRF_ari_op_A, NTCPRF_log_op_A added.
 *
 * Revision 1.6  2003/04/07 14:34:19  sbs
 * NTCPRF_cffuntab added .
 *
 * Revision 1.5  2003/03/19 10:34:30  sbs
 * NTCPRF_drop_SxV and NTCPRF_cat_VxV added.
 *
 * Revision 1.4  2002/10/28 14:04:15  sbs
 * NTCPRF_cast added
 *
 * Revision 1.3  2002/09/04 12:59:46  sbs
 * type checking of arrays changed; now sig deps will be created as well.
 *
 * Revision 1.2  2002/08/07 09:49:58  sbs
 * modulo added
 *
 * Revision 1.1  2002/08/05 16:57:52  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CT_PRF_H_
#define _SAC_CT_PRF_H_

#include "types.h"

extern ntype *NTCCTprf_dummy (te_info *info, ntype *args);
extern ntype *NTCCTprf_array (te_info *info, ntype *elems);
extern ntype *NTCCTprf_cast (te_info *info, ntype *elems);
extern ntype *NTCCTprf_type_conv (te_info *info, ntype *elems);
extern ntype *NTCCTprf_dim (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape (te_info *info, ntype *args);
extern ntype *NTCCTprf_reshape (te_info *info, ntype *args);
extern ntype *NTCCTprf_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_selS (te_info *info, ntype *args);
extern ntype *NTCCTprf_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_idx_shape_sel (te_info *info, ntype *args);
extern ntype *NTCCTprf_modarrayS (te_info *info, ntype *args);
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
