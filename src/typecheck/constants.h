/*
 * $Log$
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

#ifdef SELF
#undef SELF
#else
typedef void constant;
#endif

/*
 * Functions for creating constants:
 */
extern constant *COMakeConstant (simpletype type, shape *shp, void *elems);
extern constant *COMakeConstantFromInt (int val);
extern constant *COMakeConstantFromArray (node *a);

/*
 * Functions for extracting info from constants:
 */
extern simpletype COGetType (constant *a);
extern int COGetDim (constant *a);
extern shape *COGetShape (constant *a);

/*
 * Functions for handling / converting constants:
 */
extern constant *COCopyConstant (constant *a);
extern void COFreeConstant (constant *a);
extern node *COConstant2AST (constant *a);

/*
 * Operations on constants:
 */
extern constant *COReshape (constant *idx, constant *a);
extern constant *COPsi (constant *idx, constant *a);
extern constant *COTake (constant *idx, constant *a);
extern constant *CODrop (constant *idx, constant *a);

#endif /* _constants_h */
