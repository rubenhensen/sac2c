/*
 *
 * $Log$
 * Revision 3.4  2005/04/24 15:19:10  sah
 * modified option handling slightly to allow
 * for the setup phase to run prior to libstat
 *
 * Revision 3.3  2005/03/10 09:41:09  cg
 * Separated analysis of special options which do not lead to a
 * compilation process like -h or -V into new function.
 *
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

#ifndef _SAC_OPTIONS_H_
#define _SAC_OPTIONS_H_

extern node *OPTanalyseCommandline (node *);
extern void OPTcheckPreSetupOptions (int argc, char *argv[]);
extern void OPTcheckPostSetupOptions (int argc, char *argv[]);

#endif /* _SAC_OPTIONS_H_ */
