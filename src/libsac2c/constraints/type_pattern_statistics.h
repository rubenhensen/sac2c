#ifndef _SAC_TYPE_PATTERN_STATISTICS_H_
#define _SAC_TYPE_PATTERN_STATISTICS_H_

#include "types.h"

/******************************************************************************
 *
 * Type Pattern Statistics
 *
 * Prefix: TPS
 *
 * Part of the type pattern pipeline:
 *   - filter_fundef_conditions
 *   - type_pattern_analyse
 *   - type_pattern_resolve_(fundefs,externals)
 *     - type_pattern_constraints
 *     - type_pattern_guard
 *   - type_pattern_statistics
 *
 ******************************************************************************/

extern node *TPSdoPrintTypePatternStatistics (node *arg_node);

extern node *TPSprf (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_PATTERN_STATISTICS_H_ */
