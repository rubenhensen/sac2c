/*
 *
 * $Log$
 * Revision 2.2  1999/07/09 11:52:18  cg
 * Added consistency check for command line options.
 *
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 *
 */

/*
 * File: options.h
 *
 * Description:
 *
 * This file provides external declarations for symbols defined in options.c.
 *
 */

#ifndef _options_h
#define _options_h

extern void AnalyseCommandline (int argc, char *argv[]);
extern void CheckOptionConsistency ();

#endif /* _options_h */
