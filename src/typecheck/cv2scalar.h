/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _cv2scalar_h
#define _cv2scalar_h

#include "types.h"

typedef node *(*cv2scalarfunptr) (void *, int);

extern cv2scalarfunptr cv2scalar[];

extern node *COCv2Num (void *elems, int offset);
extern node *COCv2Double (void *elems, int offset);
extern node *COCv2ScalarDummy (void *elems, int offset);

#endif /* _cv2scalar_h */
