/*
 *
 * $Log$
 * Revision 1.11  2003/06/11 22:05:36  ktr
 * Added support for multidimensional arrays
 *
 * Revision 1.10  2002/11/04 17:41:31  sbs
 * split off SHOldShpseg2Shape from SHOldTypes2Shape in order
 * to alow preventing flattening of user defined types much better now.
 *
 * Revision 1.9  2002/11/04 13:22:08  sbs
 * SHDropFromShape added.
 *
 * Revision 1.8  2002/06/21 14:04:13  dkr
 * SHShape2Array() added
 *
 * Revision 1.6  2001/05/22 14:59:08  nmw
 * OldTypes2Shape is now aware of user defined types
 *
 * Revision 1.5  2001/05/17 12:57:46  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 1.4  2001/05/03 16:55:13  nmw
 * COTypes2OldShapes can handle scalars correctly now
 *
 * Revision 1.3  2001/04/30 12:31:34  nmw
 * SHShape2IntVec added
 *
 * Revision 1.2  2001/03/05 16:57:04  sbs
 * SHCompareShapes added
 *
 * Revision 1.1  2001/03/02 14:33:07  sbs
 * Initial revision
 *
 */

/**
 *
 * @defgroup shape Shape
 *
 * @brief Shape is an abstract datatype for keeping shapes of
 *        arbitrary length.
 *
 * @{
 */

/**
 *
 * @file shape.c
 *
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
#include <stdarg.h>
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

/*
 * Now, we include the own interface! The reason for this is twofold:
 * First, it ensures consistency betweeen the interface and the
 * implementation and second, it serves as a forward declaration for all
 * functions.
 */
#include "shape.h"

/**
 * The structure to store shape/dim information in.
 */
struct SHAPE {
    int dim;
    int *elems;
};

/*
 * For internal usage within this module only, we define the following
 * shape access macros:
 */

#define SHAPE_DIM(s) (s->dim)
#define SHAPE_ELEMS(s) (s->elems)
#define SHAPE_EXT(s, i) (s->elems[i])

/** <!--********************************************************************-->
 *
 * @fn shape *SHMakeShape( int dim)
 *
 * @brief creates a new shape-structure of size dim which is not yet initialized
 *
 * @param dim size of the shape structure to be created
 *
 * @return a new shape structure of size dim
 *
 ******************************************************************************/

shape *
SHMakeShape (int dim)
{
    shape *res;

    DBUG_ENTER ("SHMakeShape");
    DBUG_ASSERT (dim >= 0, ("SHMakeShape called with negative dimensionality!"));

    res = (shape *)Malloc (sizeof (shape));
    if (dim > 0) {
        SHAPE_ELEMS (res) = (int *)Malloc (dim * sizeof (int));
    } else {
        SHAPE_ELEMS (res) = NULL;
    }
    SHAPE_DIM (res) = dim;
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHCreateShape( int dim, ...)
 *
 * @brief creates a new shape-structure of size dim which is initialized by
 *        the given argument values
 *
 * @param dim size of the shape structure to be created
 * @param ... initialization values
 *
 * @return new allocated and initialized shape
 *
 ******************************************************************************/

shape *
SHCreateShape (int dim, ...)
{
    va_list Argp;
    int i;
    shape *result;

    DBUG_ENTER ("SHCreateShape");
    result = SHMakeShape (dim);

    DBUG_ASSERT (result != NULL, ("CreateShape: Get NULL shape from MakeShape!"));

    if (dim > 0) {
        va_start (Argp, dim);
        for (i = 0; i < dim; i++) {
            result = SHSetExtent (result, i, va_arg (Argp, int));
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHCopyShape( shape *shp)
 *
 * @brief duplicates the shape structure given as argument.
 *
 * @param shp the shape structure to be duplicated
 *
 * @return a duplicate of the shape structure shp
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

/** <!--********************************************************************-->
 *
 * @fn void SHPrintShape( FILE *file, shape *shp)
 *
 * @brief prints the contents of shp to file.
 *
 * @param file output file
 * @param shp the shape structure to be printed
 *
 ******************************************************************************/

void
SHPrintShape (FILE *file, shape *shp)
{
    int i;

    DBUG_ENTER ("SHPrintShape");
    DBUG_ASSERT ((shp != NULL), ("SHPrintShape called with NULL shape!"));

    fprintf (file, "[ ");
    if (SHAPE_DIM (shp) > 0) {
        fprintf (file, " %d", SHAPE_EXT (shp, 0));
    }
    for (i = 1; i < SHAPE_DIM (shp); i++) {
        fprintf (file, ", %d", SHAPE_EXT (shp, i));
    }
    fprintf (file, "]");

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHFreeShape( shape *shp)
 *
 * @brief frees the given shape structure.
 *
 * @param shp the shape structure to be freed
 *
 * @return hopefully NULL
 *
 ******************************************************************************/

shape *
SHFreeShape (shape *shp)
{
    DBUG_ENTER ("SHFreeShape");
    DBUG_ASSERT ((shp != NULL), ("SHFreeShape called with NULL shape!"));

    if (SHAPE_DIM (shp) > 0) {
        SHAPE_ELEMS (shp) = Free (SHAPE_ELEMS (shp));
    }
    shp = Free (shp);

    DBUG_RETURN (shp);
}

/** <!--********************************************************************-->
 *
 * @fn int SHGetDim( shape *shp)
 *
 * @brief return the dimension of the given shape structure
 *
 ******************************************************************************/

int
SHGetDim (shape *shp)
{
    DBUG_ENTER ("SHGetDim");
    DBUG_ASSERT ((shp != NULL), ("SHGetDim called with NULL shape!"));

    DBUG_RETURN (SHAPE_DIM (shp));
}

/** <!--********************************************************************-->
 *
 * @fn int SHGetExtent( shape *shp, int dim)
 *
 * @brief return the shape's extent along the given dimension
 *
 ******************************************************************************/
int
SHGetExtent (shape *shp, int dim)
{
    DBUG_ENTER ("SHGetExtent");
    DBUG_ASSERT ((shp != NULL), ("SHGetExtent called with NULL shape!"));
    DBUG_ASSERT ((SHAPE_DIM (shp) > dim) && (dim >= 0),
                 ("SHGetExtent called with dim out of range!"));

    DBUG_RETURN (SHAPE_EXT (shp, dim));
}

/** <!--********************************************************************-->
 *
 * @fn int SHGetUnrLen( shape *shp)
 *
 * @brief return the length of a vector if shp was to be unrolled
 *
 ******************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @fn int SHSubarrayDim( shape *shp, int n)
 *
 * @brief returns the dimension of each subarray if an array of shape shp
 *        would be broken up in n subarrays.
 *
 * @param shp the shape structure
 * @param n the number of subarrays.
 *
 * @return the dimensionality of each subarray
 *
 ******************************************************************************/

int
SHSubarrayDim (shape *shp, int n)
{
    int i, length;

    DBUG_ENTER ("SHSubarrayDim");
    DBUG_ASSERT ((shp != NULL), ("SHSubarrayDim called with NULL shape!"));

    length = 1;
    i = 0;

    while ((length != n) && (i < SHAPE_DIM (shp)))
        length *= SHAPE_EXT (shp, i++);

    DBUG_ASSERT ((length == n), ("SHSubarrayDim called with invalid arguments."));

    DBUG_RETURN (SHAPE_DIM (shp) - i);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHSetExtent( shape *shp, int dim, int val)
 *
 * @brief function to set the shape extent along a given axis
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

/** <!--********************************************************************-->
 *
 * @fn bool SHCompareShapes( shape *a, shape *b)
 *
 * @brief compares two given shapes a and b
 *
 * @return true iff they are identical.
 *
 ******************************************************************************/

bool
SHCompareShapes (shape *a, shape *b)
{
    bool res;
    int i;

    DBUG_ENTER ("SHCompareShapes");

    res = TRUE;
    if (SHAPE_DIM (a) == SHAPE_DIM (b)) {
        for (i = 0; i < SHAPE_DIM (a); i++)
            if (SHAPE_EXT (a, i) != SHAPE_EXT (b, i))
                res = FALSE;
    } else {
        res = FALSE;
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHAppendShapes( shape *a, shape *b)
 *
 * @brief appends two given shapes a and b into a new shape.
 *        Note here that both argument shapes are read only! They are neither
 *        used as part of the result, nor they are free'd.
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

/** <!--********************************************************************-->
 *
 * @fn shape *SHDropFromShape( int n, shape *a)
 *
 * @brief creates a new shape from a by dropping n elements.
 *        If n < 0, the the last n elements will be dropped!
 *        'a' is inspected only! The result shape is freshly created!
 *
 ******************************************************************************/

shape *
SHDropFromShape (int n, shape *a)
{
    int m, i;
    shape *res;

    DBUG_ENTER ("SHDropFromShape");
    DBUG_ASSERT ((a != NULL), ("SHDropFromShape called with NULL arg!"));

    m = SHAPE_DIM (a);
    DBUG_ASSERT ((m - abs (n)) >= 0, "dropping more elems from shape than available!");

    if (n < 0) {
        res = SHMakeShape (m + n);
        for (i = 0; i < m + n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i);
        }
    } else {
        res = SHMakeShape (m - n);
        for (i = 0; i < m - n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i + n);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHTakeFromShape( int n, shape *a)
 *
 * @brief creates a new shape from a by taking n elements.
 *        If n < 0, the the last n elements will be taken!
 *        'a' is inspected only! The result shape is freshly created!
 *
 ******************************************************************************/

shape *
SHTakeFromShape (int n, shape *a)
{
    int m, i;
    shape *res;

    DBUG_ENTER ("SHDropFromShape");
    DBUG_ASSERT ((a != NULL), ("SHDropFromShape called with NULL arg!"));

    m = SHAPE_DIM (a);
    DBUG_ASSERT ((m - abs (n)) >= 0, "taking more elems from shape than available!");

    if (n > 0) {
        res = SHMakeShape (n);
        for (i = 0; i < n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i);
        }
    } else {
        res = SHMakeShape (n);
        for (i = 0; i < n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i + m - n);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char *SHShape2String( int dots, shape *shp)
 *
 * @brief generates a string representation of a shape.
 *        The argument "dots" allows some dots to be inserted into the vector
 *        representsation. e.g. SHShape2String( 2, <2,3,4>)  =>  "[.,.,2,3,4]"
 *
 ******************************************************************************/

char *
SHShape2String (int dots, shape *shp)
{
    static char buf[256];
    char *tmp = &buf[0];
    int i, j, n;

    DBUG_ENTER ("SHShape2String");
    DBUG_ASSERT ((shp != NULL), ("SHShape2String called with NULL shape!"));

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

/** <!--********************************************************************-->
 *
 * @fn shape *SHOldTypes2Shape( types *types)
 *
 * @brief if types has a dim>=0 a shape structure is created which carries the same
 *        shape info as the types-node does. Otherwise, NULL is returned.
 *
 ******************************************************************************/
shape *
SHOldTypes2Shape (types *types)
{
    int dim;
    shape *res;
    shpseg *shpseg;

    DBUG_ENTER ("SHOldTypes2Shape");
    DBUG_ASSERT ((types != NULL), ("SHOldTypes2Shape called with NULL types!"));

    /* this function handle user defined types, too */
    shpseg = Type2Shpseg (types, &dim);

    res = SHOldShpseg2Shape (dim, shpseg);

    if (shpseg != NULL) {
        shpseg = FreeShpseg (shpseg);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHOldShpseg2Shape( int dim, shpseg *shpseg)
 *
 * @brief iff dim > 0 a new shape structure is created which contains the same
 *        shape info as the shpseg does. Otherwise, NULL is returned.
 *
 ******************************************************************************/
shape *
SHOldShpseg2Shape (int dim, shpseg *shpseg)
{
    int i, j;
    shape *res;

    DBUG_ENTER ("SHOldShpseg2Shape");

    if (dim >= 0) {
        res = SHMakeShape (dim);

        if (dim > 0) {
            i = 0;
            while (dim > SHP_SEG_SIZE) {
                DBUG_ASSERT ((shpseg != NULL),
                             "SHOldShpseg2Shape called with NULL shpseg but dim >0!");
                for (j = 0; j < SHP_SEG_SIZE; j++, i++) {
                    SHAPE_EXT (res, i) = SHPSEG_SHAPE (shpseg, j);
                }
                shpseg = SHPSEG_NEXT (shpseg);
                dim -= SHP_SEG_SIZE;
            }
            for (j = 0; j < dim; j++, i++) {
                SHAPE_EXT (res, i) = SHPSEG_SHAPE (shpseg, j);
            }
        }
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shpseg *SHShape2OldShpseg( shape *shp)
 *
 * @brief if shp has a dim>0 a shpseg structure is created which carries the
 *        same shape info as the shp does. Otherwise, NULL is returned.
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
        res = MakeShpseg (NULL);
        curr_seg = res;
        while (dim > SHP_SEG_SIZE) {
            for (j = 0; j < SHP_SEG_SIZE; j++, i++) {
                SHPSEG_SHAPE (curr_seg, j) = SHAPE_EXT (shp, i);
            }
            SHPSEG_NEXT (curr_seg) = MakeShpseg (NULL);
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

/** <!--********************************************************************-->
 *
 * @fn bool SHCompareWithCArray( shape *shp, int* shpdata, int dim)
 *
 * @brief compares given shape with a shape specified as a c integer array
 *
 * @return true iff shapes are equal in dim and shape vector
 *         false otherwise
 *
 ******************************************************************************/
bool
SHCompareWithCArray (shape *shp, int *shpdata, int dim)
{
    bool flag;
    int i;

    DBUG_ENTER ("SHCompareWithCArray");

    flag = TRUE;
    DBUG_ASSERT ((shp != NULL && shpdata != NULL),
                 ("SHCompareWithCArray called with NULL pointer(s)!\n"));
    if (dim == SHAPE_DIM (shp)) {
        for (i = 0; i < dim; i++)
            if (SHAPE_EXT (shp, i) != shpdata[i])
                flag = FALSE;
    } else {
        flag = FALSE;
    }

    DBUG_RETURN (flag);
}

/** <!--********************************************************************-->
 *
 * @fn bool SHCompareWithArguments( shape *shp, int dim, ...)
 *
 * @brief compares given shape with a shape specified as an list of arguments.
 *        usage: e.g. SHCompareWithArguments(shp, 3, 4,4,5)
 *
 * @return true iff shapes are equal in dim and shape vector
 *         false otherwise
 *
 ******************************************************************************/

bool
SHCompareWithArguments (shape *shp, int dim, ...)
{
    va_list Argp;
    bool flag;
    int i;

    DBUG_ENTER ("SHCompareWithArguments");

    flag = TRUE;
    DBUG_ASSERT ((shp != NULL), ("SHCompareWithCArray called with NULL pointer(s)!\n"));
    if (dim == SHAPE_DIM (shp)) {
        va_start (Argp, dim);
        for (i = 0; i < dim; i++)
            if (SHAPE_EXT (shp, i) != va_arg (Argp, int))
                flag = FALSE;
    } else {
        flag = FALSE;
    }

    DBUG_RETURN (flag);
}

/** <!--********************************************************************-->
 *
 * @fn int *SHShape2IntVec( shape *shp)
 *
 * @brief creates a simple int vector from the given shape vector
 *
 ******************************************************************************/

int *
SHShape2IntVec (shape *shp)
{
    int *int_vec;
    int i;
    int n;

    DBUG_ENTER ("SHShape2IntVec");

    n = SHAPE_DIM (shp);
    if (n > 0) {
        int_vec = (int *)Malloc (n * sizeof (int));
        for (i = 0; i < n; i++) {
            int_vec[i] = SHAPE_EXT (shp, i);
        }
    } else {
        int_vec = NULL;
    }

    DBUG_RETURN (int_vec);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHShape2Array( shape *shp)
 *
 * @brief creates a simple int vector from the given shape vector
 *
 ******************************************************************************/

node *
SHShape2Array (shape *shp)
{
    node *array;
    shpseg *shp_seg;
    int dim;
    int i;

    DBUG_ENTER ("SHShape2Array");

    dim = SHAPE_DIM (shp);
    array = NULL;
    for (i = dim - 1; i >= 0; i--) {
        array = MakeExprs (MakeNum (SHAPE_EXT (shp, i)), array);
    }
    array = MakeFlatArray (array);

    shp_seg = MakeShpseg (NULL);
    SHPSEG_SHAPE (shp_seg, 0) = dim;
    ARRAY_TYPE (array) = MakeTypes (T_int, 1, shp_seg, NULL, NULL);

    DBUG_RETURN (array);
}

/*@}*/ /* defgroup shape */
