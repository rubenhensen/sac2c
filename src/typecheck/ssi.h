/*
 *
 * $Log$
 * Revision 1.1  2002/03/12 15:16:29  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _ssi_h
#define _ssi_h

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

typedef struct TVAR tvar;

extern tvar *SSIMakeVariable ();

extern bool SSINewMax (tvar *var, ntype *cmax);
extern bool SSINewMin (tvar *var, ntype *cmin);
extern bool SSINewRel (tvar *small, tvar *big);

extern int SSIAssume (tvar *var, ntype *val);
extern void SSIKillAssumption (int assumption);

extern bool SSIIsFix (tvar *var);
extern ntype *SSIGetMax (tvar *var);
extern ntype *SSIGetMin (tvar *var);

extern char *SSIVariable2DebugString (tvar *var);
#endif /* _ssi_h */
