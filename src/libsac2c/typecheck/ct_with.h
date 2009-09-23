/*
 * $Log$
 * Revision 1.3  2005/09/09 16:46:24  sbs
 * NTCCTwl_multicode added
 *
 * Revision 1.2  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.1  2002/08/05 16:57:54  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CT_WITH_H_
#define _SAC_CT_WITH_H_

#include "new_types.h"

extern ntype *NTCCTwl_idx (te_info *info, ntype *args);
extern ntype *NTCCTwl_multipart (te_info *info, ntype *args);
extern ntype *NTCCTwl_multicode (te_info *info, ntype *args);
extern ntype *NTCCTwl_multifoldcode (te_info *info, ntype *args);
extern ntype *NTCCTwl_gen (te_info *info, ntype *args);
extern ntype *NTCCTwl_mod (te_info *info, ntype *args);
extern ntype *NTCCTwl_fold (te_info *info, ntype *args);

#endif /* _SAC_CT_WITH_H_ */
