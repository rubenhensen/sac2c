/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:44:34  sbs
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
extern void Error (char *string, int status);

#endif /* _Error_h */
