/*
 *
 * $Log$
 * Revision 1.5  2002/11/04 13:22:08  sbs
 * SHDropFromShape added.
 *
 * Revision 1.4  2002/06/21 14:03:48  dkr
 * SHShape2Array() added
 *
 * Revision 1.3  2001/04/30 12:31:34  nmw
 * SHShape2IntVec added
 *
 * Revision 1.2  2001/03/05 16:57:04  sbs
 * SHCompareShapes added
 *
 * Revision 1.1  2001/03/02 14:33:09  sbs
 * Initial revision
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
 * - whenever a shape is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 * - The only function for freeing a shape structure is SHFreeShape!
 * - If the result is a shape structure, it has been freshly allocated!
 *
 */

typedef struct SHAPE shape;

#include <stdio.h>
#include "types.h"

extern shape *SHMakeShape (int dim);
extern shape *SHCreateShape (int dim, ...);
extern shape *SHCopyShape (shape *shp);
extern void SHPrintShape (FILE *file, shape *shp);
extern shape *SHFreeShape (shape *shp);

extern int SHGetDim (shape *shp);
extern int SHGetExtent (shape *shp, int dim);
extern int SHGetUnrLen (shape *shp);

extern shape *SHSetExtent (shape *shp, int dim, int val);

extern bool SHCompareShapes (shape *a, shape *b);
extern shape *SHAppendShapes (shape *a, shape *b);
extern shape *SHDropFromShape (int n, shape *a);
extern char *SHShape2String (int dots, shape *shp);
extern int *SHShape2IntVec (shape *shp);
extern node *SHShape2Array (shape *shp);

extern shape *SHOldTypes2Shape (types *shpseg);
extern shpseg *SHShape2OldShpseg (shape *shp);

extern bool SHCompareWithCArray (shape *shp, int *shpdata, int dim);
extern bool SHCompareWithArguments (shape *shp, int dim, ...);

#endif /* _shape_h */
