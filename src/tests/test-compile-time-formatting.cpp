#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#define DBUG_PREFIX "TEST-CTF"
#include "debug.h"
#include "ctformatting.h"
#include "globals.h"
#include "options.h"
#include "memory.h"
#include "str.h"
#include "str_buffer.h"
#include "types.h"
#include "ctinfo.h"
}

/************************
* CTFcreateMessageBegin *
************************/

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
    long_message = STRcpy ("A pretty long message that is longer than the header and then some! "
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

/****************************
* CTFcreateMessageContinued *
****************************/

TEST (CTF, testCreateMessageContinuedSingleLineNoWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;

    str_buf *message;
    str_buf *remaining_lines;

    // Base case - no contents means we get nothing back.
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinued ("", remaining_lines);
    EXPECT_STREQ ("", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that the header is fully ignored and newlines are converted to spaces.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "Foo\nBar\nBaz");
    message = CTFcreateMessageContinued ("Header: ", remaining_lines);
    EXPECT_STREQ (" Foo Bar Baz", SBUFgetBuffer(message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageContinuedMultiLineNoWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 0;

    str_buf *message;
    str_buf *remaining_lines;

    // Base case - no contents means we only get a newline back
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinued ("", remaining_lines);
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if they are empty.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "\n\n");
    message = CTFcreateMessageContinued ("Header: ", remaining_lines);
    EXPECT_STREQ ("Header: \nHeader: \nHeader: \n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if it is the only line.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "String contents");
    message = CTFcreateMessageContinued ("H: ", remaining_lines);
    EXPECT_STREQ ("H: String contents\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if there are multiple lines of various types.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "Two empty lines after this one.\n\nAnd then a normal line.");
    message = CTFcreateMessageContinued ("Foo: ", remaining_lines);
    EXPECT_STREQ ("Foo: Two empty lines after this one.\nFoo: \nFoo: And then a normal line.\n", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageContinuedSingleLineWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    
    str_buf *remaining_lines;
    str_buf *message_no_wrap;
    str_buf *message_wrap;
    char *long_message;

    // c++ complains if we use a normal character array, so we use STRcpy to allocate it on the heap
    long_message = STRcpy ("A pretty long message that is longer than the header and then some! "
                           "If wrapping were enabled, it would surely work on this gigantic string.");

    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, long_message);
    message_no_wrap = CTFcreateMessageContinued ("Header: ", remaining_lines);

    global.cti_single_line = true;
    global.cti_message_length = 25;

    // Ensure that line wrapping is ignored when single_line is active:
    // The output should be equal to the output from single line with wrapping explicitly disabled
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, long_message);
    message_wrap = CTFcreateMessageContinued ("Header: ", remaining_lines);
    EXPECT_STREQ (SBUFgetBuffer (message_no_wrap), SBUFgetBuffer(message_wrap));
    SBUFfree (message_no_wrap);
    SBUFfree (message_wrap);

    MEMfree (long_message);
}

TEST (CTF, testCreateMessageContinuedMultiLineWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 30;

    str_buf *remaining_lines;
    str_buf *message;

    // Ensure that we get a trailing enter in the base case
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinued ("", remaining_lines);
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that message shorter than the message length aren't wrapped
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "A %s.", "string");
    message = CTFcreateMessageContinued ("Header: ", remaining_lines);
    EXPECT_STREQ ("Header: A string.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that newlines reset the wrap location.
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "A few characters\n 123456789 123456789 123456789 Start of line.");
    message = CTFcreateMessageContinued ("", remaining_lines);
    EXPECT_STREQ ("A few characters\n 123456789 123456789 123456789\nStart of line.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that multiple wraps are done correctly, even with headers.
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "56789 123456789 123456789 Start of line.9 123456789 123456789");
    message = CTFcreateMessageContinued ("012: ", remaining_lines);
    EXPECT_STREQ ("012: 56789 123456789 123456789\n012: Start of line.9 123456789\n012: 123456789\n", 
                  SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageEnd)
{
    str_buf *message;

    // Ensure a new line is returned when cti_single_line is enabled.
    global.cti_single_line = true;

    message = CTFcreateMessageEnd ();
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure an empty string buffer is returned when cti_single_line is disabled.
    global.cti_single_line = false;
    message = CTFcreateMessageEnd ();
    EXPECT_STREQ ("", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageSingleLineNoWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;

    str_buf *message;

    // Ensure that all newlines are converted to spaces except the final newline
    message = CTFcreateMessage ("First header: ", "Unused header!", "All\nnewlines\nbecome\nspaces.");
    EXPECT_STREQ ("First header: All newlines become spaces.\n", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageMultilineNoWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 0;

    str_buf *message;

    // Ensure that the headers are properly printed after each newline
    message = CTFcreateMessage ("First header: ", "Multiline header!", 
                                "After each\nnewline, we expect a\nmultiline header.");
    EXPECT_STREQ ("First header: After each\nMultiline header!newline, we expect a\nMultiline header!multiline header.\n",
                  SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageSingleLineWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 30;

    str_buf *message;

    // Ensure that line wrapping is completely ignored while single line is active
    message = CTFcreateMessage ("Header1: ", "Unused: ",
                                "0123456789 123456789 123456789 123456789\n123456789");
    EXPECT_STREQ ("Header1: 0123456789 123456789 123456789 123456789 123456789\n", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageMultiLineWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 30;
    
    str_buf *message;
    
    // Ensure that headers of varying lengths, smaller than cti_message_length - 20, the 
    // global message length of 30 is still respected
    message = CTFcreateMessage ("Tiny", "Longer",
                                " second space wraps ---->   second space wraps -->   Pretty neat.");
    EXPECT_STREQ ("Tiny second space wraps ----> \nLonger second space wraps --> \nLonger Pretty neat.\n",
                  SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that if one of the headers is larger than cti_message_length - 20, it
    // uses a line length of header_length + 20
    message = CTFcreateMessage ("20 character header:", "thirty character header:::::::",
                                " 123456789 123456789  123456789 123456789 123456789");
    EXPECT_STREQ ("20 character header: 123456789 123456789\n"
                  "thirty character header::::::: 123456789 123456789\n"
                  "thirty character header:::::::123456789\n",
                  SBUFgetBuffer (message));
    SBUFfree (message);
}


TEST (CTF, testCreateMessageLoc)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;

    str_buf *message;

    // Ensure that @'s are converted to spaces.
    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo %s\nbaz", "bar");
    EXPECT_STREQ ("Error: foo bar baz\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that @'s are converted to newlines.
    global.cti_single_line = false;
    global.cti_message_length = 0;

    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo %s\nbaz", "bar");
    EXPECT_STREQ ("Error: foo bar\n  baz\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that the location is properly converted into the header
    message = CTFcreateMessageLoc (((struct location) {.fname = "testfile", .line = 3, .col = 87}),
                                   "Warning", "%s", "Message.");
    EXPECT_STREQ ("testfile:3:87: Warning: Message.\n", SBUFgetBuffer (message));
    SBUFfree (message);
}