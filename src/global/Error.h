/*
 *
 * $Log$
 * Revision 1.3  1994/12/02 12:38:10  sbs
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
    fprintf (stderr, s)

extern void Error (char *string, int status);

#endif /* _Error_h */
