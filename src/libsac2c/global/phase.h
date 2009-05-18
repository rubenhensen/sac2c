/*
 *
 * $Id$
 *
 *
 */

#ifndef _SAC_PHASE_H_
#define _SAC_PHASE_H_

#include "types.h"

extern node *PHrunPhase (compiler_phase_t phase, node *syntax_tree, bool cond);

extern node *PHrunSubPhase (compiler_phase_t subphase, node *syntax_tree, bool cond);

extern node *PHrunCycle (compiler_phase_t cycle, node *syntax_tree, bool cond);

extern node *PHrunCyclePhase (compiler_phase_t cyclephase, node *syntax_tree, bool cond);

extern node *PHrunCycleFun (compiler_phase_t cycle, node *syntax_tree);

extern node *PHrunCyclePhaseFun (compiler_phase_t cyclephase, node *fundef, bool cond);

extern bool isSAAMode ();

#endif /* _SAC_PHASE_H_ */
