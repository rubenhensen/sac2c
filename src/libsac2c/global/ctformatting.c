/*
 * @file
 *
 * This file provides the interface for formatting any kind of output during
 * compilation. Output is guaranteed to respect the cti compiler arguments.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "ctinfo.h"

#define DBUG_PREFIX "CTF"
#include "debug.h"

#include "str.h"
#include "str_buffer.h"
#include "globals.h"
#include "free.h"
#include "namespaces.h"
#include "tree_basic.h"
#include "math_utils.h"

#include "cppcompat.h"
#undef exit

/** <!--********************************************************************-->
 *
 * @fn void CTFcheckHeaderConsistency ( char *header)
 *
 *   @brief  Checks whether the provided header is valid to be used as the
 *           global.cti_header_format or global.cti_multiline_format.
 * 
 *           If the header is valid, the function returns void.
 *           If the header is invalid, the program gracefully aborts using the 
 *           default global header and multiline format.
 * 
 *   @param  header The header to check.
 ******************************************************************************/
void
CTFcheckHeaderConsistency (char *header)
{
    size_t argument_count;
    size_t index;
    char *error_msg;

    DBUG_ENTER ();

    argument_count = 0;
    index = 0;
    error_msg = NULL;

    while (header[index] != '\0') {
        if (header[index] == '%') {
            if (header[index + 1] == '%') {
                index += 2;
                continue;
            }

            if (header[index + 1] == 's') {
                index += 2;
                argument_count += 1;
                continue;
            }

            if (STReqn (&header[index +1], ".0s", 3)) {
                index += 4;
                argument_count += 1;
                continue;
            }

            error_msg = STRformat ("Supplied header format \"%s\" is invalid.\n"
                                   "A %% must be followed by another '%%', an 's', or '.0s'.",
                                   header);
            break;
        }
        index++;
    }

    if (argument_count != 1 && error_msg == NULL) { // Don't override the first message
        error_msg = STRformat ("Supplied header format \"%s\" is invalid.\n"
                               "The substring '%%s' or '%%.0s' should occur exactly once but occurred %zu times.",
                               header, argument_count);
    }

    if (error_msg != NULL) {
        // Restore default formats
        global.cti_header_format = CTIgetDefaultHeaderFormat ();
        global.cti_multi_line_format = CTIgetDefaultMultiLineFormat ();
        // Change default formats based on the -cti-single-line option
        if (global.cti_single_line) {
            global.cti_header_format = STRsubstTokend (global.cti_header_format, "@", " ");
            global.cti_multi_line_format = STRsubstTokend (global.cti_multi_line_format, "@", " ");
        } else {
            global.cti_header_format = STRsubstTokend (global.cti_header_format, "@", "\n");
            global.cti_multi_line_format = STRsubstTokend (global.cti_multi_line_format, "@", "\n");
        }
        CTIabort (EMPTY_LOC, "%s", error_msg);
    }
}


/** <!--********************************************************************-->
 *
 * @fn void InsertWrapLocations( char *string, size_t header_length, bool return_at_newline)
 *
 *   @brief  Processes the string to replace tabs with spaces, replace spaces with newlines
 *           to wrap at word boundaries if global.cti_line_length != 0.
 * 
 *           The minimum line length is max (header_length + 20, global.cti_message_length)
 *           to make sure the header and some words can always be displayed.
 *           
 *           For all characters until the newline, tabs are replaced with spaces.
 *           If the global.cti_single_line is enabled, newlines are also replaced with spaces.
 * 
 *           If return_at_newline is enabled and global.cti_single_line is disabled, 
 *           the function returns after the first newline.
 * 
 *   @param  buffer            The string to find the wrap location in.
 *                             The string should NOT include the header.
 *   @param  header_length     The length of the header, to compute the line length
 *                             that detemrines when to insert an enter. 
 *   @param  return_at_newline Whether to return at the first newline.
 *                             This setting is ignored when single_line is active.
 *                         
 ******************************************************************************/
static void
InsertWrapLocations (char *buffer, size_t header_length, bool return_at_newline)
{
    size_t line_length, index, column, last_space;
    bool space_found;

    DBUG_ENTER ();

    line_length = MATH_MAX (header_length + 20, (size_t) global.cti_message_length);
    index = 0;
    last_space = 0;
    column = header_length; // Start each line directly after the header.
    space_found = false;

    while (buffer[index] != '\0') {

        if (buffer[index] == '\t') {
            buffer[index] = ' ';
        }

        if (buffer[index] == ' ') {
            last_space = index;
            space_found = true;
        }

        if (global.cti_single_line) {
            if (buffer[index] == '\n') {
                buffer[index] = ' ';
            }
        } else {
            // Return at a newline if the parameter demands it.
            // Note that continuing after the newline leads to problems if the header for the
            // first line and the header for the second line have different lengths.
            if (buffer[index] == '\n') {
                if (return_at_newline) {
                    DBUG_RETURN ();
                } else {    
                    // Reset the current length to account for the header after the newline
                    column = header_length - 1;
                    space_found = false;
                }
            } else if (global.cti_message_length != 0 && column >= line_length && space_found ) {
                // If line wrapping is enabled, line length is exceeded, and a space was found,
                // the most recent space becomes a newline.
                buffer[last_space] = '\n';

                if (return_at_newline) {
                    DBUG_RETURN ();
                }

                // Reset the current length to account for the header and the 
                // characters that have already been traversed since the last space, that have
                // now become a newline.
                column = header_length -1 + (index - last_space);
                space_found = false;
            }
        }
        index++;
        column++;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *Loc2buf( const struct location loc)
 *
 *   @brief  Produces a GNU format string representing the location.
 *           When NULL is given or the location is invalid, the buffer will be empty.
 *
 *   @param  loc A node representing the location of the error.
 *               If no location is available or appropriate, NULL should be supplied.
 * 
 *   @return A str_buf containing a string representation of the location.
 *
 ******************************************************************************/
static str_buf *
Loc2buf (const struct location loc) 
{
    str_buf *buf;
    
    DBUG_ENTER ();

    buf = SBUFcreate (0);

    if (loc.fname == NULL) {
        DBUG_RETURN (buf);
    }

    buf = SBUFcreate (0);
    if (loc.line != 0) {
        if (loc.col != 0) {
            SBUFprintf (buf, "%s:%zu:%zu: ", loc.fname, loc.line, loc.col);
        } else {
            SBUFprintf (buf, "%s:%zu: ", loc.fname, loc.line);
        }
    } else {
        SBUFprintf (buf, "%s: ", loc.fname);
    }
    DBUG_RETURN (buf);
}

/** <!--********************************************************************-->
 * @fn str_buf *CTFvcreateMessageBegin( str_buf *header, const char *format, 
 *                                     va_list arg_p)
 * 
 *   @brief  Creates and formats the first line of the message and separates it 
 *           from the remaining lines.
 *   
 *   @param  header The header for the first line. Stores the entire first line
 *                  after calling this function.
 *   @param  format The format on which to apply arg_p to generate the message.
 *   @param  arg_p  The arguments to apply to the format to generate the message.
 * 
 *   @return Returns the first line through the header parameter, and a str_buf of 
 *           the remaining lines as the return value.
 ******************************************************************************/
str_buf *
CTFvcreateMessageBegin (str_buf *header, const char *format, va_list arg_p)
{
    str_buf *remaining_lines;
    size_t header_length;
    char *index_p;
    size_t index;

    DBUG_ENTER ();

    header_length = SBUFlen (header);
    SBUFvprintf (header, format, arg_p);

    // Don't pass the header, just the format, message.
    InsertWrapLocations (&SBUFgetBuffer (header)[header_length], header_length, true);
    
    index_p = strchr (SBUFgetBuffer (header), '\n'); // Get the index of the first newline
    // Underflow if index_p is NULL, but we don't use index if index_p is NULL.
    index = (size_t) (index_p - SBUFgetBuffer(header));

    remaining_lines = SBUFcreate (0);

    if (!global.cti_single_line) {
        // Ensure there is always a newline, even if the entire message fits on the first line.
        if (index_p == NULL) {
            SBUFprint (header, "\n");
        } else {
            // Copy the remaining lines into its own buffer (\n belongs to the first line)
            SBUFprint (remaining_lines, &SBUFgetBuffer (header)[index + 1]);
            SBUFtruncate (header, index + 1); // Remove the remaining lines from the header
        }
    }

    // Return the first line through the header parameter
    // Return the remaining lines:
    DBUG_RETURN (remaining_lines);
}

/** <!--********************************************************************-->
 * @fn str_buf *CTFcreateMessageBegin( str_buf *header, const char *format, ...)
 * 
 *   @brief  Creates and formats the first line of the message and separates it 
 *           from the remaining lines.
 *   
 *   @param  header The header for the first line. Stores the entire first line
 *                  after calling this function.
 *   @param  format The format on which to apply extra arguments to generate 
 *                  the message.
 * 
 *   @return Returns the first line through the header parameter, and a str_buf of 
 *           the remaining lines as the return value.
 ******************************************************************************/
str_buf *
CTFcreateMessageBegin (str_buf *header, const char *format, ...)
{
    va_list arg_p;
    str_buf *remaining_lines;

    DBUG_ENTER ();

    va_start (arg_p, format);
    remaining_lines = CTFvcreateMessageBegin (header, format, arg_p);
    va_end (arg_p);

    DBUG_RETURN (remaining_lines);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *CTFcreateMessageContinued( const char *multiline_header, 
 *                                         str_buf *remaining_lines)
 * 
 *   @brief  Formats the remaining lines of a message.
 *           Frees remaining_lines.
 * 
 *   @param  multiline_header The header to be inserted at start of each line when 
 *                           cti_single_line is disabled.
 *   @param  remaining_lines  The remaining lines of a message that have to be formatted.
 *                           It is not required for this to be all of the remaining lines.
 *                           Multiple calls to CTFcreateMessageContinued is acceptable.
 *   @return A formatted version of the the remaining lines.
 * 
 ******************************************************************************/
str_buf *
CTFcreateMessageContinued (const char *multiline_header, str_buf *remaining_lines)
{
    str_buf *message;
    size_t header_len;

    DBUG_ENTER ();

    header_len = STRlen (multiline_header);
    InsertWrapLocations (SBUFgetBuffer (remaining_lines), header_len, false);

    message = SBUFcreate (SBUFlen (remaining_lines));

    if (global.cti_single_line) {
        if (!SBUFisEmpty (remaining_lines)) {
            // Prepend the 'second line' (first line of remaining_lines) with a space so 
            // the end text of the first and second line have a gap between them.
            SBUFprint (message, " ");
        }
    } else {
        // Before we add remaining_lines, we have to manually add the first header
        // because remaining_lines doesn't start with a newline
        SBUFprint (message, multiline_header);

        SBUFinsertAfterToken (remaining_lines, "\n", multiline_header);
    }
    
    SBUFprint (message, SBUFgetBuffer (remaining_lines));
    
    if (!global.cti_single_line) {
        SBUFprint (message, "\n");
    }

    SBUFfree (remaining_lines);
    DBUG_RETURN (message);
}


/** <!--********************************************************************-->
 *
 * @fn str_buf *CTFcreateMessageEnd( void)
 * 
 *   @brief  Formats the remaining lines of a message.
 *           Frees remaining_lines.
 * 
 *   @param  multiline_header The header to be inserted at start of each line when 
 *                           cti_single_line is disabled.
 *   @param  remaining_lines  The remaining lines of a message that have to be formatted.
 *                           It is not required for this to be all of the remaining lines.
 *                           Multiple calls to CTFcreateMessageContinued is acceptable.
 *   @return A formatted version of the the remaining lines.
 * 
 ******************************************************************************/
str_buf *
CTFcreateMessageEnd (void)
{
    str_buf *end;

    DBUG_ENTER ();

    end = SBUFcreate (1);
    
    if (global.cti_single_line) {
        SBUFprint (end, "\n");
    }

    DBUG_RETURN (end);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFvcreateMessage( str_buf *first_line_header, 
 *                                const char *multiline_header, 
 *                                const char *format, va_list arg_p)
 * 
 *   @brief  Creates a message based on the given headers, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  first_line_header The header for the first line of the message.
 *   @param  multiline_header  The header for subsequent lines of the message.
 *   @param  format            The format on which to apply the arguments.
 *   @param  arg_p             The arguments to apply onto the format.
 * 
 *   @return A str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
CTFvcreateMessage (const char *first_line_header, const char *multiline_header, 
                  const char *format, va_list arg_p)
{
    str_buf *message;
    str_buf *remaining_lines;
    str_buf *message_end;
    
    DBUG_ENTER ();

    message = SBUFcreate (0);
    SBUFprint (message, first_line_header);

    // message gets changed to include the entire first line, not just the header
    remaining_lines = CTFvcreateMessageBegin (message, format, arg_p);
    remaining_lines = CTFcreateMessageContinued (multiline_header, remaining_lines);
    message_end = CTFcreateMessageEnd ();
    SBUFprint (message, SBUFgetBuffer (remaining_lines));
    SBUFprint (message, SBUFgetBuffer (message_end));

    SBUFfree (remaining_lines);
    SBUFfree (message_end);
    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFcreateMessage( str_buf *first_line_header, 
 *                                const char *multiline_header, 
 *                                const char *format, ...)
 * 
 *   @brief  Creates a message based on the given headers, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  first_line_header The header for the first line of the message.
 *   @param  multiline_header  The header for subsequent lines of the message.
 *   @param  format            The format on which to apply extra arguments to 
 *                             generate the message.
 * 
 *   @return A str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
CTFcreateMessage (const char *first_line_header, const char *multiline_header, 
                  const char *format, ...)
{
    va_list arg_p;
    str_buf *message;

    DBUG_ENTER ();
    
    va_start (arg_p, format);
    message = CTFvcreateMessage (first_line_header, multiline_header, format, arg_p);
    va_end (arg_p);

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFvcreateMessageLoc ( struct location loc,
 *                                    const char *message_header,
 *                                    const char *format, va_list arg_p)
 * 
 *   @brief  Creates a message based on the given location, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  loc    The location from which to generate headers for the message.
 *   @param  format The format on which to apply the arguments.
 *   @param  arg_p  The arguments to apply onto the format.
 * 
 *   @return a str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
CTFvcreateMessageLoc (struct location loc, const char *message_header,
                     const char *format, va_list arg_p)
{
    str_buf *base_header;
    str_buf *first_line_header;
    str_buf *multiline_header;
    str_buf *message;
    
    DBUG_ENTER ();

    // First, we construct the 'base' gnu format message header

    base_header = Loc2buf (loc);
    SBUFprint (base_header, message_header);

    // The base header is used to construct the header for the first and subsequent
    // lines using the format string specified in the globals that the user has control over.
    // Naturally, this could fail with bogus input, but this checked against shortly after the globals
    // are defined.

    first_line_header = SBUFcreate (0);
    SBUFprintf (first_line_header, global.cti_header_format, SBUFgetBuffer (base_header));

    multiline_header = SBUFcreate (0);
    SBUFprintf (multiline_header, global.cti_multi_line_format, SBUFgetBuffer (base_header));

    message = CTFvcreateMessage (SBUFgetBuffer (first_line_header), SBUFgetBuffer (multiline_header), 
                                format, arg_p);

    SBUFfree (base_header);
    SBUFfree (first_line_header);
    SBUFfree (multiline_header);
    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFcreateMessageLoc ( struct location loc,
 *                                    const char *message_header,
 *                                    const char *format, ...)
 * 
 *   @brief  Creates a message based on the given location, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  loc    The location from which to generate headers for the message.
 *   @param  format The format on which to apply extra arguments to generate
 *                  the message.
 * 
 *   @return a str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
CTFcreateMessageLoc (struct location loc, const char *message_header,
                     const char *format, ...)
{
    va_list arg_p;
    str_buf *message;

    DBUG_ENTER ();

    va_start (arg_p, format);
    message = CTFvcreateMessageLoc (loc, message_header, format, arg_p);
    va_end (arg_p);

    DBUG_RETURN (message);
}

#undef DBUG_PREFIX
