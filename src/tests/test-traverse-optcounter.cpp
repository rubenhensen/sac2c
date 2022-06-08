#include <cstdio>

#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!

#include "config.h"


/* we safely ignore these */
#define DBUG_PRINT(smt, ...)
#define DBUG_PRINT_TAG(tag, smt, ...)
#define DBUG_OFF /* to not call phase.c functions */

extern "C" {
#include "traverse_optcounter.h"
}

static int counter = 0;

static int
testFunction (int input)
{
    counter++;
    return input;
}

TEST (MACRO_OPTCOUNTER, Setup)
{
    TOC_SETUP (2, COUNT_ONE, COUNT_TWO)

    ASSERT_TRUE (toc_optcount_size == 3);
    ASSERT_TRUE (toc_store[TOC_IGNORE] == 0);
    ASSERT_TRUE (toc_store[COUNT_ONE] == 0);
    ASSERT_TRUE (toc_store_old[COUNT_ONE] == 0);
}

TEST (MACRO_OPTCOUNTER, SetAndGetCounter)
{
    TOC_SETUP (1, COUNT_ONE)

    ASSERT_TRUE (TOC_GETCOUNTER (COUNT_ONE) == 0);
    TOC_SETCOUNTER (COUNT_ONE, 2)
    ASSERT_TRUE (TOC_GETCOUNTER (COUNT_ONE) == 2);
    ASSERT_TRUE (toc_store_old[COUNT_ONE] == 2);

    TOC_RESETCOUNTERS ()
    ASSERT_TRUE (TOC_GETCOUNTER (COUNT_ONE) == 0);
    ASSERT_TRUE (toc_store_old[COUNT_ONE] == 0);
}

TEST (MACRO_OPTCOUNTER, CompareCounters)
{
    bool test = false;
    TOC_SETUP (3, COUNT_ONE, COUNT_TWO, COUNT_THREE)

    TOC_COMPARE (test)

    ASSERT_TRUE (test);

    toc_store[COUNT_TWO] = 10;

    TOC_COMPARE (test)

    ASSERT_FALSE (test);
}

TEST (MACRO_OPTCOUNTER, RunOpt)
{
    int t = 4;
    TOC_SETUP (2, COUNT_ONE, COUNT_TWO)

    TOC_RUNOPT ("Blah", true, COUNT_ONE, counter, t, testFunction)
    ASSERT_TRUE (toc_store[COUNT_ONE] == 1);
    ASSERT_TRUE (toc_store[COUNT_TWO] == 0);
    TOC_RUNOPT ("Blah", true, COUNT_ONE, counter, t, testFunction)
    ASSERT_TRUE (toc_store[COUNT_ONE] == 2);
    ASSERT_TRUE (toc_store[COUNT_TWO] == 0);
    TOC_RUNOPT ("Blah", true, TOC_IGNORE, 0, t, testFunction)
    ASSERT_TRUE (toc_store[COUNT_ONE] == 2);
    ASSERT_TRUE (toc_store[COUNT_TWO] == 0);
    TOC_RUNOPT ("Blah", false, COUNT_TWO, counter, t, testFunction)
    ASSERT_TRUE (toc_store[COUNT_ONE] == 2);
    ASSERT_TRUE (toc_store[COUNT_TWO] == 0);
    TOC_RUNOPT ("Blah", true, COUNT_TWO, counter, t, testFunction)
    ASSERT_TRUE (toc_store[COUNT_ONE] == 2);
    ASSERT_TRUE (toc_store[COUNT_TWO] == 4);
}
