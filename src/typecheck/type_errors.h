/*
 * $Log$
 * Revision 1.14  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 1.13  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.12  2004/10/26 10:46:59  sbs
 * type_info now holds the module name as well.
 *
 * Revision 1.11  2003/12/02 09:53:02  sbs
 * TEAssureNonNegativeValues added.
 *
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

#ifndef _SAC_TYPE_ERRORS_H_
#define _SAC_TYPE_ERRORS_H_

#include "types.h"

extern char *TEprfArg2Obj (const char *prf_str, int pos);
extern char *TEarg2Obj (int pos);
extern char *TEarrayElem2Obj (int pos);

extern void TEassureScalar (char *obj, ntype *type);
extern void TEassureVect (char *obj, ntype *type);
extern void TEassureBoolS (char *obj, ntype *type);
extern void TEassureBoolA (char *obj, ntype *type);
extern void TEassureNumS (char *obj, ntype *type);
extern void TEassureNumA (char *obj, ntype *type);
extern void TEassureSimpleType (char *obj, ntype *type);
extern void TEassureIntS (char *obj, ntype *type);
extern void TEassureIntVect (char *obj, ntype *type);
extern void TEassureNonNegativeValues (char *obj, ntype *type);
extern void TEassureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureAbsValFitsShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureProdValMatchesProdShape (char *obj1, ntype *type1, char *obj2,
                                             ntype *type2);
extern void TEassureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureSameScalarType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern ntype *TEassureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2);

extern te_info *TEmakeInfo (int linenum, char *kind_str, char *mod_str,
                            const char *name_str, node *wrapper, node *assign,
                            const void *cffun, te_info *parent);
extern void TEextendedAbort ();
extern int TEgetLine (te_info *info);
extern char *TEgetKindStr (te_info *info);
extern char *TEgetModStr (te_info *info);
extern const char *TEgetNameStr (te_info *info);
extern node *TEgetWrapper (te_info *info);
extern node *TEgetAssign (te_info *info);
extern const void *TEgetCFFun (te_info *info);
extern te_info *TEgetParent (te_info *info);

#endif /* _SAC_TYPE_ERRORS_H_ */
