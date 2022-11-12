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
#include "ctformatting.h"

#define DBUG_PREFIX "CTI"
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
#include "math_utils.h"

#include "cppcompat.h"
#undef exit

static const char *abort_message_header = "Abort";
static const char *error_message_header = "Error";
static const char *warn_message_header = "Warning";
static const char *note_message_header = "Note";
static const char *indent_message_header = "  ";
static const char *tell_message_header = "  ";
static const char *state_message_header = "";

static int errors = 0;
static int warnings = 0;

static FILE * cti_stderr;

#define MAX_ITEM_NAME_LENGTH 255

#define PRINT_MESSAGE(header)                                           \
    {                                                                   \
        va_start (arg_p, format);                                       \
        message = CTFvCreateMessage (header, header, format, arg_p);    \
        va_end (arg_p);                                                 \
        fprintf (cti_stderr, "%s", SBUFgetBuffer (message));            \
        SBUFfree (message);                                             \
    }

#define PRINT_MESSAGE_LOC(header)                                       \
    {                                                                   \
        va_start (arg_p, format);                                       \
        message = CTFvCreateMessageLoc (loc, header, format, arg_p);    \
        va_end (arg_p);                                                 \
        fprintf (cti_stderr, "%s", SBUFgetBuffer (message));            \
        SBUFfree (message);                                             \
    }

#define PRINT_MESSAGE_BEGIN(header)                                         \
    {                                                                       \
        va_start (arg_p, format);                                           \
        message = CTFvCreateMessageBeginLoc (loc, header, format, arg_p);   \
        va_end (arg_p);                                                     \
        fprintf (cti_stderr, "%s", SBUFgetBuffer (message));                \
        SBUFfree (message);                                                 \
    }

#define PRINT_MESSAGE_CONTINUED()                                               \
    {                                                                           \
        message = SBUFcreate (0);                                               \
        va_start (arg_p, format);                                               \
        SBUFvprintf (message, format, arg_p);                                   \
        va_end (arg_p);                                                         \
        message = CTFcreateMessageContinued (message); /*deallocates message*/  \
        fprintf (cti_stderr, "%s", SBUFgetBuffer (message));                    \
        SBUFfree (message);                                                     \
    }

#define PRINT_MESSAGE_END()                                     \
    {                                                           \
        message = CTFcreateMessageEnd ();                       \
        fprintf (cti_stderr, "%s", SBUFgetBuffer (message));    \
        SBUFfree (message);                                     \
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

/** <!--********************************************************************-->
 *
 * @fn void CTIerror( const struct location loc, const char *format, ...)
 *
 *   @brief  Produces an error message with the file name, line number and
 *           column if they are available.
 *
 *   @param loc     If no location is available or relevant, you can provide
 *                  the macro EMPTY_LOC.
 *                  If the file name and line numbers are available, but the
 *                  column is unknown, use the macro LINE_TO_LOC (line).
 *                  If all information is available, just provide a location
 *                  struct as normal.
 *   @param format  format string like in printf
 *
 ******************************************************************************/
void
CTIerror(const struct location loc, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    PRINT_MESSAGE_LOC (error_message_header);
    errors++;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *CTIgetErrorMessageVA( int line, const char *file,
 *                                    const char *format, va_list arg_p)
 *
 *   @brief Generates an error message string.
 *
 *          This function allows for split-phase error messages:
 *
 *          str_buf = CTIgetErrorMessageVA (line, fname, format, ...args);
 *          CTIerrorRaw (SBUFgetBuffer (str_buf));
 *          SBUFfree (str_buf);
 *
 *          equates to
 *
 *          CTIerror (LINE_TO_LOC (line), format, ...args);
 *
 *          This allows preparing error messages in advance and activating
 *          them when needed.
 *
 *          Afterwards, an abort can be triggered with CTIabortOnError ();
 *
 *   @param line    line number to use in error message
 *   @param file    file name to use in error message
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

str_buf *
CTIgetErrorMessageVA (size_t line, const char *file, const char *format, va_list arg_p)
{
    str_buf *message;
    DBUG_ENTER ();

    message = CTFvCreateMessageLoc ((struct location) {.fname = file, .line = line, .col = 0},
                                    error_message_header, format, arg_p);

    DBUG_RETURN (message);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorBegin( const struct location loc, const char *format, ...)
 *
 *   @brief  Produces an error message with the file name, line number and
 *           column if they are available.
 *           After calling CTIerrorBegin, any number of calls to CTIerrorContinued
 *           can be made, after which *CTIerrorEnd* has to be called.
 *
 *   @param loc     If no location is available or relevant, you can provide
 *                  the macro EMPTY_LOC.
 *                  If the file name and line numbers are available, but the
 *                  column is unknown, use the macro LINE_TO_LOC (line).
 *                  If all information is available, just provide a location
 *                  struct as normal.
 *   @param format  format string like in printf
 *
 ******************************************************************************/
void
CTIerrorBegin (const struct location loc, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    PRINT_MESSAGE_BEGIN (error_message_header);
    errors++;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorContinued( const struct location loc, const char *format, ...)
 *
 *   @brief  Continues an error message that was started with CTIerrorBegin, using
 *           the file name, line number and column from the location if they available.
 *           After calling CTIerrorBegin, any number of calls to CTIerrorContinued
 *           can be made, after which *CTIerrorEnd* has to be called.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorContinued (const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    PRINT_MESSAGE_CONTINUED ();

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorEnd()
 *
 *   @brief  Ends an error message that was started with CTIerrorBegin and followed
 *           by any number of CTIerrorContinued calls.
 *           Failure to call this function will lead to memory leaks and missing
 *           newlines if the command line option cti-single-line is enabled.
 *
 ******************************************************************************/
void
CTIerrorEnd (void)
{
    str_buf *message;

    DBUG_ENTER ();

    PRINT_MESSAGE_END ();

    DBUG_RETURN ();
}


/** <!--********************************************************************-->
 *
 * @fn void CTIerrorRaw( const char *message)
 *
 *   @brief  Prints a preformatted error message and counts increments the error counter.
 *           Should only be used in conjunction with CTIgetErrorMessageVA, but should
 *           ideally be completely avoided in favor of CTIerror()
 *
 *   @param message The preformatted message to be printed to stderr.
 *
 ******************************************************************************/
void
CTIerrorRaw (const char *message)
{
    DBUG_ENTER ();

    fprintf (cti_stderr, "%s", message);
    errors++;

    DBUG_RETURN ();
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

    fprintf (cti_stderr, "%s: Internal %s failure\n", error_message_header, global.toolname);

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
 * @fn void CTIabort( const struct location loc, const char *format, ...)
 *
 *   @brief   Aborts with an abort message with the file name, line number and
 *            column if they are available.
 *
 *   @param loc     If no location is available or relevant, you can provide
 *                  the macro EMPTY_LOC.
 *                  If the file name and line numbers are available, but the
 *                  column is unknown, use the macro LINE_TO_LOC (line).
 *                  If all information is available, just provide a location
 *                  struct as normal.
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabort (const struct location loc, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    PRINT_MESSAGE_LOC (abort_message_header);

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
 * @fn void CTIwarn( const struct location loc, const char *format, ...)
 *
 *   @brief   Produces a warning message with the file name, line number and
 *            column if they are available.
 *
 *   @param loc     If no location is available or relevant, you can provide
 *                  the macro EMPTY_LOC.
 *                  If the file name and line numbers are available, but the
 *                  column is unknown, use the macro LINE_TO_LOC (line).
 *                  If all information is available, just provide a location
 *                  struct as normal.
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarn (const struct location loc, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 1) {
        PRINT_MESSAGE_LOC (warn_message_header);
        warnings++;
    }

    DBUG_RETURN ();
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
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 2) {
        PRINT_MESSAGE (state_message_header);
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
 * @fn void CTInote( const struct location loc, const char *format, ...)
 *
 *   @brief  Produces full compile time information output (verbosity level 3)
 *           If EMPTY_LOC is provided, the messages are indented with two spaces.
 *           If location information is provided, the messages have a header with
 *           `Note', similar to CTIerror/Abort/Warn.
 *
 *   @param loc     If no location is available or relevant, you can provide
 *                  the macro EMPTY_LOC.
 *                  If the file name and line numbers are available, but the
 *                  column is unknown, use the macro LINE_TO_LOC (line).
 *                  If all information is available, just provide a location
 *                  struct as normal.
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTInote (const struct location loc, const char *format, ...)
{
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= 3) {
        if (loc.fname == NULL || loc.line == 0) {
            // Legacy code - use the indent header instead of the note header if the location is empty
            PRINT_MESSAGE (indent_message_header);
        } else {
            // With a location, the message is generated 'as normal'.
            PRINT_MESSAGE_LOC (note_message_header);
        }
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
    str_buf *message;
    va_list arg_p;

    DBUG_ENTER ();

    if (global.verbose_level >= verbose_level) {
        PRINT_MESSAGE (tell_message_header);
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
            CTIwarn (EMPTY_LOC, "If you want to visualize syntax tree. Please install dot. \n");
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
