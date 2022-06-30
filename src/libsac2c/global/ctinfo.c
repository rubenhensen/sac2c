/*
 * @file
 *
 * This file provides the interface for producing any kind of output during
 * compilation. It fully replaces the macro based legacy implementation
 * in files Error.[ch]
 *
 * We have several levels of verbosity controlled by the command line option -v
 * and the global variable verbose_level.
 *
 * Verbose level 0:
 *
 * Only error messages are printed.
 *
 * Verbose level 1:
 *
 * Error messages and warnings are printed.
 *
 * Verbose level 2:
 *
 * Error messages, warnings and basic compile time information, e.g. compiler
 * phases,  are printed.
 *
 * Verbose level 3:
 *
 * Error messages, warnings and full compile time information are printed.
 *
 * Verbose level 4+:
 *
 * Additional compile time information is provided that typically is only
 * of interest in certain situation.
 *
 * Default values are 1 for the product version and 3 for the developer version.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "ctinfo.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "filemgr.h"
#include "system.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "build.h"
#include "print.h"
#include "visualize.h"
#include "convert.h"
#include "globals.h"
#include "free.h"
#include "traverse.h"
#include "phase_info.h"
#include "namespaces.h"
#include "tree_basic.h"
#include "check_mem.h"
#include "stringset.h"
#include "new_types.h" /* for TYtype2String */

#include "cppcompat.h"
#undef exit

static char *message_buffer = NULL;
static size_t message_buffer_size = 0;
static size_t message_line_length = 80;

static const char *abort_message_header = "Abort";
static const char *error_message_header = "Error";
static const char *warn_message_header = "Warning";
static const char *note_message_header = "Note";
static const char *indent_message_header = "  ";
static const char *state_message_header = "";


static int errors = 0;
static int warnings = 0;

static FILE * cti_stderr;


#define MAX_ITEM_NAME_LENGTH 255

void
set_message_line_length (size_t l)
{
    message_line_length = l;
}

int
CTIgetErrorCount (void)
{
    return errors;
}

void
CTIresetErrorCount (void)
{
    errors = 0;
}

void
CTIset_stderr (FILE *new_stderr)
{
    cti_stderr = new_stderr;
}

FILE *
CTIget_stderr ()
{
    return cti_stderr;
}


/** <!--********************************************************************-->
 *
 * @fn void ProcessMessage( char *buffer, size_t first_header_length, 
 *                          size_t multiline_header_length)
 *
 *   @brief  Formats message according to the global message length, taking into
 *           account the length of the headers.
 *           If the global message length is set to 0, no line wrapping is performed.
 *           
 *           Otherwise, the effective header length for each type is set to 
 *           min(header length + 20, cti-message-length)
 *          
 *           Tab characters are replaced with spaces.
 *           Line wrapping is done by replacing the last known space with an '@' 
 *           character (representing a newline) at the last known position of a space
 *           when the line gets too long.
 *           When no space is available to line wrap, line wrapping is delayed until
 *           a space is found or the end of the string is reached.
 *
 *           NOTE: This function does not care about the global.cti-single-line setting.
 *                 Account for it by replacing newlines with spaces on the final string.
 * 
 *   @param buffer                   message buffer
 *   @param first_header_length      the length of the header used for the first line
 *   @param multiline_header_length  the length of the header for all other lines
 *
 ******************************************************************************/

static void
ProcessMessage (char *buffer, size_t first_header_length, size_t multiline_header_length)
{
    size_t first_line_length, multiline_length, line_length, index, column, last_space;
    bool space_found;

    DBUG_ENTER ();


    if (global.cti_message_length == 0) {
        first_line_length = 0; // We don't return here because we still 
        multiline_length = 0;  // replace @ with \n and \t with space in this function.
    } else {
        // Just computes the max but I can't find a max function for size_t
        first_line_length = first_header_length + 20 <= (size_t) global.cti_message_length
                            ? (size_t) global.cti_message_length : first_header_length + 20;
        multiline_length = multiline_header_length + 20 <= (size_t) global.cti_message_length
                            ? (size_t) global.cti_message_length : multiline_header_length + 20;
    } 

    index = 0;
    last_space = 0;
    column = 0;
    line_length = first_line_length;
    space_found = false;

    while (buffer[index] != '\0') {
        if (buffer[index] == '\t') {
            buffer[index] = ' ';
        }

        if (buffer[index] == ' ') {
            last_space = index;
            space_found = true;
        }

        if (buffer[index] == '@' || buffer[index] == '\n') {
            buffer[index] = '@';
            column = 0;
            space_found = false;
            line_length = multiline_length; // Swap to multiline lengths
        } else if (line_length != 0 // line_length 0 disables insertion of newlines
                   && column >= line_length // line length is exceeded
                   && space_found) { // we have a space in the line to convert
            buffer[last_space] = '@';
            column = index - last_space;
            space_found = false;
            line_length = multiline_length; // Swap to multiline lengths
        }

        index++;
        column++;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void Format2Buffer( const char *format, va_list arg_p)
 *
 *   @brief The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It takes care of buffer overflows.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void
Format2Buffer (const char *format, va_list arg_p)
{
    int len;
    size_t len_p;
    va_list arg_p_copy;

    DBUG_ENTER ();

    va_copy (arg_p_copy, arg_p);
    len_p = (size_t)(len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy));
    va_end (arg_p_copy);

    if (len < 0) {
        DBUG_ASSERT (message_buffer_size == 0, "message buffer corruption");
        /*
         * Output error due to non-existing message buffer
         */

        len_p = 120;

        message_buffer = (char *)MEMmalloc (len_p + 2);
        CHKMdoNotReport (message_buffer);
        message_buffer_size = len_p + 2;

        va_copy (arg_p_copy, arg_p);
        len_p = (size_t)(len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy));
        va_end (arg_p_copy);
        DBUG_ASSERT (len >= 0, "message buffer corruption");
    }

    if (len_p >= message_buffer_size) {
        /* buffer too small  */

        MEMfree (message_buffer);
        message_buffer = (char *)MEMmalloc (len_p + 2);
        CHKMdoNotReport (message_buffer);
        message_buffer_size = len_p + 2;

        va_copy (arg_p_copy, arg_p);
        len_p = (size_t)(len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy));
        va_end (arg_p_copy);


        DBUG_ASSERT (len >= 0 || len_p < message_buffer_size, "message buffer corruption");
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void Format2Buffer( const char *format, ...)
 *
 *   @brief The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It takes care of buffer overflows.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void
Format2BufferDynamic (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    Format2Buffer (format, arg_p);

    va_end (arg_p);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description: Concatenate message_buffer to first, allocating memory for the
 *              new string and deallocating first.
 *              
 *
 * Parameters: - first, first string
 *
 * Return: - new concatenated string
 *
 *******************************************************************************/

char *
STRcatMessageBuffer (char *first)
{
    DBUG_ENTER ();

    char *ret = STRcat (first, message_buffer);
    MEMfree (first);

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void PrintMessage( const char *header, const char *format, va_list arg_p)
 *
 *   @brief Prints message.
 *
 *          The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It takes care of buffer overflows. Afterwards, the message
 *          is formatted to fit a certain line length and is printed to
 *          stderr.
 *
 *   @param header  string which precedes each line of the message, e.g.
                    ERROR or WARNING.
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void
PrintMessage (const char *header, const char *format, va_list arg_p)
{
    char *line;

    DBUG_ENTER ();

    Format2Buffer (format, arg_p);
    ProcessMessage (message_buffer, STRlen (header), STRlen (header));
    line = strtok (message_buffer, "@");

    while (line != NULL) {
        fprintf (cti_stderr, "%s%s\n", header, line);
        line = strtok (NULL, "@");
    }
    fflush (cti_stderr);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn static void CleanUp()
 *
 *   @brief  Does some cleanup upon termination.
 *
 *
 ******************************************************************************/

static void
CleanUp (void)
{
    DBUG_ENTER ();

    if (global.cleanup) {
        global.cleanup = FALSE;

        FMGRdeleteTmpDir ();
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn static void CleanUpInterrupted()
 *
 *   @brief  Does some clean up upon termination.
 *           This function is used by interrupt signal handlers.
 *           In contrast to CleanUp() above it avoids calling the usual
 *           function and aims to terminate as quickly as possible.
 *
 ******************************************************************************/
static void
CleanUpInterrupted (void)
{
    // DBUG_ENTER ();
    // We do not want to use the DBUG macros here to keep the code as simple
    // as possible. Note that this is the interrupt handler that is only run
    // if something has gone terribly wrong before.

    if (global.cleanup) {
        global.cleanup = FALSE;

        if (global.system_cleanup != NULL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
            system (global.system_cleanup);
#pragma GCC diagnostic pop
            /*
             * We ignore the return value here as we are already in
             * failure mode. Ignoring the result of a call to system
             * on some systems (eg. ubuntu) is designed to raise a warning!
             * Not using a defined variable on other systems (eg OSX)
             * generates a warning. Our solution: disable the unused-result
             * warning raised bu Ubuntu.
             */
        }
    }

    // DBUG_RETURN ();
    // We do not want to use the DBUG macros here to keep the code as simple
    // as possible. Note that this is the interrupt handler that is only run
    // if something has gone terribly wrong before.
}

/** <!--********************************************************************-->
 *
 * @fn void AbortCompilation()
 *
 *   @brief  Terminates the compilation process with a suitable error message.
 *
 ******************************************************************************/

FUN_ATTR_NORETURN
static void
AbortCompilation (void)
{
    int ecode = (int)global.compiler_phase;

    if (ecode == 0) {
        ecode = 255;
    }

    CleanUp ();

    (void)fprintf (cti_stderr, "Compilation failed while %s",
                   PHIphaseText (global.compiler_phase));
    if (errors > 0)
        (void)fprintf (cti_stderr, ", %d error(s)", errors);

    if (warnings > 0)
        (void)fprintf (cti_stderr, ", %d warning(s)", warnings);

    (void)fprintf (cti_stderr, ".\n");

    fflush (cti_stderr);

    exit (ecode);
}

void
CTIexit (int status)
{
    CleanUp ();
    GLOBfinalizeGlobal ();
    exit (status);
}

/** <!--********************************************************************-->
 *
 * @fn void InternalCompilerErrorBreak( int sig)
 *
 *   @brief  Interrupt handler for segmentation faults and bus errors.
 *
 *           An error message is produced and a bug report is created which
 *           the user can send in to have us fix it.
 *           Temporary files are deleted and the compilation process
 *           terminates.
 *
 *           DBUG_ENTER/RETURN are omitted on purpose to reduce risk of
 *           creating more errors during error handling.
 *
 *           Likewise, we avoid all complex support functions and use
 *           low-level string handling etc as far as possible.
 *
 *           NOTE: This function is NOT a good example of programming style
 *                 that should be used throughout sac2c otherwise.
 *
 *   @param sig  signal causing interrupt
 *
 ******************************************************************************/

static void
InternalCompilerErrorBreak (int sig)
{
    FILE *error_file;
    char error_file_name[64];
    int i;

    fprintf (cti_stderr, "\n\n"
                     "OOOOOOOPS, your program crashed the compiler 8-((\n\n"
                     "Please, submit the bug report online at\n"
                     "https://gitlab.sac-home.org/sac-group/sac2c/-/issues.\n\n");

    error_file_name[0] = '\0';

    if (global.puresacfilename != NULL) {
        i = 0;
        while ((i < 48) && (global.puresacfilename[i] != '\0')
               && (global.puresacfilename[i] != '.')) {
            error_file_name[i] = global.puresacfilename[i];
            i++;
        }
        error_file_name[i] = '\0';
    } else {
        strcpy (error_file_name, "unknown");
    }

    strcat (error_file_name, ".sacbugreport");

    error_file = fopen (error_file_name, "w");

    if (error_file != NULL) {
        fprintf (error_file,
                 "/**********************************************************************"
                 "\n"
                 " *\n"
                 " * SAC bug report: %s\n"
                 " *\n"
                 " **********************************************************************"
                 "\n"
                 " *\n"
                 " * Automatically generated on ",
                 error_file_name);
        fclose (error_file);
        SYScallNoErr ("date >> %s", error_file_name);
        error_file = fopen (error_file_name, "a");

        fprintf (error_file, " *\n");
        fprintf (error_file, " * using sac2c %s\n", global.version_id);
        fprintf (error_file, " * built %s.\n", build_date);
        fprintf (error_file, " * by user %s on host %s.\n", build_user, build_host);
        fprintf (error_file, " *\n");

        fprintf (error_file, " * The compiler was called by\n");
        fprintf (error_file, " *  %s", global.argv[0]);
        for (i = 1; i < global.argc; i++) {
            fprintf (error_file, " %s", global.argv[i]);
        }
        fprintf (error_file, "\n");
        fprintf (error_file, " *\n");

        fprintf (error_file, " * The compiler crashed in\n");
        fprintf (error_file, " *  phase: %s (%s)\n", PHIphaseName (global.compiler_phase),
                 PHIphaseText (global.compiler_phase));

        fprintf (error_file, " *  sub phase: %s (%s)\n",
                 PHIphaseName (global.compiler_subphase),
                 PHIphaseText (global.compiler_subphase));

        if (PHIphaseType (global.compiler_subphase) == PHT_cycle) {
            fprintf (error_file, " *  cycle phase: %s (%s)\n",
                     PHIphaseName (global.compiler_cyclephase),
                     PHIphaseText (global.compiler_cyclephase));
            fprintf (error_file, " *  cycle instance: %d\n", global.cycle_counter);
        }

        if (global.sacfilename != NULL) {
            fprintf (error_file,
                     " *\n"
                     " * What follows is the contents of %s.\n",
                     global.sacfilename);
            fprintf (error_file, " *\n"
                                 " ******************************************************"
                                 "****************/\n\n");
            fclose (error_file);
            SYScallNoErr ("cat %s >> %s", global.sacfilename, error_file_name);
            error_file = fopen (error_file_name, "a");
            fprintf (error_file, "\n\n"
                                 "/******************************************************"
                                 "****************\n"
                                 " *\n"
                                 " * End of bug report\n"
                                 " *\n"
                                 " ******************************************************"
                                 "****************/\n\n");
        } else {
            fprintf (error_file,
                     " *\n"
                     " * Compiler crashed before SAC file name could be determined !\n"
                     " *\n"
                     " ******************************************************************"
                     "****\n"
                     " *\n"
                     " * End of bug report\n"
                     " *\n"
                     " ******************************************************************"
                     "****/\n\n");
        }

        fclose (error_file);

        fprintf (cti_stderr,
                 "For your convenience, the compiler has pre-fabricated a bug report\n"
                 "in the file \"./%s\" !\n\n"
                 "Besides some infos concerning the compiler version and its\n"
                 "usage it contains the specified source file.\n\n"
                 "If you want to send that bug report to us, you may simply type\n"
                 "  mail bugs@sac-home.org < %s\n\n"
                 "If you decide to file a bug on our bug-tracker, please go to\n"
                 "  https://gitlab.sac-home.org/sac-group/sac2c/-/issues.\n\n",
                 error_file_name, error_file_name);
        fprintf (cti_stderr, "When filing a bug report, please copy/paste the initial "
                         "comment section of\n"
                         "the bug report into the plain text comment section of "
                         "the bug-tracker, and add\n"
                         "the whole bug report file as an attachment.\n\n");
    } else {
        fprintf (cti_stderr, "Sorry, but sac2c is unable to create a bug report file.\n\n"
                         "Please, send the source file, the exact compiler call and the\n"
                         "compiler revision number along with the terminal output that\n"
                         "led to this crash to bugs@sac-home.org\n\n");
    }

    CleanUpInterrupted ();

    abort ();
}

/** <!--********************************************************************-->
 *
 * @fn void UserForcedBreak( int sig)
 *
 *   @brief  Interrupt handler for user-forced breaks like CTRL-C.
 *
 *           Temporary files are deleted and the compilation process
 *           terminates.
 *
 *           DBUG_ENTER/RETURN are omitted on purpose to reduce risk of
 *           creating more errors during error handling.
 *
 *   @param sig  signal causing interrupt
 *
 ******************************************************************************/

static void
UserForcedBreak (int sig)
{
    CleanUpInterrupted ();
    abort ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIinstallInterruptHandlers()
 *
 *   @brief  Installs interrupt handlers.
 *
 ******************************************************************************/

void
CTIinstallInterruptHandlers ()
{
    DBUG_ENTER ();

    signal (SIGSEGV, InternalCompilerErrorBreak); /* Segmentation Fault */
    signal (SIGBUS, InternalCompilerErrorBreak);  /* Bus Error */
    signal (SIGINT, UserForcedBreak);             /* Interrupt (Control-C) */

    DBUG_RETURN ();
}

char *
CTIfinalizeFirstLine (const char *location_buffer, const char *message_header)
{
    // Goal:
    // Do ProcessMessage for location_buffer + message header, but don't continue after the first newline is found/made.
    // return result
    return NULL;
}

char *
CTIfinalizeMultiline (char *first_line_overflow, const char *format, va_list arg_p)
{
    // Goal:
    // Construct message content with format and arg_p
    // Add that to first_line_overflow
    // Do ProcessMessage with the result (so far) and multiline header
    // return result
    return NULL;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIfinalizeMessageBegin( const char *location_buffer, const char *message_header, 
 *                                   const char *format, va_list arg_p)
 *
 *   @brief  Finalizes all processing required to display the message such that 
 *           it can later be printed with `fprintf(cti_stderr, "%s", ret)`
 *           Does not include a trailing newline.
 *
 *   @param location_buffer  The buffer that contains the possible location header 
 *                           associated with the type of message of the forms:
 *                           "" (empty but not null) for generic messages
 *                           "<filepath>:line: " for messages with line info
 *                           "<filepath:line:column: " for messages with location info
 *   @param message_header   The header associated with the type of message, 
 *                           e.g. error_message_header for errors.
 *   @param format           format string like in printf
 *   @param arg_p            arguments for the format string
 *
 ******************************************************************************/

char *
CTIfinalizeMessageBegin (const char *location_buffer, const char *message_header,
                         const char *format, va_list arg_p)
{
    char *ret;
    char *accumulator;
    char *message_header_base;
    char *message_header_first_line;
    char *message_header_multiline;
    size_t message_header_first_line_length;
    size_t message_header_multiline_length;

    DBUG_ENTER ();

    // PREPARING HEADERS
    
    // "<filepath>:<line>:<column>: <MessageType>"
    message_header_base = STRcat (location_buffer, message_header);

    // "<message_header_base>" applied to the format string from cti_header_format
    Format2BufferDynamic (global.cti_header_format, message_header_base);
    message_header_first_line = STRcpy (message_buffer);
    message_header_first_line_length = STRlen (message_header_first_line);

    // "<message_header_base>" applied to the format string from cti-multi-line-format
    Format2BufferDynamic (global.cti_multi_line_format, message_header_base);
    message_header_multiline = STRcat ("\n", message_buffer);  // \n has to be added
    message_header_multiline_length = STRlen (message_buffer); // but is not counted towards the length

    // CONSTRUCTING MESSAGE CONTENT

    Format2Buffer (format, arg_p);
    accumulator = STRcatMessageBuffer (message_header_first_line); // frees message_header_first_line

    ProcessMessage (accumulator, message_header_first_line_length, message_header_multiline_length);

    if (global.cti_single_line) {
        ret = STRsubstTokend (accumulator, "@", " "); // No need to insert the multiline header
        ret = STRsubstTokend (ret, "\n", " ");        // if there are no multilines
    } else {
        ret = STRsubstTokend (accumulator, "@", message_header_multiline);
    }

    MEMfree (message_header_base);
    MEMfree (message_header_multiline);
    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIfinalizeMessage( const char *location_buffer, const char *message_header, 
 *                              const char *format, va_list arg_p)
 *
 *   @brief  Returns CTIfinalizeMessage but with a trailing newline.
 *
 *   @param location_buffer  The buffer that contains the possible location header 
 *                           associated with the type of message of the forms:
 *                           "" (empty but not null) for generic messages
 *                           "<filepath>:line: " for messages with line info
 *                           "<filepath:line:column: " for messages with location info
 *   @param message_header   The header associated with the type of message, 
 *                           e.g. error_message_header for errors.
 *   @param format           format string like in printf
 *   @param arg_p            arguments for the format string
 *
 ******************************************************************************/

char *
CTIfinalizeMessage (const char *location_buffer, const char *message_header, 
                    const char *format, va_list arg_p)
{
    char *tmp;
    char *ret;
    
    DBUG_ENTER ();
    tmp = CTIfinalizeMessageBegin (location_buffer, message_header, format, arg_p);
    ret = STRcat (tmp, "\n");

    MEMfree (tmp);
    DBUG_RETURN (ret);
}


/** <!--********************************************************************-->
 * 
 * @fn void CTIcreateMessage( const char *message_header, const char *format,
 *                            va_list arg_p)
 * 
 *   @brief  Takes care of creating the message without file name, 
 *           line number or location.
 * 
 *   @param message_header  The header associated with the type of message, 
 *                          i.e. error_message_header for errors.
 *   @param format          format string like in printf
 *   @param arg_p           arguments for the format string
 * 
 * ****************************************************************************/

char *
CTIcreateMessage (const char *message_header, const char *format, va_list arg_p)
{
    char *ret;
    char *tmp;

    DBUG_ENTER ();

    tmp = STRcpy("");
    ret = CTIfinalizeMessage (tmp, message_header, format, arg_p);

    MEMfree (tmp);
    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIcreateMessageLine( const char *message_header, size_t line, 
 *                                const char *format, va_list arg_p)
 *
 *   @brief  Takes care of creating the message with a file name and line number.
 *
 *   @param message_header  The header associated with the type of message, 
 *                          e.g. error_message_header for errors.
 *   @param line            line number
 *   @param format          format string like in printf
 *   @param arg_p           arguments for the format string
 *
 ******************************************************************************/

char *
CTIcreateMessageLine (const char *message_header, size_t line, const char *format, va_list arg_p)
{
    char *ret;
    char *tmp;
    DBUG_ENTER ();

    Format2BufferDynamic("%s:%zu: ", global.filename, line); // GNU format location
    tmp = STRcpy (message_buffer);

    ret = CTIfinalizeMessage (tmp, message_header, format, arg_p);

    MEMfree (tmp);
    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIcreateMessageLoc( const char *message_header, const struct location loc, 
 *                               const char *format, va_list arg_p)
 *
 *   @brief  Take care of creating the message with a file name, line number and column.
 *
 *   @param message_header  The header associated with the type of message, 
 *                          e.g. error_message_header for errors.
 *   @param loc             location info
 *   @param format          format string like in printf
 *   @param arg_p           arguments for the format string
 *
 ******************************************************************************/
char *
CTIcreateMessageLoc (const char *message_header, const struct location loc, const char *format, va_list arg_p)
{
    char *ret;
    char *tmp;

    DBUG_ENTER ();

    Format2BufferDynamic("%s:%zu:%zu: ", loc.fname, loc.line, loc.col); // GNU format location
    tmp = STRcpy (message_buffer);

    ret = CTIfinalizeMessage (tmp, message_header, format, arg_p);

    MEMfree (tmp);
    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerror( const char *format, ...)
 *
 *   @brief  Produces an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerror (const char *format, ...)
{
    char *error_msg;
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    error_msg = CTIcreateMessage (error_message_header, format, arg_p);
    fprintf (cti_stderr, "%s", error_msg);
    MEMfree (error_msg);

    va_end (arg_p);

    errors++;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLine( int line, const char *format, ...)
 *
 *   @brief  Produces an error message preceded by file name and line number.
 *
 *
 *   @param line    line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorLine (size_t line, const char *format, ...)
{
    char *error_msg;
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    error_msg = CTIcreateMessageLine (error_message_header, line, format, arg_p);
    fprintf (cti_stderr, "%s", error_msg);
    MEMfree (error_msg);

    va_end (arg_p);

    errors++;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn char *CTIgetErrorMessageVA( int line, const char *file,
 *                                 const char *format, va_list arg_p)
 *
 *   @brief Generates an error message string.
 *
 *          This function allows for split-phase error messages:
 *
 *          str = CTIgetErrorMessageVA (line, fname, format, ...args);
 *          CTIabortOnBottom (str);
 *
 *          equates to
 *
 *          CTIerrorLine (line, format, ...args);
 *
 *          This allows preparing error messages in advance and activating
 *          them when needed.
 *
 *   @param line    line number to use in error message
 *   @param file    file name to use in error message
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

char *
CTIgetErrorMessageVA (size_t line, const char *file, const char *format, va_list arg_p)
{
    char *ret;
    DBUG_ENTER ();

    Format2BufferDynamic("%s:%zu: ", file, line); // GNU format location
    ret = STRcpy (message_buffer);

    ret = CTIfinalizeMessage (ret, error_message_header, format, arg_p);

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLoc( struct location loc, const char *format, ...)
 *
 *   @brief   Produces an error message preceded by location info.
 *
 *   @param loc     location info
 *   @param format  format string like in printf
 *
 ******************************************************************************/
void
CTIerrorLoc (struct location loc, const char *format, ...)
{    
    char *error_msg;
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    error_msg = CTIcreateMessageLoc (error_message_header, loc, format, arg_p);
    fprintf (cti_stderr, "%s", error_msg);
    MEMfree (error_msg);

    va_end (arg_p);

    errors++;

    DBUG_RETURN ();
}

char *
CTIerrorBegin (const char *format, ...)
{
    if (global.cti_single_line) {

    }
    // Goal:
    // If cti-single-line is active:
    // • Same as CTIerror but without a trailing newline
    // If cti-single-line is inactive:
    // • Same as CTIerror
    return NULL;
}

char *
CTIerrorLineBegin (size_t line, const char *format, ...)
{
    // Goal:
    // If cti-single-line is active:
    // • Same as CTIerrorLine but without a trailing newline
    // If cti-single-line is inactive:
    // • Same as CTIerrorLine
    return NULL;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorContinued( const char *format, ...)
 *
 *   @brief  Same as CTIerror BUT does not count as new error!
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void //TODO: change to char *
CTIerrorContinued (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    // Goal:
    // If cti-single-line is active:
    // • Don't print a header
    // • Don't print a trailing newline, but print a trailing space
    // If cti-single-line is inactive:
    // • Do print a multiline header
    // • Do print a trailing newline
    PrintMessage (indent_message_header, format, arg_p);

    va_end (arg_p);

    DBUG_RETURN ();
}

char *
CTIerrorEnd (void) 
{
    // Goal:
    // If cti-single-line is active:
    // • Return a newline
    // If cti-single-line is inactive:
    // • Return an empty string
    return NULL;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorInternal( const char *format, ...)
 *
 *   @brief  Produces an error message without file name and line number, but
 *           specifies the phase where the compiler failed.
 *           This function does not respect all cti command-line arguments
 *           to avoid potential crashes or hangs during error handling.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorInternal (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    fprintf (cti_stderr, "%sInternal %s failure\n", error_message_header, global.toolname);

    fprintf (cti_stderr,
             "%sCompiler phase:    %s\n"
             "%s                   %s\n"
             "%sTraversal:         %s\n"
             "%sFunction:          %s( %s)",
             error_message_header, PHIphaseName (global.compiler_anyphase),
             error_message_header, PHIphaseText (global.compiler_anyphase),
             error_message_header, TRAVgetName (), error_message_header,
             CTIitemName (global.current_fundef), CTIfunParams (global.current_fundef));

    va_start (arg_p, format);
    fprintf (cti_stderr, "%s", error_message_header);
    fprintf (cti_stderr, format, arg_p);
    va_end (arg_p);


    errors++;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn size_t CTIgetErrorMessageLineLength( )
 *
 *   @brief  Yields useful line length for error messages.
 *
 *   @return line length
 *
 ******************************************************************************/

size_t
CTIgetErrorMessageLineLength ()
{
    DBUG_ENTER ();

    DBUG_RETURN (message_line_length - STRlen (error_message_header));
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortOnBottom( const char *err_msg)
 *
 *   @brief   Prints a preprocessed error message obtained from CTIgetErrorMessageVA
 *            and aborts the compilation.
 *
 *   @param err_msg  pre-processed error message
 *
 ******************************************************************************/

void
CTIabortOnBottom (char *err_msg)
{
    fprintf (cti_stderr, "%s", err_msg);
    AbortCompilation ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabort( const char *format, ...)
 *
 *   @brief   Produces an error message without file name and line number
 *            and terminates the compilation process.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabort (const char *format, ...)
{
    char *abort_msg;
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    abort_msg = CTIcreateMessage (abort_message_header, format, arg_p);
    fprintf (cti_stderr, "%s", abort_msg);
    MEMfree (abort_msg);

    va_end (arg_p);

    AbortCompilation ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortLoc( struct location loc, const char *format, ...)
 *
 *   @brief   produces an error message preceded by location info
 *            and terminates the compilation process
 *
 *   @param loc     location info
 *   @param format  format string like in printf
 *
 ******************************************************************************/
void
CTIabortLoc (struct location loc, const char *format, ...)
{
    va_list arg_p;

    va_start (arg_p, format);
    fprintf (cti_stderr, "%s\n", produce_header (loc, error_message_header));
    PrintMessage (second_level_header, format, arg_p);
    va_end (arg_p);

    AbortCompilation ();
}


/** <!--********************************************************************-->
 *
 * @fn void CTIabortLine( int line, const char *format, ...)
 *
 *   @brief   Produces an error message preceded by file name and line number
 *            and terminates the compilation process.
 *
 *   @param line    line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabortLine (size_t line, const char *format, ...)
{
    char *abort_msg;
    va_list arg_p;

    DBUG_ENTER ();

    va_start (arg_p, format);

    abort_msg = CTIcreateMessageLine (abort_message_header, line, format, arg_p);
    fprintf (cti_stderr, "%s", abort_msg);
    MEMfree (abort_msg);

    va_end (arg_p);

    AbortCompilation ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortOutOfMemory( size_t request)
 *
 *   @brief   Produces a specific "out of memory" error message
 *            without file name and line number and terminates the
 *            compilation process.
 *
 *            This very special function is needed because the normal
 *            procedure of formatting a message may require further
 *            allocation of memory, which in this very case generates
 *            a vicious circle of error messages instead of terminating
 *            compilation properly.
 *
 *   @param request size of requested memory
 *
 ******************************************************************************/

void
CTIabortOutOfMemory (size_t request)
{
    fprintf (cti_stderr,
             "%s:\n"
             "%sOut of memory: %zu bytes requested\n",
             abort_message_header, indent_message_header, request);

    fprintf (cti_stderr, "%s%zu bytes already allocated\n", indent_message_header,
             global.current_allocated_mem);

    AbortCompilation ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortOnError()
 *
 *   @brief  Terminates compilation process if errors have occurred.
 *
 ******************************************************************************/

void
CTIabortOnError ()
{
    DBUG_ENTER ();

    if (errors > 0) {
        AbortCompilation ();
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *******************************************************************************
 *
 * Warnings (verbosity level >= 1)
 */

/** <!--********************************************************************-->
 *
 * @fn void CTIwarnLoc( ,struct location loc, const char *format, ...)
 *
 *   @brief   Produces a warning message preceded by location info.
 *
 *   @param loc     location info
 *   @param format  format string like in printf
 *
 ******************************************************************************/
void
CTIwarnLoc (struct location loc, const char *format, ...)
{
    char *warn_msg;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        warn_msg = CTIcreateMessageLoc (warn_message_header, loc, format, arg_p);
        fprintf (cti_stderr, "%s", warn_msg);
        MEMfree (warn_msg);

        va_end (arg_p);

        warnings++;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIwarnLine( int line, const char *format, ...)
 *
 *   @brief   Produces a warning message preceded by file name and line number.
 *
 *   @param line    line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarnLine (size_t line, const char *format, ...)
{
    char *warn_msg;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        warn_msg = CTIcreateMessageLine (warn_message_header, line, format, arg_p);
        fprintf (cti_stderr, "%s", warn_msg);
        MEMfree (warn_msg);

        va_end (arg_p);

        warnings++;
    }

    DBUG_RETURN ();
}


/** <!--********************************************************************-->
 *
 * @fn void CTIwarn( const char *format, ...)
 *
 *   @brief   Produces a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarn (const char *format, ...)
{
    char *warn_msg;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        warn_msg = CTIcreateMessage (warn_message_header, format, arg_p);
        fprintf (cti_stderr, "%s", warn_msg);
        MEMfree (warn_msg);

        va_end (arg_p);

        warnings++;
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIwarnContinued( const char *format, ...)
 *
 *   @brief   Same as CTIwarn BUT does not count as new warning.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarnContinued (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        PrintMessage (indent_message_header, format, arg_p);

        va_end (arg_p);

    }

    DBUG_RETURN ();
}


/** <!--********************************************************************-->
 *
 * @fn size_t CTIgetWarnMessageLineLength( )
 *
 *   @brief  Yields useful line length for warning messages.
 *
 *   @return line length
 *
 ******************************************************************************/

size_t
CTIgetWarnMessageLineLength ()
{
    DBUG_ENTER ();

    DBUG_RETURN (message_line_length - STRlen (warn_message_header));
}



/*******************************************************************************
 *******************************************************************************
 *
 * Statements (verbosity level >= 2)
 */


/** <!--********************************************************************-->
 *
 * @fn void CTIstate( const char *format, ...)
 *
 *   @brief  Produces basic compile time information output (verbosity level 2)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIstate (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 2) {
        va_start (arg_p, format);

        PrintMessage (state_message_header, format, arg_p);

        va_end (arg_p);
    }

    DBUG_RETURN ();
}


/*******************************************************************************
 *******************************************************************************
 *
 * Notes (verbosity level >= 3)
 */

/** <!--********************************************************************-->
 *
 * @fn void CTInote( const char *format, ...)
 *
 *   @brief  Produces full compile time information output (verbosity level 3)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTInote (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 3) {
        va_start (arg_p, format);

        // Note (pun not intended) that we should not use CTIcreateMessage or
        // note_message_header here because 17 years worth of code is specifically
        // formatted to only be prefixed with 2 spaces.
        PrintMessage (indent_message_header, format, arg_p);

        va_end (arg_p);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTInoteLine( int line, const char *format, ...)
 *
 *   @brief  Produces full compile time information output, preceded by 
 *           file name and line number at verbosity level 3.
 *
 *   @param line    line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTInoteLine (size_t line, const char *format, ...)
{
    char *note_msg;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 3) {
        va_start (arg_p, format);

        note_msg = CTIcreateMessageLine (note_message_header, line, format, arg_p);
        fprintf (cti_stderr, "%s", note_msg);
        MEMfree (note_msg);

        va_end (arg_p);
    }

    DBUG_RETURN ();
}



/*******************************************************************************
 *******************************************************************************
 *
 * general output (*any* verbosity level)
 */

/** <!--********************************************************************-->
 *
 * @fn void CTItell( int verbose_level, const char *format, ...)
 *
 *   @brief Produces compile time information at the given verbosity level.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTItell (int verbose_level, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= verbose_level) {
        va_start (arg_p, format);

        PrintMessage (indent_message_header, format, arg_p);

        va_end (arg_p);
    }

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIterminateCompilation()
 *
 *   @brief  Terminates successful compilation process.
 *
 ******************************************************************************/

void
CTIterminateCompilation (node *syntax_tree)
{
    DBUG_ENTER ();
    char *visual_output;
    char *shell_command;
    bool break_found = FALSE;
    /*
     * Upon premature termination of compilation process show
     * syntax tree if available.
     */

    if (global.print_after_break && (syntax_tree != NULL)
        && ((global.tool != TOOL_sac2c) || (global.compiler_subphase <= PH_cg_pds))
        && ((global.break_after_phase < PHIlastPhase ())
            || (global.break_after_subphase < PHIlastPhase ())
            || (global.break_after_cyclephase < PHIlastPhase ()))) {
        if (!global.doprintfunsets) {
            global.doprintfunsets = global.printfunsets.imp && global.printfunsets.use
                                    && global.printfunsets.def && global.printfunsets.wrp
                                    && global.printfunsets.pre;
        }
        syntax_tree = PRTdoPrintFile (stdout, syntax_tree);
    }

    if (global.visual_after_break && (syntax_tree != NULL)) {
        if (!DOT_FLAG) {
            CTIwarn ("If you want to visualize syntax tree. Please install dot. \n");
        } else {
            if (!global.dovisualizefunsets) {
                global.dovisualizefunsets
                  = global.visualizefunsets.imp && global.visualizefunsets.use
                    && global.visualizefunsets.def && global.visualizefunsets.wrp
                    && global.visualizefunsets.pre;
            }

            if (!global.dovisualizefunsets) {
                global.dovisualizefunsets = TRUE;
            }

            visual_output = VISUALdoVisual (syntax_tree);
            shell_command
              = STRcatn (8, DOT_CMD, visual_output, " -T", global.visual_format, " -o ",
                         global.outfilename, ".", global.visual_format);
            DBUG_PRINT ("\n %s \n", shell_command);
            SYScall ("%s", shell_command);
            MEMfree (shell_command);
            MEMfree (visual_output);
        }
    }

    /*
     *  Finally, we do some clean up.
     */

    CleanUp ();

    if (syntax_tree != NULL) {
        syntax_tree = FREEdoFreeTree (syntax_tree);
    }

    /*
     *  At last, we display a success message.
     */

    CTIstate (" ");
    CTIstate ("** Compilation successful");

    if (global.break_after_cyclephase < PHIlastPhase ()) {
        CTIstate ("** BREAK during:   %s\n", PHIphaseText (global.compiler_phase));
        CTIstate ("** BREAK in cycle: %s\n", PHIphaseText (global.compiler_subphase));
        CTIstate ("** BREAK in pass:  %d\n", global.break_cycle_specifier);
        CTIstate ("** BREAK after:    %s\n",
                  PHIphaseText (global.break_after_cyclephase));
        break_found = TRUE;
    } else {
        if (global.break_after_subphase < PHIlastPhase ()) {
            CTIstate ("** BREAK during: %s\n", PHIphaseText (global.compiler_phase));
            CTIstate ("** BREAK after:  %s\n", PHIphaseText (global.compiler_subphase));
        } else {
            if (global.break_after_phase < PHIlastPhase ()) {
                CTIstate ("** BREAK after: %s\n", PHIphaseText (global.compiler_phase));
            }
        }
        break_found = TRUE;
    }

    if (global.memcheck) {
        CHKMdeinitialize ();

        CTIstate ("** Maximum allocated memory (bytes):   %s",
                  CVintBytes2String (global.max_allocated_mem));
        CTIstate ("** Currently allocated memory (bytes): %s",
                  CVintBytes2String (global.current_allocated_mem));
    }

    CTIstate ("** Exit code 0");
    CTIstate ("** 0 error(s), %d warning(s)", warnings);
    CTIstate (" ");

    GLOBfinalizeGlobal ();
    if (break_found)
        exit (0);

    DBUG_RETURN ();
}

/**
 * @fn const char *CTIitemName(node *item)
 *
 * @brief Creates a string containing the module and name of
 *        the given item. As this function uses an internal
 *        buffer, the returned string should only be used to
 *        print a single message.
 *
 * @param item a N_fundef, N_objdef or N_typedef
 *
 * @return a constant string
 */

const char *
formatItemName (namespace_t *ns, char *name, const char *divider)
{
    static char buffer[MAX_ITEM_NAME_LENGTH + 1];
    int written;

    DBUG_ENTER ();

    if (ns != NULL) {
        written = snprintf (buffer, MAX_ITEM_NAME_LENGTH, "%s%s%s", NSgetName (ns),
                            divider, name);
    } else {
        written = snprintf (buffer, MAX_ITEM_NAME_LENGTH, "%s", name);
    }

    DBUG_ASSERT (written < MAX_ITEM_NAME_LENGTH, "buffer in formatItemName too small");

    DBUG_RETURN (buffer);
}

const char *
CTIitemName (node *item)
{
    return CTIitemNameDivider (item, "::");
}

const char *
CTIitemNameDivider (node *item, const char *divider)
{
    const char *ret;

    DBUG_ENTER ();

    if (item == NULL || divider == NULL || divider[0] == '\0') {
        ret = "???";
    } else {
        switch (NODE_TYPE (item)) {
        case N_fundef:
            ret = formatItemName (FUNDEF_NS (item), FUNDEF_NAME (item), divider);
            break;
        case N_typedef:
            ret = formatItemName (TYPEDEF_NS (item), TYPEDEF_NAME (item), divider);
            break;
        case N_objdef:
            ret = formatItemName (OBJDEF_NS (item), OBJDEF_NAME (item), divider);
            break;
        default:
            DBUG_UNREACHABLE ("Wrong item in call of function 'CTIitemName`");
            ret = NULL;
        }
    }

    DBUG_RETURN (ret);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIfunParams( node *fundef)
 *
 *****************************************************************************/

const char *
CTIfunParams (node *fundef)
{
    node *arg;
    char *tmp_str, *ret;
    size_t tmp_str_size;

    static char argtype_buffer[80];
    static size_t buffer_space;

    DBUG_ENTER ();

    if (fundef == NULL) {
        ret = "???";
    } else {
        strcpy (argtype_buffer, "");
        buffer_space = 77;

        arg = FUNDEF_ARGS (fundef);
        while ((arg != NULL) && (buffer_space > 5)) {

            tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), TRUE, 0);
            tmp_str_size = STRlen (tmp_str);

            if ((tmp_str_size + 3) <= buffer_space) {
                strcat (argtype_buffer, tmp_str);
                buffer_space -= tmp_str_size;
                if (ARG_NEXT (arg) != NULL) {
                    strcat (argtype_buffer, ", ");
                    buffer_space -= 2;
                }
            } else {
                strcat (argtype_buffer, "...");
                buffer_space = 0;
            }

            tmp_str = MEMfree (tmp_str);
            arg = ARG_NEXT (arg);
        }
        ret = argtype_buffer;
    }

    DBUG_RETURN (ret);
}

#undef DBUG_PREFIX
