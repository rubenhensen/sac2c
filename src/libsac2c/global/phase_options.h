/*
 * $Id$
 */

#ifndef _PHASE_OPTIONS_H_
#define _PHASE_OPTIONS_H_

void InterpretPrintPhaseFunStartOptions (char *option);

extern void PHOinterpretBreakFunName (char *option);
extern void PHOinterpretPrintPhaseFunOption (char *option);
extern void PHOinterpretBreakOption (char *option);
extern void PHOinterpretDbugOption (char *option);
extern void PHOprintPhasesSac2c (void);
extern void PHOprintPhasesSac4c (void);

#endif
