/*
 * $Log$
 * Revision 1.2  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.1  2001/05/02 08:00:21  nmw
 * Initial revision
 *
 *
 */

#ifndef _SAC_BASECV_H_
#define _SAC_BASECV_H_

#include "types.h"

extern basecvfunptr basecv_zero[];
extern basecvfunptr basecv_one[];

#define EXT_DECLS(fun)                                                                   \
    extern constant *COBaseCvUShort##fun (shape *shp);                                   \
    extern constant *COBaseCvUInt##fun (shape *shp);                                     \
    extern constant *COBaseCvULong##fun (shape *shp);                                    \
    extern constant *COBaseCvShort##fun (shape *shp);                                    \
    extern constant *COBaseCvInt##fun (shape *shp);                                      \
    extern constant *COBaseCvLong##fun (shape *shp);                                     \
                                                                                         \
    extern constant *COBaseCvFloat##fun (shape *shp);                                    \
    extern constant *COBaseCvDouble##fun (shape *shp);                                   \
    extern constant *COBaseCvLongDouble##fun (shape *shp);                               \
                                                                                         \
    extern constant *COBaseCvBool##fun (shape *shp);                                     \
                                                                                         \
    extern constant *COBaseCvDummy##fun (shape *shp);

EXT_DECLS (Zero)
EXT_DECLS (One)

#endif /* _SAC_BASECV_H_ */
