/*
 * $Log$
 * Revision 1.10  2003/09/10 09:42:13  sbs
 * TEAssureAbsValFitsShape added.
 *
 * Revision 1.9  2003/09/09 14:56:11  sbs
 * extended type error reporting added
 *
 * Revision 1.8  2003/04/11 17:59:01  sbs
 * TEAssureProdValMatchesProdShape added.
 *
 * Revision 1.7  2003/04/09 15:35:34  sbs
 * TEAssureNumS and TEAssureNumA added.
 *
 * Revision 1.6  2003/04/07 14:32:39  sbs
 * type assertions extended for AKV types.
 * signature of TEMakeInfo extended
 * TEAssureValMatchesShape and TEGetCFFun added.
 *
 * Revision 1.5  2003/03/19 10:34:10  sbs
 * TEAssureVect added.
 *
 * Revision 1.4  2002/09/04 12:59:46  sbs
 * TEArrayElem2Obj and TEAssureSameScalarType added.
 *
 * Revision 1.3  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.2  2002/08/07 09:51:07  sbs
 * TEAssureIntS added.
 *
 * Revision 1.1  2002/08/05 16:58:40  sbs
 * Initial revision
 *
 *
 */

#ifndef _type_errors_h
#define _type_errors_h

typedef struct TE_INFO te_info;

#include "types.h"
#include "new_types.h"

extern char *TEPrfArg2Obj (char *prf_str, int pos);
extern char *TEArg2Obj (int pos);
extern char *TEArrayElem2Obj (int pos);

extern void TEAssureScalar (char *obj, ntype *type);
extern void TEAssureVect (char *obj, ntype *type);
extern void TEAssureBoolS (char *obj, ntype *type);
extern void TEAssureBoolA (char *obj, ntype *type);
extern void TEAssureNumS (char *obj, ntype *type);
extern void TEAssureNumA (char *obj, ntype *type);
extern void TEAssureSimpleType (char *obj, ntype *type);
extern void TEAssureIntS (char *obj, ntype *type);
extern void TEAssureIntVect (char *obj, ntype *type);
extern void TEAssureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEAssureValMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEAssureAbsValFitsShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEAssureProdValMatchesProdShape (char *obj1, ntype *type1, char *obj2,
                                             ntype *type2);
extern void TEAssureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEAssureSameScalarType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern ntype *TEAssureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2);

extern te_info *TEMakeInfo (int linenum, char *kind_str, char *name_str, node *wrapper,
                            node *assign, void *cffun, te_info *parent);
extern void TEExtendedAbort ();
extern int TEGetLine (te_info *info);
extern char *TEGetKindStr (te_info *info);
extern char *TEGetNameStr (te_info *info);
extern node *TEGetWrapper (te_info *info);
extern node *TEGetAssign (te_info *info);
extern void *TEGetCFFun (te_info *info);
extern te_info *TEGetParent (te_info *info);

#endif /* _type_errors_h */
