/*
 *
 * $Log$
 * Revision 1.10  1995/10/18 13:30:47  cg
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

/*
 *  The following macros should be used for all compiler messages.
 *
 *
 *  Some guidelines for error messages:
 *
 *  It must be written like in printf :
 *    ("message with %s, %d etc", stringvar, intvar)
 *
 *  It should contain the plain message:
 *  - no "\n" at the end
 *  - no "!" at the end
 *  - no "ERROR", "WARNING" etc. at the beginning
 *  - the message should start with a capital letter
 *  - all quoted items should be enclosed in ' and `
 *     for example:  "... in function 'xyz` is ..."
 *  - if the message exceeds a certain number of characters, it should
 *    be splitted by a "\n\t" in between two words.
 */

/*
 *  only for internal use
 */

#define ABORT_MESSAGE                                                                    \
    fprintf (stderr, "*** Compilation failed ***  Exit code %d\n", compiler_phase);      \
    fprintf (stderr, "*** %d Error(s), %d Warning(s)\n\n", errors, warnings);

/*
 *  NOTE simply gives the user some information.
 *  It has no impact on the compilation.
 *  It may be switched off by command line parameter 'silent`
 */

#define NOTE(message)                                                                    \
    if (!silent) {                                                                       \
        DoPrint message;                                                                 \
        fprintf (stderr, "\n");                                                          \
    }

/*
 *  WARN gives a warning to the user.
 *  The program is not absolutely correct but the compiler can deal with it.
 *  All warnings are counted.
 *  "line" should be the program line where the problem occurs.
 *  If this information is not available simply use 0.
 */

#define WARN(line, message)                                                              \
    {                                                                                    \
        if (!silent) {                                                                   \
            fprintf (stderr, "%s:%d:WARNING: ", filename, line);                         \
            DoPrint message;                                                             \
            fprintf (stderr, " !\n");                                                    \
        }                                                                                \
        warnings++;                                                                      \
    }

/*
 *  ERROR produces an error message but does not abort compilation.
 *  Be sure that this may not lead to segmentation faults.
 *  On the other hand, this feature gives the programmer the opportunity
 *  to correct several errors after a single compilation.
 *  Nevertheless, compilation should be stopped at last at the end of
 *  the respective compilation phase. Use ABORT_ON_ERROR.
 */

#define ERROR(line, message)                                                             \
    {                                                                                    \
        fprintf (stderr, "%s:%d: ERROR : ", filename, line);                             \
        DoPrint message;                                                                 \
        fprintf (stderr, " !\n");                                                        \
        errors++;                                                                        \
    }

/*
 *  ABORT writes an error message to stderr and immediately stops
 *  compilation with an abort message.
 *  The exit code encodes the compilation phase where the problem
 *  occured (starting with 1 in main, 2 for scanparse, ...)
 */

#define ABORT(line, message)                                                             \
    {                                                                                    \
        ERROR (line, message);                                                           \
        ABORT_MESSAGE;                                                                   \
        exit (compiler_phase);                                                           \
    }

/*
 *  SYSWARN gives a warning to the user.
 *  In contrast to WARN, this is designed for system warnings,
 *  such as "Unknown command line parameter".
 *  So, no line number is necessary.
 */

#define SYSWARN(message)                                                                 \
    {                                                                                    \
        if (!silent) {                                                                   \
            fprintf (stderr, "%s:SYSTEM WARNING: ", filename);                           \
            DoPrint message;                                                             \
            fprintf (stderr, " !\n");                                                    \
        }                                                                                \
        warnings++;                                                                      \
    }

/*
 *  SYSERROR is similar to ERROR, just the message differs slightly and no
 *  program line is needed. SYSERROR is designed for troubles with the
 *  system rather than the compiled program, such as "out of memory"
 *  or "file cannot be opened".
 */

#define SYSERROR(message)                                                                \
    {                                                                                    \
        fprintf (stderr, "%s:SYSTEM ERROR: ", filename);                                 \
        DoPrint message;                                                                 \
        fprintf (stderr, " !\n");                                                        \
        errors++;                                                                        \
    }

/*
 *  SYSABORT is similar to SYSERROR but immediately stops compilation.
 *  The exit code encodes the compilation phase where the problem
 *  occured (starting with 1 in main, 2 for scanparse, ...)
 */

#define SYSABORT(message)                                                                \
    {                                                                                    \
        SYSERROR (message);                                                              \
        ABORT_MESSAGE;                                                                   \
        exit (compiler_phase);                                                           \
    }

/*
 *  ABORT_ON_ERROR checks if errors have occured. In this case, an abort
 *  message is produced and the compilation stopped.
 *  The exit code encodes the compilation phase where the problem
 *  occured (starting with 1 in main, 2 for scanparse, ...)
 */

#define ABORT_ON_ERROR                                                                   \
    if (errors != 0) {                                                                   \
        ABORT_MESSAGE;                                                                   \
        exit (compiler_phase);                                                           \
    }

extern int errors;
extern int warnings;
extern int silent;
extern int compiler_phase;

extern char filename[]; /* is set in main.c */

extern void DoPrint (char *format, ...);
extern char *ModName (char *, char *);

extern void Error (char *string, int status);
/* string ist die Fehlermeldung
 * status gibt an mit welchem Wert das Programm beendet wird.
 */

/*
 *   below this line for compatibility only
 */

#define WARN1(s)                                                                         \
    if (!silent) {                                                                       \
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

#endif /* _sac_Error_h */
