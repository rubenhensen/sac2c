/*
 * $Log$
 * Revision 1.2  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.1  2002/08/05 16:58:38  sbs
 * Initial revision
 *
 *
 */

#ifndef _specialize_h
#define _specialize_h

#include "types.h"
#include "new_types.h"

extern DFT_res *SPECHandleDownProjections (DFT_res *dft, node *wrapper, ntype *args);
extern node *SPECHandleLacFun (node *fundef, node *assign, ntype *args);
extern node *SPECResetSpecChain ();

#endif /* _specialize_h */
