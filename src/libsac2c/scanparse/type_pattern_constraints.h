#ifndef _SAC_TYPE_PATTERN_CONSTRAINTS_H_
#define _SAC_TYPE_PATTERN_CONSTRAINTS_H_

#include "types.h"

/******************************************************************************
 *
 * Type Pattern Constraints
 *
 * Prefix: TPC
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

// Helper methods for N_spids chains
extern bool TPCtryAddSpid (node **spids, char *name);
extern bool TPChasMissing (node *spids, node *defined);
extern bool TPCremoveSpid (node **spids, char *name);

// Methods for generating error messages
extern char *TPCmakeNumFeatureError (node *feature, char *v, char *id, char *fundef, bool is_argument);
extern char *TPCmakeFeatureError (node *feature, char *v, char *id, char *fundef, bool is_argument);
extern char *TPCmakeNonNegativeError (node *feature, char *v, char *id, char *fundef, bool is_argument);
extern char *TPCmakeDimError (node *pattern, char *v, char *fundef, int fdim, bool is_argument);

// Methods for generating constraint expressions
extern node *TPCmakePrimitive (node *pattern, prf built_in, char *user_defined, node *arg);
extern node *TPCmakeDimSum (char *v, int fdim, node *vdim);
extern node *TPCmakeNumCheck (int num, node *expr);
extern node *TPCmakeNonNegativeCheck (char *id);
extern node *TPCmakeShapeSel (char *v, node *pattern, node *dim);
extern node *TPCmakeDimCalc (char *v, node *pattern, char *spid);
extern node *TPCmakeTakeDrop (char *v, node *pattern, node *nid, node *dim);

#endif /* _SAC_TYPE_PATTERN_CONSTRAINTS_H_ */
