/*
 * $Log$
 * Revision 1.1  2002/08/05 16:58:35  sbs
 * Initial revision
 *
 *
 */

#ifndef _sig_deps_h
#define _sig_deps_h

/*
 * The module "sig_deps" implements a new abstract datatype for representing
 * type signature dependencies. This file describes the interface to that module.
 *
 */

typedef struct SIG_DEP sig_dep;

#include "new_types.h"
#include "types.h"
#include "type_errors.h"

typedef ntype *(*ct_funptr) (te_info *, ntype *);

extern ntype *SDCreateSignatureDependency (ct_funptr CtFun, te_info *info, ntype *args);
extern bool SDHandleContradiction (sig_dep *fun_sig);
extern bool SDHandleElimination (sig_dep *fun_sig);

extern char *SDSigDep2DebugString (sig_dep *fun_sig);

#endif /* _sig_deps_h */
