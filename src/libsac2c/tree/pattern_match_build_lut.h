/*
 * $Id: trav_template.h 15657 2007-11-13 13:57:30Z cg $
 */
#ifndef _SAC_PATTERN_MATCHING_BUILD_LUT_H_
#define _SAC_PATTERN_MATCHING_BUILD_LUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Pattern Matching Build Lut ( pmbl_tab)
 *
 * Prefix: PMBL
 *
 *****************************************************************************/

extern lut_t *PMBLdoBuildPatternMatchingLut (node *fundef, pm_mode_t pmmode);

extern node *PMBLfundef (node *arg_node, info *arg_info);
extern node *PMBLap (node *arg_node, info *arg_info);
extern node *PMBLarg (node *arg_node, info *arg_info);

#endif /* _SAC_PATTERN_MATCHING_BUILD_LUT_H_ */
