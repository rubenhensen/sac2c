/*
 *
 * $Log$
 * Revision 1.11  2005/06/03 10:09:20  sbs
 * CTIerrorLineVA added
 *
 * Revision 1.10  2005/06/03 09:34:16  cg
 * Bug fixed in handling of error message buffer.
 *
 * Revision 1.9  2005/03/10 09:41:09  cg
 * Added CTIterminateCompilation() which is always called upon
 * successful termination whether or not a break option was used.
 *
 * Revision 1.8  2005/02/14 14:15:04  cg
 * Layout bug fixed.
 *
 * Revision 1.7  2005/02/07 16:35:04  cg
 * Adapted usage of vsnprintf Solaris.
 *
 * Revision 1.6  2005/01/21 18:05:09  cg
 * Layout bug fixed in error messages.
 *
 * Revision 1.5  2005/01/14 08:46:31  cg
 * Bug fixed in application of variable argument list functions.
 *
 * Revision 1.4  2005/01/12 15:50:46  cg
 * Added CTIterminateCompilation.
 *
 * Revision 1.3  2005/01/11 15:11:46  cg
 * Added some useful functionality.
 *
 * Revision 1.2  2005/01/07 19:54:13  cg
 * Some streamlining done.
 *
 * Revision 1.1  2005/01/07 16:48:21  cg1
 * Initial revision
 *
 *
 */

/**
 *
 * @file
 *
 * This file provides the interface for producing any kind of output during
 * compilation. It fully replaces the macro based legacy implementation
 * in files Error.[ch]
 *
 * We have 4 levels of verbosity controlled by the command line option -v
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
 *
 * Default values are 1 for the product version and 3 for the developer version.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "dbug.h"
#include "filemgr.h"
#include "internal_lib.h"
#include "build.h"
#include "resource.h"
#include "print.h"
#include "convert.h"
#include "globals.h"
#include "free.h"
#include "phase.h"

#include "ctinfo.h"

static char *message_buffer = NULL;
static int message_buffer_size = 0;
static int message_line_length = 76;

static char *abort_message_header = "ABORT: ";
static char *error_message_header = "ERROR: ";
static char *warn_message_header = "WARNING: ";
static char *state_message_header = "";
static char *note_message_header = "  ";

static int errors = 0;
static int warnings = 0;

/** <!--********************************************************************-->
 *
 * @fn void ProcessMessage( char *buffer, int line_length)
 *
 *   @brief  formats message according to line length
 *
 *           '@' characters are inserted into the buffer to represent line
 *           breaks.
 *
 *   @param buffer  message buffer
 *   @param line_length maximum line length
 *
 ******************************************************************************/

static void
ProcessMessage (char *buffer, int line_length)
{
    int index, column, last_space;

    DBUG_ENTER ("ProcessMessage");

    index = 0;
    last_space = 0;
    column = 0;

    while (buffer[index] != '\0') {
        if (buffer[index] == '\t') {
            buffer[index] = ' ';
        }

        if (buffer[index] == ' ') {
            last_space = index;
        }

        if (buffer[index] == '\n') {
            buffer[index] = '@';
            column = 0;
        } else {
            if (column == line_length) {
                if (buffer[last_space] == ' ') {
                    buffer[last_space] = '@';
                    column = index - last_space;
                } else {
                    break;
                }
            }
        }

        index++;
        column++;
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void PrintMessage( const char *header, const char *format, va_list arg_p)
 *
 *   @brief prints message
 *
 *          The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows. Afterwards, the message
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
    int len;

    DBUG_ENTER ("PrintMessage");

    len = vsnprintf (message_buffer, message_buffer_size, format, arg_p);

    if (len < 0) {
        DBUG_ASSERT ((message_buffer_size == 0), "message buffer corruption");
        /*
         * Output error due to non-existing message buffer
         */

        len = 120;

        message_buffer = (char *)ILIBmalloc (len + 2);
        message_buffer_size = len + 2;

        len = vsnprintf (message_buffer, message_buffer_size, format, arg_p);
        DBUG_ASSERT ((len >= 0), "message buffer corruption");
    }

    if (len >= message_buffer_size) {
        /* buffer too small  */

        ILIBfree (message_buffer);
        message_buffer = (char *)ILIBmalloc (len + 2);
        message_buffer_size = len + 2;

        len = vsnprintf (message_buffer, message_buffer_size, format, arg_p);

        DBUG_ASSERT ((len < message_buffer_size), "message buffer corruption");
    }

    ProcessMessage (message_buffer, message_line_length - strlen (header));

    line = strtok (message_buffer, "@");

    while (line != NULL) {
        fprintf (stderr, "%s%s\n", header, line);
        line = strtok (NULL, "@");
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn static void CleanUp()
 *
 *   @brief  does som clean up upon termination
 *
 *
 ******************************************************************************/

static void
CleanUp ()
{
    DBUG_ENTER ("CleanUp");

    FMGRdeleteTmpDir ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void AbortCompilation()
 *
 *   @brief  terminates the compilation process with a suitable error message.
 *
 ******************************************************************************/

static void
AbortCompilation ()
{
    DBUG_ENTER ("AbortCompilation");

    fprintf (stderr, "\n*** Compilation failed ***\n");
    fprintf (stderr, "*** Exit code %d (%s)\n", global.compiler_phase,
             PHphaseName (global.compiler_phase));
    fprintf (stderr, "*** %d Error(s), %d Warning(s)\n\n", errors, warnings);

    CleanUp ();

    exit ((int)global.compiler_phase);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void InternalCompilerErrorBreak( int sig)
 *
 *   @brief  interrupt handler for segmentation faults and bus errors
 *
 *           An error message is produced and a bug report is created which
 *           may be sent via email to an appropriate address.
 *           Temporary files are deleted and the compilation process
 *           terminated.
 *
 *           DBUG_ENTER/RETURN are omitted on purpose to reduce risk of
 *           creating more errors during error handling.
 *
 *   @param sig  signal causing interrupt
 *
 ******************************************************************************/

static void
InternalCompilerErrorBreak (int sig)
{
    FILE *error_file;
    int i;

    fprintf (stderr, "\n\n"
                     "OOOPS your program crashed the compiler 8-((\n"
                     "Please send a bug report to bug@sac-home.org.\n\n");

    error_file = fopen ("SACbugreport", "w");

    if (error_file != NULL) {
        fprintf (error_file, "/*\n"
                             " * SAC - bug report\n"
                             " * ================\n"
                             " *\n"
                             " * automatically generated on ");
        fclose (error_file);
        ILIBsystemCall2 ("date >> SACbugreport");
        error_file = fopen ("SACbugreport", "a");

        fprintf (error_file, " *\n");
        fprintf (error_file, " * using sac2c %s for %s\n", global.version_id,
                 global.target_platform);
        fprintf (error_file, " * built %s.\n", build_date);
        fprintf (error_file, " * by user %s on host %s for %s.\n", build_user, build_host,
                 build_os);
        fprintf (error_file, " *\n");

        fprintf (error_file, " * The compiler was called by\n");
        fprintf (error_file, " *  %s", global.argv[0]);
        for (i = 1; i < global.argc; i++) {
            fprintf (error_file, " %s", global.argv[i]);
        }
        fprintf (error_file, "\n");
        fprintf (error_file, " *\n");

        if (global.sacfilename != NULL) {
            fprintf (error_file, " * The contents of %s is:\n", global.sacfilename);
            fprintf (error_file, " */\n\n");
            ILIBsystemCall2 ("cat %s >> SACbugreport", global.sacfilename);
        } else {
            fprintf (error_file, " * Compiler crashed before SAC file name could be "
                                 "determined!\n");
            fprintf (error_file, " */\n\n");
        }

        fclose (error_file);

        fprintf (stderr,
                 "For your convenience, the compiler has pre-fabricated a bug report in\n"
                 "the file \"SACbugreport\" which was created in the current directory!\n"
                 "Besides some infos concerning the compiler version and its\n"
                 "usage it contains the specified source file.\n"
                 "If you want to send that bug report to us you may simply use\n\n"
                 "  mail bug@sac-home.org < SACbugreport\n\n");
    } else {
        fprintf (stderr, "Sorry, sac2c is unable to create a bug report file.\n");
    }

    CleanUp ();

    exit (1);
}

/** <!--********************************************************************-->
 *
 * @fn void UserForcedBreak( int sig)
 *
 *   @brief  interrupt handler for user-forced breaks like CTRL-C
 *
 *           Temporary files are deleted and the compilation process
 *           terminated.
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
    CleanUp ();
    exit (0);
}

/** <!--********************************************************************-->
 *
 * @fn void CTIinstallInterruptHandlers()
 *
 *   @brief  installs interrupt handlers
 *
 ******************************************************************************/

void
CTIinstallInterruptHandlers ()
{
    DBUG_ENTER ("CTIinstallInterruptHandlers");

    signal (SIGSEGV, InternalCompilerErrorBreak); /* Segmentation Fault */
    signal (SIGBUS, InternalCompilerErrorBreak);  /* Bus Error */
    signal (SIGINT, UserForcedBreak);             /* Interrupt (Control-C) */

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerror( const char *format, ...)
 *
 *   @brief  produces an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerror (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIerror");

    va_start (arg_p, format);

    fprintf (stderr, "\n");
    PrintMessage (error_message_header, format, arg_p);

    va_end (arg_p);

    errors++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLine( int line, const char *format, ...)
 *
 *   @brief  produces an error message preceded by file name and line number.
 *
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorLine (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIerrorLine");

    va_start (arg_p, format);

    fprintf (stderr, "\n");
    fprintf (stderr, "%sline %d  file: %s\n", error_message_header, line,
             global.filename);
    PrintMessage (error_message_header, format, arg_p);

    va_end (arg_p);

    errors++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorLineVA( int line, const char *format, va_list arg_p)
 *
 *   @brief  produces an error message preceded by file name and line number.
 *
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorLine (int line, const char *format, va_list arg_p)
{
    DBUG_ENTER ("CTIerrorLine");

    fprintf (stderr, "\n");
    fprintf (stderr, "%sline %d  file: %s\n", error_message_header, line,
             global.filename);
    PrintMessage (error_message_header, format, arg_p);

    errors++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIerrorContinued( const char *format, ...)
 *
 *   @brief  continues an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorContinued (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIerrorContinued");

    va_start (arg_p, format);

    PrintMessage (error_message_header, format, arg_p);

    va_end (arg_p);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn int CTIgetErrorMessageLineLength( )
 *
 *   @brief  yields useful line length for error messages
 *
 *   @return line length
 *
 ******************************************************************************/

int
CTIgetErrorMessageLineLength ()
{
    DBUG_ENTER ("CTIgetErrorMessageLineLength");

    DBUG_RETURN (message_line_length - strlen (error_message_header));
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabort( const char *format, ...)
 *
 *   @brief   produces an error message without file name and line number
 *            and terminates the compilation process.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabort (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIabort");

    va_start (arg_p, format);

    fprintf (stderr, "\n");
    PrintMessage (abort_message_header, format, arg_p);

    va_end (arg_p);

    errors++;

    AbortCompilation ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortLine( int line, const char *format, ...)
 *
 *   @brief   produces an error message preceded by file name and line number
 *            and terminates the compilation process.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabortLine (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIabortLine");

    va_start (arg_p, format);

    fprintf (stderr, "\n");
    fprintf (stderr, "%sline %d  file: %s\n", abort_message_header, line,
             global.filename);
    PrintMessage (abort_message_header, format, arg_p);

    va_end (arg_p);

    errors++;

    AbortCompilation ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabortOnError()
 *
 *   @brief  terminates compilation process if errors have occurred.
 *
 ******************************************************************************/

void
CTIabortOnError ()
{
    DBUG_ENTER ("CTIabortOnError");

    if (errors > 0) {
        AbortCompilation ();
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIwarnLine( int line, const char *format, ...)
 *
 *   @brief   produces a warning message preceded by file name and line number.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarnLine (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIwarnLine");

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        fprintf (stderr, "%sline %d  file: %s\n", warn_message_header, line,
                 global.filename);
        PrintMessage (warn_message_header, format, arg_p);

        va_end (arg_p);

        warnings++;
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIwarn( const char *format, ...)
 *
 *   @brief   produces a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarn (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIwarn");

    if (global.verbose_level >= 1) {
        va_start (arg_p, format);

        PrintMessage (warn_message_header, format, arg_p);

        va_end (arg_p);

        warnings++;
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIwarnContinued( const char *format, ...)
 *
 *   @brief  continues a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarnContinued (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIwarnContinued");

    va_start (arg_p, format);

    PrintMessage (warn_message_header, format, arg_p);

    va_end (arg_p);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn int CTIgetWarnMessageLineLength( )
 *
 *   @brief  yields useful line length for warning messages
 *
 *   @return line length
 *
 ******************************************************************************/

int
CTIgetWarnMessageLineLength ()
{
    DBUG_ENTER ("CTIgetWarnMessageLineLength");

    DBUG_RETURN (message_line_length - strlen (warn_message_header));
}

/** <!--********************************************************************-->
 *
 * @fn void CTIstate( const char *format, ...)
 *
 *   @brief  produces full compile time information output (verbose level 3)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIstate (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIstate");

    if (global.verbose_level >= 2) {
        va_start (arg_p, format);

        PrintMessage (state_message_header, format, arg_p);

        va_end (arg_p);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTInote( const char *format, ...)
 *
 *   @brief  produces basic compile time information output (verbose level 2)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTInote (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTInote");

    if (global.verbose_level >= 3) {
        va_start (arg_p, format);

        PrintMessage (note_message_header, format, arg_p);

        va_end (arg_p);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIterminateCompilation()
 *
 *   @brief  terminates successful compilation process
 *
 ******************************************************************************/

void
CTIterminateCompilation (compiler_phase_t phase, char *break_specifier, node *syntax_tree)
{
    DBUG_ENTER ("CTIterminateCompilation");

    /*
     * Upon premature termination of compilation process show compiler resources
     * or syntax tree if available.
     */

    if (global.print_after_break) {
        if (phase < PH_scanparse) {
            RSCshowResources ();
        } else {
            if (phase <= PH_compile) {
                syntax_tree = PRTdoPrint (syntax_tree);
            }
        }
    }

    /*
     *  Finally, we do some clean up.
     */

    FMGRdeleteTmpDir ();

    if (syntax_tree != NULL) {
        syntax_tree = FREEdoFreeTree (syntax_tree);
    }

    /*
     *  At last, we display a success message.
     */

    CTIstate (" ");
    CTIstate ("*** Compilation successful ***");

    if (phase < PH_final) {
        CTIstate ("*** BREAK after: %s", PHphaseName (phase));
        if (break_specifier != NULL) {
            CTIstate ("*** BREAK specifier: '%s`", break_specifier);
        }
    }

#ifdef SHOW_MALLOC
    CTIstate ("*** Maximum allocated memory (bytes):   %s",
              CVintBytes2String (global.max_allocated_mem));
    CTIstate ("*** Currently allocated memory (bytes): %s",
              CVintBytes2String (global.current_allocated_mem));
#endif

    CTIstate ("*** Exit code 0");
    CTIstate ("*** 0 error(s), %d warning(s)", warnings);
    CTIstate (" ");

    exit (0);

    DBUG_VOID_RETURN;
}
