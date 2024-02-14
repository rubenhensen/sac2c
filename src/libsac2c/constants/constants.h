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
 *   These are the exceptions:
 *    - COconst2IntAndFree frees the given constant.
 *    - The GETxyz-functions for extracting components of constants.
 *   Notes:
 *    - The only function for freeing a shape structure is COFreeConstant!
 *    - If the result is a shape structure, it has been freshly allocated!
 *      IMPORTANT: THIS RULE DOES NOT HOLD FOR COGetShape.
 *                 IT RETURNS A POINTER TO THE INTERNAL SHAPE STRUCTURE
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
extern constant *COmakeConstantFromFloat (float val);
extern constant *COmakeConstantFromBool (bool val);
extern constant *COmakeConstantFromDynamicArguments (simpletype type, int dim, ...);
extern constant *COmakeConstantFromArray (simpletype type, int dim, int *shp,
                                          void *elems);

extern constant *COmakeNegativeOne (simpletype type, shape *shp);
extern constant *COmakeZero (simpletype type, shape *shp);
extern constant *COmakeOne (simpletype type, shape *shp);
extern constant *COmakeTrue (shape *shp);
extern constant *COmakeFalse (shape *shp);

/*
 * Functions for serializing/deserializing constants
 * see constants_serialize.c
 */

extern void COserializeConstant (FILE *file, constant *cnst);
extern constant *COdeserializeConstant (simpletype type, shape *shp, size_t vlen, char *vec);

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
                                        void *accu, void *attr, bool scalaridx);

extern constant *COcopyConstant (constant *a);
extern constant *COcopyScalar2OneElementVector (constant *a);
extern char *COconstantData2String (size_t max_char, constant *a);
extern char *COconstant2String (constant *a);
extern shape *COconstant2Shape (constant *a);
extern int COconst2Int (constant *a);
extern int COconst2IntAndFree (constant *a);
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
extern bool COisNeg (constant *a, bool all);
extern bool COisPos (constant *a, bool all);
extern bool COisEmptyVect (constant *a);

/***
 ***
 *** structural operations implemented in constants_struc_ops.c:
 ***
 ***/

extern constant *COreshape (constant *idx, constant *a, constant *);
extern constant *COsel (constant *idx, constant *a, constant *);
extern constant *COidxSel (constant *idx, constant *a, constant *);
extern constant *COoverSel (constant *idx, constant *a, constant *);
extern constant *COtake (constant *idx, constant *a, constant *);
extern constant *COdrop (constant *idx, constant *a, constant *);
extern constant *COdim (constant *a, constant *b, constant *);
extern constant *COisDist (constant *a, constant *b, constant *);
extern constant *COfirstElems (constant *a, constant *b, constant *);
extern constant *COlocalFrom (constant *a, constant *tmp1, constant *tmp2);
extern constant *COoffs (constant *a, constant *b, constant *);
extern constant *COshape (constant *a, constant *b, constant *);
extern constant *COmodarray_AxVxS (constant *a, constant *idx, constant *elem);
extern constant *COmodarray_AxVxA (constant *a, constant *idx, constant *elem);
extern constant *COidx_modarray_AxSxS (constant *a, constant *idx, constant *elem);
extern constant *COidx_modarray_AxSxA (constant *a, constant *idx, constant *elem);
extern constant *COcat (constant *a, constant *b, constant *);
extern constant *COvect2offset (constant *shp, constant *iv, constant *);
/* missing: not yet implemented
extern constant *  CORotate  ( constant *dim, constant *num, constant *a);
*/

/***
 ***
 *** value transforming operations implemented in constants_ari_ops.c:
 ***
 ***/
/* numerical ops */
extern constant *COadd (constant *a, constant *b, constant *);
extern constant *COsimd_add (constant *dummy, constant *a, constant *b);
extern constant *COsimd_sub (constant *dummy, constant *a, constant *b);
extern constant *COsimd_mul (constant *dummy, constant *a, constant *b);
extern constant *COsimd_div (constant *dummy, constant *a, constant *b);
extern constant *COsimd_sel (constant *simd_length, constant *idx, constant *a);
extern constant *COsimd_sel_SxS (constant *idx, constant *a, constant *tmp);
extern constant *COsub (constant *a, constant *b, constant *);
extern constant *COmul (constant *a, constant *b, constant *);
extern constant *COdiv (constant *a, constant *b, constant *);
extern constant *COmod (constant *a, constant *b, constant *);
extern constant *COaplmod (constant *a, constant *b, constant *);
extern constant *COmin (constant *a, constant *b, constant *);
extern constant *COmax (constant *a, constant *b, constant *);

/* bool ops */
extern constant *COand (constant *a, constant *b, constant *);
extern constant *COor (constant *a, constant *b, constant *);
extern constant *COall (constant *a, constant *, constant *);

/* compare ops */
extern constant *COeq (constant *a, constant *b, constant *);
extern constant *COneq (constant *a, constant *b, constant *);

extern constant *COle (constant *a, constant *b, constant *);
extern constant *COlt (constant *a, constant *b, constant *);
extern constant *COge (constant *a, constant *b, constant *);
extern constant *COgt (constant *a, constant *b, constant *);

/* unary ops */
extern constant *COnot (constant *a, constant *, constant *);
extern constant *COtobool (constant *a, constant *, constant *);
extern constant *COtob (constant *a, constant *, constant *);
extern constant *COtoc (constant *a, constant *, constant *);
extern constant *COtos (constant *a, constant *, constant *);
extern constant *COtoi (constant *a, constant *, constant *);
extern constant *COtol (constant *a, constant *, constant *);
extern constant *COtoll (constant *a, constant *, constant *);
extern constant *COtoub (constant *a, constant *, constant *);
extern constant *COtous (constant *a, constant *, constant *);
extern constant *COtoui (constant *a, constant *, constant *);
extern constant *COtoul (constant *a, constant *, constant *);
extern constant *COtoull (constant *a, constant *, constant *);
extern constant *COtof (constant *a, constant *, constant *);
extern constant *COtod (constant *a, constant *, constant *);
extern constant *COabs (constant *a, constant *, constant *);
extern constant *COneg (constant *a, constant *, constant *);
extern constant *COreciproc (constant *a, constant *, constant *);

// Not sure where this stuff belongs, really. Neither fish nor fowl
// These should go!!! please use COvect2offset instead!!!

extern int Idx2OffsetArray (constant *idx, node *a);
extern int COidxs2offset (constant *idx, constant *a);

#endif /* _SAC_CONSTANTS_H_ */
