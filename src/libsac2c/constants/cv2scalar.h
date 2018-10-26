#ifndef _SAC_CV2SCALAR_H_
#define _SAC_CV2SCALAR_H_

#include "types.h"

extern node *COcv2Num (void *elems, size_t offset);
extern node *COcv2Numbyte (void *elems, size_t offset);
extern node *COcv2Numshort (void *elems, size_t offset);
extern node *COcv2Numint (void *elems, size_t offset);
extern node *COcv2Numlong (void *elems, size_t offset);
extern node *COcv2Numlonglong (void *elems, size_t offset);
extern node *COcv2Numubyte (void *elems, size_t offset);
extern node *COcv2Numushort (void *elems, size_t offset);
extern node *COcv2Numuint (void *elems, size_t offset);
extern node *COcv2Numulong (void *elems, size_t offset);
extern node *COcv2Numulonglong (void *elems, size_t offset);
extern node *COcv2Double (void *elems, size_t offset);
extern node *COcv2Bool (void *elems, size_t offset);
extern node *COcv2Float (void *elems, size_t offset);
extern node *COcv2Floatvec (void *elems, size_t offset);
extern node *COcv2Char (void *elems, size_t offset);
extern node *COcv2ScalarDummy (void *elems, size_t offset);

#endif /* _SAC_CV2SCALAR_H_ */
