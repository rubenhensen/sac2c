/*
 *
 * $Log$
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

#endif /* _SAC_PHASE_H_ */
