/*
 *
 * $Log$
 * Revision 1.4  1994/12/08 17:56:38  hw
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
        fprintf (stderr, s);
#define WARN1(s)                                                                         \
    if (!silent) {                                                                       \
        warnings = +1;                                                                   \
        fprintf (stderr, s);                                                             \
    }
#define ERROR(s)                                                                         \
    fprintf (stderr, s);                                                                 \
    errors = +1;
extern int errors;
extern int warnings;
extern int silent;

extern void Error (char *string, int status);

#endif /* _Error_h */
