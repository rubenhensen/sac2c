/*
 * $Id: symbolic_constant_simplification.h 15176 2007-01-29 12:14:40Z cg $
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
extern node *SCSprf_toi_S (node *arg_node, info *arg_info);
extern node *SCSprf_tof_S (node *arg_node, info *arg_info);
extern node *SCSprf_tod_S (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_add_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_add_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_sub_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_sub_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_sub_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_sub_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_mul_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_div_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_div_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_div_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_div_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_not_S (node *arg_node, info *arg_info);
extern node *SCSprf_not_V (node *arg_node, info *arg_info);
extern node *SCSprf_and_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_and_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_and_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_and_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_or_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_or_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_or_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_or_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_mod_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_mod_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_mod_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_mod_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_eq_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_eq_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_eq_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_eq_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_neq_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_neq_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_neq_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_neq_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_le_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_le_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_le_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_le_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_ge_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_ge_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_ge_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_ge_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_min_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_min_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_min_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_min_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_max_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_max_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_max_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_max_VxV (node *arg_node, info *arg_info);

#endif /* symbolic_constant_simplification.h */
