#ifndef _scnprs_h

#define _scnprs_h
/*
 *
 * $Log$
 * Revision 1.7  1997/06/03 08:57:58  sbs
 * MAX_CPP_VARS defined
 *
 * Revision 1.6  1996/01/02  15:59:58  cg
 * added external declaration of function ScanParse()
 *
 * Revision 1.5  1995/07/26  08:41:02  cg
 * extern declaration of node *sib_tree added.
 *
 * Revision 1.4  1994/12/20  13:49:27  sbs
 * linenum exported...
 *
 * Revision 1.3  1994/12/20  11:24:29  sbs
 * decl_tree inserted
 *
 * Revision 1.2  1994/12/16  14:34:28  sbs
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

#define MAX_CPP_VARS 32

extern int linenum;
extern int yyparse ();
extern FILE *yyin;
extern int start_token;

extern node *syntax_tree;
extern node *decl_tree;
extern node *sib_tree;

extern node *ScanParse ();

#endif /* _scnprs_h */
