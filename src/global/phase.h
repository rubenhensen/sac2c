/*
 *
 * $Log$
 * Revision 1.2  2005/09/04 12:49:35  ktr
 * added new global optimization counters and made all optimizations proper subphases
 *
 * Revision 1.1  2005/03/07 13:40:34  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_PHASE_H_
#define _SAC_PHASE_H_

#include "types.h"

extern const char *PHphaseName (compiler_phase_t phase);
extern node *PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree);

extern const char *PHsubPhaseName (compiler_subphase_t phase);
extern node *PHrunCompilerSubPhase (compiler_subphase_t phase, node *syntax_tree);

extern void PHsetFirstOptimization (compiler_subphase_t subphase);
extern bool PHbreakAfterCurrentPass (int pass);
extern node *PHrunOptimizationInCycle (compiler_subphase_t phase, int pass,
                                       node *syntax_tree);

#endif /* _SAC_PHASE_H_ */
