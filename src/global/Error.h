/*
 *
 * $Log$
 * Revision 1.9  1995/09/01 07:45:06  cg
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

extern int errors;
extern int warnings;
extern int silent;

extern void DoPrint (char *format, ...);
extern void Error (char *string, int status);

#endif /* _Error_h */
