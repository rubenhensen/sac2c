#ifndef _SAC_CV2CV_H_
#define _SAC_CV2CV_H_

#include "types.h"

extern void COcv2CvUByte (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvUShort (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvUInt (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvULong (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvULongLong (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvByte (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvShort (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvInt (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvLong (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvLongLong (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvBool (void *src, int off, int len, void *res, int res_off);

extern void COcv2CvFloat (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvFloatvec (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvDouble (void *src, int off, int len, void *res, int res_off);
extern void COcv2CvLongDouble (void *src, int off, int len, void *res, int res_off);

extern void COcv2CvChar (void *src, int off, int len, void *res, int res_off);

extern void COcv2CvHidden (void *src, int off, int len, void *res, int res_off);

extern void COcv2CvDummy (void *src, int off, int len, void *res, int res_off);

#endif /* _SAC_CV2CV_H_ */
