/*
 * $Log$
 * Revision 1.2  2001/03/05 16:57:04  sbs
 * COAdd, COSub, COMul, and CODiv added
 *
 * Revision 1.1  2001/03/02 14:32:54  sbs
 * Initial revision
 *
 */

#include <stdlib.h>

#include "dbug.h"
#include "internal_lib.h"
#include "constants.h"
#include "constants_internal.h"
#include "zipcv.h"

/******************************************************************************
 ***
 *** local helper functions:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant * COZip( zipcvfunptr *fun_arr, constant *a, constant *b)
 *
 * description:
 *    this function implements most of the binary value inspecting functions
 *    on constants. Iff
 *        a is a scalar (i.e. dim(a)==0)
 *     || b is a scalar (i.e. dim(b)==0)
 *     || shape(a) == shape(b)
 *    the function supplied by fun_arr is applied elementwise to the elements
 *    of a and b. It should be noted here, that COZip assumes a and b to be
 *    arrays of the same base type!
 *    If the condition above does not hold or the types are different, a
 *    DBUG_ASSERT will lead to program termination (assuming DBUG_OFF is
 *    not defined!)
 *
 ******************************************************************************/

constant *
COZip (zipcvfunptr *fun_arr, constant *a, constant *b)
{
    constant *res;
    void *cv;
    int i;

    DBUG_ENTER ("COZip");
    DBUG_ASSERT ((CONSTANT_TYPE (a) == CONSTANT_TYPE (b)),
                 "COZip called with args of different base type!");

    if (CONSTANT_DIM (a) == 0) {
        /*
         * a is a scalar so the result has the same shape as b:
         */
        res = COCopyConstant (b);
        cv = CONSTANT_ELEMS (res);
        for (i = 0; i < CONSTANT_VLEN (res); i++) {
            fun_arr[CONSTANT_TYPE (b)](CONSTANT_ELEMS (a), 0, CONSTANT_ELEMS (b), i, cv,
                                       i);
        }

    } else {
        if (CONSTANT_DIM (b) == 0) {
            /*
             * b is a scalar so the result has the same shape as a:
             */
            res = COCopyConstant (a);
            cv = CONSTANT_ELEMS (res);
            for (i = 0; i < CONSTANT_VLEN (res); i++) {
                fun_arr[CONSTANT_TYPE (b)](CONSTANT_ELEMS (a), i, CONSTANT_ELEMS (b), 0,
                                           cv, i);
            }

        } else {
            /*
             * a and b are both non-scalars, so we have to compare the shapes.
             * Iff the are identical, the given operation is applied elementwise.
             */
            if (SHCompareShapes (CONSTANT_SHAPE (a), CONSTANT_SHAPE (b)) == TRUE) {
                res = COCopyConstant (a);
                cv = CONSTANT_ELEMS (res);
                for (i = 0; i < CONSTANT_VLEN (res); i++) {
                    fun_arr[CONSTANT_TYPE (b)](CONSTANT_ELEMS (a), i, CONSTANT_ELEMS (b),
                                               i, cv, i);
                }

            } else {
                DBUG_ASSERT ((0 == 1), "COZip called with args of different shape!");
            }
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 *** Operations on constants:
 ***   Here, you will find those functions of the MOA stuff implemented that
 ***   belongs to the intrinsic functions of SAC.
 ***   AT the time being, we have implemented the following operations:
 ***
 ***   - +: add two arrays of identical shape
 ***   - -: subtract two arrays of identical shape
 ***   - *: multiply two arrays of identical shape
 ***   - /: divide two arrays of identical shape
 ***
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant *COAdd( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise addition of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COAdd (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COAdd");
    res = COZip (zipcv_plus, a, b);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COAdd", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COSub( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise subtraction of b
 *    from a, provided that    either shape(a) == shape(b),
 *                             or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COSub (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COSub");
    res = COZip (zipcv_minus, a, b);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COSub", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COMul( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise multiplication of a
 *    and b, provided that    either shape(a) == shape(b),
 *                            or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COMul (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COMul");
    res = COZip (zipcv_mul, a, b);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COMul", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CODiv( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise division of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
CODiv (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODiv");
    res = COZip (zipcv_div, a, b);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("CODiv", a, b, res););
    DBUG_RETURN (res);
}
