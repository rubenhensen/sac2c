/*
 * $Id$
 */

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

#include "dbug.h"
#include "filemgr.h"
#include "system.h"
#include "str.h"
#include "memory.h"
#include "build.h"
#include "print.h"
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

#define MAX_ITEM_NAME_LENGTH 255

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
 * @fn void Format2Buffer( const char *format, va_list arg_p)
 *
 *   @brief The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

static void
Format2Buffer (const char *format, va_list arg_p)
{
    int len;
    va_list arg_p_copy;

    DBUG_ENTER ("Format2Buffer");

    va_copy (arg_p_copy, arg_p);
    len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy);
    va_end (arg_p_copy);

    if (len < 0) {
        DBUG_ASSERT ((message_buffer_size == 0), "message buffer corruption");
        /*
         * Output error due to non-existing message buffer
         */

        len = 120;

        message_buffer = (char *)MEMmalloc (len + 2);
        CHKMdoNotReport (message_buffer);
        message_buffer_size = len + 2;

        va_copy (arg_p_copy, arg_p);
        len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy);
        va_end (arg_p_copy);
        DBUG_ASSERT ((len >= 0), "message buffer corruption");
    }

    if (len >= message_buffer_size) {
        /* buffer too small  */

        MEMfree (message_buffer);
        message_buffer = (char *)MEMmalloc (len + 2);
        CHKMdoNotReport (message_buffer);
        message_buffer_size = len + 2;

        va_copy (arg_p_copy, arg_p);
        len = vsnprintf (message_buffer, message_buffer_size, format, arg_p_copy);
        va_end (arg_p_copy);

        DBUG_ASSERT ((len < message_buffer_size), "message buffer corruption");
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn char *CTIgetErrorMessageVA( int line, const char *format, va_list arg_p)
 *
 *   @brief generates error message string
 *
 *          The message specified by format string and variable number
 *          of arguments is "printed" into the global message buffer.
 *          It is taken care of buffer overflows. Afterwards, the message
 *          is formatted to fit a certain line length and is printed to
 *          stderr.
 *
 *   @param format  format string like in printf family of functions
 *
 ******************************************************************************/

char *
CTIgetErrorMessageVA (int line, const char *format, va_list arg_p)
{
    char *first_line, *res;

    DBUG_ENTER ("CTIgetErrorMessageVA");
    Format2Buffer (format, arg_p);
    ProcessMessage (message_buffer, message_line_length - strlen (error_message_header));

    first_line = (char *)MEMmalloc (32 * sizeof (char));
    sprintf (first_line, "line %d @", line);
    res = STRcat (first_line, message_buffer);
    first_line = MEMfree (first_line);

    DBUG_RETURN (res);
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

    DBUG_ENTER ("PrintMessage");

    Format2Buffer (format, arg_p);

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

    if (global.cleanup) {
        global.cleanup = FALSE;

        FMGRdeleteTmpDir ();
        global.dependencies = STRSfree (global.dependencies);
        /*
         * This is somewhat inconsistent as the other global variables are not
         * de-allocated. However, this piece of code was moved from phase_drivers.c
         * to this place rather than being just deleted.
         */
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

    CleanUp ();

    fprintf (stderr, "\n*** Compilation failed ***\n");
    fprintf (stderr, "*** Exit code %d (%s)\n", global.compiler_phase,
             PHIphaseText (global.compiler_phase));
    fprintf (stderr, "*** %d Error(s), %d Warning(s)\n\n", errors, warnings);

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
                     "Please send a bug report to bugs@sac-home.org.\n\n");

    error_file = fopen ("SACbugreport", "w");

    if (error_file != NULL) {
        fprintf (error_file, "/*\n"
                             " * SAC - bug report\n"
                             " * ================\n"
                             " *\n"
                             " * automatically generated on ");
        fclose (error_file);
        SYScallNoErr ("date >> SACbugreport");
        error_file = fopen ("SACbugreport", "a");

        fprintf (error_file, " *\n");
        fprintf (error_file, " * using sac2c %s rev %s for %s\n", global.version_id,
                 build_rev, global.target_platform);
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
            fclose (error_file);
            SYScallNoErr ("cat %s >> SACbugreport", global.sacfilename);
            error_file = fopen ("SACbugreport", "a");
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
                 "  mail bugs@sac-home.org < SACbugreport\n\n");
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
CTIerrorLineVA (int line, const char *format, va_list arg_p)
{
    DBUG_ENTER ("CTIerrorLineVA");

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
 * @fn void CTIerrorInternal( const char *format, ...)
 *
 *   @brief  produces an error message without file name and line number.
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTIerrorInternal (const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTIerrorInternal");

    fprintf (stderr, "\n%sInternal %s failure\n", error_message_header, global.toolname);

    va_start (arg_p, format);
    PrintMessage (error_message_header, format, arg_p);
    va_end (arg_p);

    fprintf (stderr,
             "%sCompiler phase:    %s\n"
             "%s                   %s\n"
             "%sTraversal:         %s\n"
             "%sFunction:          %s( %s)",
             error_message_header, PHIphaseName (global.compiler_anyphase),
             error_message_header, PHIphaseText (global.compiler_anyphase),
             error_message_header, TRAVgetName (), error_message_header,
             CTIitemName (global.current_fundef), CTIfunParams (global.current_fundef));

    errors++;

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
 * @fn void CTIabortOnBottom( const char *err_msg)
 *
 *   @brief   produces an error message from preprocessed error message
 *            containing @ symbols as line breaks.
 *
 *   @param err_msg  pre-processed error message
 *
 ******************************************************************************/

void
CTIabortOnBottom (char *err_msg)
{
    char *line;

    DBUG_ENTER ("CTIabortOnBottom");

    fprintf (stderr, "\n");

    line = strtok (err_msg, "@");

    while (line != NULL) {
        fprintf (stderr, "%s%s\n", error_message_header, line);
        line = strtok (NULL, "@");
    }

    errors++;

    AbortCompilation ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CTIabort( const char *format, ...)
 *
 *   @brief   produces an error message without file name and line number
 *            and terminates the compilation process.
 *
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
 * @fn void CTIabortOutOfMemory( unsigned int request)
 *
 *   @brief   produces a specific "out of memory" error message
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
CTIabortOutOfMemory (unsigned int request)
{
    DBUG_ENTER ("CTIabortOutOfMemory");

    fprintf (stderr,
             "\n"
             "%sOut of memory:\n"
             "%s %u bytes requested\n",
             abort_message_header, abort_message_header, request);

#ifdef SHOW_MALLOC
    fprintf (stderr, "%s %u bytes already allocated\n", abort_message_header,
             global.current_allocated_mem);
#endif

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
 * @fn void CTInote( const char *format, ...)
 *
 *   @brief produces compile time information at given verbosity level
 *
 *   @param format  format string like in printf
 *
 ******************************************************************************/

void
CTItell (int verbose_level, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("CTInote");

    if (global.verbose_level >= verbose_level) {
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
CTIterminateCompilation (node *syntax_tree)
{
    DBUG_ENTER ("CTIterminateCompilation");

    /*
     * Upon premature termination of compilation process show
     * syntax tree if available.
     */

    if (global.print_after_break && (syntax_tree != NULL)
        && (global.compiler_subphase <= PH_cg_cpl)
        && ((global.break_after_phase < PH_final)
            || (global.break_after_subphase < PH_final)
            || (global.break_after_cyclephase < PH_final))) {
        syntax_tree = PRTdoPrint (syntax_tree);
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

    if (global.break_after_cyclephase < PH_final) {
        CTIstate ("** BREAK during:   %s\n", PHIphaseText (global.compiler_phase));
        CTIstate ("** BREAK in cycle: %s\n", PHIphaseText (global.compiler_subphase));
        CTIstate ("** BREAK in pass:  %d\n", global.break_cycle_specifier);
        CTIstate ("** BREAK after:    %s\n",
                  PHIphaseText (global.break_after_cyclephase));
    } else {
        if (global.break_after_subphase < PH_final) {
            CTIstate ("** BREAK during: %s\n", PHIphaseText (global.compiler_phase));
            CTIstate ("** BREAK after:  %s\n", PHIphaseText (global.compiler_subphase));
        } else {
            if (global.break_after_phase < PH_final) {
                CTIstate ("** BREAK after: %s\n", PHIphaseText (global.compiler_phase));
            }
        }
    }

#ifdef SHOW_MALLOC
    CHKMdeinitialize ();

    CTIstate ("** Maximum allocated memory (bytes):   %s",
              CVintBytes2String (global.max_allocated_mem));
    CTIstate ("** Currently allocated memory (bytes): %s",
              CVintBytes2String (global.current_allocated_mem));
#endif

    CTIstate ("** Exit code 0");
    CTIstate ("** 0 error(s), %d warning(s)", warnings);
    CTIstate (" ");

    exit (0);

    DBUG_VOID_RETURN;
}

/**
 * @fn const char *CTIitemName(node *item)
 *
 * @brief creates a string containing the module and name of
 *        the given item. As this function uses an internal
 *        buffer, the returned string should only be used to
 *        print one single message.
 *
 * @param item a N_fundef, N_objdef or N_typedef
 *
 * @return a constant string
 */

const char *
formatItemName (namespace_t *ns, char *name)
{
    static char buffer[MAX_ITEM_NAME_LENGTH + 1];
    int written;

    DBUG_ENTER ("formatItemName");

    if (ns != NULL) {
        written = snprintf (buffer, MAX_ITEM_NAME_LENGTH, "%s::%s", NSgetName (ns), name);
    } else {
        written = snprintf (buffer, MAX_ITEM_NAME_LENGTH, "%s", name);
    }

    DBUG_ASSERT ((written < MAX_ITEM_NAME_LENGTH), "buffer in formatItemName too small");

    DBUG_RETURN (buffer);
}

const char *
CTIitemName (node *item)
{
    const char *ret;

    DBUG_ENTER ("CTIitemName");

    if (item == NULL) {
        ret = "???";
    } else {
        switch (NODE_TYPE (item)) {
        case N_fundef:
            ret = formatItemName (FUNDEF_NS (item), FUNDEF_NAME (item));
            break;
        case N_typedef:
            ret = formatItemName (TYPEDEF_NS (item), TYPEDEF_NAME (item));
            break;
        case N_objdef:
            ret = formatItemName (OBJDEF_NS (item), OBJDEF_NAME (item));
            break;
        default:
            DBUG_ASSERT (FALSE, "Wrong item in call of function 'CTIitemName`");
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
    int tmp_str_size;

    static char argtype_buffer[80];
    static int buffer_space;

    DBUG_ENTER ("CTIfunParams");

    if (fundef == NULL) {
        ret = "???";
    } else {
        strcpy (argtype_buffer, "");
        buffer_space = 77;

        arg = FUNDEF_ARGS (fundef);
        while ((arg != NULL) && (buffer_space > 5)) {

            tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), TRUE, 0);
            tmp_str_size = strlen (tmp_str);

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