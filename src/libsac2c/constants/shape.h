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

extern shape *SHmakeShape (int dim);
extern shape *SHcreateShape (int dim, ...);
#if IS_CYGWIN
extern shape *SHcreateShapeVa (int dim, va_list Argp);
#endif

extern shape *SHcopyShape (shape *shp);
extern void SHprintShape (FILE *file, shape *shp);
extern shape *SHfreeShape (shape *shp);
extern void SHserializeShape (FILE *file, shape *shp);
extern void SHtouchShape (shape *shp, info *arg_info);

extern int SHgetDim (shape *shp);
extern int SHgetExtent (shape *shp, int dim);
extern long long SHgetUnrLen (shape *shp);

extern int SHsubarrayDim (shape *shp, int n);

extern shape *SHsetExtent (shape *shp, int dim, int val);

extern bool SHcompareShapes (shape *a, shape *b);
extern shape *SHappendShapes (shape *a, shape *b);
extern shape *SHdropFromShape (int n, shape *a);
extern shape *SHtakeFromShape (int n, shape *a);
extern char *SHshape2String (size_t dots, shape *shp);
extern int *SHshape2IntVec (shape *shp);
extern node *SHshape2Exprs (shape *shp);
extern node *SHshape2Array (shape *shp);
extern shape *SHarray2Shape (node *array);

extern bool SHcompareWithCArray (shape *shp, int *shpdata, int dim);
extern bool SHcompareWithArguments (shape *shp, int dim, ...);

#endif /* _SAC_SHAPE_H_ */
