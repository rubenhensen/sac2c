/*
 *
 * $Id$
 *
 */

#ifndef _SAC_PHASE_DRIVERS_H_
#define _SAC_PHASE_DRIVERS_H_

#include "types.h"

/*
 * The prototypes of phase driver functions are derived from phase_info.mac.
 */

#define PHASEfun(it_fun) extern node *it_fun (node *syntax_tree);

#include "phase_info.mac"

#undef SUBPHASEelement

#endif /* _SAC_PHASE_DRIVERS_H_ */
