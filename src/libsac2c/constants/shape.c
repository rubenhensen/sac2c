/**
 *
 * $Id$
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
 *   SHoldTypes2Shape
 *
 * For avoiding un-intended pointer sharing and for avoiding memory leaks
 * we establish the following rules:
 * - whenever a shape is given as argument, it will be inspected only!
 *   Neither the pointer to it nor any pointer to a sub structure will be
 *   returned or used within a data structure that serves as a result!
 * - The only function for freeing a shape structure is SHfreeShape!
 * - If the result is a shape structure, it has been freshly allocated!
 *
 */
#include <stdarg.h>
#include "shape.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "check_mem.h"

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
 * @fn shape *SHmakeShape( int dim)
 *
 * @brief creates a new shape-structure of size dim which is not yet initialized
 *
 * @param dim size of the shape structure to be created
 *
 * @return a new shape structure of size dim
 *
 ******************************************************************************/

shape *
SHmakeShape (int dim)
{
    shape *res;

    DBUG_ENTER ("SHmakeShape");
    DBUG_ASSERT (dim >= 0, ("SHmakeShape called with negative dimensionality!"));

    res = (shape *)MEMmalloc (sizeof (shape));
    if (dim > 0) {
        SHAPE_ELEMS (res) = (int *)MEMmalloc (dim * sizeof (int));
    } else {
        SHAPE_ELEMS (res) = NULL;
    }
    SHAPE_DIM (res) = dim;
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHcreateShape( int dim, ...)
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
SHcreateShape (int dim, ...)
{
    va_list Argp;
    int i;
    shape *result;

    DBUG_ENTER ("SHcreateShape");
    result = SHmakeShape (dim);

    DBUG_ASSERT (result != NULL, ("CreateShape: Get NULL shape from MakeShape!"));

    if (dim > 0) {
        va_start (Argp, dim);
        for (i = 0; i < dim; i++) {
            result = SHsetExtent (result, i, va_arg (Argp, int));
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHcopyShape( shape *shp)
 *
 * @brief duplicates the shape structure given as argument.
 *
 * @param shp the shape structure to be duplicated
 *
 * @return a duplicate of the shape structure shp
 *
 ******************************************************************************/

shape *
SHcopyShape (shape *shp)
{
    shape *res;
    int i, n;

    DBUG_ENTER ("SHcopyShape");
    DBUG_ASSERT ((shp != NULL), ("SHcopyShape called with NULL shape!"));

    n = SHAPE_DIM (shp);
    res = SHmakeShape (n);
    for (i = 0; i < n; i++) {
        SHAPE_EXT (res, i) = SHAPE_EXT (shp, i);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn void SHprintShape( FILE *file, shape *shp)
 *
 * @brief prints the contents of shp to file.
 *
 * @param file output file
 * @param shp the shape structure to be printed
 *
 ******************************************************************************/

void
SHprintShape (FILE *file, shape *shp)
{
    int i;

    DBUG_ENTER ("SHprintShape");
    DBUG_ASSERT ((shp != NULL), ("SHprintShape called with NULL shape!"));

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
 * @fn shape *SHfreeShape( shape *shp)
 *
 * @brief frees the given shape structure.
 *
 * @param shp the shape structure to be freed
 *
 * @return hopefully NULL
 *
 ******************************************************************************/

shape *
SHfreeShape (shape *shp)
{
    DBUG_ENTER ("SHfreeShape");
    DBUG_ASSERT ((shp != NULL), ("SHfreeShape called with NULL shape!"));

    if (SHAPE_DIM (shp) > 0) {
        SHAPE_ELEMS (shp) = MEMfree (SHAPE_ELEMS (shp));
    }
    shp = MEMfree (shp);

    DBUG_RETURN (shp);
}

/** <!--********************************************************************-->
 *
 * @fn void SHtouchShape( shape *shp, info *arg_info)
 *
 * @brief touches the given shape structure.
 *
 * @param *shp
 *
 * @param *arg_info
 *
 ******************************************************************************/

void
SHtouchShape (shape *shp, info *arg_info)
{
    DBUG_ENTER ("SHtouchShape");
    DBUG_ASSERT ((shp != NULL), ("SHtouchShape called with NULL shape!"));

    if (SHAPE_DIM (shp) > 0) {
        CHKMtouch (SHAPE_ELEMS (shp), arg_info);
    }
    CHKMtouch (shp, arg_info);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void SHserializeShape( FILE *file, shape *shp)
 *
 * @brief
 *
 * @param *file
 *
 * @param *shp
 *
 ******************************************************************************/
void
SHserializeShape (FILE *file, shape *shp)
{
    int cnt;

    DBUG_ENTER ("SHserializeShape");

    fprintf (file, "SHcreateShape( %d", SHAPE_DIM (shp));

    for (cnt = 0; cnt < SHAPE_DIM (shp); cnt++) {
        fprintf (file, ", %d", SHAPE_EXT (shp, cnt));
    }

    fprintf (file, ")");

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn int SHgetDim( shape *shp)
 *
 * @brief return the dimension of the given shape structure
 *
 ******************************************************************************/

int
SHgetDim (shape *shp)
{
    DBUG_ENTER ("SHgetDim");
    DBUG_ASSERT ((shp != NULL), ("SHgetDim called with NULL shape!"));

    DBUG_RETURN (SHAPE_DIM (shp));
}

/** <!--********************************************************************-->
 *
 * @fn int SHgetExtent( shape *shp, int dim)
 *
 * @brief return the shape's extent along the given dimension
 *
 ******************************************************************************/
int
SHgetExtent (shape *shp, int dim)
{
    DBUG_ENTER ("SHgetExtent");
    DBUG_ASSERT ((shp != NULL), ("SHgetExtent called with NULL shape!"));
    DBUG_ASSERT ((SHAPE_DIM (shp) > dim) && (dim >= 0),
                 ("SHgetExtent called with dim out of range!"));

    DBUG_RETURN (SHAPE_EXT (shp, dim));
}

/** <!--********************************************************************-->
 *
 * @fn int SHgetUnrLen( shape *shp)
 *
 * @brief return the length of a vector if shp was to be unrolled
 *
 ******************************************************************************/
int
SHgetUnrLen (shape *shp)
{
    int i, length;

    DBUG_ENTER ("SHgetUnrLen");
    DBUG_ASSERT ((shp != NULL), ("SHgetUnrLen called with NULL shape!"));

    length = 1;
    for (i = SHAPE_DIM (shp) - 1; i >= 0; i--) {
        length *= SHAPE_EXT (shp, i);
    }

    DBUG_RETURN (length);
}

/** <!--********************************************************************-->
 *
 * @fn int SHsubarrayDim( shape *shp, int n)
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
SHsubarrayDim (shape *shp, int n)
{
    int i, length;

    DBUG_ENTER ("SHsubarrayDim");
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
 * @fn shape *SHsetExtent( shape *shp, int dim, int val)
 *
 * @brief function to set the shape extent along a given axis
 *
 ******************************************************************************/

shape *
SHsetExtent (shape *shp, int dim, int val)
{
    DBUG_ENTER ("SHsetExtent");
    DBUG_ASSERT ((shp != NULL), ("SHsetExtent called with NULL shape!"));
    DBUG_ASSERT ((SHAPE_DIM (shp) > dim) && (dim >= 0),
                 ("SHsetExtent called with dim out of range!"));

    SHAPE_EXT (shp, dim) = val;
    DBUG_RETURN (shp);
}

/** <!--********************************************************************-->
 *
 * @fn bool SHcompareShapes( shape *a, shape *b)
 *
 * @brief compares two given shapes a and b
 *
 * @return true iff they are identical.
 *
 ******************************************************************************/

bool
SHcompareShapes (shape *a, shape *b)
{
    bool res;
    int i;

    DBUG_ENTER ("SHcompareShapes");

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
 * @fn shape *SHappendShapes( shape *a, shape *b)
 *
 * @brief appends two given shapes a and b into a new shape.
 *        Note here that both argument shapes are read only! They are neither
 *        used as part of the result, nor they are free'd.
 *
 ******************************************************************************/

shape *
SHappendShapes (shape *a, shape *b)
{
    int m, n, i, j;
    shape *res;

    DBUG_ENTER ("SHAppendShapes");
    DBUG_ASSERT ((a != NULL) && (b != NULL), ("SHAppendShapes called with NULL arg!"));

    m = SHAPE_DIM (a);
    n = SHAPE_DIM (b);

    res = SHmakeShape (m + n);
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
 * @fn shape *SHdropFromShape( int n, shape *a)
 *
 * @brief creates a new shape from a by dropping n elements.
 *        If n < 0, the the last n elements will be dropped!
 *        'a' is inspected only! The result shape is freshly created!
 *
 ******************************************************************************/

shape *
SHdropFromShape (int n, shape *a)
{
    int m, i;
    shape *res;

    DBUG_ENTER ("SHdropFromShape");
    DBUG_ASSERT ((a != NULL), ("SHDropFromShape called with NULL arg!"));

    m = SHAPE_DIM (a);
    DBUG_ASSERT ((m - abs (n)) >= 0, "dropping more elems from shape than available!");

    if (n < 0) {
        res = SHmakeShape (m + n);
        for (i = 0; i < m + n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i);
        }
    } else {
        res = SHmakeShape (m - n);
        for (i = 0; i < m - n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i + n);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHtakeFromShape( int n, shape *a)
 *
 * @brief creates a new shape from a by taking n elements.
 *        If n < 0, the the last n elements will be taken!
 *        'a' is inspected only! The result shape is freshly created!
 *
 ******************************************************************************/

shape *
SHtakeFromShape (int n, shape *a)
{
    int m, i;
    shape *res;

    DBUG_ENTER ("SHDropFromShape");
    DBUG_ASSERT ((a != NULL), ("SHDropFromShape called with NULL arg!"));

    m = SHAPE_DIM (a);
    DBUG_ASSERT ((m - abs (n)) >= 0, "taking more elems from shape than available!");

    if (n > 0) {
        res = SHmakeShape (n);
        for (i = 0; i < n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i);
        }
    } else {
        n *= -1;
        res = SHmakeShape (n);
        for (i = 0; i < n; i++) {
            SHAPE_EXT (res, i) = SHAPE_EXT (a, i + m - n);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char *SHshape2String( int dots, shape *shp)
 *
 * @brief generates a string representation of a shape.
 *        The argument "dots" allows some dots to be inserted into the vector
 *        representsation. e.g. SHshape2String( 2, <2,3,4>)  =>  "[.,.,2,3,4]"
 *
 ******************************************************************************/

char *
SHshape2String (int dots, shape *shp)
{
    static char buf[256];
    char *tmp = &buf[0];
    int i, j, n;

    DBUG_ENTER ("SHshape2String");
    DBUG_ASSERT ((shp != NULL), ("SHshape2String called with NULL shape!"));

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

    DBUG_RETURN (STRcpy (buf));
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHoldTypes2Shape( types *types)
 *
 * @brief if types has a dim>=0 a shape structure is created which carries the same
 *        shape info as the types-node does. Otherwise, NULL is returned.
 *
 ******************************************************************************/
shape *
SHoldTypes2Shape (types *types)
{
    int dim;
    shape *res;
    shpseg *shpseg;

    DBUG_ENTER ("SHoldTypes2Shape");
    DBUG_ASSERT ((types != NULL), ("SHoldTypes2Shape called with NULL types!"));

    /* this function handle user defined types, too */
    shpseg = TCtype2Shpseg (types, &dim);

    res = SHoldShpseg2Shape (dim, shpseg);

    if (shpseg != NULL) {
        shpseg = FREEfreeShpseg (shpseg);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHoldShpseg2Shape( int dim, shpseg *shpseg)
 *
 * @brief iff dim > 0 a new shape structure is created which contains the same
 *        shape info as the shpseg does. Otherwise, NULL is returned.
 *
 ******************************************************************************/
shape *
SHoldShpseg2Shape (int dim, shpseg *shpseg)
{
    int i, j;
    shape *res;

    DBUG_ENTER ("SHoldShpseg2Shape");

    if (dim >= 0) {
        res = SHmakeShape (dim);

        if (dim > 0) {
            i = 0;
            while (dim > SHP_SEG_SIZE) {
                DBUG_ASSERT ((shpseg != NULL),
                             "SHoldShpseg2Shape called with NULL shpseg but dim >0!");
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
 * @fn shpseg *SHshape2OldShpseg( shape *shp)
 *
 * @brief if shp has a dim>0 a shpseg structure is created which carries the
 *        same shape info as the shp does. Otherwise, NULL is returned.
 *
 ******************************************************************************/

shpseg *
SHshape2OldShpseg (shape *shp)
{
    int dim, i, j;
    shpseg *res, *curr_seg;

    DBUG_ENTER ("SHshape2OldShpseg");
    DBUG_ASSERT ((shp != NULL), ("SHshape2OldShpseg called with NULL shp!"));

    dim = SHAPE_DIM (shp);
    if (dim > 0) {
        i = 0;
        res = TBmakeShpseg (NULL);
        curr_seg = res;
        while (dim > SHP_SEG_SIZE) {
            for (j = 0; j < SHP_SEG_SIZE; j++, i++) {
                SHPSEG_SHAPE (curr_seg, j) = SHAPE_EXT (shp, i);
            }
            SHPSEG_NEXT (curr_seg) = TBmakeShpseg (NULL);
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
 * @fn bool SHcompareWithCArray( shape *shp, int* shpdata, int dim)
 *
 * @brief compares given shape with a shape specified as a c integer array
 *
 * @return true iff shapes are equal in dim and shape vector
 *         false otherwise
 *
 ******************************************************************************/
bool
SHcompareWithCArray (shape *shp, int *shpdata, int dim)
{
    bool flag;
    int i;

    DBUG_ENTER ("SHcompareWithCArray");

    flag = TRUE;

    DBUG_ASSERT (((shp != NULL) && ((dim == 0) || (shpdata != NULL))),
                 ("SHcompareWithCArray called with NULL pointer(s)!\n"));

    if (dim == SHAPE_DIM (shp)) {
        for (i = 0; i < dim; i++) {
            if (SHAPE_EXT (shp, i) != shpdata[i]) {
                flag = FALSE;
            }
        }
    } else {
        flag = FALSE;
    }

    DBUG_RETURN (flag);
}

/** <!--********************************************************************-->
 *
 * @fn bool SHcompareWithArguments( shape *shp, int dim, ...)
 *
 * @brief compares given shape with a shape specified as an list of arguments.
 *        usage: e.g. SHcompareWithArguments(shp, 3, 4,4,5)
 *
 * @return true iff shapes are equal in dim and shape vector
 *         false otherwise
 *
 ******************************************************************************/

bool
SHcompareWithArguments (shape *shp, int dim, ...)
{
    va_list Argp;
    bool flag;
    int i;

    DBUG_ENTER ("SHcompareWithArguments");

    flag = TRUE;
    DBUG_ASSERT ((shp != NULL), ("SHcompareWithCArray called with NULL pointer(s)!\n"));
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
 * @fn int *SHshape2IntVec( shape *shp)
 *
 * @brief creates a simple int vector from the given shape vector
 *
 ******************************************************************************/

int *
SHshape2IntVec (shape *shp)
{
    int *int_vec;
    int i;
    int n;

    DBUG_ENTER ("SHshape2IntVec");

    n = SHAPE_DIM (shp);
    if (n > 0) {
        int_vec = (int *)MEMmalloc (n * sizeof (int));
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
 * @fn node *SHshape2Exprs( shape *shp)
 *
 * @brief creates a nesting of N_exprs nodes from the given shape vector
 *
 ******************************************************************************/

node *
SHshape2Exprs (shape *shp)
{
    node *exprs;
    int dim;
    int i;

    DBUG_ENTER ("SHshape2Exprs");

    dim = SHAPE_DIM (shp);
    exprs = NULL;
    for (i = dim - 1; i >= 0; i--) {
        exprs = TBmakeExprs (TBmakeNum (SHAPE_EXT (shp, i)), exprs);
    }

    DBUG_RETURN (exprs);
}

/** <!--********************************************************************-->
 *
 * @fn node *SHshape2Array( shape *shp)
 *
 * @brief creates a simple int vector from the given shape vector
 *
 ******************************************************************************/

node *
SHshape2Array (shape *shp)
{
    node *array;

    DBUG_ENTER ("SHshape2Array");

    array = TCmakeIntVector (SHshape2Exprs (shp));

    DBUG_RETURN (array);
}

/** <!--********************************************************************-->
 *
 * @fn shape *SHarray2Shape( node *array)
 *
 * @brief creates a shape vector from a given simple int vector
 *
 ******************************************************************************/
shape *
SHarray2Shape (node *array)
{
    shape *result;
    node *exprs;
    int cnt;

    DBUG_ENTER ("SHarray2Shape");

    DBUG_ASSERT ((NODE_TYPE (array) == N_array),
                 "SHarray2Shape called on non array node");

    exprs = ARRAY_AELEMS (array);

    result = SHmakeShape (TCcountExprs (exprs));

    for (cnt = 0; cnt < SHAPE_DIM (result); cnt++) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_num),
                     "SHarray2Shape can handle constant int vectors only!");

        SHAPE_EXT (result, cnt) = NUM_VAL (EXPRS_EXPR (exprs));

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (result);
}

/*@}*/ /* defgroup shape */
