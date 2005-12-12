/*
 * $Id$
 *
 */

#ifndef _SAC_CT_BASIC_H_
#define _SAC_CT_BASIC_H_

#include "new_types.h"
#include "type_errors.h"

extern ntype *NTCCTcomputeType (ct_funptr CtFun, te_info *info, ntype *args);
extern ntype *NTCCTcomputeTypeNonStrict (ct_funptr CtFun, te_info *info, ntype *args);
extern ntype *NTCCTcond (te_info *info, ntype *args);
extern ntype *NTCCTfuncond (te_info *info, ntype *args);

#endif /* _SAC_CT_BASIC_H_ */
