#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#define DBUG_PREFIX "TEST-STR"
#include "debug.h"
#include "memory.h"
#include "str.h"
}

TEST (StringOperations, testToUpper)
{
    char *t = STRcpy ("test");
    STRtoupper (t, 0, 1);
    EXPECT_STREQ ("Test", t);

    STRtoupper (t, 0, 2);
    EXPECT_STREQ ("TEst", t);

    MEMfree (t);
}

// The code is total shite, it overruns the buffer
// FIXME This test shows a problem in the code.  Fix it in str.c,
//       uncomment the test and write a few more.
//TEST (StringOperations, testToUpperDoNotSegfault)
//{
//    char *t = strdup ("test");
//    EXPECT_NO_THROW (STRtoupper (t, 0, 1000000));
//    EXPECT_STREQ ("TEST", t);
//    MEMfree (t);
//}

TEST (StringOperations, testStrCpy)
{
    char *t = STRcpy ("test");
    EXPECT_STREQ ("test", t);
    MEMfree (t);

    char *tt = STRcpy (NULL);
    EXPECT_EQ (tt, (void *)NULL);
}


TEST (StringOperations, testStrNCpy)
{
    char *t = STRncpy ("test", 2);
    EXPECT_STREQ ("te", t);
    MEMfree (t);
}

TEST (StringOperations, testStrLen)
{
    char *t = STRcpy ("test");
    EXPECT_EQ (4, STRlen (t));
    MEMfree (t);

    t = STRcpy ("");
    EXPECT_EQ (0, STRlen (t));
    MEMfree (t);

    t = NULL;
    EXPECT_EQ (0, STRlen (t));
}

// This code is shite too, as it declares the size as int and doesn't check
// for negative sizes.
//
// FIXME These tests demonstrates errors in the existing imdplementation of
//       String functions.  Adjust str.c, uncomment these, and write more.
//TEST (StringOperations, testStrNCpyDie)
//{
//    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
//    char *tt;
//    ASSERT_DEATH (tt = STRncpy ("test", -1), "Assertion");
//}
//
//// Again, what the hell we are supposed to do with negative lengths?
//TEST (StringOperations, testSubStr01)
//{
//    char *t = STRsubStr ("test01", -1, 1);
//    EXPECT_STREQ ("1", t);
//    MEMfree (t);
//}
//
//
//TEST (StringOperations, testSubStr02)
//{
//    char *t = STRsubStr ("test01", -10000, 1);
//    EXPECT_STREQ ("t", t);
//    MEMfree (t);
//}

TEST (StringOperations, testSubStr03)
{
    char *t = STRsubStr ("test01", 1, 1);
    EXPECT_STREQ ("e", t);
    MEMfree (t);
}

TEST (StringOperations, testSubStr04)
{
    char *t = STRsubStr ("test01", 100, 1);
    EXPECT_STREQ ("", t);
    MEMfree (t);
}

TEST (StringOperations, testSubStr05)
{
    char *t = STRsubStr ("test01", 2, -100);
    EXPECT_STREQ ("", t);
    MEMfree (t);
}

TEST (StringOperations, testStrip)
{
    char *t = STRcpy ("    hello world     \n");
    t = STRstrip (t);
    size_t len = STRlen (t);
    EXPECT_STREQ ("hello world", t);
    EXPECT_EQ (STRlen ("hello world"), len);
    MEMfree (t);

    t = STRcpy ("       ");
    t = STRstrip (t);
    EXPECT_STREQ ("", t);
    MEMfree (t);
}

TEST (StringOperations, testIsIntFalse)
{
    bool t = STRisInt ("3a");
    EXPECT_FALSE (t);
}

TEST (StringOperations, testIsIntTrue)
{
    bool t = STRisInt ("3");
    EXPECT_TRUE (t);
}

TEST (StringOperations, testIsIntPlusSign)
{
    bool t = STRisInt ("+3");
    EXPECT_TRUE (t);
}

TEST (StringOperations, testIsIntDash)
{
    bool t = STRisInt ("-3");
    EXPECT_TRUE (t);
}
