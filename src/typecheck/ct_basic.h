/*
 * $Log$
 * Revision 1.2  2004/03/05 12:06:44  sbs
 * NTCCond added.
 *
 * Revision 1.1  2002/08/05 16:57:46  sbs
 * Initial revision
 *
 *
 */

#ifndef _ct_basic_h
#define _ct_basic_h

#include "new_types.h"
#include "type_errors.h"

extern ntype *NTCCTComputeType (ct_funptr CtFun, te_info *info, ntype *args);
extern ntype *NTCCond (te_info *info, ntype *args);

#endif /* _ct_basic_h */
