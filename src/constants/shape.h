/*
 *
 * $Log$
 * Revision 1.11  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.10  2004/10/14 11:47:49  sbs
 * SHShape2Exprs added
 *
 * Revision 1.9  2004/09/27 13:15:20  sah
 * added serialization support
 *
 * Revision 1.8  2004/09/22 20:07:57  sah
 * added SHSerializeShape
 *
 * Revision 1.7  2003/06/11 22:05:36  ktr
 * Added support for multidimensional arrays
 *
 * Revision 1.6  2002/11/04 17:41:31  sbs
 * split off SHOldShpseg2Shape from SHOldTypes2Shape in order
 * to alow preventing flattening of user defined types much better now.
 *
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

#ifndef _SAC_SHAPE_H_
#define _SAC_SHAPE_H_

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

#include <stdio.h>
#include "types.h"

extern shape *SHMakeShape (int dim);
extern shape *SHCreateShape (int dim, ...);
extern shape *SHCopyShape (shape *shp);
extern void SHPrintShape (FILE *file, shape *shp);
extern shape *SHFreeShape (shape *shp);
extern void SHSerializeShape (FILE *file, shape *shp);

extern int SHGetDim (shape *shp);
extern int SHGetExtent (shape *shp, int dim);
extern int SHGetUnrLen (shape *shp);

extern int SHSubarrayDim (shape *shp, int n);

extern shape *SHSetExtent (shape *shp, int dim, int val);

extern bool SHCompareShapes (shape *a, shape *b);
extern shape *SHAppendShapes (shape *a, shape *b);
extern shape *SHDropFromShape (int n, shape *a);
extern shape *SHTakeFromShape (int n, shape *a);
extern char *SHShape2String (int dots, shape *shp);
extern int *SHShape2IntVec (shape *shp);
extern node *SHShape2Exprs (shape *shp);
extern node *SHShape2Array (shape *shp);

extern shape *SHOldTypes2Shape (types *shpseg);
extern shape *SHOldShpseg2Shape (int dim, shpseg *shpseg);
extern shpseg *SHShape2OldShpseg (shape *shp);

extern bool SHCompareWithCArray (shape *shp, int *shpdata, int dim);
extern bool SHCompareWithArguments (shape *shp, int dim, ...);

#endif /* _SAC_SHAPE_H_ */
