/*
 * $Log$
 * Revision 1.1  2002/08/05 16:57:52  sbs
 * Initial revision
 *
 *
 */

#ifndef _ct_prf_h
#define _ct_prf_h

#include "types.h"
#include "new_types.h"
#include "type_errors.h"

extern ct_funptr NTCPRF_funtab[];

extern ntype *NTCPRF_dummy (te_info *info, ntype *args);
extern ntype *NTCPRF_dim (te_info *info, ntype *args);
extern ntype *NTCPRF_shape (te_info *info, ntype *args);
extern ntype *NTCPRF_reshape (te_info *info, ntype *args);
extern ntype *NTCPRF_selS (te_info *info, ntype *args);
extern ntype *NTCPRF_modarrayS (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_SxA (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_AxS (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_AxA (te_info *info, ntype *args);
extern ntype *NTCPRF_rel_op_AxA (te_info *info, ntype *args);
extern ntype *NTCPRF_log_op_AxA (te_info *info, ntype *args);

#endif /* _ct_prf_h */
