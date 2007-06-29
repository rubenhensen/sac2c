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
extern node *SCSprf_tob_S (node *arg_node, info *arg_info);
extern node *SCSprf_toc_S (node *arg_node, info *arg_info);
extern node *SCSprf_toi_S (node *arg_node, info *arg_info);
extern node *SCSprf_tof_S (node *arg_node, info *arg_info);
extern node *SCSprf_tod_S (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_add_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_add_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_sub (node *arg_node, info *arg_info);
extern node *SCSprf_sub_VxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxS (node *arg_node, info *arg_info);
extern node *SCSprf_mul_SxV (node *arg_node, info *arg_info);
extern node *SCSprf_mul_VxS (node *arg_node, info *arg_info);
extern node *SCSprf_div (node *arg_node, info *arg_info);
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

#endif /* symbolic_constant_simplification.h */