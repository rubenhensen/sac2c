/*
 *
 * $Log$
 * Revision 1.14  2005/08/19 17:25:06  sbs
 * changed TUrettypes2alpha into TUrettypes2alphaFix, etc.
 *
 * Revision 1.13  2005/07/21 12:02:24  ktr
 * added TUdimKnown
 *
 * Revision 1.12  2005/07/16 19:06:08  sbs
 * TUshapeKnown added.
 *
 * Revision 1.11  2005/07/16 12:30:50  sbs
 * TUisIntVect added.
 *
 * Revision 1.10  2005/06/18 13:52:03  sah
 * moved SignatureMatches and ActualArgs2Ntype from
 * create_wrapper_code to type_utils
 *
 * Revision 1.9  2005/05/24 08:26:34  sbs
 * TUretypes2alpha modified.
 *
 * Revision 1.8  2004/12/09 12:32:27  sbs
 * TUargtypes2AUD and TYrettypes2AUD eliminated
 *
 * Revision 1.7  2004/12/07 14:36:16  sbs
 * added TUtypeSignature2String
 *
 * Revision 1.6  2004/12/05 19:19:55  sbs
 * return type of LaC funs changed into alphas.
 *
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

#endif /* _SAC_TYPE_UTILS_H_*/
