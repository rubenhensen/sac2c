/*
 * $Log$
 * Revision 1.4  2001/03/22 14:25:08  nmw
 * primitive ari ops implemented
 *
 * Revision 1.3  2001/03/06 10:29:56  sbs
 * res in COZip post-mortem initialized in order to please gcc 8-))
 *
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
#include "globals.h"
#include "zipcv.h"

/******************************************************************************
 ***
 *** local helper functions:
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    constant * COZip( zipcvfunptr *fun_arr, constant *a, constant *b,
 *                      simpletype target_type)
 *
 * description:
 *    this function implements most of the binary value inspecting functions
 *    on constants.
 *    the type of the resulting constant is equal to the arguments except
 *    argument target_type forces a result type differrent from T_unknown
 *    (this is used for comparison operations).
 *
 *    Iff
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
COZip (zipcvfunptr *fun_arr, constant *a, constant *b, simpletype target_type)
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
        if (target_type != T_unknown) {
            res
              = COMakeConstant (target_type, SHCopyShape (COGetShape (b)),
                                MALLOC (CONSTANT_VLEN (b) * basetype_size[target_type]));
        } else {
            res = COCopyConstant (b);
        }
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
            if (target_type != T_unknown) {
                res = COMakeConstant (target_type, SHCopyShape (COGetShape (a)),
                                      MALLOC (CONSTANT_VLEN (a)
                                              * basetype_size[target_type]));
            } else {
                res = COCopyConstant (a);
            }

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
                if (target_type != T_unknown) {
                    res = COMakeConstant (target_type, SHCopyShape (COGetShape (a)),
                                          MALLOC (CONSTANT_VLEN (a)
                                                  * basetype_size[target_type]));
                } else {
                    res = COCopyConstant (a);
                }
                cv = CONSTANT_ELEMS (res);
                for (i = 0; i < CONSTANT_VLEN (res); i++) {
                    fun_arr[CONSTANT_TYPE (b)](CONSTANT_ELEMS (a), i, CONSTANT_ELEMS (b),
                                               i, cv, i);
                }

            } else {
                DBUG_ASSERT ((0 == 1), "COZip called with args of different shape!");
                res = NULL;
            }
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant * COZipUnary( zipcvfunptr *fun_arr, constant *a,
 *                           simpletype targettype)
 *
 * description:
 *    this function implements most of the unary value inspecting functions
 *    on constants.
 *    the function supplied by fun_arr is applied elementwise to the elements
 *    of a.
 *    When caling a functions from fun_arr the second argument is not used,
 *    it is only set to get one single function signature for all constant
 *    evaluation functions.
 *
 ******************************************************************************/

constant *
COZipUnary (zipcvfunptr *fun_arr, constant *a, simpletype target_type)
{
    constant *res;
    void *cv;
    int i;

    DBUG_ENTER ("COZipUnary");

    if (target_type != T_unknown) {
        res = COMakeConstant (target_type, SHCopyShape (COGetShape (a)),
                              MALLOC (CONSTANT_VLEN (a) * basetype_size[target_type]));
    } else {
        res = COCopyConstant (a);
    }

    cv = CONSTANT_ELEMS (res);
    for (i = 0; i < CONSTANT_VLEN (res); i++) {
        fun_arr[CONSTANT_TYPE (a)](CONSTANT_ELEMS (a), i, NULL, 0, cv, i);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 *** Operations on constants:
 ***   Here, you will find those functions of the MOA stuff implemented that
 ***   belongs to the intrinsic functions of SAC:
 ***  +, -, *, /, %, min, max
 ***  &&, ||, !
 ***  <, <=, >, >=, ==, !=
 ***  toi, tod, tof, abs
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
    res = COZip (zipcv_plus, a, b, T_unknown);
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
    res = COZip (zipcv_minus, a, b, T_unknown);
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
    res = COZip (zipcv_mul, a, b, T_unknown);
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
    res = COZip (zipcv_div, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("CODiv", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COMod( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise modulo of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COMod (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODMod");
    res = COZip (zipcv_mod, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COMod", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *Min( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise minima of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COMin (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODMin");
    res = COZip (zipcv_min, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COMin", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COMax( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise maximum of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COMax (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODMax");
    res = COZip (zipcv_max, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COMax", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COAnd( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise log. AND of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COAnd (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COAnd");
    res = COZip (zipcv_and, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COAnd", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COOr( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise log.OR of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COOr (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COOr");
    res = COZip (zipcv_or, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COOr", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COLe( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a<=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COLe (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COLe");
    res = COZip (zipcv_le, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COLe", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COLt( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a<b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COLt (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COLt");
    res = COZip (zipcv_lt, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COLt", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COGe( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a>=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COGe (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COGe");
    res = COZip (zipcv_ge, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COGe", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COGt( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a>b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COGt (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COGt");
    res = COZip (zipcv_gt, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COGt", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COEq( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a==b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COEq (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COEq");
    res = COZip (zipcv_eq, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("COEq", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CONeq( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a!=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
CONeq (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CONeq");
    res = COZip (zipcv_neq, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", DbugPrintBinOp ("CONeq", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *CONot( constant *a)
 *
 * description:
 *    returns a constant whose elements are the elementwise logical !a ,
 *
 ******************************************************************************/

constant *
CONot (constant *a)
{
    constant *res;

    DBUG_ENTER ("CONot");
    res = COZipUnary (zipcv_not, a, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintUnaryOp ("CONot", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COToi( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to  ,
 *
 ******************************************************************************/

constant *
COToi (constant *a)
{
    constant *res;

    DBUG_ENTER ("COToi");
    res = COZipUnary (zipcv_toi, a, T_int);
    DBUG_EXECUTE ("COOPS", DbugPrintUnaryOp ("COToi", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COTof( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to float,
 *
 ******************************************************************************/

constant *
COTof (constant *a)
{
    constant *res;

    DBUG_ENTER ("COTof");
    res = COZipUnary (zipcv_tof, a, T_float);
    DBUG_EXECUTE ("COOPS", DbugPrintUnaryOp ("COtof", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COTod( constant *a)
 *
 * description:
 *    returns a constant whose elements elementwise converted to double,
 *
 ******************************************************************************/

constant *
COTod (constant *a)
{
    constant *res;

    DBUG_ENTER ("COTod");
    res = COZipUnary (zipcv_tod, a, T_double);
    DBUG_EXECUTE ("COOPS", DbugPrintUnaryOp ("COTod", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COAbs( constant *a)
 *
 * description:
 *    returns a constant whose elements are the elementwise absolute value,
 *
 ******************************************************************************/

constant *
COAbs (constant *a)
{
    constant *res;

    DBUG_ENTER ("COAbs");
    res = COZipUnary (zipcv_abs, a, T_unknown);
    DBUG_EXECUTE ("COOPS", DbugPrintUnaryOp ("COAbs", a, res););
    DBUG_RETURN (res);
}
