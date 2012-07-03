/*
 * $Id$
 */

#include "types.h"

#ifndef _PHASE_OPTIONS_H_
#define _PHASE_OPTIONS_H_

/*
 * enum to pass to InterpretPrintOptionPhase to detirmine which global phase
 * to assign.
 */
enum phase_mode_t { START, STOP };

void InterpretPrintOptionPhase (char *option, enum phase_mode_t mode);
void CheckStartStopPhase (void);

extern void PHOinterpretStartPhase (char *option);
extern void PHOinterpretStopPhase (char *option);
extern void PHOinterpretPrintPhaseFunOption (char *option);

extern void PHOinterpretBreakFunName (char *option);
extern void PHOinterpretBreakOption (char *option);
extern void PHOinterpretDbugOption (char *option);
extern void PHOprintPhasesSac2c (void);
extern void PHOprintPhasesSac4c (void);

#endif //_PHASE_OPTIONS_H_
