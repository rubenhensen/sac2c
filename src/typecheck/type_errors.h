/*
 * $Log$
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

extern void TEAssureScalar (char *obj, ntype *type);
extern void TEAssureBoolS (char *obj, ntype *type);
extern void TEAssureBoolA (char *obj, ntype *type);
extern void TEAssureSimpleType (char *obj, ntype *type);
extern void TEAssureIntS (char *obj, ntype *type);
extern void TEAssureIntVect (char *obj, ntype *type);
extern void TEAssureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEAssureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern ntype *TEAssureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2);

extern te_info *TEMakeInfo (int linenum, char *kind_str, char *name_str, node *wrapper);
extern int TEGetLine (te_info *info);
extern char *TEGetKindStr (te_info *info);
extern char *TEGetNameStr (te_info *info);
extern node *TEGetWrapper (te_info *info);

#endif /* _type_errors_h */
