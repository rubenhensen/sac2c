#ifndef _SAC_TYPE_PATTERN_RESOLVE_FUNDEFS_H_
#define _SAC_TYPE_PATTERN_RESOLVE_FUNDEFS_H_

#include "types.h"

/******************************************************************************
 *
 * Resolve Type Pattern Fundefs
 *
 * Prefix: RTPF
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
 * Note that the implementations of type_pattern_resolve_fundefs.h, and
 * type_pattern_resolve_externals.h both live in type_pattern_resolve.c
 *
 ******************************************************************************/

extern node *RTPFdoResolveTypePatternFundefs (node *arg_node);

extern node *RTPFmodule (node *arg_node, info *arg_info);
extern node *RTPFfundef (node *arg_node, info *arg_info);
extern node *RTPFarg (node *arg_node, info *arg_info);
extern node *RTPFblock (node *arg_node, info *arg_info);
extern node *RTPFret (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_PATTERN_RESOLVE_FUNDEFS_H_ */
