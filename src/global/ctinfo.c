/*
 *
 * $Log$
 * Revision 1.1  2005/01/07 16:48:21  cg
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

#include "ctinfo.h"

static char *message_buffer = NULL;
static int message_buffer_size = 0;
static int message_line_length = 76;

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
 * @fn void PrintMessage( const char *header, const char *format,  ...)
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
PrintMessage (const char *header, const char *format, ...)
{
    va_list arg_p;
    char *line;
    int len;

    DBUG_ENTER ("PrintMessage");

    va_start (arg_p, format);

    len = vsnprintf (message_buffer, message_buffer_size, format, arg_p);

    if (len >= message_buffer_size) {
        /* buffer too small */
        ILIBfree (message_buffer);
        message_buffer = (char *)ILIBmalloc (len + 2);
        message_buffer_size = len + 2;

        len = vsnprintf (message_buffer, message_buffer_size, format, arg_p);
        DBUG_ASSERT ((len < message_buffer_size), "message buffer corruption");
    }

    va_end (arg_p);

    ProcessMessage (message_buffer, message_line_length - strlen (header));

    line = strtok (message_buffer, "@");

    while (line != NULL) {
        fprintf (stderr, "%s", header);
        fprintf (stderr, "%s\n", line);
        line = strtok (NULL, "@");
    }

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
             global.compiler_phase_name[(int)global.compiler_phase]);
    fprintf (stderr, "*** %d Error(s), %d Warning(s)\n\n", errors, warnings);

    FMGRcleanUp ();

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

    FMGRcleanUp ();

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
    FMGRcleanUp ();
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
 * @fn void CTIerror( int line, const char *format, ...)
 *
 *   @brief  produces an error message preceded by file name and line number.
 *
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerror (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIerror");

    va_start (arg_p, format);

    fprintf (stderr, "ERROR: line %d  file: %s", line, global.filename);
    PrintMessage ("ERROR: ", format, arg_p);

    va_end (arg_p);

    errors++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIsyserror( const char *format, ...)
 *
 *   @brief  produces an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIsyserror (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIsyserror");

    va_start (arg_p, format);

    PrintMessage ("ERROR: ", format, arg_p);

    va_end (arg_p);

    errors++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabort( int line, const char *format, ...)
 *
 *   @brief   produces an error message preceded by file name and line number
 *            and terminates the compilation process.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIabort (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIabort");

    va_start (arg_p, format);

    CTIerror (line, format, arg_p);

    va_end (arg_p);

    AbortCompilation ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIsysabort( const char *format, ...)
 *
 *   @brief   produces an error message without file name and line number
 *            and terminates the compilation process.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIsysabort (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIsysabort");

    va_start (arg_p, format);

    CTIsyserror (format, arg_p);

    va_end (arg_p);

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
 * @fn void CTIwarning( int line, const char *format, ...)
 *
 *   @brief   produces a warning message preceded by file name and line number.
 *
 *   @param line  line number
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIwarning (int line, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIwarning");

    if (global.verbose_level > 0) {
        va_start (arg_p, format);

        fprintf (stderr, "WARNING: line %d  file: %s", line, global.filename);
        PrintMessage ("WARNING: ", format, arg_p);

        va_end (arg_p);

        warnings++;
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIsyswarning( const char *format, ...)
 *
 *   @brief   produces a warning message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIsyswarning (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIsyswarning");

    if (global.verbose_level > 0) {
        va_start (arg_p, format);

        PrintMessage ("WARNING: ", format, arg_p);

        va_end (arg_p);

        warnings++;
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

    if (global.verbose_level > 1) {
        va_start (arg_p, format);

        PrintMessage ("", format, arg_p);

        va_end (arg_p);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTItell( const char *format, ...)
 *
 *   @brief  produces full compile time information output (verbose level 3)
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTItell (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTItell");

    if (global.verbose_level > 2) {
        va_start (arg_p, format);

        PrintMessage ("  ", format, arg_p);

        va_end (arg_p);
    }

    DBUG_VOID_RETURN;
}
