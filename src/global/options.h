/*
 *
 * $Log$
 * Revision 3.2  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 3.1  2000/11/20 17:59:36  sacbase
 * new release made
 *
 * Revision 2.2  1999/07/09 11:52:18  cg
 * Added consistency check for command line options.
 *
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 *
 */

#ifndef _SAC_OPTIONS_H_
#define _SAC_OPTIONS_H_

/******************************************************************************
 *
 * Options
 *
 * Prefix: OPT
 *
 * Description:
 *
 * This file provides external declarations for symbols defined in options.c.
 *
 *****************************************************************************/
extern void OPTanalyseCommandline (int argc, char *argv[]);
extern void OPTcheckOptionConsistency ();

#endif /* _SAC_OPTIONS_H_ */
