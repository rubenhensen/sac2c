/*
 *
 * $Log$
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

#ifndef SAC_ICM_H

#define SAC_ICM_H

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
 *  The cat? macros are used to glue items together. Thus, the result of:
 *    #define foo( tuple) \
 *      cat0( cat0( Item4 tuple, Item3 tuple), Item2 tuple)
 *    foo( (I, (see, (rats, (and, (mice,))))))
 *  is:
 *    miceandrats
 *  NB. Note the trailing comma in the invocation's innermost nest
 *
 *  If nested macro expansion is used on each expansion level different
 *  cat? macros have to be used!!! The result of:
 *    #define TEST1 cat0( TEST, 2)
 *    #define TEST2 cat1( TEST, 3)
 *    TEST1
 *  is:
 *    TEST3
 *  But the result of:
 *    #define TEST1 cat0( TEST, 2)
 *    #define TEST2 cat0( TEST, 3)
 *    TEST1
 *  is:
 *    cat0( TEST, 3)
 */

#define Item0(a, b) a
#define Item1(a, b) Item0 b
#define Item2(a, b) Item1 b
#define Item3(a, b) Item2 b
#define Item4(a, b) Item3 b
#define Item5(a, b) Item4 b

/*
 * The odd-looking cat? macros is required to provide a degree of indirection
 * for mixed catenates and macro expansions.
 * Replacing it with ## will NOT work.
 * Replacing [x]cat?() with [x]cat1() will NOT work either.
 * This is documented in K&R, Section A.12.3.
 *
 * For the same reason, we define AddParens to wrap parentheses around a
 * generated item, and BuildArgs2 to add commas and parentheses to a
 * two-parameter argument list.
 * If you need a 3-parameter argument list, write BuildArgs3, etc.
 */

#define cat0(x, y) xcat0 (x, y)
#define xcat0(x, y) x##y

#define cat1(x, y) xcat1 (x, y)
#define xcat1(x, y) x##y

#define cat2(x, y) xcat2 (x, y)
#define xcat2(x, y) x##y

#define cat3(x, y) xcat3 (x, y)
#define xcat3(x, y) x##y

#define cat4(x, y) xcat4 (x, y)
#define xcat4(x, y) x##y

#define cat5(x, y) xcat5 (x, y)
#define xcat5(x, y) x##y

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
 * Handling of undefined ICMs
 */

#define ICM_UNDEF                                                                        \
    ICM_IS_UNDEFINED /* CC will report a undefined symbol ICM_IS_UNDEFINED */

/*
 * VIEW is a handy viewer for testing macros
 */

#define VIEW(a) #a expands to AddParens(a)

#endif /* SAC_ICM_H */
