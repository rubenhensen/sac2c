/*
 * $Log$
 * Revision 1.1  2002/08/05 16:57:49  sbs
 * Initial revision
 *
 *
 */

#ifndef _ct_fun_h
#define _ct_fun_h

#include "new_types.h"
#include "type_errors.h"

extern DFT_res *NTCFUNDispatchFunType (node *wrapper, ntype *args);

extern ntype *NTCFUN_udf (te_info *info, ntype *args);

#endif /* _ct_fun_h */
