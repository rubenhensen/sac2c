/*
 * $Log$
 * Revision 1.1  2001/03/02 14:32:56  sbs
 * Initial revision
 *
 *
 */

#ifndef _constants_internal_h
#define _constants_internal_h

/*
 * This header file is ment for internal usage only! I.e., only those
 * files implementing the module constant are allowed to include it!
 *
 */

#include "shape.h"

struct CONSTANT {
    simpletype type;
    shape *shape;
    void *elems;
    int vlen;
};

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define CONSTANT_TYPE(c) (c->type)
#define CONSTANT_SHAPE(c) (c->shape)
#define CONSTANT_ELEMS(c) (c->elems)
#define CONSTANT_VLEN(c) (c->vlen)

#define CONSTANT_DIM(c) (SHGetDim (CONSTANT_SHAPE (c)))

/*
 * here some extern decls for helper functions defined in constants_basic.c
 */

extern constant *MakeConstant (simpletype type, shape *shp, void *elems, int vlen);
extern void *AllocCV (simpletype type, int length);
extern void *PickNElemsFromCV (simpletype type, void *elems, int offset, int length);
extern void CopyElemsFromCVToCV (simpletype type, void *from, int off, int len, void *to,
                                 int to_off);
extern void DbugPrintBinOp (char *fun, constant *arg1, constant *arg2, constant *res);

#endif /* _constants_internal_h */
