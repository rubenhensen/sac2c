#ifndef _SAC_TYPE_UTILS_H_
#define _SAC_TYPE_UTILS_H_

#include "types.h"

/**
 * Function types:
 */
extern ntype *TUcreateFuntype (node *fundef);
extern ntype *TUcreateFuntypeIgnoreArtificials (node *fundef);
extern char *TUtypeSignature2String (node *fundef);
extern char *TUwrapperTypeSignature2String( node *fundef);
extern bool TUsignatureMatches (node *formal, ntype *actual_prod_type, bool exact);

/**
 * User types:
 */
extern ntype *TUcheckUdtAndSetBaseType (usertype udt, int *visited);
extern simpletype TUgetBaseSimpleType (ntype *type);

/**
 * AKV types:
 */
extern int TUakvScalInt2Int (ntype *ty);
extern ntype *TUint2akv (int val);

/**
 * dispatch-wrapper related:
 */
extern ntype *TUrebuildWrapperTypeAlphaFix (ntype *);
extern ntype *TUrebuildWrapperTypeAlpha (ntype *);

/**
 * N_arg related:
 */
extern ntype *TUmakeProductTypeFromArgs (node *args);
extern node *TUmakeTypeExprsFromArgs (node *args);
extern node *TUargtypes2unknownAUD (node *args);
extern ntype *TUactualArgs2Ntype (node *actual);

/**
 * N_ret related:
 */
extern ntype *TUmakeProductTypeFromRets (node *rets);
extern node *TUcreateTmpVardecsFromRets (node *rets);
extern node *TUmakeTypeExprsFromRets (node *rets);
extern node *TUrettypes2unknownAUD (node *rets);
extern node *TUreplaceRetTypes (node *rets, ntype *prodt);
extern node *TUrettypes2alpha (node *rets);
extern node *TUrettypes2alphaFix (node *rets);
extern node *TUalphaRettypes2bottom (node *rets, const char *msg);
extern node *TUrettypes2alphaMax (node *rets);
extern node *TUrettypes2alphaAUDMax (node *rets);
extern bool TUretsContainBottom (node *rets);
extern bool TUretsAreConstant (node *rets);

/**
 * general type computations:
 */
extern ntype *TUtype2alphaMax (ntype *type);
extern ntype *TUtype2alphaAUDMax (ntype *type);
extern ntype *TUstripImplicitNestingOperations (ntype *poly);
extern ntype *TUcomputeImplementationType (ntype *ty);

/**
 * dealing with bottom types:
 */
extern ntype *TUcombineBottom (ntype *left, ntype *right);
extern ntype *TUcombineBottoms (ntype *prod);
extern ntype *TUcombineBottomsFromRets (node *rets);
extern ntype *TUspreadBottoms (ntype *prod);

/**
 * advanced inspection functions:
 */
extern bool TUdimKnown (ntype *ty);
extern bool TUshapeKnown (ntype *ty);

extern bool TUisBoolScalar (ntype *ty);
extern bool TUisIntScalar (ntype *ty);
extern bool TUisIntVect (ntype *ty);
extern bool TUisEmptyVect (ntype *ty);
extern bool TUisScalar (ntype *ty);
extern bool TUisVector (ntype *ty);

extern bool TUisUnsigned (ntype *ty);

extern bool TUhasBasetype (ntype *ty, simpletype smpl);
extern bool TUisUniqueUserType (ntype *type);
extern bool TUisArrayOfUser (ntype *type);
extern bool TUisArrayOfSimple (ntype *type);
extern bool TUcontainsUser (ntype *type);
extern bool TUisHidden (ntype *type);
extern bool TUisNested (ntype *type);
extern bool TUisBoxed (ntype *type);
extern bool TUisPolymorphic (ntype *type);

/**
 * functions mainly needed for code generation:
 */
extern int TUgetFullDimEncoding (ntype *type);
extern int TUgetDimEncoding (ntype *type);
extern int TUgetLengthEncoding (ntype *type);
extern simpletype TUgetSimpleImplementationType (ntype *type);

/**
 * type relations:
 */
extern bool TUeqShapes (ntype *a, ntype *b);
extern bool TUleShapeInfo (ntype *a, ntype *b);
extern bool TUeqElementSize (ntype *a, ntype *b);
extern bool TUravelsHaveSameStructure (ntype *a, ntype *b);


#endif /* _SAC_TYPE_UTILS_H_*/
