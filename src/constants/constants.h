/*
 * $Log$
 * Revision 1.17  2003/06/11 22:05:36  ktr
 * Added support for multidimensional arrays
 *
 * Revision 1.16  2003/04/11 17:55:41  sbs
 * COConstant2Shape added.
 *
 * Revision 1.15  2003/04/09 15:37:35  sbs
 * CONeg added.
 *
 * Revision 1.14  2003/04/08 12:27:09  sbs
 * new typedefs monCF and triCF added.
 *
 * Revision 1.13  2003/04/07 14:23:27  sbs
 * COMakeConstantFromShape, COCopyScalar2OneElementVector, COConstantData2String, and
 * COConstant2String added.
 * COCat implemented as a binary operation (on the outermost axis!).
 *
 * Revision 1.12  2002/10/07 23:45:03  dkr
 * signature of COGetDataVec() corrected
 *
 * Revision 1.11  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 1.10  2001/05/09 15:53:03  nmw
 * COCompareConstants() added
 *
 * Revision 1.9  2001/05/08 13:15:09  nmw
 * signature for IsZero... changed
 *
 * Revision 1.8  2001/05/03 16:52:55  nmw
 *
 * Revision 1.7  2001/05/02 08:00:46  nmw
 * COIsZero, COIsOne, ... and COMakeZero, COMakeOne, ... added
 *
 * Revision 1.6  2001/04/30 12:28:43  nmw
 * GetDataVec added
 *
 * Revision 1.4  2001/03/23 12:49:02  nmw
 * CODim/COShape implemented
 *
 * Revision 1.3  2001/03/22 14:24:37  nmw
 * primitove ari ops implemented
 *
 * Revision 1.2  2001/03/05 16:57:04  sbs
 * COAdd, COSub, COMul, and CODiv added
 *
 * Revision 1.1  2001/03/02 14:32:52  sbs
 * Initial revision
 *
 * Revision 3.2  2001/02/23 18:04:22  sbs
 * extended for negative take's and drop's
 * added print facility
 *
 * Revision 3.1  2000/11/20 18:00:03  sacbase
 * new release made
 *
 * Revision 1.3  2000/05/03 16:49:17  dkr
 * COFreeConstant returns NULL now
 *
 * Revision 1.2  1999/10/22 14:12:24  sbs
 * inserted comments and added reshape, take, drop, and psi with non
 * scalar results.
 * ..
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _constants_h
#define _constants_h

/*
 * The module "constants" implements an abstract datatype for keeping "machine"
 * constants (scalars as well as arbitrary shaped arrays). This file describes
 * the interface to that module.
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a constant is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 *   There are EXACTLY ONE CLASSE OF FUNCTIONS that is an EXEPTION OF
 *   THIS RULE: - the GETxyz - functions for extracting components of constants
 * - The only function for freeing a shape structure is COFreeConstant!
 * - If the result is a shape structure, it has been freshly allocated!
 *
 */

#include "shape.h"
#include "types.h"

typedef struct CONSTANT constant;
typedef constant *(*monCF) (constant *);
typedef constant *(*binCF) (constant *, constant *);
typedef constant *(*triCF) (constant *, constant *, constant *);

/***
 ***
 *** Basic operations implemented in constants_basic.c:
 ***
 ***/

/*
 * Functions for creating constants:
 */
extern constant *COMakeConstant (simpletype type, shape *shp, void *elems);
extern constant *COMakeConstantFromShape (shape *shp);
extern constant *COMakeConstantFromInt (int val);
extern constant *COMakeConstantFromArray (node *a);

extern constant *COMakeZero (simpletype type, shape *shp);
extern constant *COMakeOne (simpletype type, shape *shp);
extern constant *COMakeTrue (shape *shp);
extern constant *COMakeFalse (shape *shp);

/*
 * Functions for extracting info from constants:
 */
extern simpletype COGetType (constant *a);
extern int COGetDim (constant *a);
extern shape *COGetShape (constant *a);
extern void *COGetDataVec (constant *a);

/*
 * Functions for handling / converting constants:
 */
extern constant *COCopyConstant (constant *a);
extern constant *COCopyScalar2OneElementVector (constant *a);
extern char *COConstantData2String (int max_char, constant *a);
extern char *COConstant2String (constant *a);
extern shape *COConstant2Shape (constant *a);
extern void COPrintConstant (FILE *file, constant *a);
extern constant *COFreeConstant (constant *a);
extern node *COConstant2AST (constant *a);
extern constant *COAST2Constant (node *a);
extern bool COIsConstant (node *a);
extern bool COCompareConstants (constant *c1, constant *c2);

/* basic value compares, if all==true the condition must hold for all elements */
extern bool COIsZero (constant *a, bool all);
extern bool COIsOne (constant *a, bool all);
extern bool COIsTrue (constant *a, bool all);
extern bool COIsFalse (constant *a, bool all);

/***
 ***
 *** structural operations implemented in constants_struc_ops.c:
 ***
 ***/

extern constant *COReshape (constant *idx, constant *a);
extern constant *COSel (constant *idx, constant *a);
extern constant *COTake (constant *idx, constant *a);
extern constant *CODrop (constant *idx, constant *a);
extern constant *CODim (constant *a);
extern constant *COShape (constant *a);
extern constant *COModarray (constant *a, constant *idx, constant *elem);
extern constant *COCat (constant *a, constant *b);
/* missing: not yet implemented
extern constant *  CORotate  ( constant *dim, constant *num, constant *a);
*/

/***
 ***
 *** value transforming operations implemented in constants_ari_ops.c:
 ***
 ***/
/* numerical ops */
extern constant *COAdd (constant *a, constant *b);
extern constant *COSub (constant *a, constant *b);
extern constant *COMul (constant *a, constant *b);
extern constant *CODiv (constant *a, constant *b);
extern constant *COMod (constant *a, constant *b);
extern constant *COMin (constant *a, constant *b);
extern constant *COMax (constant *a, constant *b);

/* bool ops */
extern constant *COAnd (constant *a, constant *b);
extern constant *COOr (constant *a, constant *b);

/* compare ops */
extern constant *COEq (constant *a, constant *b);
extern constant *CONeq (constant *a, constant *b);

extern constant *COLe (constant *a, constant *b);
extern constant *COLt (constant *a, constant *b);
extern constant *COGe (constant *a, constant *b);
extern constant *COGt (constant *a, constant *b);

/* unary ops */
extern constant *CONot (constant *a);
extern constant *COToi (constant *a);
extern constant *COTof (constant *a);
extern constant *COTod (constant *a);
extern constant *COAbs (constant *a);
extern constant *CONeg (constant *a);

#endif /* _constants_h */
