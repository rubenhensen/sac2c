/**
 * @file
 * @brief ArgCount CPP macros
 *
 * The macros defined in here deal primarily with handling macro VA_ARGS,
 * allowing for a form of macro-level overloading.
 */
#ifndef _SAC_LIBSAC2C_ARGCOUNT_H_
#define _SAC_LIBSAC2C_ARGCOUNT_H_

/* this hackishly safe guards use from `-pedantic` related warnings
 * caused by our use zero-argument variadic macros.
 */
#ifdef __GNUC__
#pragma GCC system_header
#endif

/**
 * @brief Static assert like macro, it causes the compiler to fail
 *        if condition is false.
 *
 * @param e Condition to fail on if false
 * @param x value
 */
#define STATIC_ASSERT(cond, x) \
    ((struct {const int ASSERT[cond ? 1 : -1];}){.ASSERT={1}}.ASSERT[0] ? x : x)

/**
 * @brief Concatenate two values together.
 * @param x first value
 * @param y second value
 */
#define MACRO_GLUE(x, y) __MACRO_GLUE(x,y)
#define __MACRO_GLUE(x, y) x ## y

/**
 * @brief Count number of arguments, return number (up to 64).
 *
 * This macro allows us to define macros which expand to different values
 * depending on the number of arguments that are passed.
 *
 * Example:
 *
 * We make use of macros #MACRO_GLUE and #MACRO_ARGCOUNT, the first
 * concatenates two values, the latter returns number of arguments passed.
 * For instance, we have some macro that returns the second argument when
 * give two arguments, but gives the first argument when passing three
 * arguments:
 *
 * __TEST(x,y) y
 * __TEST(x,y,z) x
 *
 * We can hide this behind a single macro TEST by doing the following:
 *
 * __TEST2(x,y) y
 * __TEST3(x,y,z) x
 * TEST(...) MACRO_GLUE(__TEST, MACRO_ARGCOUNT(__VA_ARGS__))(__VA_ARGS__)
 *
 * Here we use #MACRO_GLUE and #MACRO_ARGCOUNT to achieve this, the first
 * is used to concatenate two values together (while fully expanding macros)
 * and the second counts the number of arguments. An expansion of the macros
 * looks like this:
 *
 * MACRO_GLUE (__TEST, 2)(x, y) -> __TEST2(x,y)
 *
 * @param ... any number of arguments
 */
#define MACRO_ARGCOUNT(...) \
    IFF (FOLD (SND, IS_EMPTY, __VA_ARGS__), \
        IFF (IS_EMPTY(__VA_ARGS__), \
            0, \
            _ARG64 (__VA_ARGS__, \
                63,62,61,60,59,58,57,56,55,54,53,52,51, \
                50,49,48,47,46,45,44,43,42,41,40,39,38, \
                37,36,35,34,33,32,31,30,29,28,27,26,25, \
                24,23,22,21,20,19,18,17,16,15,14,13,12, \
                11,10,9,8,7,6,5,4,3,2,1,0)), \
            STATIC_ASSERT (0, -1))

/*** THESE MACROS SHOULD NOT BE USED DIRECTLY ***/

/**
 * @brief Determine whether macro arguments contain commas.
 *
 * We only handle up to 64 arguments here.
 *
 * NOTE: if this macro is used directly we have a problem when the
 * number of arguments exceeds the maximum.
 *
 * @param ... any number args
 */
#define HAS_COMMA(...) \
    _ARG64(__VA_ARGS__, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,1,1,1,1,1,1,1,1, \
        1,1,0)
#define _ARG64(_0,_1, _2, _3, _4, _5, _6, _7, _8, \
          _9,_10,_11,_12,_13,_14,_15,_16,_17,_18, \
         _19,_20,_21,_22,_23,_24,_25,_26,_27,_28, \
         _29,_30,_31,_32,_33,_34,_35,_36,_37,_38, \
         _39,_40,_41,_42,_43,_44,_45,_46,_47,_48, \
         _49,_50,_51,_52,_53,_54,_55,_56,_57,_58, \
         _59,_60,_61,_62,_63,...) _63

/**
 * Figure out whether we have an empty argument list or not.
 * This uses the above two macros and serves a particular
 * use case of zero or one arguments.
 *
 * For more details see:
 * https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
 * and P99 library where this trick is used as well: http://p99.gforge.inria.fr/
 */
#define IS_EMPTY(...)                                                    \
_ISEMPTY(                                                               \
          /* test if there is just one argument, eventually an empty    \
             one */                                                     \
          HAS_COMMA(__VA_ARGS__),                                       \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
             adds a comma */                                            \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
          /* test if the argument together with a parenthesis           \
             adds a comma */                                            \
          HAS_COMMA(__VA_ARGS__ (/*empty*/)),                           \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the   \
             parenthesis adds a comma */                                \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))      \
          )
#define PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define _TRIGGER_PARENTHESIS_(...) ,
#define _ISEMPTY(_0, _1, _2, _3) \
    HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,

/**
 * @brief A fold-like macro function
 *
 * This macro turns a list of arguments x1, x2, ... xn into:
 *
 *   CONS (FARG (x1), CONS (FARG (x2), CONS..., FARG(xn)))
 *
 * Note that we have to expand the macro as many times as the
 * maximum length of arguments we want to support.
 */
#define FOLD(CONS, FARG, ...) F0 (CONS, FARG, __VA_ARGS__)
#define  F0(CONS,FARG, x, ...) CONS (FARG (x),  F1 (CONS, FARG, __VA_ARGS__))
#define  F1(CONS,FARG, x, ...) CONS (FARG (x),  F2 (CONS, FARG, __VA_ARGS__))
#define  F2(CONS,FARG, x, ...) CONS (FARG (x),  F3 (CONS, FARG, __VA_ARGS__))
#define  F3(CONS,FARG, x, ...) CONS (FARG (x),  F4 (CONS, FARG, __VA_ARGS__))
#define  F4(CONS,FARG, x, ...) CONS (FARG (x),  F5 (CONS, FARG, __VA_ARGS__))
#define  F5(CONS,FARG, x, ...) CONS (FARG (x),  F6 (CONS, FARG, __VA_ARGS__))
#define  F6(CONS,FARG, x, ...) CONS (FARG (x),  F7 (CONS, FARG, __VA_ARGS__))
#define  F7(CONS,FARG, x, ...) CONS (FARG (x),  F8 (CONS, FARG, __VA_ARGS__))
#define  F8(CONS,FARG, x, ...) CONS (FARG (x),  F9 (CONS, FARG, __VA_ARGS__))
#define  F9(CONS,FARG, x, ...) CONS (FARG (x), F10 (CONS, FARG, __VA_ARGS__))
#define F10(CONS,FARG, x, ...) CONS (FARG (x), F11 (CONS, FARG, __VA_ARGS__))
#define F11(CONS,FARG, x, ...) CONS (FARG (x), F12 (CONS, FARG, __VA_ARGS__))
#define F12(CONS,FARG, x, ...) CONS (FARG (x), F13 (CONS, FARG, __VA_ARGS__))
#define F13(CONS,FARG, x, ...) CONS (FARG (x), F14 (CONS, FARG, __VA_ARGS__))
#define F14(CONS,FARG, x, ...) CONS (FARG (x), F15 (CONS, FARG, __VA_ARGS__))
#define F15(CONS,FARG, x, ...) CONS (FARG (x), F16 (CONS, FARG, __VA_ARGS__))
#define F16(CONS,FARG, x, ...) CONS (FARG (x), F17 (CONS, FARG, __VA_ARGS__))
#define F17(CONS,FARG, x, ...) CONS (FARG (x), F18 (CONS, FARG, __VA_ARGS__))
#define F18(CONS,FARG, x, ...) CONS (FARG (x), F19 (CONS, FARG, __VA_ARGS__))
#define F19(CONS,FARG, x, ...) CONS (FARG (x), F20 (CONS, FARG, __VA_ARGS__))
#define F20(CONS,FARG, x, ...) CONS (FARG (x), F21 (CONS, FARG, __VA_ARGS__))
#define F21(CONS,FARG, x, ...) CONS (FARG (x), F22 (CONS, FARG, __VA_ARGS__))
#define F22(CONS,FARG, x, ...) CONS (FARG (x), F23 (CONS, FARG, __VA_ARGS__))
#define F23(CONS,FARG, x, ...) CONS (FARG (x), F24 (CONS, FARG, __VA_ARGS__))
#define F24(CONS,FARG, x, ...) CONS (FARG (x), F25 (CONS, FARG, __VA_ARGS__))
#define F25(CONS,FARG, x, ...) CONS (FARG (x), F26 (CONS, FARG, __VA_ARGS__))
#define F26(CONS,FARG, x, ...) CONS (FARG (x), F27 (CONS, FARG, __VA_ARGS__))
#define F27(CONS,FARG, x, ...) CONS (FARG (x), F28 (CONS, FARG, __VA_ARGS__))
#define F28(CONS,FARG, x, ...) CONS (FARG (x), F29 (CONS, FARG, __VA_ARGS__))
#define F29(CONS,FARG, x, ...) CONS (FARG (x), F30 (CONS, FARG, __VA_ARGS__))
#define F30(CONS,FARG, x, ...) CONS (FARG (x), F31 (CONS, FARG, __VA_ARGS__))
#define F31(CONS,FARG, x, ...) CONS (FARG (x), F32 (CONS, FARG, __VA_ARGS__))
#define F32(CONS,FARG, x, ...) CONS (FARG (x), F33 (CONS, FARG, __VA_ARGS__))
#define F33(CONS,FARG, x, ...) CONS (FARG (x), F34 (CONS, FARG, __VA_ARGS__))
#define F34(CONS,FARG, x, ...) CONS (FARG (x), F35 (CONS, FARG, __VA_ARGS__))
#define F35(CONS,FARG, x, ...) CONS (FARG (x), F36 (CONS, FARG, __VA_ARGS__))
#define F36(CONS,FARG, x, ...) CONS (FARG (x), F37 (CONS, FARG, __VA_ARGS__))
#define F37(CONS,FARG, x, ...) CONS (FARG (x), F38 (CONS, FARG, __VA_ARGS__))
#define F38(CONS,FARG, x, ...) CONS (FARG (x), F39 (CONS, FARG, __VA_ARGS__))
#define F39(CONS,FARG, x, ...) CONS (FARG (x), F40 (CONS, FARG, __VA_ARGS__))
#define F40(CONS,FARG, x, ...) CONS (FARG (x), F41 (CONS, FARG, __VA_ARGS__))
#define F41(CONS,FARG, x, ...) CONS (FARG (x), F42 (CONS, FARG, __VA_ARGS__))
#define F42(CONS,FARG, x, ...) CONS (FARG (x), F43 (CONS, FARG, __VA_ARGS__))
#define F43(CONS,FARG, x, ...) CONS (FARG (x), F44 (CONS, FARG, __VA_ARGS__))
#define F44(CONS,FARG, x, ...) CONS (FARG (x), F45 (CONS, FARG, __VA_ARGS__))
#define F45(CONS,FARG, x, ...) CONS (FARG (x), F46 (CONS, FARG, __VA_ARGS__))
#define F46(CONS,FARG, x, ...) CONS (FARG (x), F47 (CONS, FARG, __VA_ARGS__))
#define F47(CONS,FARG, x, ...) CONS (FARG (x), F48 (CONS, FARG, __VA_ARGS__))
#define F48(CONS,FARG, x, ...) CONS (FARG (x), F49 (CONS, FARG, __VA_ARGS__))
#define F49(CONS,FARG, x, ...) CONS (FARG (x), F50 (CONS, FARG, __VA_ARGS__))
#define F50(CONS,FARG, x, ...) CONS (FARG (x), F51 (CONS, FARG, __VA_ARGS__))
#define F51(CONS,FARG, x, ...) CONS (FARG (x), F52 (CONS, FARG, __VA_ARGS__))
#define F52(CONS,FARG, x, ...) CONS (FARG (x), F53 (CONS, FARG, __VA_ARGS__))
#define F53(CONS,FARG, x, ...) CONS (FARG (x), F54 (CONS, FARG, __VA_ARGS__))
#define F54(CONS,FARG, x, ...) CONS (FARG (x), F55 (CONS, FARG, __VA_ARGS__))
#define F55(CONS,FARG, x, ...) CONS (FARG (x), F56 (CONS, FARG, __VA_ARGS__))
#define F56(CONS,FARG, x, ...) CONS (FARG (x), F57 (CONS, FARG, __VA_ARGS__))
#define F57(CONS,FARG, x, ...) CONS (FARG (x), F58 (CONS, FARG, __VA_ARGS__))
#define F58(CONS,FARG, x, ...) CONS (FARG (x), F59 (CONS, FARG, __VA_ARGS__))
#define F59(CONS,FARG, x, ...) CONS (FARG (x), F60 (CONS, FARG, __VA_ARGS__))
#define F60(CONS,FARG, x, ...) CONS (FARG (x), F61 (CONS, FARG, __VA_ARGS__))
#define F61(CONS,FARG, x, ...) CONS (FARG (x), F62 (CONS, FARG, __VA_ARGS__))
#define F62(CONS,FARG, x, ...) CONS (FARG (x), F63 (CONS, FARG, __VA_ARGS__))
#define F63(CONS,FARG, x, ...) FARG (x)

/**
 * @brief Take the second argument, and throw away the first one.
 *
 * @param x dummy value
 * @param y value to be expanded to
 */
#define SND(x,y) y

/**
 * @brief Conditionally print value
 *
 * @param c Conditional that evaluates to 0 or 1
 * @param t What to return on true
 * @param f What to return on false
 */
#define IFF(c, t, f) MACRO_GLUE (IFF_, c) (t, f)
#define IFF_1(t, f) t
#define IFF_0(t, f) f

#endif /* _SAC_LIBSAC2C_ARGCOUNT_H_ */
