/*
 * $Log$
 * Revision 1.4  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.3  2001/04/30 12:30:20  nmw
 * COCv2CvHidden() added
 *
 * Revision 1.2  2001/04/04 09:59:47  nmw
 *  missing convert functions for basetype char added
 *
 * Revision 1.1  2001/03/02 14:33:00  sbs
 * Initial revision
 *
 * Revision 3.1  2000/11/20 18:00:04  sacbase
 * new release made
 *
 * Revision 1.2  1999/10/22 14:15:19  sbs
 * commented and added versions for almost all simpletypes...
 *
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_CV2CV_H_
#define _SAC_CV2CV_H_

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

extern void COCv2CvChar (void *src, int off, int len, void *res, int res_off);

extern void COCv2CvHidden (void *src, int off, int len, void *res, int res_off);

extern void COCv2CvDummy (void *src, int off, int len, void *res, int res_off);

#endif /* _SAC_CV2CV_H_ */
