/*
 * $Log$
 * Revision 1.4  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.3  2003/06/11 22:05:36  ktr
 * Added support for multidimensional arrays
 *
 * Revision 1.2  2001/03/22 14:27:12  nmw
 * DebugPrint added
 *
 * Revision 1.1  2001/03/02 14:32:56  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_CONSTANTS_INTERNAL_H_
#define _SAC_CONSTANTS_INTERNAL_H_

/*
 * This header file is ment for internal usage only! I.e., only those
 * files implementing the module constant are allowed to include it!
 *
 */

#include "types.h"
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
#define CONSTANT_ELEMDIM(c) (SHSubarrayDim (CONSTANT_SHAPE (c), CONSTANT_VLEN (c)))
#define CONSTANT_ELEMSIZE(c) (SHGetUnrLen (CONSTANT_SHAPE (c)) / CONSTANT_VLEN (c))

/*
 * here some extern decls for helper functions defined in constants_basic.c
 */

extern constant *MakeConstant (simpletype type, shape *shp, void *elems, int vlen);
extern void *AllocCV (simpletype type, int length);
extern void *PickNElemsFromCV (simpletype type, void *elems, int offset, int length);
extern void CopyElemsFromCVToCV (simpletype type, void *from, int off, int len, void *to,
                                 int to_off);
extern void DbugPrintBinOp (char *fun, constant *arg1, constant *arg2, constant *res);
extern void DbugPrintUnaryOp (char *fun, constant *arg1, constant *res);

#endif /* _SAC_CONSTANTS_INTERNAL_H_ */
