/*
 * $Log$
 * Revision 1.4  1999/10/22 14:13:32  sbs
 * corrected some typos; added SHGetUnrLen for computing the prod of the extents
 *
 * Revision 1.3  1999/10/20 13:28:31  sbs
 * some minor brushing done.
 *
 * Revision 1.2  1999/10/20 13:03:03  sbs
 * typo corrected
 *
 * Revision 1.1  1999/10/11 08:47:34  sbs
 * Initial revision
 *
 *
 */

/*
 * This module implements an abstract datatype for keeping shapes
 * of arbitrary length.
 * Unfortunately, it is not yet used in all parts of the compiler 8-(.
 * In fact, the only part where it is used is the new typechecker.
 * In all other parts of the compiler, the shape is hidden within
 * the (also to be replaced) types structure.
 * Therefore, this module also implements some conversion routines, e.g.
 *   SHOldTypes2Shape
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a shape is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 * - The only function for freeing a shape structure is SHFreeShape!
 * - If the result is a shape structure, it has been freshly allocated!
 *
 */

#include "dbug.h"

#include "tree_basic.h"
#include "free.h"

typedef struct SHAPE {
    int dim;
    int *elems;
} shape;

/*
 * Now, we include the own interface! The reason fot this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 * The only problem this technique has is that we either have to "export"
 * the definition of the the type "shape" (which we want to avoid for
 * software engeneering reasons) or we have to include all but the
 *   typedef void shape;
 * We achieve the later by using the compilation flag "SELF"
 */

#define SELF
#include "shape.h"

/*
 * For internal usage within this module only, we define the following
 * shape access macros:
 */

#define SHAPE_DIM(s) s->dim
#define SHAPE_ELEMS(s) s->elems
#define SHAPE_EXT(s, i) s->elems[i]

/******************************************************************************
 *
 * function:
 *    shape *SHMakeShape( int dim)
 *
 * description:
 *    creates a new shape-structure of size dim which is not yet initialized!
 *
 ******************************************************************************/

shape *
SHMakeShape (int dim)
{
    shape *res;

    DBUG_ENTER ("SHMakeShape");
    DBUG_ASSERT (dim >= 0, ("SHMakeShape called with negative dimensionality!"));

    res = (shape *)MALLOC (sizeof (shape));
    if (dim > 0) {
        SHAPE_ELEMS (res) = (int *)MALLOC (dim * sizeof (int));
    } else {
        SHAPE_ELEMS (res) = NULL;
    }
    SHAPE_DIM (res) = dim;
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    shape *SHCopyShape( shape *shp)
 *
 * description:
 *    duplicates the shape structure given as argument.
 *
 ******************************************************************************/

shape *
SHCopyShape (shape *shp)
{
    shape *res;
    int i, n;

    DBUG_ENTER ("SHCopyShape");
    DBUG_ASSERT ((shp != NULL), ("SHCopyShape called with NULL shape!"));

    n = SHAPE_DIM (shp);
    res = SHMakeShape (n);
    for (i = 0; i < n; i++) {
        SHAPE_EXT (res, i) = SHAPE_EXT (shp, i);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void SHFreeShape( shape *shp)
 *
 * description:
 *    frees the given shape structure.
 *
 ******************************************************************************/

void
SHFreeShape (shape *shp)
{
    DBUG_ENTER ("SHFreeShape");
    DBUG_ASSERT ((shp != NULL), ("SHFreeShape called with NULL shape!"));

    if (SHAPE_DIM (shp) > 0) {
        FREE (SHAPE_ELEMS (shp));
    }
    FREE (shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    int SHGetDim( shape *shp)
 *    int SHGetExtent( shape*shp, int dim)
 *    int SHGetUnrLen( shape*shp)
 *
 * description:
 *    several functions for retrieving shape infos.
 *
 ******************************************************************************/

int
SHGetDim (shape *shp)
{
    DBUG_ENTER ("SHGetDim");
    DBUG_ASSERT ((shp != NULL), ("SHGetDim called with NULL shape!"));

    DBUG_RETURN (SHAPE_DIM (shp));
}

int
SHGetExtent (shape *shp, int dim)
{
    DBUG_ENTER ("SHGetExtent");
    DBUG_ASSERT ((shp != NULL), ("SHGetExtent called with NULL shape!"));
    DBUG_ASSERT ((SHAPE_DIM (shp) > dim) && (dim >= 0),
                 ("SHGetExtent called with dim out of range!"));

    DBUG_RETURN (SHAPE_EXT (shp, dim));
}

int
SHGetUnrLen (shape *shp)
{
    int i, length;

    DBUG_ENTER ("SHGetUnrLen");
    DBUG_ASSERT ((shp != NULL), ("SHGetUnrLen called with NULL shape!"));

    length = 1;
    for (i = SHAPE_DIM (shp) - 1; i >= 0; i--) {
        length *= SHAPE_EXT (shp, i);
    }

    DBUG_RETURN (length);
}

/******************************************************************************
 *
 * function:
 *    shape *SHSetExtent( shape *shp, int dim, int val)
 *
 * description:
 *    several functions for setting shape values.
 *
 *
 ******************************************************************************/

shape *
SHSetExtent (shape *shp, int dim, int val)
{
    DBUG_ENTER ("SHSetExtent");
    DBUG_ASSERT ((shp != NULL), ("SHSetExtent called with NULL shape!"));
    DBUG_ASSERT ((SHAPE_DIM (shp) > dim) && (dim >= 0),
                 ("SHSetExtent called with dim out of range!"));

    SHAPE_EXT (shp, dim) = val;
    DBUG_RETURN (shp);
}

/******************************************************************************
 *
 * function:
 *    shape *SHAppendShapes( shape *a, shape *b)
 *
 * description:
 *    appends two given shapes a and b into a new shape!
 *    Note here that both argument shapes are read only! They are neither
 *    used as part of the result, nor they are free'd.
 *
 ******************************************************************************/

shape *
SHAppendShapes (shape *a, shape *b)
{
    int m, n, i, j;
    shape *res;

    DBUG_ENTER ("SHAppendShapes");
    DBUG_ASSERT ((a != NULL) && (b != NULL), ("SHAppendShapes called with NULL arg!"));

    m = SHAPE_DIM (a);
    n = SHAPE_DIM (b);

    res = SHMakeShape (m + n);
    for (i = 0; i < m; i++) {
        SHAPE_EXT (res, i) = SHAPE_EXT (a, i);
    }
    for (j = 0; j < n; i++, j++) {
        SHAPE_EXT (res, i) = SHAPE_EXT (b, j);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    char * SHShape2String( int dots, shape *shp)
 *
 * description:
 *    generates a string representation of a shape. The argument "dots" allows
 *    some dots to be inserted into the vector representsation. e.g.
 *    SHShape2String( 2, <2,3,4>)  =>  "[.,.,2,3,4]"
 *
 ******************************************************************************/

char *
SHShape2String (int dots, shape *shp)
{
    static char buf[256];
    char *tmp = &buf[0];
    int i, j, n;

    DBUG_ENTER ("SHShape2String");
    DBUG_ASSERT ((shp != NULL), ("SHSHape2String called with NULL shape!"));

    tmp += sprintf (tmp, "[");
    for (i = 0; i < dots; i++) {
        if (i == 0) {
            tmp += sprintf (tmp, ".");
        } else {
            tmp += sprintf (tmp, ",.");
        }
    }
    n = SHAPE_DIM (shp);
    for (j = 0; j < n; i++, j++) {
        if (i == 0) {
            tmp += sprintf (tmp, "%d", SHAPE_EXT (shp, i));
        } else {
            tmp += sprintf (tmp, ",%d", SHAPE_EXT (shp, i));
        }
    }
    tmp += sprintf (tmp, "]");

    DBUG_RETURN (StringCopy (buf));
}

/******************************************************************************
 *
 * function:
 *    shape *SHOldTypes2Shape( types * types)
 *
 * description:
 *    if types has a dim>0 a shape structure is created which carries the same
 *    shape info as the types-node does. Otherwise, NULL is returned.
 *
 ******************************************************************************/

shape *
SHOldTypes2Shape (types *types)
{
    int dim, i, j;
    shape *res;
    shpseg *shpseg;

    DBUG_ENTER ("SHOldTypes2Shape");
    DBUG_ASSERT ((types != NULL), ("SHOldTypes2Shape called with NULL types!"));

    dim = TYPES_DIM (types);
    if (dim > 0) {
        res = SHMakeShape (dim);

        i = 0;
        shpseg = TYPES_SHPSEG (types);
        while (dim > SHP_SEG_SIZE) {
            DBUG_ASSERT ((shpseg != NULL), "types structure corrupted!");
            for (j = 0; j < SHP_SEG_SIZE; j++, i++) {
                SHAPE_EXT (res, i) = SHPSEG_SHAPE (shpseg, j);
            }
            shpseg = SHPSEG_NEXT (shpseg);
            dim -= SHP_SEG_SIZE;
        }
        for (j = 0; j < dim; j++, i++) {
            SHAPE_EXT (res, i) = SHPSEG_SHAPE (shpseg, j);
        }
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    shpseg *SHShape2OldShpseg( shape *shp)
 *
 * description:
 *    if shp has a dim>0 a shpseg structure is created which carries the same
 *    shape info as the shp does. Otherwise, NULL is returned.
 *
 ******************************************************************************/

shpseg *
SHShape2OldShpseg (shape *shp)
{
    int dim, i, j;
    shpseg *res, *curr_seg;

    DBUG_ENTER ("SHShape2OldShpseg");
    DBUG_ASSERT ((shp != NULL), ("SHShape2OldShpseg called with NULL shp!"));

    dim = SHAPE_DIM (shp);
    if (dim > 0) {
        i = 0;
        res = (shpseg *)MALLOC (sizeof (shpseg));
        curr_seg = res;
        while (dim > SHP_SEG_SIZE) {
            for (j = 0; j < SHP_SEG_SIZE; j++, i++) {
                SHPSEG_SHAPE (curr_seg, j) = SHAPE_EXT (shp, i);
            }
            SHPSEG_NEXT (curr_seg) = (shpseg *)MALLOC (sizeof (shpseg));
            curr_seg = SHPSEG_NEXT (curr_seg);
            dim -= SHP_SEG_SIZE;
        }
        for (j = 0; j < dim; j++, i++) {
            SHPSEG_SHAPE (curr_seg, j) = SHAPE_EXT (shp, i);
        }
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}
