/*
 * $Id$
 */
#ifndef _SAC_saa_constantfolding_h_
#define _SAC_saa_constantfolding_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   saaconstantfolding.h
 *
 * prefix: SAACF
 *
 *****************************************************************************/

extern node *SAACF_ids (node *arg_node, info *arg_info);

extern node *SAACFprf_shape (node *arg_node, info *arg_info);
extern node *SAACFprf_reshape (node *arg_node, info *arg_info);
extern node *SAACFprf_dim (node *arg_node, info *arg_info);
extern node *SAACFprf_idx_shape_sel (node *arg_node, info *arg_info);
extern node *SAACFprf_take_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_drop_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_same_shape_AxA (node *arg_node, info *arg_info);
extern node *SAACFprf_shape_matches_dim (node *arg_node, info *arg_info);
extern node *SAACFprf_non_neg_val_V (node *arg_node, info *arg_info);
extern node *SAACFprf_val_lt_shape_VxA (node *arg_node, info *arg_info);

extern node *SAACFprf_lt_SxS (node *arg_node, info *arg_info);
extern node *SAACFprf_lt_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_lt_VxS (node *arg_node, info *arg_info);
extern node *SAACFprf_lt_VxV (node *arg_node, info *arg_info);

extern node *SAACFprf_le_SxS (node *arg_node, info *arg_info);
extern node *SAACFprf_le_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_le_VxS (node *arg_node, info *arg_info);
extern node *SAACFprf_le_VxV (node *arg_node, info *arg_info);

extern node *SAACFprf_ge_SxS (node *arg_node, info *arg_info);
extern node *SAACFprf_ge_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_ge_VxS (node *arg_node, info *arg_info);
extern node *SAACFprf_ge_VxV (node *arg_node, info *arg_info);

extern node *SAACFprf_gt_SxS (node *arg_node, info *arg_info);
extern node *SAACFprf_gt_SxV (node *arg_node, info *arg_info);
extern node *SAACFprf_gt_VxS (node *arg_node, info *arg_info);
extern node *SAACFprf_gt_VxV (node *arg_node, info *arg_info);

#endif /* _SAC_saa_constantfolding_h_ */
