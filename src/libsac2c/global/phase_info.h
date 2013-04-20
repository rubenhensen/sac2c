#ifndef _PHASE_INFO_H_
#define _PHASE_INFO_H_

#include "types.h"

typedef node *(*phase_fun_t) (node *);

typedef enum {
    PHT_dummy,
    PHT_phase,
    PHT_subphase,
    PHT_cycle,
    PHT_cycle_fun,
    PHT_cyclephase,
    PHT_cyclephase_fun
} phase_type_t;

extern phase_fun_t PHIphaseFun (compiler_phase_t phase);
extern const char *PHIphaseText (compiler_phase_t phase);
extern phase_type_t PHIphaseType (compiler_phase_t phase);
extern const char *PHIphaseName (compiler_phase_t phase);
extern compiler_phase_t PHIphaseParent (compiler_phase_t phase);
extern const char *PHIphaseIdent (compiler_phase_t phase);
extern bool PHIisFunBased (compiler_phase_t phase);
extern compiler_phase_t PHIfirstPhase (void);
extern compiler_phase_t PHIlastPhase (void);

#endif
