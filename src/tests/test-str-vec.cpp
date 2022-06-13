#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#include "memory.h"
#include "str_vec.h"
}

char* constStr = (char*) "const";
char* helloStr = (char*) "Hello";
char* worldStr = (char*) "world!";
char* emptyStr = (char*) "";

TEST (StringVectorOperations, testMake)
{
    strvec* vec = STRVECmake (2, helloStr, worldStr);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

TEST (StringVectorOperations, testConst)
{
    strvec* vec = STRVECconst (2, constStr);

    EXPECT_STREQ (STRVECsel (vec, 0), constStr);
    EXPECT_STREQ (STRVECsel (vec, 1), constStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

TEST (StringVectorOperations, testFromArray)
{
    char* strarray[2] = { helloStr, worldStr };
    strvec* vec = STRVECfromArray (strarray, 2);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

char* StrVecGenerator () {
    return constStr;
}

TEST (StringVectorOperations, testGen)
{
    strvec* vec = STRVECgen (2, StrVecGenerator);

    EXPECT_STREQ (STRVECsel (vec, 0), constStr);
    EXPECT_STREQ (STRVECsel (vec, 1), constStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

TEST (StringVectorOperations, testResize)
{
    strvec* vec = STRVECmake (2, helloStr, worldStr);
    STRVECresize (vec, 3, StrVecGenerator);
    STRVECresize (vec, 4, NULL);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_STREQ (STRVECsel (vec, 2), constStr);
    EXPECT_STREQ (STRVECsel (vec, 3), emptyStr);
    EXPECT_EQ (STRVEClen (vec), 4);

    STRVECresize (vec, 2, NULL);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

TEST (StringVectorOperations, testIsEmpty)
{
    strvec* vec = STRVECmake (0);
    EXPECT_TRUE (STRVECisEmpty (vec));

    STRVECresize(vec, 2, NULL);
    EXPECT_FALSE (STRVECisEmpty (vec));

    STRVECfree (vec);
}

TEST (StringVectorOperations, testCopy)
{
    strvec* vec = STRVECmake (2, helloStr, worldStr);
    strvec* scopy = STRVECcopy (vec);
    strvec* dcopy = STRVECcopyDeep (vec);

    EXPECT_EQ (STRVEClen (scopy), 2);
    EXPECT_EQ (STRVEClen (dcopy), 2);

    EXPECT_EQ (STRVECsel (vec, 0), STRVECsel (scopy, 0));
    EXPECT_NE (STRVECsel (vec, 0), STRVECsel (dcopy, 0));
    EXPECT_EQ (STRVECsel (vec, 1), STRVECsel (scopy, 1));
    EXPECT_NE (STRVECsel (vec, 1), STRVECsel (dcopy, 1));

    EXPECT_STREQ (STRVECsel (vec, 0), STRVECsel (scopy, 0));
    EXPECT_STREQ (STRVECsel (vec, 0), STRVECsel (dcopy, 0));
    EXPECT_STREQ (STRVECsel (vec, 1), STRVECsel (scopy, 1));
    EXPECT_STREQ (STRVECsel (vec, 1), STRVECsel (dcopy, 1));

    STRVECfree (vec);
    STRVECfree (scopy);
    STRVECfreeDeep (dcopy);
}

TEST (StringVectorOperations, testAppend)
{
    strvec* vec = STRVECmake (1, helloStr);
    STRVECappend (vec, worldStr);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
}

TEST (StringVectorOperations, testConcat)
{
    strvec* vec = STRVECmake(1, helloStr);
    strvec* vec2 = STRVECmake (1, worldStr);
    STRVECconcat (vec, vec2);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);

    STRVECfree (vec);
    STRVECfree (vec2);
}

TEST (StringVectorOperations, testSwap)
{
    strvec* vec = STRVECmake (2, helloStr, helloStr);
    char* str = STRVECswap (vec, 1, worldStr);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);
    EXPECT_STREQ (helloStr, str);

    STRVECfree (vec);
}


TEST (StringVectorOperations, testPop)
{
    strvec* vec = STRVECmake (3, helloStr, worldStr, constStr);
    char* str = STRVECpop (vec);

    EXPECT_STREQ (STRVECsel (vec, 0), helloStr);
    EXPECT_STREQ (STRVECsel (vec, 1), worldStr);
    EXPECT_EQ (STRVEClen (vec), 2);
    EXPECT_STREQ (constStr, str);

    STRVECfree (vec);
}
