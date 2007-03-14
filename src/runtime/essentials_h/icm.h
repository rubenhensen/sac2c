/*
 *
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   sac_icm.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides utility macros that are used by the Intermediate
 *   Code Macros (ICM).
 *
 *****************************************************************************/

#ifndef _SAC_ICM_H
#define _SAC_ICM_H

/*
 * README: The ICM parameter access macros
 *
 * The following macros are used to perform positional access
 * to variable-length arguments in macro invocations.
 *
 * The ItemN macros are used to extract the Nth item from
 * a lisp-like list. This approach offers a number of advantages
 * in code generation, including:
 *   - Parameters in invocations  may be variable-length, thereby avoiding
 *     the C preprocessor's desire for fixed-length parameters.
 *     It also offers a rich field of potential bugs when you
 *     make a parameter too short: The generated code will look
 *     as if the macro was not expanded. This is because the macro
 *     was not, in fact, expanded.
 *
 *   - When more parameters are added, the existing macro definitions
 *     need not be changed. Neither do the existing macro invocations have
 *     to be changed.
 *
 *  Adding a new parameter only requires that someone write an Item(N+1)
 *  macro, if need be.
 *
 *  Using a new parameter requires that all parameters up to the new one
 *  be supplied (or that commas be supplied), because access is positional.
 *
 *  Parameters are specified as (parm0, (parm1, (parm2, (parm3 ...)))).
 *
 *  The CAT? macros are used to glue items together. Thus, the result of:
 *    #define foo( tuple) \
 *      CAT0( CAT0( Item4 tuple, Item3 tuple), Item2 tuple)
 *    foo( (I, (see, (rats, (and, (mice,))))))
 *  is:
 *    miceandrats
 *  NB. Note the trailing comma in the invocation's innermost nest
 *
 *  If nested macro expansion is used on each expansion level different
 *  CAT? macros have to be used!!! The result of:
 *    #define TEST1 CAT0( TEST, 2)
 *    #define TEST2 CAT1( TEST, 3)
 *    TEST1
 *  is:
 *    TEST3
 *  But the result of:
 *    #define TEST1 CAT0( TEST, 2)
 *    #define TEST2 CAT0( TEST, 3)
 *    TEST1
 *  is:
 *    CAT0( TEST, 3)
 */

#define Item0(a, b) a
#define Item1(a, b) Item0 b
#define Item2(a, b) Item1 b
#define Item3(a, b) Item2 b
#define Item4(a, b) Item3 b
#define Item5(a, b) Item4 b

/*
 * The odd-looking CAT? macros is required to provide a degree of indirection
 * for mixed catenates and macro expansions.
 * Replacing it with ## will NOT work.
 * Replacing [x]CAT?() with [x]CAT1() will NOT work either.
 * This is documented in K&R, Section A.12.3.
 *
 * For the same reason, we define AddParens to wrap parentheses around a
 * generated item, and BuildArgs2 to add commas and parentheses to a
 * two-parameter argument list.
 * If you need a 3-parameter argument list, write BuildArgs3, etc.
 */

#define CAT0(x, y) xCAT0 (x, y)
#define xCAT0(x, y) x##y

#define CAT1(x, y) xCAT1 (x, y)
#define xCAT1(x, y) x##y

#define CAT2(x, y) xCAT2 (x, y)
#define xCAT2(x, y) x##y

#define CAT3(x, y) xCAT3 (x, y)
#define xCAT3(x, y) x##y

#define CAT4(x, y) xCAT4 (x, y)
#define xCAT4(x, y) x##y

#define CAT5(x, y) xCAT5 (x, y)
#define xCAT5(x, y) x##y

#define CAT6(x, y) xCAT6 (x, y)
#define xCAT6(x, y) x##y

#define CAT7(x, y) xCAT7 (x, y)
#define xCAT7(x, y) x##y

#define CAT8(x, y) xCAT8 (x, y)
#define xCAT8(x, y) x##y

#define CAT9(x, y) xCAT9 (x, y)
#define xCAT9(x, y) x##y

#define CAT10(x, y) xCAT10 (x, y)
#define xCAT10(x, y) x##y

#define CAT11(x, y) xCAT11 (x, y)
#define xCAT11(x, y) x##y

#define CAT12(x, y) xCAT12 (x, y)
#define xCAT12(x, y) x##y

#define CAT13(x, y) xCAT13 (x, y)
#define xCAT13(x, y) x##y

#define CAT14(x, y) xCAT14 (x, y)
#define xCAT14(x, y) x##y

#define CAT15(x, y) xCAT15 (x, y)
#define xCAT15(x, y) x##y

#define CAT16(x, y) xCAT16 (x, y)
#define xCAT16(x, y) x##y

#define CAT17(x, y) xCAT17 (x, y)
#define xCAT17(x, y) x##y

#define CAT18(x, y) xCAT18 (x, y)
#define xCAT18(x, y) x##y

#define CAT19(x, y) xCAT19 (x, y)
#define xCAT19(x, y) x##y

#define CAT20(x, y) xCAT20 (x, y)
#define xCAT20(x, y) x##y

#define CAT21(x, y) xCAT21 (x, y)
#define xCAT21(x, y) x##y

#define CAT22(x, y) xCAT22 (x, y)
#define xCAT22(x, y) x##y

#define CAT23(x, y) xCAT23 (x, y)
#define xCAT23(x, y) x##y

#define CAT24(x, y) xCAT24 (x, y)
#define xCAT24(x, y) x##y

#define CAT25(x, y) xCAT25 (x, y)
#define xCAT25(x, y) x##y

#define CAT26(x, y) xCAT26 (x, y)
#define xCAT26(x, y) x##y

#define CAT27(x, y) xCAT27 (x, y)
#define xCAT27(x, y) x##y

#define CAT28(x, y) xCAT28 (x, y)
#define xCAT28(x, y) x##y

#define CAT29(x, y) xCAT29 (x, y)
#define xCAT29(x, y) x##y

#define CAT30(x, y) xCAT30 (x, y)
#define xCAT30(x, y) x##y

#define CAT31(x, y) xCAT31 (x, y)
#define xCAT31(x, y) x##y

#define AddParens(a) xAddParens (a)
#define xAddParens(a) (a)

/*
 * Catenate arguments, insert commas, add parens.
 */

#define BuildArgs1(a) xBuildArgs1 (a)
#define xBuildArgs1(a) (a)

#define BuildArgs2(a, b) xBuildArgs2 (a, b)
#define xBuildArgs2(a, b) (a, b)

#define BuildArgs3(a, b, c) xBuildArgs3 (a, b, c)
#define xBuildArgs3(a, b, c) (a, b, c)

#define BuildArgs4(a, b, c, d) xBuildArgs4 (a, b, c, d)
#define xBuildArgs4(a, b, c, d) (a, b, c, d)

/*
 * TO_STR converts the *expanded* argument into a string
 */

#define TO_STR(a) xTO_STR (a)
#define xTO_STR(a) #a

/*
 * VIEW is a handy viewer for testing macros
 */

#define VIEW(a) #a expands to AddParens(a)

#endif /* _SAC_ICM_H */