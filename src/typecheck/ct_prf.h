/*
 * $Log$
 * Revision 1.9  2004/02/03 11:25:44  sbs
 * NTCPRF_phi added.
 *
 * Revision 1.8  2003/09/10 09:42:35  sbs
 * NTCPRF_drop_SxV improved /
 * NTCPRF_take_SxV added.
 *
 * Revision 1.7  2003/04/09 15:35:57  sbs
 * NTCPRF_toiS, NTCPRF_toiA, NTCPRF_tofS, NTCPRF_tofA, NTCPRF_todS, NTCPRF_todA,
 * NTCPRF_ari_op_A, NTCPRF_log_op_A added.
 *
 * Revision 1.6  2003/04/07 14:34:19  sbs
 * NTCPRF_cffuntab added .
 *
 * Revision 1.5  2003/03/19 10:34:30  sbs
 * NTCPRF_drop_SxV and NTCPRF_cat_VxV added.
 *
 * Revision 1.4  2002/10/28 14:04:15  sbs
 * NTCPRF_cast added
 *
 * Revision 1.3  2002/09/04 12:59:46  sbs
 * type checking of arrays changed; now sig deps will be created as well.
 *
 * Revision 1.2  2002/08/07 09:49:58  sbs
 * modulo added
 *
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
extern void *NTCPRF_cffuntab[];

extern ntype *NTCPRF_dummy (te_info *info, ntype *args);
extern ntype *NTCPRF_array (te_info *info, ntype *elems);
extern ntype *NTCPRF_cast (te_info *info, ntype *elems);
extern ntype *NTCPRF_dim (te_info *info, ntype *args);
extern ntype *NTCPRF_shape (te_info *info, ntype *args);
extern ntype *NTCPRF_reshape (te_info *info, ntype *args);
extern ntype *NTCPRF_selS (te_info *info, ntype *args);
extern ntype *NTCPRF_modarrayS (te_info *info, ntype *args);
extern ntype *NTCPRF_toiS (te_info *info, ntype *args);
extern ntype *NTCPRF_toiA (te_info *info, ntype *args);
extern ntype *NTCPRF_tofS (te_info *info, ntype *args);
extern ntype *NTCPRF_tofA (te_info *info, ntype *args);
extern ntype *NTCPRF_todS (te_info *info, ntype *args);
extern ntype *NTCPRF_todA (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_SxS (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_SxA (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_AxS (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_AxA (te_info *info, ntype *args);
extern ntype *NTCPRF_ari_op_A (te_info *info, ntype *args);
extern ntype *NTCPRF_rel_op_AxA (te_info *info, ntype *args);
extern ntype *NTCPRF_log_op_AxA (te_info *info, ntype *args);
extern ntype *NTCPRF_log_op_A (te_info *info, ntype *args);
extern ntype *NTCPRF_int_op_SxS (te_info *info, ntype *args);
extern ntype *NTCPRF_drop_SxV (te_info *info, ntype *args);
extern ntype *NTCPRF_take_SxV (te_info *info, ntype *args);
extern ntype *NTCPRF_cat_VxV (te_info *info, ntype *args);
extern ntype *NTCPRF_phi (te_info *info, ntype *args);

#endif /* _ct_prf_h */
