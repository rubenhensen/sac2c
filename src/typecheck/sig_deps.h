/*
 * $Log$
 * Revision 1.2  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.1  2002/08/05 16:58:35  sbs
 * Initial revision
 *
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

extern ntype *SDcreateSignatureDependency (ct_funptr CtFun, te_info *info, ntype *args);
extern bool SDhandleContradiction (sig_dep *fun_sig);
extern bool SDhandleElimination (sig_dep *fun_sig);

extern char *SDsigDep2DebugString (sig_dep *fun_sig);

#endif /* _SAC_SIG_DEPS_H_ */
