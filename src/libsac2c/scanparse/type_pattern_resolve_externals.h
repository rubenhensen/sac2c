#ifndef _SAC_TYPE_PATTERN_RESOLVE_EXTERNALS_H_
#define _SAC_TYPE_PATTERN_RESOLVE_EXTERNALS_H_

#include "types.h"

/******************************************************************************
 *
 * Resolve External Type Patterns
 *
 * Prefix: RTPE
 *
 * Part of the type pattern pipeline:
 *   - filter_fundef_conditions
 *   - type_pattern_analyse
 *   - type_pattern_resolve_(fundefs,externals)
 *     - type_pattern_constraints
 *     - type_pattern_guard
 *   - type_pattern_statistics
 *
 * Note that the implementations of type_pattern_resolve_fundefs.h, and
 * type_pattern_resolve_externals.h both live in type_pattern_resolve.c
 *
 ******************************************************************************/

extern node *RTPEdoResolveTypePatternExternals (node *arg_node);

extern node *RTPEmodule (node *arg_node, info *arg_info);
extern node *RTPEfundef (node *arg_node, info *arg_info);
extern node *RTPEarg (node *arg_node, info *arg_info);
extern node *RTPEret (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_PATTERN_RESOLVE_EXTERNALS_H_ */
