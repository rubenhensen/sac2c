/*
 *
 * $Log$
 * Revision 1.2  1997/05/16 09:52:19  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.1  1997/05/14  08:26:39  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_analyse_h

#define _sac_analyse_h

#define AT_MAXFUN 100
#define AT_MAXFUNAP 100
#define AT_MAXFUNNAMELEN 100

extern int ATfuncntr;                           /* defined in analyse.c */
extern char *ATfunnme[AT_MAXFUN];               /* defined in analyse.c */
extern int ATfunapcntr[AT_MAXFUN];              /* defined in analyse.c */
extern int ATfunapline[AT_MAXFUN][AT_MAXFUNAP]; /* defined in analyse.c */

extern void ATprintInitGlobals ();

#define NO_ANALYSE 0x0000
#define ANALYSE_TIME 0x0001
#define ANALYSE_ALL 0xffff

#endif /* _sac_analyse_h */
