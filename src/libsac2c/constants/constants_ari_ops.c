/*
 * $Log$
 * Revision 1.8  2004/11/26 15:49:05  jhb
 * compile
 *
 * Revision 1.7  2004/11/26 14:29:53  sbs
 * change run
 *
 * Revision 1.6  2003/04/09 15:38:14  sbs
 * CONeg added.
 *
 * Revision 1.5  2001/05/17 14:16:21  nmw
 * MALLOC/FREE replaced by MEMmalloc/Free, using result of MEMfree()
 *
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
#include "str.h"
#include "memory.h"
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
 *    constant * COzip( zipcvfunptr *fun_arr, constant *a, constant *b,
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
 *    of a and b. It should be noted here, that COzip assumes a and b to be
 *    arrays of the same base type!
 *    If the condition above does not hold or the types are different, a
 *    DBUG_ASSERT will lead to program termination (assuming DBUG_OFF is
 *    not defined!)
 *
 ******************************************************************************/

constant *
COzip (const zipcvfunptr *fun_arr, constant *a, constant *b, simpletype target_type)
{
    constant *res;
    void *cv;
    int i;

    DBUG_ENTER ("COzip");
    DBUG_ASSERT ((CONSTANT_TYPE (a) == CONSTANT_TYPE (b)),
                 "COzip called with args of different base type!");

    if (CONSTANT_DIM (a) == 0) {
        /*
         * a is a scalar so the result has the same shape as b:
         */
        if (target_type != T_unknown) {
            res = COmakeConstant (target_type, SHcopyShape (COgetShape (b)),
                                  MEMmalloc (CONSTANT_VLEN (b)
                                             * global.basetype_size[target_type]));
        } else {
            res = COcopyConstant (b);
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
                res = COmakeConstant (target_type, SHcopyShape (COgetShape (a)),
                                      MEMmalloc (CONSTANT_VLEN (a)
                                                 * global.basetype_size[target_type]));
            } else {
                res = COcopyConstant (a);
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
            if (SHcompareShapes (CONSTANT_SHAPE (a), CONSTANT_SHAPE (b)) == TRUE) {
                if (target_type != T_unknown) {
                    res
                      = COmakeConstant (target_type, SHcopyShape (COgetShape (a)),
                                        MEMmalloc (CONSTANT_VLEN (a)
                                                   * global.basetype_size[target_type]));
                } else {
                    res = COcopyConstant (a);
                }
                cv = CONSTANT_ELEMS (res);
                for (i = 0; i < CONSTANT_VLEN (res); i++) {
                    fun_arr[CONSTANT_TYPE (b)](CONSTANT_ELEMS (a), i, CONSTANT_ELEMS (b),
                                               i, cv, i);
                }

            } else {
                DBUG_ASSERT ((0 == 1), "COzip called with args of different shape!");
                res = NULL;
            }
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant * COzipUnary( zipcvfunptr *fun_arr, constant *a,
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
COzipUnary (const zipcvfunptr *fun_arr, constant *a, simpletype target_type)
{
    constant *res;
    void *cv;
    int i;

    DBUG_ENTER ("COzipUnary");

    if (target_type != T_unknown) {
        res = COmakeConstant (target_type, SHcopyShape (COgetShape (a)),
                              MEMmalloc (CONSTANT_VLEN (a)
                                         * global.basetype_size[target_type]));
    } else {
        res = COcopyConstant (a);
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
 *    constant *COadd( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise addition of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COadd (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COadd");
    res = COzip (global.zipcv_plus, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COadd", a, b, res););
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
COsub (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COsub");
    res = COzip (global.zipcv_minus, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COsub", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COmul( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise multiplication of a
 *    and b, provided that    either shape(a) == shape(b),
 *                            or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COmul (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COmul");
    res = COzip (global.zipcv_mul, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COmul", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COdiv( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise division of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COdiv (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COdiv");
    res = COzip (global.zipcv_div, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COdiv", a, b, res););
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
COmod (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODmod");
    res = COzip (global.zipcv_mod, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COmod", a, b, res););
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
COmin (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("CODmin");
    res = COzip (global.zipcv_min, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COmin", a, b, res););
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
COmax (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COmax");
    res = COzip (global.zipcv_max, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COmax", a, b, res););
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
COand (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COand");
    res = COzip (global.zipcv_and, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COand", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COor( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise log.OR of a and b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COor (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COor");
    res = COzip (global.zipcv_or, a, b, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COor", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COle( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a<=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COle (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COle");
    res = COzip (global.zipcv_le, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COle", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COlt( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a<b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COlt (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COlt");
    res = COzip (global.zipcv_lt, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COlt", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COge( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a>=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COge (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COge");
    res = COzip (global.zipcv_ge, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COge", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COgt( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a>b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COgt (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COgt");
    res = COzip (global.zipcv_gt, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COgt", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COeq( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a==b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COeq (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COeq");
    res = COzip (global.zipcv_eq, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COeq", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COneq( constant *a, constant *b)
 *
 * description:
 *    returns a constant whose elements are the elementwise comparison a!=b,
 *    provided that    either shape(a) == shape(b),
 *                     or at least one of { a, b } is scalar
 *
 ******************************************************************************/

constant *
COneq (constant *a, constant *b)
{
    constant *res;

    DBUG_ENTER ("COneq");
    res = COzip (global.zipcv_neq, a, b, T_bool);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintBinOp ("COneq", a, b, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COnot( constant *a)
 *
 * description:
 *    returns a constant whose elements are the elementwise logical !a ,
 *
 ******************************************************************************/

constant *
COnot (constant *a)
{
    constant *res;

    DBUG_ENTER ("COnot");
    res = COzipUnary (global.zipcv_not, a, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COnot", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoi( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to  ,
 *
 ******************************************************************************/

constant *
COtoi (constant *a)
{
    constant *res;

    DBUG_ENTER ("COtoi");
    res = COzipUnary (global.zipcv_toi, a, T_int);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COtoi", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtof( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to float,
 *
 ******************************************************************************/

constant *
COtof (constant *a)
{
    constant *res;

    DBUG_ENTER ("COtof");
    res = COzipUnary (global.zipcv_tof, a, T_float);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COtof", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtod( constant *a)
 *
 * description:
 *    returns a constant whose elements elementwise converted to double,
 *
 ******************************************************************************/

constant *
COtod (constant *a)
{
    constant *res;

    DBUG_ENTER ("COtod");
    res = COzipUnary (global.zipcv_tod, a, T_double);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COtod", a, res););
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COabs( constant *a)
 *
 * description:
 *    returns a constant whose elements are the elementwise absolute value,
 *
 ******************************************************************************/

constant *
COabs (constant *a)
{
    constant *res;

    DBUG_ENTER ("COabs");
    res = COzipUnary (global.zipcv_abs, a, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COabs", a, res););
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COneg( constant *a)
 *
 *   @brief  negates all elements of a
 *
 *   @param a   array whose elements are to be negated
 *   @return    the result of the negation
 *
 ******************************************************************************/

constant *
COneg (constant *a)
{
    constant *res;

    DBUG_ENTER ("COneg");
    res = COzipUnary (global.zipcv_neg, a, T_unknown);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COneg", a, res););
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COrec( constant *a)
 *
 *   @brief  yields reciprocal of all elements of a
 *
 *   @param a   array whose elements are to be made reciprocal
 *   @return    the result of the negation
 *
 ******************************************************************************/

constant *
COrec (constant *a)
{
    constant *one;
    constant *res;

    DBUG_ENTER ("COrec");
    one = COmakeOne (COgetType (a), COgetShape (a));
    res = COzip (global.zipcv_div, one, a, T_unknown);
    one = COfreeConstant (one);
    DBUG_EXECUTE ("COOPS", COINTdbugPrintUnaryOp ("COrec", a, res););
    DBUG_RETURN (res);
}
