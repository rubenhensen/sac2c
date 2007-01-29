/*
 * $Log$
 * Revision 1.2  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.1  2002/08/05 16:57:49  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CT_FUN_H_
#define _SAC_CT_FUN_H_

#include "types.h"

extern dft_res *NTCCTdispatchFunType (node *wrapper, ntype *args);

extern ntype *NTCCTudf (te_info *info, ntype *args);
extern ntype *NTCCTudfDispatched (te_info *info, ntype *args);

#endif /* _SAC_CT_FUN_H_ */
