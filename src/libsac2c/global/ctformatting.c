/*
 * @file
 *
 * This file provides the interface for formatting any kind of output during
 * compilation. Output is guaranteed to respect the cti compiler arguments.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "ctinfo.h"

#define DBUG_PREFIX "CTF"
#include "debug.h"

#include "ctformatting.h" // To have access to the default header macros

#include "str.h"
#include "str_buffer.h"
#include "globals.h"
#include "memory.h"
#include "free.h"
#include "namespaces.h"
#include "tree_basic.h"
#include "math_utils.h"

#include "cppcompat.h"
#undef exit

static char *processed_header_format = NULL;
static char *processed_multi_line_format = NULL;

static bool initialized = false;

static char *current_multi_line_header = NULL;

/** <!--********************************************************************-->
 *
 * @fn void ProcessHeaders( void)
 *
 *   @brief  Copies over the header format globals and substitutes @ and newline 
 *           characters with newlines or spaces based on global.cti_single_line.
 ******************************************************************************/
static void
ProcessHeaders (void)
{
    if (processed_header_format != NULL) {
        MEMfree (processed_header_format);
    }
    if (processed_multi_line_format != NULL) {
        MEMfree (processed_multi_line_format);
    }

    if (global.cti_single_line) {
        // We shouldn't encounter \n in any of the headers, but it's not bad to account for it.
        processed_header_format = STRsubstTokens (global.cti_header_format, 2, "@", " ", "\n", " ");
    } else {
        processed_header_format = STRsubstToken (global.cti_header_format, "@", "\n");
    }
    // Newlines in the multi-line header format lead to stupid results, so @ characters are 
    // never converted to newlines, and newlines are always converted to spaces.
    processed_multi_line_format = STRsubstToken (global.cti_multi_line_format, "\n", " ");
}


/** <!--********************************************************************-->
 *
 * @fn void CTFcheckHeaderConsistency( char *header)
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
        if (header[index] == '%') { // We allow %% to escape %
            if (header[index + 1] == '%') {
                index += 2;
                continue;
            }

            if (header[index + 1] == 's') { // We allow a single occurence of %s
                index += 2;
                argument_count += 1;
                continue;
            }

            if (STReqn (&header[index +1], ".0s", 3)) { // We allow an occurrence of %.0s
                index += 4;
                argument_count += 1;
                continue;
            }

            // A % followed by anything else leads to an error
            error_msg = STRformat ("Supplied header format \"%s\" is invalid.\n"
                                   "A %% must be followed by another '%%', an 's', or '.0s'.",
                                   header);
            break;
        }
        index++;
    }

    // If the previous part didn't lead to errors, we can error here if
    // %s or %.0s occurred more than once.
    if (argument_count != 1 && error_msg == NULL) {
        error_msg = STRformat ("Supplied header format \"%s\" is invalid.\n"
                               "The substring '%%s' or '%%.0s' should occur exactly once but occurred %zu times.",
                               header, argument_count);
    }

    if (error_msg != NULL) {
        // Before we can raise the error, we have to replace the format with something that
        // we know will work: the default error format.
        
        MEMfree (global.cti_header_format);
        MEMfree (global.cti_multi_line_format);
        global.cti_header_format = STRcpy (CTF_DEFAULT_FIRST_LINE_HEADER);
        global.cti_multi_line_format = STRcpy (CTF_DEFAULT_MULTI_LINE_HEADER);
        ProcessHeaders();

        initialized = true; // Avoid initializing again for no reason
        CTIabort (EMPTY_LOC, "%s", error_msg);
    }
}

void
CTFinitialize (void)
{
    // If the headers don't exist, which can happen during unit-tests, we apply the default formats
    if (global.cti_header_format == NULL) {
        global.cti_header_format = STRcpy (CTF_DEFAULT_FIRST_LINE_HEADER);
    }
    if (global.cti_multi_line_format == NULL) {
        global.cti_multi_line_format = STRcpy (CTF_DEFAULT_MULTI_LINE_HEADER);
    }

    // Gracefully aborts if the format is not invalid.
    CTFcheckHeaderConsistency (global.cti_header_format);
    CTFcheckHeaderConsistency (global.cti_multi_line_format);

    ProcessHeaders ();

    // Set initialized before potentially raising warnings to avoid infinite loops
    initialized = true;

    if (global.cti_single_line) {
        // If either of the headers are not the default format and contain newlines,
        // then warn that the newlines are replaced with a spaces.
        if ((!STReq (global.cti_header_format, CTF_DEFAULT_FIRST_LINE_HEADER)
                && strchr (global.cti_header_format, '\n') == NULL)
                || (!STReq (global.cti_multi_line_format, CTF_DEFAULT_MULTI_LINE_HEADER)
                && strchr (global.cti_multi_line_format, '\n') == NULL)) {
            CTIwarn (EMPTY_LOC, "Option -cti-single-line replaces newlines with spaces in the header formats.");
        }

        if (global.cti_message_length != 0) {
            CTIwarn (EMPTY_LOC, "Option -cti-single-line implies option -cti-message-length 0.\n"
                    "Option -cti-message-length will be ignored.");
            global.cti_message_length = 0;
        }

        if (!STReq (global.cti_multi_line_format, CTF_DEFAULT_MULTI_LINE_HEADER)) {
            CTIwarn (EMPTY_LOC, "Option -cti-single-line makes option -cti-multi-line-format redundant.");
        }
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
 *   @param  loc The location.
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
 *
 * @fn str_buf *CreateAndLoadGNUformatHeader( const struct location loc, 
 *                                            const char *message_header)
 *
 *   @brief  Produces a GNU format header of the location and message header.
 *           Formats that header using the processed header formats.
 *           It loads the multiline header into the current_multi_line_header variable,
 *           and returns the first line header.
 *
 *   @param  loc            The location of the message.
 *                          If no location is available or appropriate, NULL should be supplied.
 *   @param  message_header The header of the message. Should be of the form "[A-Z][a-z]*".
 *                          Examples are "Error", "Warn" and "Abort"
 * 
 *   @return A str_buf containing the first line header constructed from loc 
 *           and message_header in the GNU format.
 *
 ******************************************************************************/
static str_buf *
CreateAndLoadGNUformatHeader (const struct location loc, const char *message_header)
{
    str_buf *base_header;
    str_buf *first_line_header;
    str_buf *multi_line_header;
    size_t len;

    DBUG_ENTER ();

    // If this assert triggers, a call to CTFcreateMessageEnd is missing.
    DBUG_ASSERT (current_multi_line_header == NULL,
                 "Expected a deallocated current_multi_line_header, but it was allocated, "
                 "leading to memory leaks.");
    
    base_header = Loc2buf (loc);

    len = SBUFlen (base_header);
    SBUFprint (base_header, message_header);
    // If the base header is not empty, the first character of the message header is
    // made lowercase.
    if (len != 0) {
        SBUFgetBuffer (base_header)[len] = (char) tolower (SBUFgetBuffer (base_header)[len]);
    }

    // The base header is used to construct the header for the first and subsequent
    // lines using the processed format string.
    first_line_header = SBUFcreate (0);
    SBUFprintf (first_line_header, processed_header_format, SBUFgetBuffer (base_header));

    // Load the multiline header into the static variable current_multi_line_header.
    // This prevents having to store the location/header on the calling side when invoking
    // functions like CTIerrorContinued.
    multi_line_header = SBUFcreate (0);
    SBUFprintf (multi_line_header, processed_multi_line_format, SBUFgetBuffer (base_header));
    current_multi_line_header = SBUF2strAndFree (&multi_line_header);

    SBUFfree (base_header);
    DBUG_RETURN (first_line_header);
}

/** <!--********************************************************************-->
 * @fn str_buf *vCreateMessageBegin( str_buf **header, const char *format, 
 *                                   va_list arg_p)
 * 
 *   @brief  Formats the message based on the given header and format string.
 *   
 *   @param  header The header for the first line.
 *   @param  format The format on which to apply arg_p to generate the message.
 *   @param  arg_p  The arguments to apply to the format to generate the message.
 * 
 *   @return Returns the formatted message.
 ******************************************************************************/
str_buf *
vCreateMessageBegin (str_buf **header, const char *format, va_list arg_p)
{
    str_buf *message;
    str_buf *remaining_lines;
    size_t header_length;
    char *index_p;
    size_t index;

    DBUG_ENTER ();

    header_length = SBUFlen (*header);
    
    // Rename header to message - "deallocates" header
    message = *header; *header = NULL;

    SBUFvprintf (message, format, arg_p);

    // Don't pass the header part of message, only the body of the message should be formatted.
    InsertWrapLocations (&SBUFgetBuffer (message)[header_length], header_length, true);

    index_p = strchr (SBUFgetBuffer (message), '\n'); // Get the index of the first newline
    // Underflow if index_p is NULL, but we don't use index if index_p is NULL.
    index = (size_t) (index_p - SBUFgetBuffer(message));

    remaining_lines = SBUFcreate (0);

    if (!global.cti_single_line) {
        // Ensure there is always a newline, even if the entire message fits on the first line.
        if (index_p == NULL) {
            SBUFprint (message, "\n");
        } else {
            // Copy the remaining lines into its own buffer (\n belongs to the first line)
            SBUFprint (remaining_lines, &SBUFgetBuffer (message)[index + 1]);
            SBUFtruncate (message, index + 1); // Remove the remaining lines from the header
        }
    }


    // We only parse the remaining lines if they are not empty.
    // There is a nuance here to take into account:
    // If we manually call CTFcreateMessageContinued with an empty line, we want that
    // empty line to be printed with a header.
    // If we call CTFcreateMessage, we want repeating newlines "\n\n" to also be respected.
    // However, if the call to this function doesn't contain any newline, but would still 
    // process the remanining lines, the result would be something of the following form:
    // concat(first_line_header, message, multi_line_header, newline)
    // Clearly, we don't want the multi_line header there.
    if (!SBUFisEmpty (remaining_lines)) {
        remaining_lines = CTFcreateMessageContinued (remaining_lines);
        SBUFprint (message, SBUFgetBuffer (remaining_lines));
    }

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * @fn str_buf *CTFCreateMessageBegin( str_buf **header, 
 *                                     const char *multi_line_header,
 *                                     const char *format, va_list arg_p)
 * 
 *   @brief  Formats the message based on the given header and format string.
 *   
 *   @param  header            The header for the first line.
 *   @param  multi_line_header The header for subsequent lines.
 *   @param  format            The format on which to apply arg_p to generate the message.
 *   @param  arg_p             The arguments to apply to the format to generate the message.
 * 
 *   @return Returns the formatted message.
 ******************************************************************************/
str_buf *
CTFcreateMessageBegin (str_buf **header, const char *multi_line_header, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    // If this assert triggers, a call to CTFcreateMessageEnd is missing.
    DBUG_ASSERT (current_multi_line_header == NULL,
                 "Expected a deallocated current_multi_line_header, but it was allocated, "
                 "leading to memory leaks.");
    current_multi_line_header = STRcpy (multi_line_header);

    va_start (arg_p, format);
    message = vCreateMessageBegin (header, format, arg_p); // frees header
    va_end (arg_p);

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * @fn str_buf *CTFvCreateMessageBeginLoc( const struct location loc, 
 *                                         const char *message_header,
 *                                         const char *format, va_list arg_p)
 * 
 *   @brief  Formats the message based on the given location, message header and format string.
 *   
 *   @param  location       The location to construct the header from.
 *   @param  message_header The header of the message. Should be of the form "[A-Z][a-z]*".
 *                          Examples are "Error", "Warn" and "Abort"
 *   @param  format         The format on which to apply arg_p to generate the message.
 *   @param  arg_p          The arguments to apply to the format to generate the message.
 * 
 *   @return Returns the formatted message.
 ******************************************************************************/
str_buf *
CTFvCreateMessageBeginLoc (const struct location loc, const char *message_header, 
                           const char *format, va_list arg_p)
{
    str_buf *header;
    str_buf *message;
    
    DBUG_ENTER ();

    header = CreateAndLoadGNUformatHeader (loc, message_header);
    message = vCreateMessageBegin (&header, format, arg_p); // frees header

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *CTFcreateMessageContinued( str_buf *remaining_lines)
 * 
 *   @brief  Formats the remaining lines of a message.
 *           Frees remaining_lines.
 * 
 *   @param  remaining_lines The remaining lines of a message that have to be formatted.
 *                           It is not required for this to be all of the remaining lines.
 *                           Multiple calls to CTFcreateMessageContinued are acceptable.
 *   @return A formatted version of the the remaining lines.
 * 
 ******************************************************************************/
str_buf *
CTFcreateMessageContinued (str_buf *remaining_lines)
{
    str_buf *message;
    size_t header_len;

    DBUG_ENTER ();
    
    DBUG_ASSERT (current_multi_line_header != NULL, 
                 "Attempted to continue a message but there was no message to continue.");

    header_len = STRlen (current_multi_line_header);
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
        SBUFprint (message, current_multi_line_header);

        SBUFinsertAfterToken (remaining_lines, "\n", current_multi_line_header);
    }
    
    SBUFprint (message, SBUFgetBuffer (remaining_lines));
    
    if (!global.cti_single_line) {
        SBUFprint (message, "\n");
    }

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *CTFcreateMessageEnd( void)
 * 
 *   @brief  Ends the message.
 *           Frees the static variable current_multi_line_header and sets it to NULL.
 * 
 *   @return A str_buf containing "\n" if cti_single_line is enabled, 
 *           an empty str_buf otherwise.
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

    current_multi_line_header = MEMfree (current_multi_line_header);
    DBUG_RETURN (end);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *vCreateMessage( str_buf *first_line_header,
 *                              const char *format, va_list arg_p)
 * 
 *   @brief  Creates a message based on the first_line header, the static
 *           variable current_multi_line_header, and on the format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  first_line_header The header for the first line of the message.
 *   @param  format            The format on which to apply the arguments.
 *   @param  arg_p             The arguments to apply onto the format.
 * 
 *   @return A str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
vCreateMessage (const char *first_line_header, const char *format, va_list arg_p)
{
    str_buf *header;
    str_buf *message;
    str_buf *message_end;
    
    DBUG_ENTER ();

    DBUG_ASSERT (current_multi_line_header != NULL, 
                 "Attempted to create a message but not all information was present.");

    header = SBUFcreate (0);
    SBUFprint (header, first_line_header);

    message = vCreateMessageBegin (&header, format, arg_p); // frees header

    message_end = CTFcreateMessageEnd ();
    SBUFprint (message, SBUFgetBuffer (message_end));

    SBUFfree (message_end);
    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFvCreateMessage( str_buf *first_line_header,
 *                                 const char *multiline_header,
 *                                 const char *format, va_list arg_p)
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
CTFvCreateMessage (const char *first_line_header, const char *multiline_header, 
                   const char *format, va_list arg_p)
{
    DBUG_ENTER ();

    // current_multi_line_header is always NULL at this point, no need to deallocate it.
    current_multi_line_header = STRcpy (multiline_header);
    DBUG_RETURN (vCreateMessage (first_line_header, format, arg_p));
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
    message = CTFvCreateMessage (first_line_header, multiline_header, format, arg_p);
    va_end (arg_p);

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFvcreateMessageLoc( struct location loc,
 *                                    const char *message_header,
 *                                    const char *format, va_list arg_p)
 * 
 *   @brief  Creates a message based on the given location, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  loc            The location from which to generate headers for the message.
 *   @param  message_header The header of the message. Should be of the form "[A-Z][a-z]*".
 *   @param  format         The format on which to apply the arguments.
 *   @param  arg_p          The arguments to apply onto the format.
 * 
 *   @return a str_buf containing the finalized message.
 * 
 ******************************************************************************/
str_buf *
CTFvcreateMessageLoc (struct location loc, const char *message_header,
                      const char *format, va_list arg_p)
{
    str_buf *first_line_header;
    str_buf *message;
    
    DBUG_ENTER ();

    first_line_header = CreateAndLoadGNUformatHeader (loc, message_header);

    message = vCreateMessage (SBUFgetBuffer (first_line_header), format, arg_p);

    SBUFfree (first_line_header);
    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 * 
 * @fn str_buf *CTFcreateMessageLoc( struct location loc,
 *                                   const char *message_header,
 *                                   const char *format, ...)
 * 
 *   @brief  Creates a message based on the given location, format string, 
 *           and list of arguments for the format string.
 * 
 *   @param  loc            The location from which to generate headers for the message.
 *   @param  message_header The header of the message. Should be of the form "[A-Z][a-z]*".
 *   @param  format         The format on which to apply extra arguments to generate
 *                          the message.
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
