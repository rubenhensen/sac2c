
#ifndef _SAC_PATTERN_MATCH_MODES_H_
#define _SAC_PATTERN_MATCH_MODES_H_

#include "types.h"

/**
 *
 * pre-defined skipping functions of type ( skip_fun_t * ) :
 */
extern node *PMMskipId (intptr_t param, node *expr);
/*
 * expects (lut_t*) as parameter
 */

extern node *PMMskipPrf (intptr_t param, node *expr);
/*
 * expects (prf_match_fun_t *) as parameter
 * pre-defined parameters:
 */
extern bool PMMisInExtrema (prf prfun);
extern bool PMMisInGuards (prf prfun);
extern bool PMMisInExtremaOrGuards (prf prfun);
extern bool PMMisGuard (prf prfun);

/**
 *
 * statically pre-defined modes:
 */
extern pm_mode_t *PMMexact (void);
extern pm_mode_t *PMMflat (void);
extern pm_mode_t *PMMflatLut (lut_t *f_lut);
extern pm_mode_t *PMMflatPrf (prf_match_fun_t *prfInspectFun);
extern pm_mode_t *PMMflatPrfLut (prf_match_fun_t *prfInspectFun, lut_t *f_lut);

#endif /* _SAC_PATTERN_MATCH_MODES_H_ */
