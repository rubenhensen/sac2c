/*
 *
 * $Log$
 * Revision 1.5  1994/12/13 11:26:34  hw
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

#ifndef _Error_h
#define _Error_h

#include <stdio.h>
/* string ist die Fehlermeldung
 * status gibt an mit welchem Wert das Programm beendet wird.
 */

#define NOTE(s)                                                                          \
    if (!silent)                                                                         \
        DoPrint s;
#define WARN1(s)                                                                         \
    if (!silent) {                                                                       \
        warnings = +1;                                                                   \
        DoPrint s;                                                                       \
    }
#define ERROR1(s)                                                                        \
    {                                                                                    \
        fprintf (stderr, "/n");                                                          \
        DoPrint s;                                                                       \
        errors = +1;                                                                     \
    }

#define ERROR2(n, s)                                                                     \
    {                                                                                    \
        fprintf (stderr, "\n");                                                          \
        DoPrint s;                                                                       \
        fprintf (stderr, "\n");                                                          \
        exit (n);                                                                        \
    }

extern int errors;
extern int warnings;
extern int silent;

extern void DoPrint (char *format, ...);
extern void Error (char *string, int status);

#endif /* _Error_h */
