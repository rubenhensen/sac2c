/*
 * This module provides a set of functions for applying arithmetic functions
 * to two elements of constant vectors.
 * The problem with this task is that each such function has to be overloaded on
 * all potential types.
 * Since C does not support such specifications, a function table is created
 * which is parameterized over simpletype.
 * This is done by using the    .mac-mechanism     on type_info.mac in order
 * to make sure that all simpletypes are dealt with.
 */

#include <strings.h>
#include <stdlib.h>

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"

#include "zipcv.h"

/******************************************************************************
 *
 * function:
 *    void COzipCvXXXYYY( void *arg1, size_t pos1, void *arg2, size_t pos2,
 *                        void *res, size_t res_pos)
 *
 * description:
 *    Functions for applying function YYY to two elements of cv's arg1 and arg2
 *    each of which are of type XXX.
 *    Since all these functions are identical apart from the type casts,
 *    they are implemented using the C macro mechanism.
 *
 *
 * function:
 *    void COzipCvDummyYYY( void *arg1, size_t pos1, void *arg2, size_t pos2,
 *                        void *res, size_t res_pos)
 *
 * description:
 *    Finally, we provide a dummy function for each operation YYY which should
 *    never be called! It is defined for being able to make entries for all
 *     simpletypes in type_info.mac!
 *
 ******************************************************************************/

#define str(s) #s

#define COzipCvBINARYTEMPLATE(fun, fun_ext, arg_t, arg_ext, tfun, res_t)                 \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((tfun(res_t) *)res)[res_pos] = (tfun(res_t)) (fun(((arg_t *)arg1)[pos1],        \
                                                        ((arg_t *)arg2)[pos2]));         \
        DBUG_RETURN ();                                                                  \
    }

#define COzipCvUNARYTEMPLATE(fun, fun_ext, arg_t, arg_ext, tfun, res_t)                  \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        ((tfun(res_t) *)res)[res_pos] = (tfun(res_t)) (fun (((arg_t *)arg1)[pos1]));     \
        DBUG_RETURN ();                                                                  \
    }


#define COzipCvDUMMYTEMPLATE(fun, fun_ext, arg_t, arg_ext, tfun, res_t)                  \
    void COzipCv##arg_ext##fun_ext (void *arg1, size_t pos1, void *arg2, size_t pos2,    \
                                    void *res, size_t res_pos)                           \
    {                                                                                    \
        DBUG_ENTER ();                                                                   \
        DBUG_UNREACHABLE (str (COzipCv##arg_ext##fun_ext called !));                     \
        DBUG_RETURN ();                                                                  \
    }


/*
 * mappers for type classes:
 */
#define MAP_SINT_ALONE(arity, fun, fname, tfun)                                          \
    COzipCv##arity##TEMPLATE (fun, fname, signed char, Byte, tfun, signed char)          \
    COzipCv##arity##TEMPLATE (fun, fname, short, Short, tfun, short)                     \
    COzipCv##arity##TEMPLATE (fun, fname, int, Int, tfun, int)                           \
    COzipCv##arity##TEMPLATE (fun, fname, long, Long, tfun, long)                        \
    COzipCv##arity##TEMPLATE (fun, fname, long long, LongLong, tfun, long long)

#define MAP_UINT_ALONE(arity, fun, fname, tfun)                                          \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned char, UByte, tfun, unsigned char)     \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned short, UShort, tfun, unsigned short)  \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned int, UInt, tfun, unsigned int)        \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned long, ULong, tfun, unsigned long)     \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned long long, ULongLong, tfun, unsigned long long)

#define MAP_FLOAT_ALONE(arity, fun, fname, tfun)                                         \
    COzipCv##arity##TEMPLATE (fun, fname, float, Float, tfun, float)                     \
    COzipCv##arity##TEMPLATE (fun, fname, double, Double, tfun, double)                  \
    COzipCv##arity##TEMPLATE (fun, fname, long double, LongDouble, tfun, long double)

#define MAP_BOOL_ALONE(arity, fun, fname, tfun)                                          \
    COzipCv##arity##TEMPLATE (fun, fname, bool, Bool, tfun, bool)

#define MAP_CHAR_ALONE(arity, fun, fname, tfun)                                          \
    COzipCv##arity##TEMPLATE (fun, fname, unsigned char, Char, tfun, unsigned char)

#define MAP_DUMMY(fname)                                                                 \
    COzipCvDUMMYTEMPLATE (_,fname,_,Dummy,_,_)


#define MAP_INT(arity, fun, fname, tfun)                                                 \
MAP_SINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (DUMMY, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)

#define MAP_NUM(arity, fun, fname, tfun)                                                 \
MAP_SINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (arity, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)

#define MAP_BOOL(arity, fun, fname, tfun)                                                \
MAP_SINT_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (DUMMY, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (arity, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)

#define MAP_NUMoBOOL(arity, fun, fname, tfun)                                            \
MAP_SINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (arity, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (arity, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)

#define MAP_SNUM(arity, fun, fname, tfun)                                                \
MAP_SINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (arity, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (DUMMY, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)

#define MAP_ANY(arity, fun, fname, tfun)                                                 \
MAP_SINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_UINT_ALONE (arity, fun, fname, tfun)                                                 \
MAP_FLOAT_ALONE (arity, fun, fname, tfun)                                                \
MAP_BOOL_ALONE (arity, fun, fname, tfun)                                                 \
MAP_CHAR_ALONE (arity, fun, fname, tfun)                                                 \
MAP_DUMMY (fname)



/*
 * The actual function definitions are defined by the following macro usages:
 */
/*
 * The function creation is highly abstracted using several macros.
 * We have several macros creating functions for a set up types:
 *   MAP_SINT     for signed integer types
 *   MAP_UINT     for unsigned integer types
 *   MAP_FLOAT    for floating point types
 *   MAP_BOOL     for bool
 *   MAP_CHAR     for char
 *
 * All these macros expect 4 arguments:
 *   arity        being either BINARY or UNARY
 *   fun          this needs to be a macro defining the scalar function!
 *   fname        the operation name used in the generated zipper function
 *   tfun         this needs to be a macro defining a function that maps
 *                the argument type to the result type!
 *
 *   MAP_DUMMY    for generating a dummy entry
 */

/*
 * our current tfuns:
 */
#define ID(x) x
#define FIXBOOL(x) bool

/*
 * fun definitions followed by the macro-generators:
 */
/*
 * NUM x NUM => NUM
 */
#define PLUS(a,b)  (a)+(b)
#define MINUS(a,b) (a)-(b)
#define MUL(a,b)   (a)*(b)
#define DIV(a,b)   (a)/(b)
#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

MAP_NUM (BINARY, PLUS, Plus, ID)
MAP_NUM (BINARY, MINUS, Minus, ID)
MAP_NUM (BINARY, MUL, Mul, ID)
MAP_NUM (BINARY, DIV, Div, ID)
MAP_NUM (BINARY, MIN, Min, ID)
MAP_NUM (BINARY, MAX, Max, ID)

#define MOD(a,b) (a)%(b)
#define SIGNUM(x) ((0 == (x)) ? 0 : (0 < (x)) ? 1 : -1)
#define APLREM(a,b) ((b) == 0 ? (a) : ((a) % (b)))
#define APLMOD(a,b) ((APLREM(a,b) != 0) && (SIGNUM(a) != SIGNUM(b)) ? \
                     APLREM(a,b) + (b) : APLREM(a,b))

/*
 * INT x INT => INT
 */
MAP_INT (BINARY, MOD, Mod, ID)
MAP_INT (BINARY, APLMOD, AplMod, ID)

/*
 * BOOL x BOOL => BOOL
 */
#define AND(a,b) (a)&&(b)
#define OR(a,b)  (a)||(b)

MAP_BOOL (BINARY, AND, And, ID)
MAP_BOOL (BINARY, OR, Or, ID)


/*
 * ANY x ANY => BOOL
 */
#define EQ(a,b) (a)==(b)
#define NE(a,b) (a)!=(b)
#define LE(a,b) (a)<=(b)
#define LT(a,b) (a)<(b)
#define GT(a,b) (a)>(b)
#define GE(a,b) (a)>=(b)

MAP_ANY (BINARY, EQ, Eq, FIXBOOL)
MAP_ANY (BINARY, NE, Neq, FIXBOOL)
MAP_ANY (BINARY, LE, Le, FIXBOOL)
MAP_ANY (BINARY, LT, Lt, FIXBOOL)
MAP_ANY (BINARY, GT, Gt, FIXBOOL)
MAP_ANY (BINARY, GE, Ge, FIXBOOL)


/*
 * BOOL -> BOOL
 */
#define NOT(a) !(a)
MAP_BOOL (UNARY, NOT, Not, ID)

/*
 * SNUM -> SNUM
 */
#define ABS(a) ((a)<0?-(a):(a))
#define NEG(a) -(a)

MAP_SNUM (UNARY, ABS, Abs, ID)
MAP_SNUM (UNARY, NEG, Neg, ID)

/*
 * ANY -> BYTE (signed char)
 */
#define TOB(a) (signed char)(a)
#define FIXB(a) signed char
MAP_ANY (UNARY, TOB, Tob, FIXB)

/*
 * ANY -> SHORT (short)
 */
#define TOS(a) (short)(a)
#define FIXS(a) short
MAP_ANY (UNARY, TOS, Tos, FIXS)

/*
 * ANY -> INT (int)
 */
#define TOI(a) (int)(a)
#define FIXI(a) int
MAP_ANY (UNARY, TOI, Toi, FIXI)

/*
 * ANY -> LONG (long)
 */
#define TOL(a) (long)(a)
#define FIXL(a) long
MAP_ANY (UNARY, TOL, Tol, FIXL)

/*
 * ANY -> LONGLONG (long long)
 */
#define TOLL(a) (long long)(a)
#define FIXLL(a) long long
MAP_ANY (UNARY, TOLL, Toll, FIXLL)

/*
 * ANY -> UNSIGNED BYTE (unsigned char)
 */
#define TOUB(a) (unsigned char)(a)
#define FIXUB(a) unsigned char
MAP_ANY (UNARY, TOUB, Toub, FIXUB)

/*
 * ANY -> UNSIGNED SHORT (unsigned short)
 */
#define TOUS(a) (unsigned short)(a)
#define FIXUS(a) unsigned short
MAP_ANY (UNARY, TOUS, Tous, FIXUS)

/*
 * ANY -> UNSIGNED INT (unsigned int)
 */
#define TOUI(a) (unsigned int)(a)
#define FIXUI(a) unsigned int
MAP_ANY (UNARY, TOUI, Toui, FIXUI)

/*
 * ANY -> UNSIGNED LONG (unsigned long)
 */
#define TOUL(a) (unsigned long)(a)
#define FIXUL(a) unsigned long
MAP_ANY (UNARY, TOUL, Toul, FIXUL)

/*
 * ANY -> UNSIGNED LONGLONG (unsigned long long)
 */
#define TOULL(a) (unsigned long long)(a)
#define FIXULL(a) unsigned long long
MAP_ANY (UNARY, TOULL, Toull, FIXULL)

/*
 * ANY -> FLOAT (float)
 */
#define TOF(a) (float)(a)
#define FIXF(a) float
MAP_ANY (UNARY, TOF, Tof, FIXF)

/*
 * ANY -> DOUBLE (double)
 */
#define TOD(a) (double)(a)
#define FIXD(a) double
MAP_ANY (UNARY, TOD, Tod, FIXD)

/*
 * INT -> CHAR (unsigned char)
 */
#define TOC(a) (unsigned char)(a)
#define FIXC(a) unsigned char
MAP_ANY (UNARY, TOC, Toc, FIXC)

/*
 * NUMoBOOL -> BOOL (bool)
 */
#define TOBOOL(a) ((a)==1?1:0)
MAP_NUMoBOOL (UNARY, TOBOOL, Tobool, FIXBOOL)



#undef DBUG_PREFIX
