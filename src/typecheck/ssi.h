/*
 *
 * $Log$
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

#ifndef _ssi_h
#define _ssi_h

#include "new_types.h"
#include "sig_deps.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

typedef struct TVAR tvar;
typedef bool (*tvar_ass_handle_fun) (sig_dep *handle);

extern tvar *SSIMakeVariable ();

extern bool SSINewMax (tvar *var, ntype *cmax);
extern bool SSINewMin (tvar *var, ntype *cmin);
extern bool SSINewRel (tvar *small, tvar *big);
extern bool SSINewTypeRel (ntype *small, ntype *big);

extern bool SSIInitAssumptionSystem (tvar_ass_handle_fun HandleContra,
                                     tvar_ass_handle_fun HandleFix);
extern bool SSIAssumeLow (tvar *var, sig_dep *handle);
extern bool SSIFixLow (tvar *var);

extern bool SSIIsFix (tvar *var);
extern bool SSIIsLe (tvar *var1, tvar *var2);
extern ntype *SSIGetMax (tvar *var);
extern ntype *SSIGetMin (tvar *var);

extern char *SSIVariable2String (tvar *var);
extern char *SSIVariable2DebugString (tvar *var);
#endif /* _ssi_h */
