#include <sys/types.h>
#include <dirent.h>

#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!

extern "C" {
#define DBUG_PREFIX "TEST-STRBUF"
#include "debug.h"
#include "memory.h"
#include "str.h"
#include "str_buffer.h"
}

TEST (StrBuffer, testCreate)
{
    str_buf *buf;

    // Neither creation nor freeing should fail
    buf = SBUFcreate (0);
    ASSERT_EQ ('\0', SBUFgetBuffer (buf)[0]);
    SBUFfree (buf);

    // Regardless of size
    buf = SBUFcreate (128);
    ASSERT_EQ ('\0', SBUFgetBuffer (buf)[0]);
    SBUFfree (buf);
}

TEST (StrBuffer, testPrint)
{
    str_buf *buf;

    // An empty buffer should be able to reserve 0 characters.
    buf = SBUFcreate (0);
    SBUFprint (buf, "");
    EXPECT_EQ (0, SBUFlen (buf));

    // Reserving a string containing a nullbyte ignores everything past the nullbyte
    SBUFprint (buf, "a\0b");
    EXPECT_EQ (1, SBUFlen (buf));

    // A non-empty buffer should be able to write a string
    SBUFprint (buf, "bcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ (26, SBUFlen (buf));

    SBUFfree (buf);
}

TEST (StrBuffer, testFlush)
{
    str_buf *buf;

    // Both the length should be reset, and a null byte should be set at position 0 so all
    // functions recognize the buffer as an empty string.
    buf = SBUFcreate (5);
    SBUFprint (buf, "Data.");
    SBUFflush (buf);
    EXPECT_EQ (0, SBUFlen (buf));
    EXPECT_EQ ('\0', SBUFgetBuffer (buf)[0]);
    EXPECT_STREQ ("", SBUFgetBuffer (buf));

    SBUFfree (buf);
}

TEST (StrBuffer, testIsEmpty)
{
    str_buf *buf;

    // An empty buffer should be empty.
    buf = SBUFcreate (0);
    EXPECT_TRUE (SBUFisEmpty (buf));
    SBUFfree (buf);

    // An empty buffer with more than 0 allocated memory should still be empty .
    buf = SBUFcreate (50);
    EXPECT_TRUE (SBUFisEmpty (buf));
    
    // After adding a 0 length string, the buffer should still be empty.
    SBUFprint (buf, "");
    EXPECT_TRUE (SBUFisEmpty (buf));

    // After adding a null byte string, the buffer should still be empty.
    SBUFprint (buf, "\0");
    EXPECT_TRUE (SBUFisEmpty (buf));

    // After adding a string of length > 1, the buffer should no longer be empty.
    SBUFprint (buf, ".");
    EXPECT_FALSE (SBUFisEmpty (buf));

    SBUFfree (buf);
}

TEST (StrBuffer, testPrintf)
{
    str_buf *buf;
    str_buf *snd_buf;

    // An empty string with no arguments should work.
    buf = SBUFcreate (0);
    SBUFprintf (buf, "");
    EXPECT_TRUE (SBUFisEmpty (buf));

    // An effectively empty string with arguments should work.
    SBUFprintf (buf, "%0.s", "test");
    EXPECT_TRUE (SBUFisEmpty (buf));

    // A format without arguments should work.
    SBUFprintf (buf, "test");
    EXPECT_EQ (4, SBUFlen (buf));

    // An equivalent option with arguments should give the same result.
    snd_buf = SBUFcreate (0);
    SBUFprintf (snd_buf, "%s", "test");
    EXPECT_EQ (4, SBUFlen (snd_buf));

    SBUFfree (buf);
    SBUFfree (snd_buf);

    // The null byte should be written properly in a standard case.
    buf = SBUFcreate (5); 
    SBUFprintf (buf, "%s", "Test!");
    EXPECT_EQ ('\0', SBUFgetBuffer (buf)[5]);

    // The null byte should be written properly when new memory is allocated.
    SBUFprintf (buf, "%s", "Test.");
    EXPECT_EQ ('\0', SBUFgetBuffer (buf)[10]);

    // The null byte should be written properly after flushing the buffer.
    SBUFflush (buf);
    SBUFprintf (buf, "%s", "test");
    EXPECT_EQ ('\0', SBUFgetBuffer (buf)[4]);

    SBUFfree (buf);
}

TEST (StrBuffer, testToStr)
{
    str_buf *buf;
    char *str;

    // We should get the same string back that we put into an empty buffer.
    buf = SBUFcreate (0);
    SBUFprint (buf, "test");
    str = SBUF2str (buf);
    EXPECT_STREQ ("test", str);
    MEMfree (str);

    // If we append to the buffer, we should get the first string appended with the second.
    SBUFprint (buf, "123");
    str = SBUF2str (buf);
    EXPECT_STREQ ("test123", str);

    SBUFfree (buf);
    MEMfree (str);
}

TEST (StrBuffer, testToStrAndFree)
{
    str_buf *buf;
    char *str;

    // We should get the same string back that we put into an empty buffer.
    buf = SBUFcreate (0);
    SBUFprint (buf, "test");
    str = SBUF2strAndFree (&buf);
    EXPECT_STREQ ("test", str);
    MEMfree (str);

    // Freeing the buffer at this point should lead to a double free, but that can't be tested.
}

TEST (StrBuffer, testSubstToken)
{
    str_buf *buf;
    char *str;

    // Applying SBUFsubstToken(args) should give the same result as STRsubstToken(args)
    buf = SBUFcreate (0);
    SBUFprint (buf, "A sentence that we will apply a substitution on");
    str = STRsubstToken (SBUFgetBuffer (buf), "a", "ae");
    SBUFsubstToken (buf, "a", "ae");
    EXPECT_STREQ (str, SBUFgetBuffer (buf));

    // Appending something doesn't break things
    SBUFprint(buf, ".");
    EXPECT_STREQ ("A sentence thaet we will aepply ae substitution on.", SBUFgetBuffer (buf));

    SBUFfree (buf);
    MEMfree (str);
}

TEST (StrBuffer, testInsertAfterToken)
{
    str_buf *buf_insert;
    str_buf *buf_subst;

    // Applying SBUFinsertAfterToken(buf, token, insert) should give the
    // same result as SBUFsubstToken(buf, token, STRcat (token, insert))
    buf_insert = SBUFcreate (0);
    buf_subst = SBUFcreate (0);
    SBUFprint (buf_insert, "A,glorious,sentence,containing,many,commas.");
    SBUFprint (buf_subst, "A,glorious,sentence,containing,many,commas.");

    SBUFinsertAfterToken (buf_insert, ",", " ");
    SBUFsubstToken (buf_subst, ",", ", ");
    EXPECT_STREQ (SBUFgetBuffer (buf_subst), SBUFgetBuffer (buf_insert));

    SBUFinsertAfterToken (buf_insert, ", ", "A larger test");
    SBUFsubstToken (buf_subst, ", ", ", A larger test");
    EXPECT_STREQ (SBUFgetBuffer (buf_subst), SBUFgetBuffer (buf_insert));

    SBUFfree (buf_insert);
    SBUFfree (buf_subst);
}

TEST (StrBuffer, testTruncate)
{
    str_buf *buf_one;
    str_buf *buf_two;
    char *string;

    buf_one = SBUFcreate(0);
    buf_two = SBUFcreate(15);

    // Initially, the two buffers differ
    SBUFprint (buf_two, "Buffer contents");
    EXPECT_EQ (15, SBUFlen (buf_two)); // To test the internal len parameter
    EXPECT_EQ (15, STRlen (SBUFgetBuffer(buf_two))); // To test that the null byte is set correctly
    EXPECT_STRNE (SBUFgetBuffer(buf_one), SBUFgetBuffer (buf_two));
    
    // After truncating and leaving buf_two at a length of 1, the buffers still differ
    SBUFtruncate (buf_two, 1);
    EXPECT_EQ (1, SBUFlen (buf_two));
    EXPECT_STRNE (SBUFgetBuffer(buf_one), SBUFgetBuffer (buf_two));

    // After truncating the buffer to be empty, the buffers are equal.
    SBUFtruncate (buf_two, 0);
    EXPECT_TRUE (SBUFisEmpty (buf_two));
    EXPECT_STREQ (SBUFgetBuffer(buf_one), SBUFgetBuffer (buf_two));

    SBUFfree (buf_one);
    SBUFfree (buf_two);
}