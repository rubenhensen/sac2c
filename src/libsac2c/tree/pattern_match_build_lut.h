/*
 * $Id$
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
