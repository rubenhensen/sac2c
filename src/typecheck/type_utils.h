/*
 *
 * $Log$
 * Revision 1.2  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 1.1  2004/11/24 09:50:20  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_TYPE_UTILS_H_
#define _SAC_TYPE_UTILS_H_

#include "types.h"

extern node *TUcreateTmpVardecsFromRets (node *rets);
extern ntype *TUmakeProductTypeFromRets (node *rets);
extern node *TUreplaceRetTypes (node *rets, ntype *prodt);
extern node *TUrettypes2unknownAUD (node *rets);
extern node *TUargtypes2unknownAUD (node *rets);
extern node *TUrettypes2AUD (node *rets);
extern node *TUargtypes2AUD (node *rets);
extern node *TUrettypes2alphaAUD (node *rets);
extern bool TUisUniqueUserType (ntype *type);

#endif /* _SAC_TYPE_UTILS_H_*/
