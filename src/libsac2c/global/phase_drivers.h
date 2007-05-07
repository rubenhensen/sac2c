/*
 *
 * $Id$
 *
 */

#ifndef _SAC_PHASE_DRIVERS_H_
#define _SAC_PHASE_DRIVERS_H_

#include "types.h"

extern node *PHDdriveSac2c (node *syntax_tree);
extern node *PHDdriveSac4c (node *syntax_tree);

#define PHASEname(name) extern node *PHDdrivePhase_##name (node *syntax_tree);
#define CYCLEname(name) extern node *PHDdriveCycle_##name (node *syntax_tree);
#define FUNBEGINname(name) extern node *PHDdriveCycleFun_##name (node *fundef);

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"

#undef PHASEname
#undef CYCLEname
#undef FUNBEGINname

#endif /* _SAC_PHASE_DRIVERS_H_ */
