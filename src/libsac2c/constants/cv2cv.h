#ifndef _SAC_CV2CV_H_
#define _SAC_CV2CV_H_

#include "types.h"

extern void COcv2CvUByte (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvUShort (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvUInt (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvULong (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvULongLong (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvByte (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvShort (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvInt (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvLong (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvLongLong (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvBool (void *src, size_t off, size_t len, void *res, size_t res_off);

extern void COcv2CvFloat (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvFloatvec (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvDouble (void *src, size_t off, size_t len, void *res, size_t res_off);
extern void COcv2CvLongDouble (void *src, size_t off, size_t len, void *res, size_t res_off);

extern void COcv2CvChar (void *src, size_t off, size_t len, void *res, size_t res_off);

extern void COcv2CvHidden (void *src, size_t off, size_t len, void *res, size_t res_off);

extern void COcv2CvDummy (void *src, size_t off, size_t len, void *res, size_t res_off);

#endif /* _SAC_CV2CV_H_ */
