/*
 * $Id$
 */

#ifndef _PHASE_OPTIONS_H_
#define _PHASE_OPTIONS_H_

void InterpretPrintOptionPhase (char *option, int isstart);

extern void PHOinterpretStartPhase (char *option);
extern void PHOinterpretStopPhase (char *option);
extern void PHOinterpretPrintPhaseFunOption (char *option);

extern void PHOinterpretBreakFunName (char *option);
extern void PHOinterpretBreakOption (char *option);
extern void PHOinterpretDbugOption (char *option);
extern void PHOprintPhasesSac2c (void);
extern void PHOprintPhasesSac4c (void);

#endif
