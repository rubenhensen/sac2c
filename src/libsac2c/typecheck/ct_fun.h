
#ifndef _SAC_CT_FUN_H_
#define _SAC_CT_FUN_H_

#include "types.h"

extern dft_res *NTCCTdispatchFunType (node *wrapper, ntype *args);

extern ntype *NTCCTudf (te_info *info, ntype *args);
extern ntype *NTCCTudfDispatched (te_info *info, ntype *args);

#endif /* _SAC_CT_FUN_H_ */
