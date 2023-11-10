#ifndef _SAC_FILTER_FUNDEF_CONDITIONS_H_
#define _SAC_FILTER_FUNDEF_CONDITIONS_H_

#include "types.h"

/******************************************************************************
 *
 * Filter Fundef Conditions
 *
 * Prefix: FFC
 *
 * Part of the type pattern pipeline:
 *   - filter_fundef_conditions
 *   - type_pattern_analyse
 *   - type_pattern_resolve_(fundefs,externals)
 *     - type_pattern_constraints
 *     - type_pattern_guard
 *   - strip_type_pattern_checks
 *   - type_pattern_statistics
 *
 ******************************************************************************/
extern node *FFCdoFilterFundefConditions (node *arg_node);

extern node *FFCfundef (node *arg_node, info *arg_info);
extern node *FFCret (node *arg_node, info *arg_info);
extern node *FFCarg (node *arg_node, info *arg_info);
extern node *FFCspap (node *arg_node, info *arg_info);
extern node *FFCspid (node *arg_node, info *arg_info);

#endif /* _SAC_FILTER_FUNDEF_CONDITIONS_H_ */
