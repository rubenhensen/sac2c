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
    shape *mshape;
    void *elems;
    size_t vlen;
};

/*
 * For internal usage within this module only, we define the following
 * access macros:
 */

#define CONSTANT_TYPE(c) (c->type)
#define CONSTANT_SHAPE(c) (c->mshape)
#define CONSTANT_ELEMS(c) (c->elems)
#define CONSTANT_VLEN(c) (c->vlen)

#define CONSTANT_DIM(c) (SHgetDim (CONSTANT_SHAPE (c)))
#define CONSTANT_ELEMDIM(c) (SHSubarrayDim (CONSTANT_SHAPE (c), CONSTANT_VLEN (c)))
#define CONSTANT_ELEMSIZE(c) (SHgetUnrLen (CONSTANT_SHAPE (c)) / CONSTANT_VLEN (c))

#define CONSTANT_SIZEOF(__c, __t) (CONSTANT_VLEN (__c) * global.basetype_size[__t])

/*
 * here some extern decls for helper functions defined in constants_basic.c
 */

extern constant *COINTmakeConstant (simpletype type, shape *shp, void *elems, size_t vlen);
extern void *COINTallocCV (simpletype type, size_t length);
extern void *COINTpickNElemsFromCV (simpletype type, void *elems, size_t offset, size_t length);
extern void COINTcopyElemsFromCVToCV (simpletype type, void *from, size_t off, size_t len,
                                      void *to, size_t to_off);
extern void COINTdbugPrintBinOp (char *fun, constant *arg1, constant *arg2,
                                 constant *res);
extern void COINTdbugPrintUnaryOp (char *fun, constant *arg1, constant *res);

#endif /* _SAC_CONSTANTS_INTERNAL_H_ */
