#ifndef _SAC_BASECV_H_
#define _SAC_BASECV_H_

#include "types.h"

#define EXT_DECLS(fun)                                                                   \
    extern constant *CObaseCvUByte##fun (shape *shp);                                    \
    extern constant *CObaseCvUShort##fun (shape *shp);                                   \
    extern constant *CObaseCvUInt##fun (shape *shp);                                     \
    extern constant *CObaseCvULong##fun (shape *shp);                                    \
    extern constant *CObaseCvULongLong##fun (shape *shp);                                \
    extern constant *CObaseCvByte##fun (shape *shp);                                     \
    extern constant *CObaseCvChar##fun (shape *shp);                                     \
    extern constant *CObaseCvShort##fun (shape *shp);                                    \
    extern constant *CObaseCvInt##fun (shape *shp);                                      \
    extern constant *CObaseCvLong##fun (shape *shp);                                     \
    extern constant *CObaseCvLongLong##fun (shape *shp);                                 \
                                                                                         \
    extern constant *CObaseCvFloat##fun (shape *shp);                                    \
    extern constant *CObaseCvFloatvec##fun (shape *shp);                                 \
    extern constant *CObaseCvDouble##fun (shape *shp);                                   \
    extern constant *CObaseCvLongDouble##fun (shape *shp);                               \
                                                                                         \
    extern constant *CObaseCvBool##fun (shape *shp);                                     \
                                                                                         \
    extern constant *CObaseCvDummy##fun (shape *shp);

EXT_DECLS (NegativeOne)
EXT_DECLS (Zero)
EXT_DECLS (One)

#undef EXT_DECLS

#endif /* _SAC_BASECV_H_ */
