/*
 *
 * $Log$
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

#ifndef SAC_ICM_H_

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
 *  Parameters are specified as (parm0,(parm1,(parm2,(parm3...)))).
 *
 *  The xcat macro is used to glue items together. Thus, the result of:
 *  #define foo(tuple) \
 *  xcat(xcat(Item4 tuple,Item3 tuple),Item2 tuple)
 *  foo((I,(see,(rats,(and,(mice,))))))
 * is:
 *     miceandrats
 *  NB. Note the trailing comma in the invocation's innermost nest
 */

#define Item0(a, b) a
#define Item1(a, b) Item0 b
#define Item2(a, b) Item1 b
#define Item3(a, b) Item2 b
#define Item4(a, b) Item3 b
#define Item5(a, b) Item4 b

/*
 * The odd-looking cat macro is required to provide a degree
 * of indirection for mixed catenates and macro expansions.
 * Replacing it with ## will NOT work. This is documented in
 * K&R, Section A.12.3.
 *
 *  For the same reason, we define AddParens to wrap
 *  parentheses around a generated item, and BuildArgs2 to
 *  add commas and parentheses to a two-parameter argument list.
 *  If you need a 3-parameter argument list, write BuildArgs3, etc.
 */

#define cat(x, y) xcat (x, y)
#define xcat(x, y) x##y

#define AddParens(a) xAddParens (a)
#define xAddParens(a) (##a##)

#define PrependComma(a) xPrependComma (a)
#define xPrependComma(a) , ##a

/*
 * Catenate two arguments, insert commas, add parens.
 * Sigh.
 */

#define BuildArgs2(a, b) xBuildArgs2 (a, b)
#define xBuildArgs2(a, b) (##a##, ##b##)

/*
 * VIEW is  Handy viewer for testing macros
 */

#define VIEW(a) This makes : AddParens (a)

#endif /* SAC_ICM_H_ */
