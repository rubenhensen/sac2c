/*
 * $Id$
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
extern constant *COmakeConstantFromDynamicArguments (simpletype type, int dim, ...);
extern constant *COmakeConstantFromArray (simpletype type, int dim, int *shp,
                                          void *elems);

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
extern int COgetExtent (constant *a, int i);
extern void *COgetDataVec (constant *a);

/*
 * Functions for handling / converting constants:
 */

extern void *COcreateAllIndicesAndFold (shape *shp,
                                        void *(*foldfun) (constant *idx, void *, void *),
                                        void *accu, void *attr);

extern constant *COcopyConstant (constant *a);
extern constant *COcopyScalar2OneElementVector (constant *a);
extern char *COconstantData2String (int max_char, constant *a);
extern char *COconstant2String (constant *a);
extern shape *COconstant2Shape (constant *a);
extern int COconst2Int (constant *a);
extern void COprintConstant (FILE *file, constant *a);
extern constant *COfreeConstant (constant *a);
extern void COtouchConstant (constant *a, info *arg_info);
extern node *COconstant2AST (constant *a);
extern constant *COaST2Constant (node *a);
extern bool COisConstant (node *a);
extern bool COcompareConstants (constant *c1, constant *c2);

/* basic value compares, if all==true the condition must hold for all elements */
extern bool COisZero (constant *a, bool all);
extern bool COisOne (constant *a, bool all);
extern bool COisTrue (constant *a, bool all);
extern bool COisFalse (constant *a, bool all);
extern bool COisNonNeg (constant *a, bool all);

extern bool COisEmptyVect (constant *a);

/***
 ***
 *** structural operations implemented in constants_struc_ops.c:
 ***
 ***/

extern constant *COreshape (constant *idx, constant *a);
extern constant *COsel (constant *idx, constant *a);
extern constant *COidxSel (constant *idx, constant *a);
extern constant *COoverSel (constant *idx, constant *a);
extern constant *COtake (constant *idx, constant *a);
extern constant *COdrop (constant *idx, constant *a);
extern constant *COdim (constant *a);
extern constant *COshape (constant *a);
extern constant *COmodarray (constant *a, constant *idx, constant *elem);
extern constant *COcat (constant *a, constant *b);
/* missing: not yet implemented
extern constant *  CORotate  ( constant *dim, constant *num, constant *a);
*/

/* special ops */
extern int COvect2offset (constant *shp, constant *iv);

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
extern constant *COtoby (constant *a);
extern constant *COtos (constant *a);
extern constant *COtoi (constant *a);
extern constant *COtol (constant *a);
extern constant *COtoll (constant *a);
extern constant *COtouby (constant *a);
extern constant *COtous (constant *a);
extern constant *COtoui (constant *a);
extern constant *COtoul (constant *a);
extern constant *COtoull (constant *a);
extern constant *COtof (constant *a);
extern constant *COtod (constant *a);
extern constant *COabs (constant *a);
extern constant *COneg (constant *a);
extern constant *COreciproc (constant *a);

// Not sure where this stuff belongs, really. Neither fish nor fowl
// These should go!!! please use COvect2offset instead!!!

extern int Idx2OffsetArray (constant *idx, node *a);

#endif /* _SAC_CONSTANTS_H_ */
