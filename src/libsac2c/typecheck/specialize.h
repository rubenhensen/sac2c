/*
 * $Log$
 * Revision 1.4  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 1.3  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.2  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.1  2002/08/05 16:58:38  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_SPECIALIZE_H_
#define _SAC_SPECIALIZE_H_

#include "types.h"

extern dft_res *SPEChandleDownProjections (dft_res *dft, node *wrapper, ntype *args,
                                           ntype *rets);
extern node *SPEChandleLacFun (node *fundef, node *assign, ntype *args);
extern node *SPECresetSpecChain ();

#endif /* _SAC_SPECIALIZE_H_ */
