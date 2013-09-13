#ifndef _SAC_CV2SCALAR_H_
#define _SAC_CV2SCALAR_H_

#include "types.h"

extern node *COcv2Num (void *elems, int offset);
extern node *COcv2Numbyte (void *elems, int offset);
extern node *COcv2Numshort (void *elems, int offset);
extern node *COcv2Numint (void *elems, int offset);
extern node *COcv2Numlong (void *elems, int offset);
extern node *COcv2Numlonglong (void *elems, int offset);
extern node *COcv2Numubyte (void *elems, int offset);
extern node *COcv2Numushort (void *elems, int offset);
extern node *COcv2Numuint (void *elems, int offset);
extern node *COcv2Numulong (void *elems, int offset);
extern node *COcv2Numulonglong (void *elems, int offset);
extern node *COcv2Double (void *elems, int offset);
extern node *COcv2Bool (void *elems, int offset);
extern node *COcv2Float (void *elems, int offset);
extern node *COcv2Floatvec (void *elems, int offset);
extern node *COcv2Char (void *elems, int offset);
extern node *COcv2ScalarDummy (void *elems, int offset);

#endif /* _SAC_CV2SCALAR_H_ */
