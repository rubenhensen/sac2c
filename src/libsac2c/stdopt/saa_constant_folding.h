/*
 * $Id: saa_constantfolding.h 15176 2007-01-29 12:14:40Z cg $
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
extern node *SAACFprf_modarray (node *arg_node, info *arg_info);

#endif /* _SAC_saa_constantfolding_h_ */
