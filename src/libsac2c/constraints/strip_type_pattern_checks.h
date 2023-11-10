#ifndef _SAC_STRIP_TYPE_PATTERN_CHECKS_H_
#define _SAC_STRIP_TYPE_PATTERN_CHECKS_H_

#include "types.h"

/******************************************************************************
 *
 * Strip Type Pattern Checks
 *
 * Prefix: STPC
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

extern node *STPCdoStripTypePatternChecks (node *arg_node);

extern node *STPCblock (node *arg_node, info *arg_info);
extern node *STPCassign (node *arg_node, info *arg_info);
extern node *STPClet (node *arg_node, info *arg_info);
extern node *STPCprf (node *arg_node, info *arg_info);
extern node *STPCid (node *arg_node, info *arg_info);
extern node *STPCvardec (node *arg_node, info *arg_info);

#endif /* _SAC_STRIP_TYPE_PATTERN_CHECKS_H_ */
