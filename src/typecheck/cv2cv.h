/*
 * $Log$
 * Revision 1.2  1999/10/22 14:15:19  sbs
 * commented and added versions for almost all simpletypes...
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _cv2cv_h
#define _cv2cv_h

typedef void (*cv2cvfunptr) (void *, int, int, void *, int);

extern cv2cvfunptr cv2cv[];

extern void COCv2CvUShort (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvUInt (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvULong (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvShort (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvInt (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvLong (void *src, int off, int len, void *res, int res_off);

extern void COCv2CvFloat (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvDouble (void *src, int off, int len, void *res, int res_off);
extern void COCv2CvLongDouble (void *src, int off, int len, void *res, int res_off);

extern void COCv2CvDummy (void *src, int off, int len, void *res, int res_off);

#endif /* _cv2cv_h */
