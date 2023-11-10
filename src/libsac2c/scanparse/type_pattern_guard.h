#ifndef _SAC_TYPE_PATTERN_GUARD_H_
#define _SAC_TYPE_PATTERN_GUARD_H_

#include "types.h"

/******************************************************************************
 *
 * Type Pattern Guard
 *
 * Prefix: GTP
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

// Helper methods for generating type pattern functions
extern char *GTPmakeValidFundefName (char *guard_str, char *name);

// Methods for generating type pattern functions
extern node *GTPmakePreCheck (node *fundef, char *pred, node *assigns, node *checks);
extern node *GTPmakePostCheck (node *fundef, char *pred, node *assigns, node *checks, node *return_ids);
extern node *GTPmodifyFundef (node *fundef, char *pred, node *impl, node *pre, node *post);
extern node *GTPmakeImpl (node *fundef);

#endif /* _SAC_TYPE_PATTERN_GUARD_H_ */
