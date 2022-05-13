#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#define DBUG_PREFIX "TEST-CTF"
#include "debug.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "str_buffer.h"
#include "ctformatting.h"
}

TEST (CTF, testCreateMessageBeginSingleLineNoWrapping)
{
    str_buf *header;
    str_buf *remaining_lines;
    char *format;

    global.cti_single_line = true;
    global.cti_message_length = 0; // disables line wrapping

    // Ensure that we don't get a trailing enter in the base case
    header = SBUFcreate (0);
    remaining_lines = CTFcreateMessageBegin (header, "");
    EXPECT_TRUE (SBUFisEmpty (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that headerless normal messages are printed correctly
    remaining_lines = CTFcreateMessageBegin (header, "A %s yet %s test.", "normal", "fabulous");
    EXPECT_STREQ ("A normal yet fabulous test.", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that multiline messages are still printed on one line
    SBUFflush (header);
    remaining_lines = CTFcreateMessageBegin (header, "A\nmessage\ncontaining%sspaces", "\nmany\n");
    EXPECT_STREQ ("A message containing many spaces", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that if a non-empty header is given, it is prepended to the message.
    SBUFflush (header);
    SBUFprint (header, "Some header: ");
    remaining_lines = CTFcreateMessageBegin (header, "A %s\nmessage", "neat");
    EXPECT_STREQ ("Some header: A neat message", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    SBUFfree (header);
}

TEST (CTF, testCreateMessageBeginMultiLineNoWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 0; // disables line wrapping

    str_buf *header;
    str_buf *remaining_lines;

    // Ensure that we still get a newline if we give empty inputs
    header = SBUFcreate (0);
    remaining_lines = CTFcreateMessageBegin (header, "");
    EXPECT_STREQ ("\n", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that normal messages are working correctly
    SBUFflush (header);
    remaining_lines = CTFcreateMessageBegin (header, "A %s test.", "normal");
    EXPECT_STREQ ("A normal test.\n", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that remaining_lines contains the remaining lines if there are multiple lines
    // Ensure that the header is prepended only to the first line
    SBUFflush (header);
    SBUFprint (header, "Interesting header: ");
    remaining_lines = CTFcreateMessageBegin (header, "A\nMultiline\nTest");
    EXPECT_STREQ ("Interesting header: A\n", SBUFgetBuffer (header));
    EXPECT_STREQ ("Multiline\nTest", SBUFgetBuffer (remaining_lines));
    SBUFfree (remaining_lines);

    SBUFfree (header);
}

TEST (CTF, testCreateMessageBeginSingleLineWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    
    str_buf *header_no_wrap;
    str_buf *remaining_lines_no_wrap;
    str_buf *header_wrap;
    str_buf *remaining_lines_wrap;
    char *long_message;

    // c++ complains if we use a normal character array, so we use STRcpy to allocate it on the heap
    long_message = STRcpy ("A pretty long message that is longer than the header and then some.! "
                           "If wrapping were enabled, it would surely work on this gigantic string.");

    header_no_wrap = SBUFcreate (0);
    remaining_lines_no_wrap = CTFcreateMessageBegin (header_no_wrap, "%s", long_message);

    global.cti_single_line = true;
    global.cti_message_length = 25;

    // Ensure that line wrapping is ignored when single_line is active:
    // The output should be equal to the output from single line with wrapping explicitly disabled
    header_wrap = SBUFcreate (0);
    remaining_lines_wrap = CTFcreateMessageBegin (header_wrap, "%s", long_message);
    EXPECT_STREQ (SBUFgetBuffer (header_no_wrap), SBUFgetBuffer(header_wrap));
    EXPECT_STREQ (SBUFgetBuffer (remaining_lines_no_wrap), SBUFgetBuffer (remaining_lines_wrap)); 
    SBUFfree (remaining_lines_no_wrap);
    SBUFfree (remaining_lines_wrap);

    SBUFfree (header_no_wrap);
    SBUFfree (header_wrap);
    MEMfree (long_message);
}

TEST (CTF, testCreateMessageBeginMultiLineWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 30;

    str_buf *header;
    str_buf *remaining_lines;

    // Ensure that we get a trailing enter in the base case
    header = SBUFcreate (0);
    remaining_lines = CTFcreateMessageBegin (header, "");
    EXPECT_STREQ ("\n", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that message shorter than the message length aren't wrapped.
    SBUFflush (header);
    SBUFprint (header, "Header: ");
    remaining_lines = CTFcreateMessageBegin (header, "A %s.", "string");
    EXPECT_STREQ ("Header: A string.\n", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that messages longer than the message length without any wrapping locations aren't wrapped.
    SBUFflush (header);
    remaining_lines = CTFcreateMessageBegin (header, "0123456789012345678901234567890123456789");
    EXPECT_STREQ ("0123456789012345678901234567890123456789\n", SBUFgetBuffer (header));
    EXPECT_TRUE (SBUFisEmpty (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that messages longer than the message length with a valid wrapping location
    // are wrappped at the last possible location.
    SBUFflush (header);
    remaining_lines = CTFcreateMessageBegin (header, " 123456789 123456789 123456789  23456789");
    EXPECT_STREQ (" 123456789 123456789 123456789\n", SBUFgetBuffer (header));
    EXPECT_STREQ (" 23456789", SBUFgetBuffer (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that tabs are converted to spaces and potentially used for wrapping
    SBUFflush (header);
    remaining_lines = CTFcreateMessageBegin (header, "\t123456789\t123456789\t123456789\t123456789");
    EXPECT_STREQ (" 123456789 123456789 123456789\n", SBUFgetBuffer (header));
    EXPECT_STREQ ("123456789", SBUFgetBuffer (remaining_lines));
    SBUFfree (remaining_lines);

    // Ensure that wrapping takes into account non-empty headers
    SBUFflush (header);
    SBUFprint (header, "Header! ");
    remaining_lines = CTFcreateMessageBegin (header, "123456789 123456789 123456789 123456789");
    EXPECT_STREQ ("Header! 123456789 123456789\n", SBUFgetBuffer (header));
    EXPECT_STREQ ("123456789 123456789", SBUFgetBuffer (remaining_lines));
    SBUFfree (remaining_lines);

    // If message length is smaller than the header length + 20 characters, line wrapping will instead
    // be done at header length + 20 characters
    global.cti_message_length = 1;
    SBUFflush (header);
    SBUFprint (header, "Ten chars:");
    remaining_lines = CTFcreateMessageBegin (header, " 123456789 123456789  ");
    EXPECT_STREQ ("Ten chars: 123456789 123456789\n", SBUFgetBuffer (header));
    EXPECT_STREQ (" ", SBUFgetBuffer (remaining_lines));
    SBUFfree (remaining_lines);

    SBUFfree (header);
}
