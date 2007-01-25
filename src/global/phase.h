/*
 *
 * $Id$
 *
 *
 */

#ifndef _SAC_PHASE_H_
#define _SAC_PHASE_H_

#include "types.h"

extern node *PHidentity (node *syntax_tree);
extern node *PHdummy (node *syntax_tree);

extern const char *PHphaseName (compiler_phase_t phase);
extern node *PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree, bool cond);

extern const char *PHsubPhaseName (compiler_subphase_t phase);
extern node *PHrunCompilerSubPhase (compiler_subphase_t phase, node *syntax_tree,
                                    bool cond);

extern void PHsetFirstOptimization (compiler_subphase_t subphase);
extern bool PHbreakAfterCurrentPass (int pass);
extern node *PHrunOptimizationInCycle (compiler_subphase_t phase, int pass,
                                       node *syntax_tree, bool cond);

#endif /* _SAC_PHASE_H_ */
