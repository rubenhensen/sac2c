#ifndef _scnprs_h

#define _scnprs_h
/*
 *
 * $Log$
 * Revision 1.2  1994/12/16 14:34:28  sbs
 * moddec  and start_token inserted
 *
 * Revision 1.1  1994/11/22  13:47:03  sbs
 * Initial revision
 *
 *
 */

/*
 * This file contains external declarations for all functions/global variables
 * provided by the scanner/parser which are used from outside.
 *
 */

#include "y.tab.h"

extern int yyparse ();
extern FILE *yyin;
extern int start_token;

#endif /* _scnprs_h */
