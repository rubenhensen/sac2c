#ifndef _SAC_TYPE_PATTERN_ANALYSE_H_
#define _SAC_TYPE_PATTERN_ANALYSE_H_

#include "types.h"

/******************************************************************************
 *
 * Analyse Type Pattern
 *
 * Prefix: ATP
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

extern node *ATPdoAnalyseTypePattern (node *arg_node);

extern node *ATPfundef (node *arg_node, info *arg_info);
extern node *ATPtypedef (node *arg_node, info *arg_info);
extern node *ATPstructelem (node *arg_node, info *arg_info);
extern node *ATPobjdef (node *arg_node, info *arg_info);
extern node *ATParray (node *arg_node, info *arg_info);
extern node *ATPcast (node *arg_node, info *arg_info);
extern node *ATPret (node *arg_node, info *arg_info);
extern node *ATPavis (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_PATTERN_ANALYSE_H_ */
