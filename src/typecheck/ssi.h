/*
 *
 * $Log$
 * Revision 1.6  2005/07/26 16:04:22  sbs
 * added SSIfreeAllTvars
 *
 * Revision 1.5  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.4  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
 * Revision 1.3  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 1.2  2002/05/31 14:51:54  sbs
 * intermediate version to ensure compilable overall state.
 *
 * Revision 1.1  2002/03/12 15:16:29  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SSI_H_
#define _SAC_SSI_H_

#include "types.h"

extern tvar *SSImakeVariable ();
extern void SSIfreeAllTvars ();

extern bool SSInewMax (tvar *var, ntype *cmax);
extern bool SSInewMin (tvar *var, ntype *cmin);
extern bool SSInewRel (tvar *small, tvar *big);
extern bool SSInewTypeRel (ntype *small, ntype *big);

extern bool SSIinitAssumptionSystem (tvar_ass_handle_fun HandleContra,
                                     tvar_ass_handle_fun HandleFix);
extern bool SSIassumeLow (tvar *var, sig_dep *handle);
extern bool SSIfixLow (tvar *var);

extern bool SSIisFix (tvar *var);
extern bool SSIisLe (tvar *var1, tvar *var2);
extern ntype *SSIgetMax (tvar *var);
extern ntype *SSIgetMin (tvar *var);

extern char *SSIvariable2String (tvar *var);
extern char *SSIvariable2DebugString (tvar *var);

#endif /* _SAC_SSI_H_ */
