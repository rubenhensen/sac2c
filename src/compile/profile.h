/*
 *
 * $Log$
 * Revision 1.1  1997/05/28 12:41:06  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_profile_h

#define _sac_profile_h

#define PF_MAXFUN 100
#define PF_MAXFUNAP 100
#define PF_MAXFUNNAMELEN 100

extern int PFfuncntr;                           /* defined in profile.c */
extern char *PFfunnme[PF_MAXFUN];               /* defined in profile.c */
extern int PFfunapcntr[PF_MAXFUN];              /* defined in profile.c */
extern int PFfunapline[PF_MAXFUN][PF_MAXFUNAP]; /* defined in profile.c */

extern void PFprintInitGlobals ();

#define NO_PROFILE 0x0000
#define PROFILE_FUN 0x0001
#define PROFILE_INL 0x0002
#define PROFILE_LIB 0x0004
#define PROFILE_WITH 0x0008
#define PROFILE_ALL 0xffff

#endif /* _sac_profile_h */
