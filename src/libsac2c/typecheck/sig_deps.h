/*
 * $Id$
 *
 */

#ifndef _SAC_SIG_DEPS_H_
#define _SAC_SIG_DEPS_H_

/*
 * The module "sig_deps" implements a new abstract datatype for representing
 * type signature dependencies. This file describes the interface to that module.
 *
 */

#include "types.h"

extern ntype *SDcreateSignatureDependency (ct_funptr CtFun, te_info *info, ntype *args,
                                           bool strict);
extern void SDfreeAllSignatureDependencies (void);
extern bool SDhandleContradiction (sig_dep *fun_sig);
extern bool SDhandleElimination (sig_dep *fun_sig);

extern char *SDsigDep2DebugString (sig_dep *fun_sig);

#endif /* _SAC_SIG_DEPS_H_ */
