/*
 *
 * $Log$
 * Revision 3.6  2002/07/31 15:35:52  dkr
 * some more CAT?? macros added
 *
 * Revision 3.5  2002/07/23 16:14:13  dkr
 * more CAT? macros added
 *
 * Revision 3.4  2002/07/15 19:44:19  dkr
 * CAT6 added
 *
 * Revision 3.3  2002/06/28 12:58:46  dkr
 * ICM_UNDEF moved to sac_misc.h
 *
 * Revision 3.2  2002/04/30 08:46:28  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:15  sacbase
 * new release made
 *
 * Revision 1.9  2000/08/24 11:16:36  dkr
 * macros cat? renamed to CAT?
 *
 * Revision 1.8  2000/08/17 10:21:12  dkr
 * some comments modified
 * macro ICM_UNDEF added
 *
 * Revision 1.7  2000/07/06 08:37:33  dkr
 * [y]ycat() und [n]ncat() moved from sac_std.tagged.h to sac_icm.h
 *
 * Revision 1.6  2000/07/06 08:24:07  dkr
 * macros BuildArgs? added
 *
 * Revision 1.5  1999/06/24 14:31:32  rob
 * Fix Bernecky's lack of understanding of RCS vs RCS-files.
 *
 * Revision 1.4  1999/06/24 14:26:37  rob
 * Try to resolve RCS missing module problem.
 *
 * Revision 1.3  1999/06/24 14:24:30  rob
 * Try to resolve RCS problem with missing modules.
 *
 * Revision 1.2  1999/06/24 14:18:20  rob
 * Add this to standard set of sac2c files.
 *
 * Revision 1.1  1999/06/10 14:46:45  rob
 * Initial revision
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

#define AddParens(a) xAddParens (a)
#define xAddParens(a) (##a##)

#define PrependComma(a) xPrependComma (a)
#define xPrependComma(a) , ##a

/*
 * Catenate arguments, insert commas, add parens.
 */

#define BuildArgs2(a, b) xBuildArgs2 (a, b)
#define xBuildArgs2(a, b) (##a##, ##b##)

#define BuildArgs3(a, b, c) xBuildArgs3 (a, b, c)
#define xBuildArgs3(a, b, c) (##a##, ##b##, ##c##)

#define BuildArgs4(a, b, c, d) xBuildArgs4 (a, b, c, d)
#define xBuildArgs4(a, b, c, d) (##a##, ##b##, ##c##, ##d##)

/*
 * VIEW is a handy viewer for testing macros
 */

#define VIEW(a) #a expands to AddParens(a)

#endif /* _SAC_ICM_H */
