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
 *   - type_pattern_statistics
 *
 ******************************************************************************/

#define GTP_PRED_NAME "_gtp_pred"

// Helper methods for generating type pattern functions
extern char *GTPmakeValidFundefName (const char *guard_str, char *name);

// Methods for generating type pattern functions
extern node *GTPmakePreCheck (node *fundef, node *assigns, node *checks);
extern node *GTPmakePostCheck (node *fundef, node *assigns, node *checks, node *return_ids);
extern node *GTPmodifyFundef (node *fundef, node *impl, node *pre, node *post);
extern node *GTPmakeImpl (node *fundef);

#endif /* _SAC_TYPE_PATTERN_GUARD_H_ */
