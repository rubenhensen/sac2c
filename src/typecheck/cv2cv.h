/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _cv2cv_h
#define _cv2cv_h

typedef void *(*cv2cvfunptr) (void *, int, int);

extern cv2cvfunptr cv2cv[];

extern void *COCv2CvInt (void *elems, int offset, int length);
extern void *COCv2CvDouble (void *elems, int offset, int length);
extern void *COCv2CvFloat (void *elems, int offset, int length);

extern void *COCv2CvDummy (void *elems, int offset, int length);

#endif /* _cv2cv_h */
