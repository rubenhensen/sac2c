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

extern const char *PHphaseText (compiler_phase_t phase);

extern node *PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree, bool cond);

extern node *PHrunCompilerSubPhase (compiler_phase_t subphase, node *syntax_tree,
                                    bool cond);

extern node *PHrunCompilerCyclePhase (compiler_phase_t cyclephase, int pass,
                                      node *syntax_tree, bool cond, bool funbased);

extern void PHinterpretBreakOption (char *option);

#endif /* _SAC_PHASE_H_ */
