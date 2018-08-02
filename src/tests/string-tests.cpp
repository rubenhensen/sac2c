#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#include "memory.h"
#include "str.h"
}

TEST (StringOperations, testToUpper)
{
    char *t = strdup ("test");
    STRtoupper (t, 0, 1);
    EXPECT_STREQ ("Test", t);

    STRtoupper (t, 0, 2);
    EXPECT_STREQ ("TEst", t);

    MEMfree (t);
}

// The code is total shite, it overruns the buffer
TEST (StringOperations, testToUpperDoNotSegfault)
{
    char *t = strdup ("test");
    EXPECT_NO_THROW (STRtoupper (t, 0, 1000000));
    EXPECT_STREQ ("TEST", t);
    MEMfree (t);
}

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

// This code is shite too, as it declares the size as int and doesn't check
// for negative sizes.
TEST (StringOperations, testStrNCpyDie)
{
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    char *tt;
    ASSERT_DEATH (tt = STRncpy ("test", -1), "Assertion");
}

// Again, what the hell we are supposed to do with negative lengths?
TEST (StringOperations, testSubStr01)
{
    char *t = STRsubStr ("test01", -1, 1);
    EXPECT_STREQ ("1", t);
    MEMfree (t);
}


TEST (StringOperations, testSubStr02)
{
    char *t = STRsubStr ("test01", -10000, 1);
    EXPECT_STREQ ("t", t);
    MEMfree (t);
}

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

