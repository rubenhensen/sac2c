/*
 * $Log$
 * Revision 1.8  2001/05/03 16:52:55  nmw
 * *** empty log message ***
 *
 * Revision 1.7  2001/05/02 08:00:46  nmw
 * COIsZero, COIsOne, ... and COMakeZero, COMakeOne, ... added
 *
 * Revision 1.6  2001/04/30 12:28:43  nmw
 * GetDataVec added
 *
 * Revision 1.5  2001/04/25 13:36:50  nmw
 * *** empty log message ***
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

/***
 ***
 *** Basic operations implemented in constants_basic.c:
 ***
 ***/

/*
 * Functions for creating constants:
 */
extern constant *COMakeConstant (simpletype type, shape *shp, void *elems);
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
extern void **COGetDataVec (constant *a);

/*
 * Functions for handling / converting constants:
 */
extern constant *COCopyConstant (constant *a);
extern void COPrintConstant (FILE *file, constant *a);
extern constant *COFreeConstant (constant *a);
extern node *COConstant2AST (constant *a);
extern constant *COAST2Constant (node *a);
extern bool COIsConstant (node *a);

/* basic value compares */
extern bool COIsZero (constant *a);
extern bool COIsOne (constant *a);
extern bool COIsTrue (constant *a);
extern bool COIsFalse (constant *a);

/***
 ***
 *** structural operations implemented in constants_struc_ops.c:
 ***
 ***/

extern constant *COReshape (constant *idx, constant *a);
extern constant *COPsi (constant *idx, constant *a);
extern constant *COTake (constant *idx, constant *a);
extern constant *CODrop (constant *idx, constant *a);
extern constant *CODim (constant *a);
extern constant *COShape (constant *a);
extern constant *COModarray (constant *a, constant *idx, constant *elem);
/* missing: not yet implemented
extern constant *  COCat     ( constant *dim, constant *a, constant *b);
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

#endif /* _constants_h */
