#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);

#include "config.h"

extern "C" {
#include "argcount-cpp.h"
}

/* we do this to avoid causing an assert */
#ifdef STATIC_ASSERT
#undef STATIC_ASSERT
#define STATIC_ASSERT(cond, ret) cond
#endif

#define TEMP_TRUE TRUE
#define TEMP_FALSE FALSE
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

TEST (Macros, EmptyArgs)
{
    ASSERT_TRUE (IS_EMPTY ());
    ASSERT_FALSE (IS_EMPTY (,));
    ASSERT_FALSE (IS_EMPTY (1,2));
}

TEST (Macros, HasCommas)
{
    ASSERT_FALSE (HAS_COMMA());
    ASSERT_TRUE (HAS_COMMA(,));
}

TEST (Macros, IfCond)
{
    ASSERT_EQ (IFF (TRUE, 10, -10), 10);
    ASSERT_EQ (IFF (FALSE, 10, -10), -10);
}

TEST (Macros, Assert)
{
    // normally we would have a compiler error, but assert is rewritten to return false instead.
    ASSERT_FALSE (STATIC_ASSERT (FALSE, 10));
    ASSERT_TRUE (STATIC_ASSERT (TRUE, TRUE));
}

TEST (Macros, ArgCount)
{
    ASSERT_EQ (MACRO_ARGCOUNT (), 0);
    ASSERT_EQ (MACRO_ARGCOUNT (x), 1);
    ASSERT_EQ (MACRO_ARGCOUNT (a, b, c), 3);
    ASSERT_EQ (MACRO_ARGCOUNT (a1, a2, a3, a4, a5), 5);
    ASSERT_EQ (MACRO_ARGCOUNT (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), 10);
    ASSERT_EQ (MACRO_ARGCOUNT (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, a61, a62, a63), 63);

    // argument count is grater-than 64, normally we'd error out with an assertion, but this has be rewriten to return false instead
    ASSERT_FALSE (MACRO_ARGCOUNT (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, a61, a62, a63, a64, a65, a66, a67));
}

TEST (Macros, VAArgs)
{
#define VA(...) MACRO_ARGCOUNT (__VA_ARGS__)
    ASSERT_EQ (VA(), 0);
    ASSERT_EQ (VA(i), 1);
    ASSERT_EQ (VA(i,), 2);
#undef VA

#define T_0 0
#define T_1 1
#define T_2 2
#define VA(...) MACRO_GLUE(T_, MACRO_ARGCOUNT (__VA_ARGS__))
    ASSERT_EQ (VA(), 0);
    ASSERT_EQ (VA(i), 1);
    ASSERT_EQ (VA(i,), 2);
#undef VA
#undef T_0
#undef T_1
#undef T_2

#define T_0() 0
#define T_1(arg1) arg1
#define T_2(arg1, arg2) arg2
#define VA(...) MACRO_GLUE(T_, MACRO_ARGCOUNT (__VA_ARGS__))(__VA_ARGS__)
    ASSERT_EQ (VA(), 0);
    ASSERT_EQ (VA(1), 1);
    ASSERT_EQ (VA(1,2), 2);
#undef VA
#undef T_0
#undef T_1
#undef T_2
}

#undef TRUE
#undef FALSE
#define TRUE TEMP_TRUE
#define FALSE TEMP_FALSE
#undef TEMP_TRUE
#undef TEMP_FALSE
