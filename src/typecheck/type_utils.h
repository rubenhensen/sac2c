/*
 * $Id$
 */

#ifndef _SAC_TYPE_UTILS_H_
#define _SAC_TYPE_UTILS_H_

#include "types.h"

extern ntype *TUrebuildWrapperType (ntype *);
extern node *TUcreateTmpVardecsFromRets (node *rets);
extern ntype *TUmakeProductTypeFromRets (node *rets);
extern node *TUreplaceRetTypes (node *rets, ntype *prodt);
extern node *TUrettypes2unknownAUD (node *rets);
extern node *TUargtypes2unknownAUD (node *rets);
extern ntype *TUtype2alphaMax (ntype *type);
extern node *TUrettypes2alphaFix (node *rets);
extern node *TUrettypes2alphaMax (node *rets);
extern bool TUdimKnown (ntype *ty);
extern bool TUshapeKnown (ntype *ty);
extern bool TUisIntVect (ntype *ty);
extern bool TUisUniqueUserType (ntype *type);
extern bool TUisArrayOfUser (ntype *type);
extern bool TUisHidden (ntype *type);
extern bool TUisBoxed (ntype *type);
extern ntype *TUcomputeImplementationType (ntype *ty);
extern char *TUtypeSignature2String (node *fundef);
extern ntype *TUactualArgs2Ntype (node *actual);
extern bool TUsignatureMatches (node *formal, ntype *actual_prod_type);
extern bool TUretsContainBottom (node *rets);
extern bool TUretsAreConstant (node *rets);
extern ntype *TUcombineBottom (ntype *left, ntype *right);
extern ntype *TUcombineBottomsFromRets (node *rets);
extern ntype *TUcheckUdtAndSetBaseType (usertype udt, int *visited);

#endif /* _SAC_TYPE_UTILS_H_*/
