#ifndef _SAC_TYPE_ERRORS_H_
#define _SAC_TYPE_ERRORS_H_

#include "types.h"

extern char *TEprfArg2Obj (const char *prf_str, int pos);
extern char *TEarg2Obj (int pos);
extern char *TEanotherArg2Obj (int pos);
extern char *TEarrayElem2Obj (int pos);

extern void TEassureScalar (char *obj, ntype *type);
extern void TEassureVect (char *obj, ntype *type);
extern void TEassureBoolS (char *obj, ntype *type);
extern void TEassureBoolV (char *obj, ntype *type);
extern void TEassureBoolA (char *obj, ntype *type);
extern void TEassureNumS (char *obj, ntype *type);
extern void TEassureNumV (char *obj, ntype *type);
extern void TEassureNumA (char *obj, ntype *type);
extern void TEassureSimpleType (char *obj, ntype *type);
extern void TEassureSimpleS (char *obj, ntype *type);
extern void TEassureSimpleV (char *obj, ntype *type);
extern void TEassureIntS (char *obj, ntype *type);
extern void TEassureIntV (char *obj, ntype *type);
extern void TEassureIntVectLengthOne (char *obj, ntype *type);
extern void TEassureNonNegativeValues_S (char *obj, ntype *type);
extern void TEassureNonNegativeValues_V (char *obj, ntype *type);
extern void TEassureShpMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureShpMatchesInt (char *obj1, ntype *shp, int len);
extern void TEassureShpPlusDimMatchesDim (char *obj1, ntype *type1, char *obj2,
                                          ntype *type2, char *obj3, ntype *type3);
extern void TEassureShpIsPostfixOfShp (char *obj1, ntype *type1, char *obj2,
                                       ntype *type2);
extern void TEassureValMatchesDim (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValLeVal (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValLtVal_SxS (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValLeVal_SxS (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureValNonZero (char *obj1, ntype *type1);
extern void TEassureIdxMatchesShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureAbsValFitsShape (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureProdValMatchesProdShape (char *obj1, ntype *type1, char *obj2,
                                             ntype *type2);
extern void TEassureSameSimpleType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern void TEassureSameScalarType (char *obj1, ntype *type1, char *obj2, ntype *type2);
extern ntype *TEassureSameShape (char *obj1, ntype *type1, char *obj2, ntype *type2);

extern te_info *TEmakeInfo (int linenum, const char *file, te_kind_t kind,
                            const char *name_str);
extern te_info *TEmakeInfoUdf (int linenum, const char *file, te_kind_t kind,
                               const char *mod_str, const char *name_str, node *wrapper,
                               node *assign, te_info *parent);
extern te_info *TEmakeInfoPrf (int linenum, const char *file, te_kind_t kind,
                               const char *name_str, prf prf_no, int num_rets);
extern int TEone (ntype *args);
extern int TEtwo (ntype *args);
extern int TEthree (ntype *args);
extern int TEnMinusOne (ntype *args);
extern int TEn (ntype *args);
extern int TEval (ntype *args);

extern void TEfreeAllTypeErrorInfos (void);

extern void TEhandleError (int line, const char *file, const char *format, ...);
extern char *TEfetchErrors (void);
extern void TEextendedAbort (void);
extern int TEgetLine (te_info *info);
extern const char *TEgetFile (te_info *info);
extern char *TEgetKindStr (te_info *info);
extern te_kind_t TEgetKind (te_info *info);
extern const char *TEgetModStr (te_info *info);
extern const char *TEgetNameStr (te_info *info);
extern node *TEgetWrapper (te_info *info);
extern int TEgetNumRets (te_info *info);
extern node *TEgetAssign (te_info *info);
extern prf TEgetPrf (te_info *info);
extern constant *(*TEgetCFFun (te_info *info)) (constant *, constant *, constant *);
extern te_info *TEgetParent (te_info *info);

#endif /* _SAC_TYPE_ERRORS_H_ */
