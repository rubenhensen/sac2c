/*
 * $Log$
 * Revision 1.2  2001/03/22 14:27:31  nmw
 * functions to convert float, bool, char added
 *
 * Revision 1.1  2001/03/02 14:33:03  sbs
 * Initial revision
 *
 * Revision 3.1  2000/11/20 18:00:05  sacbase
 * new release made
 *
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
extern node *COCv2Bool (void *elems, int offset);
extern node *COCv2Float (void *elems, int offset);
extern node *COCv2Char (void *elems, int offset);
extern node *COCv2ScalarDummy (void *elems, int offset);

#endif /* _cv2scalar_h */
