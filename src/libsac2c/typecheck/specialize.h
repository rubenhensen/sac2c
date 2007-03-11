/*
 * $Id$
 *
 */

#ifndef _SAC_SPECIALIZE_H_
#define _SAC_SPECIALIZE_H_

#include "types.h"

extern dft_res *SPEChandleDownProjections (dft_res *dft, node *wrapper, ntype *args,
                                           ntype *rets);
extern node *SPEChandleLacFun (node *fundef, node *assign, ntype *args);
extern node *SPECresetSpecChain (void);
extern void SPECinitSpecChain (void);

#endif /* _SAC_SPECIALIZE_H_ */
