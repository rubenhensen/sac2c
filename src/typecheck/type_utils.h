/*
 *
 * $Log$
 * Revision 1.5  2004/11/26 22:58:51  sbs
 * some new utils added
 * \.
 *
 * Revision 1.4  2004/11/26 21:15:43  cg
 * Added TUisBoxed()
 *
 * Revision 1.3  2004/11/26 20:56:26  sbs
 * TUisHidden added
 *
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
extern bool TUisArrayOfUser (ntype *type);
extern bool TUisHidden (ntype *type);
extern bool TUisBoxed (ntype *type);
extern ntype *TUcomputeImplementationType (ntype *ty);

#endif /* _SAC_TYPE_UTILS_H_*/
