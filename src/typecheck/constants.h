/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _constants_h
#define _constants_h

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
 * Functions for converting constants:
 */
extern node *COConstant2AST (constant *a);

/*
 * Operations on constants:
 */
extern constant *COPsi (constant *idx, constant *a);

#endif /* _constants_h */
