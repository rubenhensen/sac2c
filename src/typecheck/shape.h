/*
 * $Log$
 * Revision 1.1  1999/10/11 08:47:34  sbs
 * Initial revision
 *
 *
 */

#ifndef _shape_h
#define _shape_h

/*
 * The module "shape" implements an abstract datatype for keeping
 * shapes of arbitrary length. This file describes the interface to that
 * module.
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a shape is given as argument, neither the pointer to it nor
 *   any potential sub structure will be copied in any data structure
 *   that serves as a result!
 * - The only function for freeing a shape structure is SHFreeShape!
 * - If the result is a shape structure, it has been dynamically allocated!
 *
 */

#ifdef SELF
#undef SELF
#else
typedef void shape;
#endif

#include "types.h"

extern shape *SHMakeShape (int dim);
extern shape *SHCopyShape (shape *shp);
extern void SHFreeShape (shape *shp);

extern int SHGetDim (shape *shp);
extern int SHGetExtent (shape *shp, int dim);

extern shape *SHSetExtend (shape *shp, int dim, int val);

extern shape *SHAppendShapes (shape *a, shape *b);
extern char *SHShape2String (int dots, shape *shp);

extern shape *SHOldTypes2Shape (types *shpseg);

#endif /* _shape_h */
