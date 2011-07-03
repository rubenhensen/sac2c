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

#define DBUG_PREFIX "COOPS"
#include "debug.h"

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

    DBUG_ENTER ();
    DBUG_ASSERT (CONSTANT_TYPE (a) == CONSTANT_TYPE (b),
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
                DBUG_ASSERT (0 == 1, "COzip called with args of different shape!");
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

    DBUG_ENTER ();

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
COadd (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_plus, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COadd", a, b, res));
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
COsub (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_minus, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COsub", a, b, res));
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
COmul (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_mul, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COmul", a, b, res));
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
COdiv (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_div, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COdiv", a, b, res));
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
COmod (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_mod, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COmod", a, b, res));
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
COmin (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_min, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COmin", a, b, res));
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
COmax (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_max, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COmax", a, b, res));
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
COand (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_and, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COand", a, b, res));
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
COor (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_or, a, b, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COor", a, b, res));
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
COle (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_le, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COle", a, b, res));
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
COlt (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_lt, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COlt", a, b, res));
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
COge (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_ge, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COge", a, b, res));
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
COgt (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_gt, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COgt", a, b, res));
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
COeq (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_eq, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COeq", a, b, res));
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
COneq (constant *a, constant *b, constant *tmp1)
{
    constant *res;

    DBUG_ENTER ();
    res = COzip (global.zipcv_neq, a, b, T_bool);
    DBUG_EXECUTE (COINTdbugPrintBinOp ("COneq", a, b, res));
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
COnot (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_not, a, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COnot", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtob( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to byte,
 *
 ******************************************************************************/

constant *
COtob (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tob, a, T_byte);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtob", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtos( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to short.
 *
 ******************************************************************************/

constant *
COtos (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tos, a, T_short);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtos", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoi( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise convertet to int.
 *
 ******************************************************************************/

constant *
COtoi (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toi, a, T_int);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoi", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtol( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to long.
 *
 ******************************************************************************/

constant *
COtol (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tol, a, T_long);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtol", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoll( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to longlong.
 *
 ******************************************************************************/

constant *
COtoll (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toll, a, T_longlong);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoll", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoub( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to ubyte.
 *
 ******************************************************************************/

constant *
COtoub (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toub, a, T_ubyte);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoub", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtous( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to ushort.
 *
 ******************************************************************************/

constant *
COtous (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tous, a, T_ushort);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtous", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoui( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to  uint.
 *
 ******************************************************************************/

constant *
COtoui (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toui, a, T_uint);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoui", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoul( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to ulong.
 *
 ******************************************************************************/

constant *
COtoul (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toul, a, T_ulong);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoul", a, res));
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    constant *COtoull( constant *a)
 *
 * description:
 *    returns a constant whose elements are elementwise converted to ulonglong.
 *
 ******************************************************************************/

constant *
COtoull (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_toull, a, T_ulonglong);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtoull", a, res));
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
COtof (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tof, a, T_float);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtof", a, res));
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
COtod (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_tod, a, T_double);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COtod", a, res));
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
COabs (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_abs, a, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COabs", a, res));
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
COneg (constant *a, constant *tmp1, constant *tmp2)
{
    constant *res;

    DBUG_ENTER ();
    res = COzipUnary (global.zipcv_neg, a, T_unknown);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COneg", a, res));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn constant *COreciproc( constant *a)
 *
 *   @brief  yields reciprocal of all elements of a
 *
 *   @param a   array whose elements are to be made reciprocal
 *   @return    the result of the negation
 *
 ******************************************************************************/

constant *
COreciproc (constant *a, constant *tmp1, constant *tmp2)
{
    constant *one;
    constant *res;

    DBUG_ENTER ();
    one = COmakeOne (COgetType (a), COgetShape (a));
    res = COzip (global.zipcv_div, one, a, T_unknown);
    one = COfreeConstant (one);
    DBUG_EXECUTE (COINTdbugPrintUnaryOp ("COreciproc", a, res));
    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
