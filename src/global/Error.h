/*
 *
 * $Log$
 * Revision 1.18  1996/01/05 12:24:20  cg
 * Now, CleanUp is called by macro EXIT instead of removing the
 * sibfile only
 *
 * Revision 1.17  1996/01/02  15:44:25  cg
 * modified external declaration of global variable filename
 *
 * Revision 1.16  1995/12/01  16:13:21  cg
 * extern declaration of global variable 'silent' removed
 * no longer required for optimize.h
 *
 * Revision 1.15  1995/11/10  14:57:11  cg
 * Error.h entirely revised !!!
 * Lots of phantastic new macros for error handling and compile time output.
 * Please, have a look at this first when programming compile time output.
 *
 * Revision 1.14  1995/11/06  09:24:54  cg
 * added macro NOTE_INDENT
 *
 * Revision 1.13  1995/10/26  15:56:48  cg
 * modified lay-out of error messages slightly.
 *
 * Revision 1.12  1995/10/19  10:04:45  cg
 * new function ItemName for convenient output of combined names
 * of types, functions, or global objects.
 *
 * Revision 1.11  1995/10/18  16:47:34  cg
 * some beautifications
 *
 * Revision 1.10  1995/10/18  13:30:47  cg
 * new error macros : WARN, ERROR, ABORT, SYSWARN, SYSERROR, SYSABORT
 * allow for more comfortable error handling and a uniform
 * look of error messages.
 *
 * Revision 1.9  1995/09/01  07:45:06  cg
 * small layout change
 *
 * Revision 1.8  1995/05/16  09:05:03  hw
 * changed ERROR1 & WARN1 ( no '\n' is required at beginning or end of
 *                          formatstring anymore )
 *
 * Revision 1.7  1995/05/04  11:39:51  sbs
 * DoPrint implemented by vfprintf!
 *
 * Revision 1.6  1994/12/20  14:01:14  hw
 * bug fixed in ERROR1
 *
 * Revision 1.5  1994/12/13  11:26:34  hw
 * changed macros WARN1, NOTE
 * changed macro ERROR to ERROR1
 * inserted macro ERROR2
 * added declaration: void DoPrint( char *format, ...)
 *
 * Revision 1.4  1994/12/08  17:56:38  hw
 * added WARN1 , ERRORS and some external definitions see Error.c
 *
 * Revision 1.3  1994/12/02  12:38:10  sbs
 * NOTE macro inserted
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _sac_Error_h
#define _sac_Error_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "globals.h"

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  1.  General information
 *
 *  - All compile time output should be done via the macros in this file.
 *  - All output is sent to stderr.
 *
 *
 *  1.1  Verbose level
 *
 *  - All output is filtered by the verbose level, which is stored in the
 *    global variable verbose_level.
 *  - It can be set by the command line option -v<n>
 *  - So far, there are 4 different verbose levels:
 *      0: only error messages
 *      1: error messages and warnings
 *      2: basic compile time information
 *      3: full compile time information (default)
 *
 *
 *  1.2  Message format
 *
 *  - A single message is written in the following form:
 *     ("<format string>", parameters ...)
 *  - The format string and the variable number of parameters is exactly
 *    as known from printf, etc. except for the following subjects.
 *  - Newlines \n as first or last character are ignored.
 *  - Tabs \t are not allowed.
 *
 *
 *  1.3  Message format guidelines
 *
 *  These may be a matter of taste, but the appearance of compile time
 *  output should at least be similar.
 *
 *  - The message should start with a capital letter.
 *  - All quoted items should be enclosed in ' and `,
 *     for example:  "... in function 'xyz` is ..."
 *  - All quoted file names should be enclosed in " and ",
 *     for example:  "... file "xyz" is ..."
 *
 *
 *  1.4  Message indentation
 *
 *  All messages can be indented between the message header and the pure
 *  message.
 *  This indentation is triggered by the global variable message_indent.
 *  Make sure, that at the end of a message complex, this variable is
 *  reset to 0. Otherwise, the next message would be affected.
 *
 *
 *  1.5  Compiler phase names
 *
 *  These are stored in the global variable compiler_phase_name[],
 *  which is initialized in Error.c. Make sure, that all new compiler
 *  phases are given names, otherwise the wrong names are used.
 *
 */

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  2.  System constants
 *
 *
 *  2.1  MAX_ERROR_MESSAGE_LENGTH
 *
 *    Maximum length for a single message, used to allocate memory.
 *    Exceeding this limit will probably result in a segmentation fault.
 *
 *
 *  2.2  MAX_LINE_LENGTH
 *
 *    Maximum length of a single line, used for formatting messages.
 *
 *
 */

#define MAX_ERROR_MESSAGE_LENGTH 400

#define MAX_LINE_LENGTH 77

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  3.  Macros for internal use
 *
 *  These should not be used in the program !
 *
 *
 *  3.1  ABORT_MESSAGE
 *
 *  writes an abort message containing the exit code, the compiler
 *  phase which failed and the numbers of warnings and errors.
 *
 *
 *  3.2  ERROR_INDENT(n)
 *
 *  writes n consecutive spaces, used for indentation.
 *
 *
 *  3.3  EXIT(n)
 *
 *  removes a sib file if already generated and terminates program
 *  execution with exit code n.
 *
 *
 *  3.4  PRINT_MESSAGE(message, header, ind1, ind2, first_line_ind, excl)
 *
 *  writes the given message in a formatted way. Formatting is done by
 *  the function ProcessErrorMessage.
 *
 *  "header" is a string that is written in the beginning of each line.
 *  "ind1" determines the indentation before the header
 *  "ind2" determines the indentation behind the header
 *  "first_line_ind" says whether or not the first message line is indented.
 *  "excl" says whether or not an exclamation mark is added to the pure
 *  message.
 *
 */

#define ABORT_MESSAGE                                                                    \
    {                                                                                    \
        fprintf (stderr, "\n*** Compilation failed ***\n");                              \
        fprintf (stderr, "*** Exit code %d (%s)\n", compiler_phase,                      \
                 compiler_phase_name[compiler_phase]);                                   \
        fprintf (stderr, "*** %d Error(s), %d Warning(s)\n\n", errors, warnings);        \
    }

#define ERROR_INDENT(n)                                                                  \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < n; i++)                                                          \
            fprintf (stderr, " ");                                                       \
    }

#define EXIT(n)                                                                          \
    {                                                                                    \
        CleanUp ();                                                                      \
        exit (n);                                                                        \
    }

#define PRINT_MESSAGE(message, header, ind1, ind2, first_line_ind, excl)                 \
    {                                                                                    \
        char *line;                                                                      \
                                                                                         \
        current_line_length                                                              \
          = MAX_LINE_LENGTH - ind1 - ind2 - (header == NULL ? 0 : strlen (header));      \
        ProcessErrorMessage message;                                                     \
                                                                                         \
        if (excl)                                                                        \
            strcat (error_message_buffer, " !");                                         \
        line = strtok (error_message_buffer, "@");                                       \
        if (first_line_ind)                                                              \
            ERROR_INDENT (ind1);                                                         \
                                                                                         \
        do {                                                                             \
            fprintf (stderr, "%s", header);                                              \
            ERROR_INDENT (ind2);                                                         \
            fprintf (stderr, "%s\n", (line == NULL) ? "" : line);                        \
            line = strtok (NULL, "@");                                                   \
            if (line != NULL)                                                            \
                ERROR_INDENT (ind1);                                                     \
        } while (line != NULL);                                                          \
    }

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  4.  General macros
 *
 *
 *  4.1  NEWLINE(verbose)
 *
 *  makes a new line, if the verbose level is at least "verbose"
 *
 */

#define NEWLINE(verbose)                                                                 \
    if (verbose_level >= verbose)                                                        \
        fprintf (stderr, "\n");

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  5.  Error messages (verbose_level 0)
 *
 *
 *  5.1  ERROR(line, message)
 *
 *  ERROR produces an error message containing the file name and line
 *  number where the error occurred, but does not abort compilation.
 *  Be sure that this may not lead to segmentation faults.
 *  On the other hand, this feature gives the programmer the opportunity
 *  to correct several errors after a single compilation.
 *  Nevertheless, compilation should be stopped at last at the end of
 *  the respective compilation phase. Use ABORT_ON_ERROR.
 *
 *
 *  5.2  SYSERROR(message)
 *
 *  SYSERROR produces an error message similar to ERROR, but this contains
 *  no line number. Compilation continues just as with ERROR.
 *  SYSERROR is designed for troubles with the
 *  system rather than the compiled program, such as "out of memory"
 *  or "file cannot be opened" or ... .
 *
 *
 *  5.3  ABORT(line, message)
 *
 *  ABORT produces an error message just as ERROR, but immediately stops
 *  compilation with an abort message.
 *  The exit code encodes the compilation phase where the problem
 *  occured (refer to compiler_phase_name[] in Error.c)
 *
 *
 *  5.4  SYSABORT(message)
 *
 *  SYSABORT produces an error message just as SYSERROR but immediately
 *  stops compilation right as ABORT does.
 *
 *
 *  5.5  CONT_ERROR(message)
 *
 *  CONT_ERROR continues the last error message. This macro can be used
 *  to write multi-line messages without \n within a single message.
 *  The layout would be the same.
 *
 *
 *  5.6  ABORT_ON_ERROR
 *
 *  ABORT_ON_ERROR checks if errors have occured. In this case, an abort
 *  message is produced and the compilation stopped.
 *  The exit code encodes the compilation phase where the problem
 *  occured (refer to compiler_phase_name[] in Error.c)
 *
 *
 *
 */

#define ERROR(line, message)                                                             \
    {                                                                                    \
        ERROR_INDENT (((verbose_level > 1) ? 2 : 0));                                    \
        fprintf (stderr, "%s:%d", filename, line);                                       \
        last_indent = ((verbose_level > 1) ? 2 : 0) + strlen (filename)                  \
                      + NumberOfDigits (line) + 1;                                       \
        PRINT_MESSAGE (message, ":ERROR: ", last_indent, message_indent, 0, 1);          \
        errors++;                                                                        \
    }

#define SYSERROR(message)                                                                \
    {                                                                                    \
        ERROR_INDENT (((verbose_level > 1) ? 2 : 0));                                    \
        fprintf (stderr, "SYSTEM:");                                                     \
        last_indent = ((verbose_level > 1) ? 2 : 0) + 7;                                 \
        PRINT_MESSAGE (message, ":ERROR: ", last_indent, message_indent, 0, 1);          \
        errors++;                                                                        \
    }

#define ABORT(line, message)                                                             \
    {                                                                                    \
        ERROR (line, message);                                                           \
        ABORT_MESSAGE;                                                                   \
        EXIT (compiler_phase);                                                           \
    }

#define SYSABORT(message)                                                                \
    {                                                                                    \
        SYSERROR (message);                                                              \
        ABORT_MESSAGE;                                                                   \
        EXIT (compiler_phase);                                                           \
    }

#define CONT_ERROR(message)                                                              \
    PRINT_MESSAGE (message, ":ERROR: ", last_indent, message_indent, 1, 0);

#define ABORT_ON_ERROR                                                                   \
    {                                                                                    \
        if (errors > 0) {                                                                \
            ABORT_MESSAGE;                                                               \
            EXIT (compiler_phase);                                                       \
        }                                                                                \
    }

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  6.  Warnings (verbose_level 1)
 *
 *
 *  6.1  WARN(line, message)
 *
 *  WARN produces a warning message containing the file name and line
 *  number where the problem occurred.
 *
 *
 *  6.2  SYSWARN(message)
 *
 *  SYSWARN produces a warning message similar to WARN, but this contains
 *  no line number. SYSWARN is designed for problems with the
 *  system rather than the compiled program.
 *
 *
 *  6.3  CONT_WARN(message)
 *
 *  CONT_WARN continues the last warning message similar to CONT_ERROR.
 *
 *
 *
 */

#define WARN(line, message)                                                              \
    {                                                                                    \
        if (verbose_level > 0) {                                                         \
            ERROR_INDENT (((verbose_level > 1) ? 2 : 0));                                \
            fprintf (stderr, "%s:%d", filename, line);                                   \
            last_indent = ((verbose_level > 1) ? 2 : 0) + strlen (filename)              \
                          + NumberOfDigits (line) + 1;                                   \
            PRINT_MESSAGE (message, ":WARNING: ", last_indent, message_indent, 0, 1);    \
            warnings++;                                                                  \
        }                                                                                \
    }

#define SYSWARN(message)                                                                 \
    {                                                                                    \
        if (verbose_level > 0) {                                                         \
            ERROR_INDENT (((verbose_level > 1) ? 2 : 0));                                \
            fprintf (stderr, "SYSTEM:");                                                 \
            last_indent = ((verbose_level > 1) ? 2 : 0) + 7;                             \
            PRINT_MESSAGE (message, ":WARNING: ", last_indent, message_indent, 0, 1);    \
            warnings++;                                                                  \
        }                                                                                \
    }

#define CONT_WARN(message)                                                               \
    {                                                                                    \
        if (verbose_level > 0) {                                                         \
            PRINT_MESSAGE (message, ":WARNING: ", last_indent, message_indent, 1, 0);    \
        }                                                                                \
    }

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  7. Basic compile time information (verbose_level 2)
 *
 *
 *  7.1  NOTE_COMPILER_PHASE
 *
 *  NOTE_COMPILER_PHASE writes the current compiler phase in a formatted
 *  way.
 *
 *
 *  7.2  NOTE2(message)
 *
 *  NOTE2 writes the given message.
 *
 *
 */

#define NOTE_COMPILER_PHASE                                                              \
    {                                                                                    \
        if (verbose_level > 1) {                                                         \
            fprintf (stderr, "\n** %d: %s: ...\n", compiler_phase,                       \
                     compiler_phase_name[compiler_phase]);                               \
        }                                                                                \
    }

#define NOTE2(message)                                                                   \
    {                                                                                    \
        if (verbose_level > 1) {                                                         \
            PRINT_MESSAGE (message, "", 0, message_indent, 1, 0);                        \
        }                                                                                \
    }

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  8.  Full compile time information (verbose_level 3)
 *
 *
 *  8.1  NOTE(message)
 *
 *  NOTE writes the given message, which is indented by 2 spaces.
 *  The only difference to NOTE2 is this indentation and the verbose level.
 *
 *
 */

#define NOTE(message)                                                                    \
    {                                                                                    \
        if (verbose_level > 2) {                                                         \
            PRINT_MESSAGE (message, "", 2, message_indent, 1, 0);                        \
        }                                                                                \
    }

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  9.  Functions for generating messages
 *
 *
 *  9.1  char *ModName(char *, char *)
 *
 *  combines the 2 strings. The first is assumed the module name and the
 *  second the function, object, type, ... name. The module name may be
 *  NULL.
 *
 *
 *  9.2  char *ItemName(node *)
 *
 *  generates the combined (with module name) name of the given
 *  function, object, type, ...
 *
 *
 */

extern char *ModName (char *, char *);

extern char *ItemName (node *);

/*
 ********************************************
 *  SAC2C compile time information system
 ********************************************
 *
 *  10.  External declarations of
 *       global variables and functions
 *
 */

extern int errors;
extern int warnings;
extern int last_indent;
extern int current_line_length;
extern int message_indent;
extern int verbose_level;
extern int compiler_phase;
extern int max_compiler_phase;

extern char *filename;
extern char *compiler_phase_name[];
extern char error_message_buffer[];

extern void ProcessErrorMessage (char *format, ...);
extern int NumberOfDigits (int);
extern void CleanUp ();

/*
 *************************************************************************
 *   below this line for compatibility only
 */

#define WARN1(s)                                                                         \
    if (verbose_level > 0) {                                                             \
        warnings += 1;                                                                   \
        fprintf (stderr, "\n");                                                          \
        DoPrint s;                                                                       \
    }

#define ERROR1(s)                                                                        \
    {                                                                                    \
        fprintf (stderr, "\n");                                                          \
        DoPrint s;                                                                       \
        errors += 1;                                                                     \
    }

#define ERROR2(n, s)                                                                     \
    {                                                                                    \
        fprintf (stderr, "\n");                                                          \
        DoPrint s;                                                                       \
        fprintf (stderr, "\n\n");                                                        \
        exit (n);                                                                        \
    }

extern void Error (char *string, int status);

extern void DoPrint (char *format, ...);

#endif /* _sac_Error_h */
