/*
 * $Log$
 * Revision 1.23  2004/11/23 00:46:20  ktr
 * Ismop SacDEVCamp
 *
 * Revision 1.22  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.21  2004/09/27 13:15:20  sah
 * added serialization support
 *
 * Revision 1.20  2004/09/22 10:03:58  ktr
 * Modified the comment to point out the common pitfall that COGetDim does
 * not return a freshly allocated shape structure
 *
 * Revision 1.19  2004/05/30 13:03:43  khf
 * COIdxSel added
 *
 * Revision 1.18  2003/09/26 10:14:23  sbs
 * COIsEmptyVect added
 *
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

#ifndef _SAC_CONSTANTS_H_
#define _SAC_CONSTANTS_H_

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
 *   IMPORTANT: THIS RULE DOES NOT HOLD FOR COGetShape, WHICH RETURNS A
 *              POINTER TO THE INTERNAL SHAPE STRUCTURE
 *
 */

#include <stdio.h>
#include "types.h"

/***
 ***
 *** Basic operations implemented in constants_basic.c:
 ***
 ***/

/*
 * Functions for creating constants:
 */
/******************************************************************************
 *
 * Constants
 *
 * Prefix: CO
 *
 *****************************************************************************/
extern constant *COmakeConstant (simpletype type, shape *shp, void *elems);
extern constant *COmakeConstantFromShape (shape *shp);
extern constant *COmakeConstantFromInt (int val);
extern constant *COmakeConstantFromArray (node *a);

extern constant *COmakeZero (simpletype type, shape *shp);
extern constant *COmakeOne (simpletype type, shape *shp);
extern constant *COmakeTrue (shape *shp);
extern constant *COmakeFalse (shape *shp);

/*
 * Functions for serializing/deserializing constants
 * see constants_serialize.c
 */

extern void COserializeConstant (FILE *file, constant *cnst);
extern constant *COdeserializeConstant (simpletype type, shape *shp, int vlen, char *vec);

/*
 * Functions for extracting info from constants:
 *
 * IMPORTANT: THESE FUNCTIONS YIELD POINTERS TO INTERNAL STRUCTURES
 *                        *** TREAT THEM CAREFULLY ***
 *                        ***** DO NOT FREE THEM *****
 */
extern simpletype COgetType (constant *a);
extern int COgetDim (constant *a);
extern shape *COgetShape (constant *a);
extern void *COgetDataVec (constant *a);

/*
 * Functions for handling / converting constants:
 */
extern constant *COcopyConstant (constant *a);
extern constant *COcopyScalar2OneElementVector (constant *a);
extern char *COconstantData2String (int max_char, constant *a);
extern char *COconstant2String (constant *a);
extern shape *COconstant2Shape (constant *a);
extern void COprintConstant (FILE *file, constant *a);
extern constant *COfreeConstant (constant *a);
extern node *COconstant2AST (constant *a);
extern constant *COaST2Constant (node *a);
extern bool COisConstant (node *a);
extern bool COcompareConstants (constant *c1, constant *c2);

/* basic value compares, if all==true the condition must hold for all elements */
extern bool COisZero (constant *a, bool all);
extern bool COisOne (constant *a, bool all);
extern bool COisTrue (constant *a, bool all);
extern bool COisFalse (constant *a, bool all);

extern bool COisEmptyVect (constant *a);

/***
 ***
 *** structural operations implemented in constants_struc_ops.c:
 ***
 ***/

extern constant *COreshape (constant *idx, constant *a);
extern constant *COsel (constant *idx, constant *a);
extern constant *COidxSel (constant *idx, constant *a);
extern constant *COtake (constant *idx, constant *a);
extern constant *COdrop (constant *idx, constant *a);
extern constant *COdim (constant *a);
extern constant *COshape (constant *a);
extern constant *COmodarray (constant *a, constant *idx, constant *elem);
extern constant *COcat (constant *a, constant *b);
/* missing: not yet implemented
extern constant *  CORotate  ( constant *dim, constant *num, constant *a);
*/

/***
 ***
 *** value transforming operations implemented in constants_ari_ops.c:
 ***
 ***/
/* numerical ops */
extern constant *COadd (constant *a, constant *b);
extern constant *COsub (constant *a, constant *b);
extern constant *COmul (constant *a, constant *b);
extern constant *COdiv (constant *a, constant *b);
extern constant *COmod (constant *a, constant *b);
extern constant *COmin (constant *a, constant *b);
extern constant *COmax (constant *a, constant *b);

/* bool ops */
extern constant *COand (constant *a, constant *b);
extern constant *COor (constant *a, constant *b);

/* compare ops */
extern constant *COeq (constant *a, constant *b);
extern constant *COneq (constant *a, constant *b);

extern constant *COle (constant *a, constant *b);
extern constant *COlt (constant *a, constant *b);
extern constant *COge (constant *a, constant *b);
extern constant *COgt (constant *a, constant *b);

/* unary ops */
extern constant *COnot (constant *a);
extern constant *COtoi (constant *a);
extern constant *COtof (constant *a);
extern constant *COtod (constant *a);
extern constant *COabs (constant *a);
extern constant *COneg (constant *a);

#endif /* _SAC_CONSTANTS_H_ */
