/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 *
 */

#include "free.h"
#include "dbug.h"
#include "shape.h"
#include "cv2scalar.h"
#include "cv2cv.h"
#include "tree_compound.h"

typedef struct CONSTANT {
    simpletype type;
    shape *shape;
    void *elems;
} constant;

#define CONSTANT_TYPE(c) (c->type)
#define CONSTANT_SHAPE(c) (c->shape)
#define CONSTANT_ELEMS(c) (c->elems)

#define SELF
#include "constants.h"

/***
 *** local helper functions:
 ***/

constant *
MakeScalarConstantFromCV (simpletype type, void *val)
{
    constant *res;

    DBUG_ENTER ("MakeScalarConstantFromCV");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = SHMakeShape (0);
    CONSTANT_ELEMS (res) = val;

    DBUG_RETURN (res);
}

void *
PickNElemsFromCV (simpletype type, void *elems, int offset, int length)
{
    void *res;

    DBUG_ENTER ("PickNElemsFromCV");

    res = cv2cv[type](elems, offset, length);

    DBUG_RETURN (res);
}

int
Idx2Offset (constant *idx, constant *a)
{
    int offset;
    int *cvidx;
    int lenidx;
    shape *shp;
    int lenshp;
    int i;

    DBUG_ENTER ("Idx2Offset");
    DBUG_ASSERT ((CONSTANT_TYPE (idx) == T_int), "Idx2Offset called with non-int index");
    DBUG_ASSERT ((COGetDim (idx) == 1), "Idx2Offset called with non-vector index");

    cvidx = (int *)CONSTANT_ELEMS (idx);
    lenidx = SHGetExtent (COGetShape (idx), 0);

    shp = CONSTANT_SHAPE (a);
    lenshp = SHGetDim (shp);

    DBUG_ASSERT ((lenshp >= lenidx), "Idx2Offset called with longer idx than array dim");

    if (lenidx > 0) {
        offset = cvidx[0];
    } else {
        offset = 0;
    }
    for (i = 1; i < lenidx; i++) {
        offset = offset * SHGetExtent (shp, i) + cvidx[i];
    }
    for (; i < lenshp; i++) {
        offset *= SHGetExtent (shp, i + 1);
    }

    DBUG_RETURN (offset);
}

/***
 *** Functions for creating constants:
 ***/

constant *
COMakeConstant (simpletype type, shape *shp, void *elems)
{
    constant *res;

    DBUG_ENTER ("COMakeConstant");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = type;
    CONSTANT_SHAPE (res) = shp;
    CONSTANT_ELEMS (res) = elems;

    DBUG_RETURN (res);
}

constant *
COMakeConstantFromInt (int val)
{
    constant *res;
    int *intelems;

    DBUG_ENTER ("COMakeConstantFromInt");

    res = (constant *)MALLOC (sizeof (constant));
    CONSTANT_TYPE (res) = T_int;
    CONSTANT_SHAPE (res) = SHMakeShape (0);
    intelems = (int *)MALLOC (sizeof (int));
    intelems[0] = val;
    CONSTANT_ELEMS (res) = intelems;

    DBUG_RETURN (res);
}

constant *
COMakeConstantFromArray (node *a)
{
    types *type;
    constant *res;

    type = ARRAY_TYPE (a);

    res = COMakeConstant (TYPES_BASETYPE (type), SHOldTypes2Shape (type),
                          Array2IntVec (ARRAY_AELEMS (a), NULL));
    return (res);
}

/***
 *** Functions for extracting info from constants:
 ***/

simpletype
COGetType (constant *a)
{
    return (CONSTANT_TYPE (a));
}

int
COGetDim (constant *a)
{
    return (SHGetDim (CONSTANT_SHAPE (a)));
}

shape *
COGetShape (constant *a)
{
    return (CONSTANT_SHAPE (a));
}

/***
 *** Functions for converting constants:
 ***/

node *
COConstant2AST (constant *a)
{
    node *res, *exprs;
    int length;
    shape *shp;
    int i;

    if (COGetDim (a) == 0) {
        res = cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), 0);
    } else {
        /* First, we compute the length of the unrolling */
        length = 1;
        shp = CONSTANT_SHAPE (a);
        for (i = COGetDim (a) - 1; i >= 0; i--) {
            length *= SHGetExtent (shp, i);
        }
        /* Now, we build the exprs! */
        exprs = NULL;
        for (i = length - 1; i >= 0; i--) {
            exprs
              = MakeExprs (cv2scalar[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), i), exprs);
        }
        /* Finally, the N_array node is created! */
        res = MakeArray (exprs);
    }
    return (res);
}

/***
 *** Operations on constants:
 ***/

constant *
COPsi (constant *idx, constant *a)
{
    void *val;
    constant *res;

    DBUG_ASSERT ((COGetType (idx) == T_int), "idx to COPsi not int!");
    DBUG_ASSERT ((COGetDim (idx) == 1), "idx to COPsi not vector!");
    DBUG_ASSERT ((COGetDim (a)) == SHGetExtent (COGetShape (idx), 0),
                 "non-element selection in COPsi!");

    val
      = PickNElemsFromCV (CONSTANT_TYPE (a), CONSTANT_ELEMS (a), Idx2Offset (idx, a), 1);
    res = MakeScalarConstantFromCV (COGetType (a), val);

    return (res);
}
