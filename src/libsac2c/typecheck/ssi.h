#ifndef _SAC_SSI_H_
#define _SAC_SSI_H_

#include "types.h"

extern tvar *SSImakeVariable (void);
extern void SSIfreeAllTvars (void);

extern bool SSInewMax (tvar *var, ntype *cmax);
extern bool SSInewMin (tvar *var, ntype *cmin);
extern bool SSInewRel (tvar *small, tvar *big);
extern bool SSInewTypeRel (ntype *small, ntype *big);

extern bool SSIinitAssumptionSystem (tvar_ass_handle_fun HandleContra,
                                     tvar_ass_handle_fun HandleFix);
extern bool SSIassumptionSystemIsInitialized (void);

extern bool SSIassumeLow (tvar *var, sig_dep *handle);
extern bool SSIfixLow (tvar *var);

extern bool SSIisFix (tvar *var);
extern bool SSIisLe (tvar *var1, tvar *var2);
extern ntype *SSIgetMax (tvar *var);
extern ntype *SSIgetMin (tvar *var);

extern char *SSIvariable2String (tvar *var);
extern char *SSIvariable2DebugString (tvar *var);

#endif /* _SAC_SSI_H_ */
