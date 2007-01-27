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

extern const char *PHsubPhaseName (compiler_subphase_t subphase);
extern node *PHrunCompilerSubPhase (compiler_subphase_t subphase, node *syntax_tree,
                                    bool cond);

extern const char *PHoptInCycName (compiler_optincyc_t optincyc);
extern node *PHrunOptimizationInCycle (compiler_optincyc_t optincyc, int pass,
                                       node *syntax_tree, bool cond);
extern node *PHrunOptimizationInCycleFun (compiler_optincyc_t optincyc, int pass,
                                          node *syntax_tree, bool cond);

extern void PHinterpretBreakOption (char *option);

#endif /* _SAC_PHASE_H_ */
