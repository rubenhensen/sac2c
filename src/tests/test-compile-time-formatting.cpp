#include <sys/types.h>
#include <dirent.h>

#include "gtest/gtest.h"
#include "base-test-environment.h" // All unit test files need to import this!
testing::Environment* base_test_env = testing::AddGlobalTestEnvironment(new BaseEnvironment);

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

// A function to call CTFcreateMessageContinued more conveniently during the tests
// without exposing additional API.
static str_buf *
CTFcreateMessageContinuedRaw (const char *multiline_header, str_buf *remaining_lines)
{
    str_buf *primary_header;
    str_buf *message;
    
    primary_header = SBUFcreate (0);
    SBUFfree (CTFcreateMessageBegin (&primary_header, multiline_header, "%s", ""));
    message = CTFcreateMessageContinued (remaining_lines);
    SBUFfree (CTFcreateMessageEnd());

    return message;
}

/************************
* CTFcreateMessageBegin *
************************/

TEST (CTF, testCreateMessageBeginSingleLineNoWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0; // disables line wrapping
    CTFinitialize ();

    str_buf *header;
    str_buf *message;
    char *format;

    // Ensure that we don't get a trailing enter in the base case
    header = SBUFcreate (0);
    fprintf (stderr, "%s\n", SBUFgetBuffer (header));
    #pragma GCC diagnostic ignored "-Wformat-zero-length"
    message = CTFcreateMessageBegin (&header, "", "");
    SBUFfree (CTFcreateMessageEnd ()); // Free the multiline header that CTFcreateMessageBegin allocates
    #pragma GCC diagnostic warning "-Wformat-zero-length"
    EXPECT_TRUE (SBUFisEmpty (message));
    EXPECT_TRUE (header == NULL);
    SBUFfree (message);

    // Ensure that headerless normal messages are printed correctly
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", "A %s yet %s test.", "normal", "fabulous");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("A normal yet fabulous test.", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that multiline messages are still printed on one line
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", "A\nmessage\ncontaining%sspaces", "\nmany\n");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("A message containing many spaces", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that if a non-empty header is given, it is prepended to the message.
    header = SBUFcreate (0);
    SBUFprint (header, "Some header: ");
    message = CTFcreateMessageBegin (&header, "", "A %s\nmessage", "neat");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("Some header: A neat message", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageBeginMultiLineNoWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 0; // disables line wrapping
    CTFinitialize ();

    str_buf *header;
    str_buf *message;

    // Ensure that we still get a newline if we give empty inputs
    header = SBUFcreate (0);
    #pragma GCC diagnostic ignored "-Wformat-zero-length"
    message = CTFcreateMessageBegin (&header, "", "");
    SBUFfree (CTFcreateMessageEnd ());
    #pragma GCC diagnostic warning "-Wformat-zero-length"
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that normal messages are working correctly
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", "A %s test.", "normal");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("A normal test.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that the first_line_header is prepended only to the first line
    // and that the multi_line_header is prepended to the other lines
    header = SBUFcreate (0);
    SBUFprint (header, "Interesting header: ");
    message = CTFcreateMessageBegin (&header, "  ", "A\nMultiline\nTest");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("Interesting header: A\n  Multiline\n  Test\n", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageBeginSingleLineWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    CTFinitialize ();
    
    str_buf *header;
    str_buf *message_no_wrap;
    str_buf *message_wrap;
    char *long_message;

    // c++ complains if we use a normal character array, so we use STRcpy to allocate it on the heap
    long_message = STRcpy ("A pretty long message that is longer than the header and then some! "
                           "If wrapping were enabled, it would surely work on this gigantic string.");

    header = SBUFcreate (0);
    message_no_wrap = CTFcreateMessageBegin (&header, "", "%s", long_message);
    SBUFfree (CTFcreateMessageEnd ());

    global.cti_single_line = true;
    global.cti_message_length = 25;
    CTFinitialize ();

    // Ensure that line wrapping is ignored when single_line is active:
    // The output should be equal to the output from single line with wrapping explicitly disabled
    header = SBUFcreate (0);
    message_wrap = CTFcreateMessageBegin (&header, "", "%s", long_message);
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ (SBUFgetBuffer (message_no_wrap), SBUFgetBuffer(message_wrap));
    SBUFfree (message_no_wrap);
    SBUFfree (message_wrap);

    MEMfree (long_message);
}

TEST (CTF, testCreateMessageBeginMultiLineWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 30;
    CTFinitialize ();

    str_buf *header;
    str_buf *message;

    // Ensure that we get a trailing enter in the base case
    header = SBUFcreate (0);
    #pragma GCC diagnostic ignored "-Wformat-zero-length"
    message = CTFcreateMessageBegin (&header, "", "");
    SBUFfree (CTFcreateMessageEnd ());
    #pragma GCC diagnostic warning "-Wformat-zero-length"
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that message shorter than the message length aren't wrapped.
    header = SBUFcreate (0);
    SBUFprint (header, "Header: ");
    message = CTFcreateMessageBegin (&header, "", "A %s.", "string");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("Header: A string.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that messages longer than the message length without any wrapping locations aren't wrapped.
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", "0123456789012345678901234567890123456789");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("0123456789012345678901234567890123456789\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that messages longer than the message length with a valid wrapping location
    // are wrappped at the last possible location.
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", " 123456789 123456789 123456789  23456789");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ (" 123456789 123456789 123456789\n 23456789\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that tabs are converted to spaces and potentially used for wrapping
    header = SBUFcreate (0);
    message = CTFcreateMessageBegin (&header, "", "\t123456789\t123456789\t123456789\t123456789");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ (" 123456789 123456789 123456789\n123456789\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that wrapping takes into account non-empty headers
    header = SBUFcreate (0);
    SBUFprint (header, "Header! ");
    message = CTFcreateMessageBegin (&header, "MultiHeader! ", "123456789 123456789 123456789 123456789");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("Header! 123456789 123456789\nMultiHeader! 123456789 123456789\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // If message length is smaller than the header length + 20 characters, line wrapping will instead
    // be done at header length + 20 characters
    global.cti_message_length = 1;
    header = SBUFcreate (0);
    SBUFprint (header, "Ten chars:");
    message = CTFcreateMessageBegin (&header, "", " 123456789 123456789  ");
    SBUFfree (CTFcreateMessageEnd ());
    EXPECT_STREQ ("Ten chars: 123456789 123456789\n \n", SBUFgetBuffer (message));
    SBUFfree (message);
}

/****************************
* CTFcreateMessageContinuedRaw *
****************************/

TEST (CTF, testCreateMessageContinuedSingleLineNoWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    CTFinitialize ();

    str_buf *message;
    str_buf *remaining_lines;

    // Base case - no contents means we get nothing back.
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinuedRaw ("", remaining_lines);
    EXPECT_STREQ ("", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that the header is fully ignored and newlines are converted to spaces.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "Foo\nBar\nBaz");
    message = CTFcreateMessageContinuedRaw ("Header: ", remaining_lines);
    EXPECT_STREQ (" Foo Bar Baz", SBUFgetBuffer(message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageContinuedMultiLineNoWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 0;
    CTFinitialize ();

    str_buf *message;
    str_buf *remaining_lines;

    // Base case - no contents means we only get a newline back
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinuedRaw ("", remaining_lines);
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if they are empty.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "\n\n");
    message = CTFcreateMessageContinuedRaw ("Header: ", remaining_lines);
    EXPECT_STREQ ("Header: \nHeader: \nHeader: \n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if it is the only line.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "String contents");
    message = CTFcreateMessageContinuedRaw ("H: ", remaining_lines);
    EXPECT_STREQ ("H: String contents\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that all lines are prefixed with the header, even if there are multiple lines of various types.
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, "Two empty lines after this one.\n\nAnd then a normal line.");
    message = CTFcreateMessageContinuedRaw ("Foo: ", remaining_lines);
    EXPECT_STREQ ("Foo: Two empty lines after this one.\nFoo: \nFoo: And then a normal line.\n", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageContinuedSingleLineWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    CTFinitialize ();
    
    str_buf *remaining_lines;
    str_buf *message_no_wrap;
    str_buf *message_wrap;
    char *long_message;

    // c++ complains if we use a normal character array, so we use STRcpy to allocate it on the heap
    long_message = STRcpy ("A pretty long message that is longer than the header and then some! "
                           "If wrapping were enabled, it would surely work on this gigantic string.");

    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, long_message);
    message_no_wrap = CTFcreateMessageContinuedRaw ("Header: ", remaining_lines);

    global.cti_single_line = true;
    global.cti_message_length = 25;

    // Ensure that line wrapping is ignored when single_line is active:
    // The output should be equal to the output from single line with wrapping explicitly disabled
    remaining_lines = SBUFcreate (0);
    SBUFprint (remaining_lines, long_message);
    message_wrap = CTFcreateMessageContinuedRaw ("Header: ", remaining_lines);
    EXPECT_STREQ (SBUFgetBuffer (message_no_wrap), SBUFgetBuffer(message_wrap));
    SBUFfree (message_no_wrap);
    SBUFfree (message_wrap);

    MEMfree (long_message);
}

TEST (CTF, testCreateMessageContinuedMultiLineWrapping)
{
    global.cti_single_line = false;
    global.cti_message_length = 30;
    CTFinitialize ();

    str_buf *remaining_lines;
    str_buf *message;

    // Ensure that we get a trailing enter in the base case
    remaining_lines = SBUFcreate (0);
    message = CTFcreateMessageContinuedRaw ("", remaining_lines);
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that message shorter than the message length aren't wrapped
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "A %s.", "string");
    message = CTFcreateMessageContinuedRaw ("Header: ", remaining_lines);
    EXPECT_STREQ ("Header: A string.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that newlines reset the wrap location.
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "A few characters\n 123456789 123456789 123456789 Start of line.");
    message = CTFcreateMessageContinuedRaw ("", remaining_lines);
    EXPECT_STREQ ("A few characters\n 123456789 123456789 123456789\nStart of line.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that multiple wraps are done correctly, even with headers.
    remaining_lines = SBUFcreate (0);
    SBUFprintf (remaining_lines, "56789 123456789 123456789 Start of line.9 123456789 123456789");
    message = CTFcreateMessageContinuedRaw ("012: ", remaining_lines);
    EXPECT_STREQ ("012: 56789 123456789 123456789\n012: Start of line.9 123456789\n012: 123456789\n", 
                  SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageEnd)
{
    str_buf *message;
    str_buf *primary_header;


    // Ensure a new line is returned when cti_single_line is enabled.
    global.cti_single_line = true;
    CTFinitialize ();

    message = CTFcreateMessageEnd ();
    EXPECT_STREQ ("\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure an empty string buffer is returned when cti_single_line is disabled.
    global.cti_single_line = false;
    CTFinitialize ();

    message = CTFcreateMessageEnd ();
    EXPECT_STREQ ("", SBUFgetBuffer (message));
    SBUFfree (message);
}

TEST (CTF, testCreateMessageSingleLineNoWrapping)
{
    global.cti_single_line = true;
    global.cti_message_length = 0;
    CTFinitialize ();

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
    CTFinitialize ();

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
    CTFinitialize ();

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
    CTFinitialize ();
    
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
    MEMfree (global.cti_header_format);
    MEMfree (global.cti_multi_line_format);
    global.cti_header_format = STRcpy ("%s: ");
    global.cti_multi_line_format = STRcpy ("%.0s  ");
    CTFinitialize ();

    str_buf *message;

    // Ensure that newlines are converted to spaces with cti_single_line enabled.
    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo %s\nbaz", "bar");
    EXPECT_STREQ ("Error: foo bar baz\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that newlines stay newlines when cti_single_line is disabled.
    global.cti_single_line = false;
    global.cti_message_length = 0;
    CTFinitialize ();

    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo %s\nbaz", "bar");
    EXPECT_STREQ ("Error: foo bar\n  baz\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that the location is properly converted into the header
    // Also ensure that "Warning" is decapitalized because there is a location before it
    message = CTFcreateMessageLoc (((struct location) {.fname = "testfile", .line = 3, .col = 87}),
                                   "Warning", "%s", "Message.");
    EXPECT_STREQ ("testfile:3:87: warning: Message.\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that *only* @ symbols in the header are converted 
    // to spaces if cti_single_line is enabled.
    global.cti_single_line = true;
    global.cti_message_length = 0;
    MEMfree (global.cti_header_format);
    global.cti_header_format = STRcpy ("%s:@");
    CTFinitialize ();

    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo\nbar");
    EXPECT_STREQ ("Error: foo bar\n", SBUFgetBuffer (message));
    SBUFfree (message);

    // Ensure that *only* @ symbols the first line header are converted to newlines
    // and ensure that @ symbols in the multiline header remain @ symbols.
    global.cti_single_line = false;
    global.cti_message_length = 0;
    MEMfree (global.cti_header_format);
    MEMfree (global.cti_multi_line_format);
    global.cti_header_format = STRcpy ("%s:@First:");
    global.cti_multi_line_format = STRcpy ("%.0sContinued:@");
    CTFinitialize ();

    message = CTFcreateMessageLoc (EMPTY_LOC, "Error", "foo\nbar");
    EXPECT_STREQ ("Error:\nContinued:@First:foo\nContinued:@bar\n", SBUFgetBuffer (message));
    SBUFfree (message);
}
