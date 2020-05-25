#include "gtest/gtest.h"
#include "config.h"

extern "C" {
#include "LookUpTable.h"
}

class LUTTest : public ::testing::Test {
    protected:
#if __cplusplus >= 201103L
        void SetUp() override
#else
        void SetUp()
#endif
        {
            lut_1 = LUTgenerateLut ();
            lut_2 = LUTgenerateLut ();
        }
#if __cplusplus >= 201103L
        void TearDown() override
#else
        void TearDown()
#endif
        {
            lut_1 = LUTremoveLut (lut_1);
            lut_2 = LUTremoveLut (lut_2);
        }

    lut_t *lut_1;
    lut_t *lut_2;
};

TEST_F (LUTTest, CreateDestroyTable)
{
    ASSERT_TRUE (lut_1);
    ASSERT_TRUE (LUTisEmptyLut (lut_1));
}

TEST_F (LUTTest, AddItemP)
{
    void *key = (void*)'a';
    void *value = (void*)'b';
    void **res;

    lut_1 = LUTinsertIntoLutP (lut_1, key, value);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, value);
}

TEST_F (LUTTest, AddItemS)
{
    char *key = const_cast<char*>("a");
    void *value = (void*)'b';
    void **res;

    lut_1 = LUTinsertIntoLutS (lut_1, key, value);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, value);
}

TEST_F (LUTTest, UpdateItemP)
{
    void *key = (void*)'a';
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    lut_1 = LUTupdateLutP (lut_1, key, v1, &ret);
    ASSERT_FALSE (ret);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    lut_1 = LUTupdateLutP (lut_1, key, v2, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);

    lut_1 = LUTupdateLutP (lut_1, key, v3, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v2);

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
}

TEST_F (LUTTest, UpdateItemS)
{
    char *key = const_cast<char*>("a");
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    lut_1 = LUTupdateLutS (lut_1, key, v1, &ret);
    ASSERT_FALSE (ret);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    lut_1 = LUTupdateLutS (lut_1, key, v2, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);

    lut_1 = LUTupdateLutS (lut_1, key, v3, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v2);

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
}

TEST_F (LUTTest, InsertItemP)
{
    void *key = (void*)'a';
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void **res;

    lut_1 = LUTinsertIntoLutP (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutP (lut_1, key, v2);
    lut_1 = LUTinsertIntoLutP (lut_1, key, v3);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);
    res = LUTsearchInLutNextP ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextP ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
    /* we should have reached the end of the collision table */
    res = LUTsearchInLutNextP ();
    ASSERT_FALSE (res);
}

TEST_F (LUTTest, InsertItemS)
{
    char *key = const_cast<char*>("a");
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void **res;

    lut_1 = LUTinsertIntoLutS (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutS (lut_1, key, v2);
    lut_1 = LUTinsertIntoLutS (lut_1, key, v3);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);
    res = LUTsearchInLutNextS ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextS ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
    /* we should have reached the end of the collision table */
    res = LUTsearchInLutNextS ();
    ASSERT_FALSE (res);
}

TEST_F (LUTTest, InsertAndUpdateP)
{
    void *key = (void*)'a';
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    lut_1 = LUTinsertIntoLutP (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutP (lut_1, key, v2);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);
    res = LUTsearchInLutNextP ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextP ();
    ASSERT_FALSE (res);

    lut_1 = LUTupdateLutP (lut_1, key, v3, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
    res = LUTsearchInLutNextP ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextP ();
    ASSERT_FALSE (res);
}

TEST_F (LUTTest, InsertAndUpdateS)
{
    char *key = const_cast<char*>("a");
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    lut_1 = LUTinsertIntoLutS (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutS (lut_1, key, v2);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);
    res = LUTsearchInLutNextS ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextS ();
    ASSERT_FALSE (res);

    lut_1 = LUTupdateLutS (lut_1, key, v3, &ret);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v3);
    res = LUTsearchInLutNextS ();
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v2);
    res = LUTsearchInLutNextS ();
    ASSERT_FALSE (res);
}

TEST_F (LUTTest, SearchP)
{
    void *key = (void*)'a';
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_FALSE (res);

    lut_1 = LUTinsertIntoLutP (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutP (lut_1, key, v2);
    lut_1 = LUTinsertIntoLutP (lut_1, key, v3);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutP (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);

    ret = LUTsearchInLutPp (lut_1, key);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);
}

TEST_F (LUTTest, SearchS)
{
    char *key = const_cast<char*>("a");
    void *v1 = (void*)'b';
    void *v2 = (void*)'c';
    void *v3 = (void*)'d';
    void *ret;
    void **res;

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_FALSE (res);

    lut_1 = LUTinsertIntoLutS (lut_1, key, v1);
    lut_1 = LUTinsertIntoLutS (lut_1, key, v2);
    lut_1 = LUTinsertIntoLutS (lut_1, key, v3);
    ASSERT_FALSE (LUTisEmptyLut (lut_1));

    res = LUTsearchInLutS (lut_1, key);
    ASSERT_TRUE (res);
    ASSERT_EQ (*res, v1);

    ret = LUTsearchInLutSs (lut_1, key);
    ASSERT_TRUE (ret);
    ASSERT_EQ (ret, v1);
}
