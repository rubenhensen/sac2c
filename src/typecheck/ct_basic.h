/*
 * $Log$
 * Revision 1.3  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.2  2004/03/05 12:06:44  sbs
 * NTCCond added.
 *
 * Revision 1.1  2002/08/05 16:57:46  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CT_BASIC_H_
#define _SAC_CT_BASIC_H_

#include "new_types.h"
#include "type_errors.h"

extern ntype *NTCCTcomputeType (ct_funptr CtFun, te_info *info, ntype *args);
extern ntype *NTCCTcond (te_info *info, ntype *args);

#endif /* _SAC_CT_BASIC_H_ */
