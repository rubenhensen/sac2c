/*
 * $Log$
 * Revision 1.1  2001/05/02 08:00:21  nmw
 * Initial revision
 *
 *
 */

#ifndef _basecv_h
#define _basecv_h

#include "constants.h"
#include "shape.h"

typedef constant *(*basecvfunptr) (shape *shp);

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

#endif /* _basecv_h */
